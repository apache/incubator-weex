/*
 * Copyright (C) 2011-2013, 2015-2016 Apple Inc. All rights reserved.
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
#include "DFGOSRExitCompiler.h"

#if ENABLE(DFG_JIT)

#include "CallFrame.h"
#include "DFGCommon.h"
#include "DFGJITCode.h"
#include "DFGOSRExitPreparation.h"
#include "LinkBuffer.h"
#include "OperandsInlines.h"
#include "JSCInlines.h"
#include <wtf/StringPrintStream.h>

namespace JSC { namespace DFG {

void OSRExitCompiler::emitRestoreArguments(const Operands<ValueRecovery>& operands)
{
    HashMap<MinifiedID, int> alreadyAllocatedArguments; // Maps phantom arguments node ID to operand.
    for (size_t index = 0; index < operands.size(); ++index) {
        const ValueRecovery& recovery = operands[index];
        int operand = operands.operandForIndex(index);
        
        if (recovery.technique() != DirectArgumentsThatWereNotCreated
            && recovery.technique() != ClonedArgumentsThatWereNotCreated)
            continue;
        
        MinifiedID id = recovery.nodeID();
        auto iter = alreadyAllocatedArguments.find(id);
        if (iter != alreadyAllocatedArguments.end()) {
            JSValueRegs regs = JSValueRegs::withTwoAvailableRegs(GPRInfo::regT0, GPRInfo::regT1);
            m_jit.loadValue(CCallHelpers::addressFor(iter->value), regs);
            m_jit.storeValue(regs, CCallHelpers::addressFor(operand));
            continue;
        }
        
        InlineCallFrame* inlineCallFrame =
            m_jit.codeBlock()->jitCode()->dfg()->minifiedDFG.at(id)->inlineCallFrame();

        int stackOffset;
        if (inlineCallFrame)
            stackOffset = inlineCallFrame->stackOffset;
        else
            stackOffset = 0;
        
        if (!inlineCallFrame || inlineCallFrame->isClosureCall) {
            m_jit.loadPtr(
                AssemblyHelpers::addressFor(stackOffset + CallFrameSlot::callee),
                GPRInfo::regT0);
        } else {
            m_jit.move(
                AssemblyHelpers::TrustedImmPtr(inlineCallFrame->calleeRecovery.constant().asCell()),
                GPRInfo::regT0);
        }
        
        if (!inlineCallFrame || inlineCallFrame->isVarargs()) {
            m_jit.load32(
                AssemblyHelpers::payloadFor(stackOffset + CallFrameSlot::argumentCount),
                GPRInfo::regT1);
        } else {
            m_jit.move(
                AssemblyHelpers::TrustedImm32(inlineCallFrame->arguments.size()),
                GPRInfo::regT1);
        }
        
        m_jit.setupArgumentsWithExecState(
            AssemblyHelpers::TrustedImmPtr(inlineCallFrame), GPRInfo::regT0, GPRInfo::regT1);
        switch (recovery.technique()) {
        case DirectArgumentsThatWereNotCreated:
            m_jit.move(AssemblyHelpers::TrustedImmPtr(bitwise_cast<void*>(operationCreateDirectArgumentsDuringExit)), GPRInfo::nonArgGPR0);
            break;
        case ClonedArgumentsThatWereNotCreated:
            m_jit.move(AssemblyHelpers::TrustedImmPtr(bitwise_cast<void*>(operationCreateClonedArgumentsDuringExit)), GPRInfo::nonArgGPR0);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        m_jit.call(GPRInfo::nonArgGPR0);
        m_jit.storeCell(GPRInfo::returnValueGPR, AssemblyHelpers::addressFor(operand));
        
        alreadyAllocatedArguments.add(id, operand);
    }
}

extern "C" {

void compileOSRExit(ExecState* exec)
{
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);

    if (vm->callFrameForCatch)
        RELEASE_ASSERT(vm->callFrameForCatch == exec);
    
    CodeBlock* codeBlock = exec->codeBlock();
    ASSERT(codeBlock);
    ASSERT(codeBlock->jitType() == JITCode::DFGJIT);
    
    // It's sort of preferable that we don't GC while in here. Anyways, doing so wouldn't
    // really be profitable.
    DeferGCForAWhile deferGC(vm->heap);

    uint32_t exitIndex = vm->osrExitIndex;
    OSRExit& exit = codeBlock->jitCode()->dfg()->osrExit[exitIndex];
    
    if (vm->callFrameForCatch)
        ASSERT(exit.m_kind == GenericUnwind);
    if (exit.isExceptionHandler())
        ASSERT_UNUSED(scope, !!scope.exception());
        
    
    prepareCodeOriginForOSRExit(exec, exit.m_codeOrigin);
    
    // Compute the value recoveries.
    Operands<ValueRecovery> operands;
    codeBlock->jitCode()->dfg()->variableEventStream.reconstruct(codeBlock, exit.m_codeOrigin, codeBlock->jitCode()->dfg()->minifiedDFG, exit.m_streamIndex, operands);
    
    SpeculationRecovery* recovery = 0;
    if (exit.m_recoveryIndex != UINT_MAX)
        recovery = &codeBlock->jitCode()->dfg()->speculationRecovery[exit.m_recoveryIndex];

    {
        CCallHelpers jit(vm, codeBlock);
        OSRExitCompiler exitCompiler(jit);

        if (exit.m_kind == GenericUnwind) {
            // We are acting as a defacto op_catch because we arrive here from genericUnwind().
            // So, we must restore our call frame and stack pointer.
            jit.restoreCalleeSavesFromVMEntryFrameCalleeSavesBuffer();
            jit.loadPtr(vm->addressOfCallFrameForCatch(), GPRInfo::callFrameRegister);
        }
        jit.addPtr(
            CCallHelpers::TrustedImm32(codeBlock->stackPointerOffset() * sizeof(Register)),
            GPRInfo::callFrameRegister, CCallHelpers::stackPointerRegister);

        jit.jitAssertHasValidCallFrame();
        
        if (vm->m_perBytecodeProfiler && codeBlock->jitCode()->dfgCommon()->compilation) {
            Profiler::Database& database = *vm->m_perBytecodeProfiler;
            Profiler::Compilation* compilation = codeBlock->jitCode()->dfgCommon()->compilation.get();
            
            Profiler::OSRExit* profilerExit = compilation->addOSRExit(
                exitIndex, Profiler::OriginStack(database, codeBlock, exit.m_codeOrigin),
                exit.m_kind, exit.m_kind == UncountableInvalidation);
            jit.add64(CCallHelpers::TrustedImm32(1), CCallHelpers::AbsoluteAddress(profilerExit->counterAddress()));
        }

        exitCompiler.compileExit(exit, operands, recovery);
        
        LinkBuffer patchBuffer(*vm, jit, codeBlock);
        exit.m_code = FINALIZE_CODE_IF(
            shouldDumpDisassembly() || Options::verboseOSR(),
            patchBuffer,
            ("DFG OSR exit #%u (%s, %s) from %s, with operands = %s",
                exitIndex, toCString(exit.m_codeOrigin).data(),
                exitKindToString(exit.m_kind), toCString(*codeBlock).data(),
                toCString(ignoringContext<DumpContext>(operands)).data()));
    }
    
    MacroAssembler::repatchJump(exit.codeLocationForRepatch(codeBlock), CodeLocationLabel(exit.m_code.code()));
    
    vm->osrExitJumpDestination = exit.m_code.code().executableAddress();
}

} // extern "C"

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
