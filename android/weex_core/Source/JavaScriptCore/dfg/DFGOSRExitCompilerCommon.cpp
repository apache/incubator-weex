/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "DFGOSRExitCompilerCommon.h"

#if ENABLE(DFG_JIT)

#include "DFGJITCode.h"
#include "DFGOperations.h"
#include "JIT.h"
#include "JSCJSValueInlines.h"
#include "JSCInlines.h"
#include "StructureStubInfo.h"

namespace JSC { namespace DFG {

void handleExitCounts(CCallHelpers& jit, const OSRExitBase& exit)
{
    jit.add32(AssemblyHelpers::TrustedImm32(1), AssemblyHelpers::AbsoluteAddress(&exit.m_count));

    if (!exitKindMayJettison(exit.m_kind))
        return;
    
    jit.move(AssemblyHelpers::TrustedImmPtr(jit.codeBlock()), GPRInfo::regT0);
    
    AssemblyHelpers::Jump tooFewFails;
    
    jit.load32(AssemblyHelpers::Address(GPRInfo::regT0, CodeBlock::offsetOfOSRExitCounter()), GPRInfo::regT2);
    jit.add32(AssemblyHelpers::TrustedImm32(1), GPRInfo::regT2);
    jit.store32(GPRInfo::regT2, AssemblyHelpers::Address(GPRInfo::regT0, CodeBlock::offsetOfOSRExitCounter()));
    
    jit.move(AssemblyHelpers::TrustedImmPtr(jit.baselineCodeBlock()), GPRInfo::regT0);
    AssemblyHelpers::Jump reoptimizeNow = jit.branch32(
        AssemblyHelpers::GreaterThanOrEqual,
        AssemblyHelpers::Address(GPRInfo::regT0, CodeBlock::offsetOfJITExecuteCounter()),
        AssemblyHelpers::TrustedImm32(0));
    
    // We want to figure out if there's a possibility that we're in a loop. For the outermost
    // code block in the inline stack, we handle this appropriately by having the loop OSR trigger
    // check the exit count of the replacement of the CodeBlock from which we are OSRing. The
    // problem is the inlined functions, which might also have loops, but whose baseline versions
    // don't know where to look for the exit count. Figure out if those loops are severe enough
    // that we had tried to OSR enter. If so, then we should use the loop reoptimization trigger.
    // Otherwise, we should use the normal reoptimization trigger.
    
    AssemblyHelpers::JumpList loopThreshold;
    
    for (InlineCallFrame* inlineCallFrame = exit.m_codeOrigin.inlineCallFrame; inlineCallFrame; inlineCallFrame = inlineCallFrame->directCaller.inlineCallFrame) {
        loopThreshold.append(
            jit.branchTest8(
                AssemblyHelpers::NonZero,
                AssemblyHelpers::AbsoluteAddress(
                    inlineCallFrame->baselineCodeBlock->ownerScriptExecutable()->addressOfDidTryToEnterInLoop())));
    }
    
    jit.move(
        AssemblyHelpers::TrustedImm32(jit.codeBlock()->exitCountThresholdForReoptimization()),
        GPRInfo::regT1);
    
    if (!loopThreshold.empty()) {
        AssemblyHelpers::Jump done = jit.jump();

        loopThreshold.link(&jit);
        jit.move(
            AssemblyHelpers::TrustedImm32(
                jit.codeBlock()->exitCountThresholdForReoptimizationFromLoop()),
            GPRInfo::regT1);
        
        done.link(&jit);
    }
    
    tooFewFails = jit.branch32(AssemblyHelpers::BelowOrEqual, GPRInfo::regT2, GPRInfo::regT1);
    
    reoptimizeNow.link(&jit);
    
    // Reoptimize as soon as possible.
#if !NUMBER_OF_ARGUMENT_REGISTERS
    jit.poke(GPRInfo::regT0);
    jit.poke(AssemblyHelpers::TrustedImmPtr(&exit), 1);
#else
    jit.move(GPRInfo::regT0, GPRInfo::argumentGPR0);
    jit.move(AssemblyHelpers::TrustedImmPtr(&exit), GPRInfo::argumentGPR1);
#endif
    jit.move(AssemblyHelpers::TrustedImmPtr(bitwise_cast<void*>(triggerReoptimizationNow)), GPRInfo::nonArgGPR0);
    jit.call(GPRInfo::nonArgGPR0);
    AssemblyHelpers::Jump doneAdjusting = jit.jump();
    
    tooFewFails.link(&jit);
    
    // Adjust the execution counter such that the target is to only optimize after a while.
    int32_t activeThreshold =
        jit.baselineCodeBlock()->adjustedCounterValue(
            Options::thresholdForOptimizeAfterLongWarmUp());
    int32_t targetValue = applyMemoryUsageHeuristicsAndConvertToInt(
        activeThreshold, jit.baselineCodeBlock());
    int32_t clippedValue;
    switch (jit.codeBlock()->jitType()) {
    case JITCode::DFGJIT:
        clippedValue = BaselineExecutionCounter::clippedThreshold(jit.codeBlock()->globalObject(), targetValue);
        break;
    case JITCode::FTLJIT:
        clippedValue = UpperTierExecutionCounter::clippedThreshold(jit.codeBlock()->globalObject(), targetValue);
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
        clippedValue = 0; // Make some compilers, and mhahnenberg, happy.
#endif
        break;
    }
    jit.store32(AssemblyHelpers::TrustedImm32(-clippedValue), AssemblyHelpers::Address(GPRInfo::regT0, CodeBlock::offsetOfJITExecuteCounter()));
    jit.store32(AssemblyHelpers::TrustedImm32(activeThreshold), AssemblyHelpers::Address(GPRInfo::regT0, CodeBlock::offsetOfJITExecutionActiveThreshold()));
    jit.store32(AssemblyHelpers::TrustedImm32(formattedTotalExecutionCount(clippedValue)), AssemblyHelpers::Address(GPRInfo::regT0, CodeBlock::offsetOfJITExecutionTotalCount()));
    
    doneAdjusting.link(&jit);
}

void reifyInlinedCallFrames(CCallHelpers& jit, const OSRExitBase& exit)
{
    // FIXME: We shouldn't leave holes on the stack when performing an OSR exit
    // in presence of inlined tail calls.
    // https://bugs.webkit.org/show_bug.cgi?id=147511
    ASSERT(jit.baselineCodeBlock()->jitType() == JITCode::BaselineJIT);
    jit.storePtr(AssemblyHelpers::TrustedImmPtr(jit.baselineCodeBlock()), AssemblyHelpers::addressFor((VirtualRegister)CallFrameSlot::codeBlock));

    const CodeOrigin* codeOrigin;
    for (codeOrigin = &exit.m_codeOrigin; codeOrigin && codeOrigin->inlineCallFrame; codeOrigin = codeOrigin->inlineCallFrame->getCallerSkippingTailCalls()) {
        InlineCallFrame* inlineCallFrame = codeOrigin->inlineCallFrame;
        CodeBlock* baselineCodeBlock = jit.baselineCodeBlockFor(*codeOrigin);
        InlineCallFrame::Kind trueCallerCallKind;
        CodeOrigin* trueCaller = inlineCallFrame->getCallerSkippingTailCalls(&trueCallerCallKind);
        GPRReg callerFrameGPR = GPRInfo::callFrameRegister;

        if (!trueCaller) {
            ASSERT(inlineCallFrame->isTail());
            jit.loadPtr(AssemblyHelpers::Address(GPRInfo::callFrameRegister, CallFrame::returnPCOffset()), GPRInfo::regT3);
            jit.storePtr(GPRInfo::regT3, AssemblyHelpers::addressForByteOffset(inlineCallFrame->returnPCOffset()));
            jit.loadPtr(AssemblyHelpers::Address(GPRInfo::callFrameRegister, CallFrame::callerFrameOffset()), GPRInfo::regT3);
            callerFrameGPR = GPRInfo::regT3;
        } else {
            CodeBlock* baselineCodeBlockForCaller = jit.baselineCodeBlockFor(*trueCaller);
            unsigned callBytecodeIndex = trueCaller->bytecodeIndex;
            void* jumpTarget = nullptr;

            switch (trueCallerCallKind) {
            case InlineCallFrame::Call:
            case InlineCallFrame::Construct:
            case InlineCallFrame::CallVarargs:
            case InlineCallFrame::ConstructVarargs:
            case InlineCallFrame::TailCall:
            case InlineCallFrame::TailCallVarargs: {
                CallLinkInfo* callLinkInfo =
                    baselineCodeBlockForCaller->getCallLinkInfoForBytecodeIndex(callBytecodeIndex);
                RELEASE_ASSERT(callLinkInfo);

                jumpTarget = callLinkInfo->callReturnLocation().executableAddress();
                break;
            }

            case InlineCallFrame::GetterCall:
            case InlineCallFrame::SetterCall: {
                StructureStubInfo* stubInfo =
                    baselineCodeBlockForCaller->findStubInfo(CodeOrigin(callBytecodeIndex));
                RELEASE_ASSERT(stubInfo);

                jumpTarget = stubInfo->doneLocation().executableAddress();
                break;
            }

            default:
                RELEASE_ASSERT_NOT_REACHED();
            }

            if (trueCaller->inlineCallFrame) {
                jit.addPtr(
                    AssemblyHelpers::TrustedImm32(trueCaller->inlineCallFrame->stackOffset * sizeof(EncodedJSValue)),
                    GPRInfo::callFrameRegister,
                    GPRInfo::regT3);
                callerFrameGPR = GPRInfo::regT3;
            }

            jit.storePtr(AssemblyHelpers::TrustedImmPtr(jumpTarget), AssemblyHelpers::addressForByteOffset(inlineCallFrame->returnPCOffset()));
        }

        jit.storePtr(AssemblyHelpers::TrustedImmPtr(baselineCodeBlock), AssemblyHelpers::addressFor((VirtualRegister)(inlineCallFrame->stackOffset + CallFrameSlot::codeBlock)));

        // Restore the inline call frame's callee save registers.
        // If this inlined frame is a tail call that will return back to the original caller, we need to
        // copy the prior contents of the tag registers already saved for the outer frame to this frame.
        jit.emitSaveOrCopyCalleeSavesFor(
            baselineCodeBlock,
            static_cast<VirtualRegister>(inlineCallFrame->stackOffset),
            trueCaller ? AssemblyHelpers::UseExistingTagRegisterContents : AssemblyHelpers::CopyBaselineCalleeSavedRegistersFromBaseFrame,
            GPRInfo::regT2);

        if (!inlineCallFrame->isVarargs())
            jit.store32(AssemblyHelpers::TrustedImm32(inlineCallFrame->arguments.size()), AssemblyHelpers::payloadFor((VirtualRegister)(inlineCallFrame->stackOffset + CallFrameSlot::argumentCount)));
#if USE(JSVALUE64)
        jit.store64(callerFrameGPR, AssemblyHelpers::addressForByteOffset(inlineCallFrame->callerFrameOffset()));
        uint32_t locationBits = CallSiteIndex(codeOrigin->bytecodeIndex).bits();
        jit.store32(AssemblyHelpers::TrustedImm32(locationBits), AssemblyHelpers::tagFor((VirtualRegister)(inlineCallFrame->stackOffset + CallFrameSlot::argumentCount)));
        if (!inlineCallFrame->isClosureCall)
            jit.store64(AssemblyHelpers::TrustedImm64(JSValue::encode(JSValue(inlineCallFrame->calleeConstant()))), AssemblyHelpers::addressFor((VirtualRegister)(inlineCallFrame->stackOffset + CallFrameSlot::callee)));
#else // USE(JSVALUE64) // so this is the 32-bit part
        jit.storePtr(callerFrameGPR, AssemblyHelpers::addressForByteOffset(inlineCallFrame->callerFrameOffset()));
        Instruction* instruction = baselineCodeBlock->instructions().begin() + codeOrigin->bytecodeIndex;
        uint32_t locationBits = CallSiteIndex(instruction).bits();
        jit.store32(AssemblyHelpers::TrustedImm32(locationBits), AssemblyHelpers::tagFor((VirtualRegister)(inlineCallFrame->stackOffset + CallFrameSlot::argumentCount)));
        jit.store32(AssemblyHelpers::TrustedImm32(JSValue::CellTag), AssemblyHelpers::tagFor((VirtualRegister)(inlineCallFrame->stackOffset + CallFrameSlot::callee)));
        if (!inlineCallFrame->isClosureCall)
            jit.storePtr(AssemblyHelpers::TrustedImmPtr(inlineCallFrame->calleeConstant()), AssemblyHelpers::payloadFor((VirtualRegister)(inlineCallFrame->stackOffset + CallFrameSlot::callee)));
#endif // USE(JSVALUE64) // ending the #else part, so directly above is the 32-bit part
    }

    // Don't need to set the toplevel code origin if we only did inline tail calls
    if (codeOrigin) {
#if USE(JSVALUE64)
        uint32_t locationBits = CallSiteIndex(codeOrigin->bytecodeIndex).bits();
#else
        Instruction* instruction = jit.baselineCodeBlock()->instructions().begin() + codeOrigin->bytecodeIndex;
        uint32_t locationBits = CallSiteIndex(instruction).bits();
#endif
        jit.store32(AssemblyHelpers::TrustedImm32(locationBits), AssemblyHelpers::tagFor((VirtualRegister)(CallFrameSlot::argumentCount)));
    }
}

static void osrWriteBarrier(CCallHelpers& jit, GPRReg owner, GPRReg scratch)
{
    AssemblyHelpers::Jump ownerIsRememberedOrInEden = jit.barrierBranchWithoutFence(owner);

    // We need these extra slots because setupArgumentsWithExecState will use poke on x86.
#if CPU(X86)
    jit.subPtr(MacroAssembler::TrustedImm32(sizeof(void*) * 3), MacroAssembler::stackPointerRegister);
#endif

    jit.setupArgumentsWithExecState(owner);
    jit.move(MacroAssembler::TrustedImmPtr(reinterpret_cast<void*>(operationOSRWriteBarrier)), scratch);
    jit.call(scratch);

#if CPU(X86)
    jit.addPtr(MacroAssembler::TrustedImm32(sizeof(void*) * 3), MacroAssembler::stackPointerRegister);
#endif

    ownerIsRememberedOrInEden.link(&jit);
}

void adjustAndJumpToTarget(CCallHelpers& jit, const OSRExitBase& exit)
{
    jit.memoryFence();
    
    jit.move(
        AssemblyHelpers::TrustedImmPtr(
            jit.codeBlock()->baselineAlternative()), GPRInfo::argumentGPR1);
    osrWriteBarrier(jit, GPRInfo::argumentGPR1, GPRInfo::nonArgGPR0);

    // We barrier all inlined frames -- and not just the current inline stack --
    // because we don't know which inlined function owns the value profile that
    // we'll update when we exit. In the case of "f() { a(); b(); }", if both
    // a and b are inlined, we might exit inside b due to a bad value loaded
    // from a.
    // FIXME: MethodOfGettingAValueProfile should remember which CodeBlock owns
    // the value profile.
    InlineCallFrameSet* inlineCallFrames = jit.codeBlock()->jitCode()->dfgCommon()->inlineCallFrames.get();
    if (inlineCallFrames) {
        for (InlineCallFrame* inlineCallFrame : *inlineCallFrames) {
            jit.move(
                AssemblyHelpers::TrustedImmPtr(
                    inlineCallFrame->baselineCodeBlock.get()), GPRInfo::argumentGPR1);
            osrWriteBarrier(jit, GPRInfo::argumentGPR1, GPRInfo::nonArgGPR0);
        }
    }

    if (exit.m_codeOrigin.inlineCallFrame)
        jit.addPtr(AssemblyHelpers::TrustedImm32(exit.m_codeOrigin.inlineCallFrame->stackOffset * sizeof(EncodedJSValue)), GPRInfo::callFrameRegister);

    CodeBlock* codeBlockForExit = jit.baselineCodeBlockFor(exit.m_codeOrigin);
    Vector<BytecodeAndMachineOffset>& decodedCodeMap = jit.decodedCodeMapFor(codeBlockForExit);
    
    BytecodeAndMachineOffset* mapping = binarySearch<BytecodeAndMachineOffset, unsigned>(decodedCodeMap, decodedCodeMap.size(), exit.m_codeOrigin.bytecodeIndex, BytecodeAndMachineOffset::getBytecodeIndex);
    
    ASSERT(mapping);
    ASSERT(mapping->m_bytecodeIndex == exit.m_codeOrigin.bytecodeIndex);
    
    void* jumpTarget = codeBlockForExit->jitCode()->executableAddressAtOffset(mapping->m_machineCodeOffset);

    jit.addPtr(AssemblyHelpers::TrustedImm32(JIT::stackPointerOffsetFor(codeBlockForExit) * sizeof(Register)), GPRInfo::callFrameRegister, AssemblyHelpers::stackPointerRegister);
    if (exit.isExceptionHandler()) {
        // Since we're jumping to op_catch, we need to set callFrameForCatch.
        jit.storePtr(GPRInfo::callFrameRegister, jit.vm()->addressOfCallFrameForCatch());
    }
    
    jit.move(AssemblyHelpers::TrustedImmPtr(jumpTarget), GPRInfo::regT2);
    jit.jump(GPRInfo::regT2);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

