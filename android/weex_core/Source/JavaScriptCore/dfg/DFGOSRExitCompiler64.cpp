/*
 * Copyright (C) 2011, 2013-2016 Apple Inc. All rights reserved.
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

#if ENABLE(DFG_JIT) && USE(JSVALUE64)

#include "DFGOperations.h"
#include "DFGOSRExitCompilerCommon.h"
#include "DFGSpeculativeJIT.h"
#include "JSCInlines.h"
#include "VirtualRegister.h"

#include <wtf/DataLog.h>

namespace JSC { namespace DFG {

void OSRExitCompiler::compileExit(const OSRExit& exit, const Operands<ValueRecovery>& operands, SpeculationRecovery* recovery)
{
    m_jit.jitAssertTagsInPlace();

    // Pro-forma stuff.
    if (Options::printEachOSRExit()) {
        SpeculationFailureDebugInfo* debugInfo = new SpeculationFailureDebugInfo;
        debugInfo->codeBlock = m_jit.codeBlock();
        debugInfo->kind = exit.m_kind;
        debugInfo->bytecodeOffset = exit.m_codeOrigin.bytecodeIndex;
        
        m_jit.debugCall(debugOperationPrintSpeculationFailure, debugInfo);
    }
    
    // Perform speculation recovery. This only comes into play when an operation
    // starts mutating state before verifying the speculation it has already made.
    
    if (recovery) {
        switch (recovery->type()) {
        case SpeculativeAdd:
            m_jit.sub32(recovery->src(), recovery->dest());
            m_jit.or64(GPRInfo::tagTypeNumberRegister, recovery->dest());
            break;

        case SpeculativeAddImmediate:
            m_jit.sub32(AssemblyHelpers::Imm32(recovery->immediate()), recovery->dest());
            m_jit.or64(GPRInfo::tagTypeNumberRegister, recovery->dest());
            break;
            
        case BooleanSpeculationCheck:
            m_jit.xor64(AssemblyHelpers::TrustedImm32(static_cast<int32_t>(ValueFalse)), recovery->dest());
            break;
            
        default:
            break;
        }
    }

    // Refine some array and/or value profile, if appropriate.
    
    if (!!exit.m_jsValueSource) {
        if (exit.m_kind == BadCache || exit.m_kind == BadIndexingType) {
            // If the instruction that this originated from has an array profile, then
            // refine it. If it doesn't, then do nothing. The latter could happen for
            // hoisted checks, or checks emitted for operations that didn't have array
            // profiling - either ops that aren't array accesses at all, or weren't
            // known to be array acceses in the bytecode. The latter case is a FIXME
            // while the former case is an outcome of a CheckStructure not knowing why
            // it was emitted (could be either due to an inline cache of a property
            // property access, or due to an array profile).
            
            CodeOrigin codeOrigin = exit.m_codeOriginForExitProfile;
            if (ArrayProfile* arrayProfile = m_jit.baselineCodeBlockFor(codeOrigin)->getArrayProfile(codeOrigin.bytecodeIndex)) {
                GPRReg usedRegister;
                if (exit.m_jsValueSource.isAddress())
                    usedRegister = exit.m_jsValueSource.base();
                else
                    usedRegister = exit.m_jsValueSource.gpr();
                
                GPRReg scratch1;
                GPRReg scratch2;
                scratch1 = AssemblyHelpers::selectScratchGPR(usedRegister);
                scratch2 = AssemblyHelpers::selectScratchGPR(usedRegister, scratch1);
                
                if (isARM64()) {
                    m_jit.pushToSave(scratch1);
                    m_jit.pushToSave(scratch2);
                } else {
                    m_jit.push(scratch1);
                    m_jit.push(scratch2);
                }
                
                GPRReg value;
                if (exit.m_jsValueSource.isAddress()) {
                    value = scratch1;
                    m_jit.loadPtr(AssemblyHelpers::Address(exit.m_jsValueSource.asAddress()), value);
                } else
                    value = exit.m_jsValueSource.gpr();
                
                m_jit.load32(AssemblyHelpers::Address(value, JSCell::structureIDOffset()), scratch1);
                m_jit.store32(scratch1, arrayProfile->addressOfLastSeenStructureID());
                m_jit.load8(AssemblyHelpers::Address(value, JSCell::indexingTypeAndMiscOffset()), scratch1);
                m_jit.move(AssemblyHelpers::TrustedImm32(1), scratch2);
                m_jit.lshift32(scratch1, scratch2);
                m_jit.or32(scratch2, AssemblyHelpers::AbsoluteAddress(arrayProfile->addressOfArrayModes()));
                
                if (isARM64()) {
                    m_jit.popToRestore(scratch2);
                    m_jit.popToRestore(scratch1);
                } else {
                    m_jit.pop(scratch2);
                    m_jit.pop(scratch1);
                }
            }
        }
        
        if (MethodOfGettingAValueProfile profile = exit.m_valueProfile) {
            if (exit.m_jsValueSource.isAddress()) {
                // We can't be sure that we have a spare register. So use the tagTypeNumberRegister,
                // since we know how to restore it.
                m_jit.load64(AssemblyHelpers::Address(exit.m_jsValueSource.asAddress()), GPRInfo::tagTypeNumberRegister);
                profile.emitReportValue(m_jit, JSValueRegs(GPRInfo::tagTypeNumberRegister));
                m_jit.move(AssemblyHelpers::TrustedImm64(TagTypeNumber), GPRInfo::tagTypeNumberRegister);
            } else
                profile.emitReportValue(m_jit, JSValueRegs(exit.m_jsValueSource.gpr()));
        }
    }
    
    // What follows is an intentionally simple OSR exit implementation that generates
    // fairly poor code but is very easy to hack. In particular, it dumps all state that
    // needs conversion into a scratch buffer so that in step 6, where we actually do the
    // conversions, we know that all temp registers are free to use and the variable is
    // definitely in a well-known spot in the scratch buffer regardless of whether it had
    // originally been in a register or spilled. This allows us to decouple "where was
    // the variable" from "how was it represented". Consider that the
    // Int32DisplacedInJSStack recovery: it tells us that the value is in a
    // particular place and that that place holds an unboxed int32. We have two different
    // places that a value could be (displaced, register) and a bunch of different
    // ways of representing a value. The number of recoveries is two * a bunch. The code
    // below means that we have to have two + a bunch cases rather than two * a bunch.
    // Once we have loaded the value from wherever it was, the reboxing is the same
    // regardless of its location. Likewise, before we do the reboxing, the way we get to
    // the value (i.e. where we load it from) is the same regardless of its type. Because
    // the code below always dumps everything into a scratch buffer first, the two
    // questions become orthogonal, which simplifies adding new types and adding new
    // locations.
    //
    // This raises the question: does using such a suboptimal implementation of OSR exit,
    // where we always emit code to dump all state into a scratch buffer only to then
    // dump it right back into the stack, hurt us in any way? The asnwer is that OSR exits
    // are rare. Our tiering strategy ensures this. This is because if an OSR exit is
    // taken more than ~100 times, we jettison the DFG code block along with all of its
    // exits. It is impossible for an OSR exit - i.e. the code we compile below - to
    // execute frequently enough for the codegen to matter that much. It probably matters
    // enough that we don't want to turn this into some super-slow function call, but so
    // long as we're generating straight-line code, that code can be pretty bad. Also
    // because we tend to exit only along one OSR exit from any DFG code block - that's an
    // empirical result that we're extremely confident about - the code size of this
    // doesn't matter much. Hence any attempt to optimize the codegen here is just purely
    // harmful to the system: it probably won't reduce either net memory usage or net
    // execution time. It will only prevent us from cleanly decoupling "where was the
    // variable" from "how was it represented", which will make it more difficult to add
    // features in the future and it will make it harder to reason about bugs.

    // Save all state from GPRs into the scratch buffer.
    
    ScratchBuffer* scratchBuffer = m_jit.vm()->scratchBufferForSize(sizeof(EncodedJSValue) * operands.size());
    EncodedJSValue* scratch = scratchBuffer ? static_cast<EncodedJSValue*>(scratchBuffer->dataBuffer()) : 0;
    
    for (size_t index = 0; index < operands.size(); ++index) {
        const ValueRecovery& recovery = operands[index];
        
        switch (recovery.technique()) {
        case InGPR:
        case UnboxedInt32InGPR:
        case UnboxedInt52InGPR:
        case UnboxedStrictInt52InGPR:
        case UnboxedCellInGPR:
            m_jit.store64(recovery.gpr(), scratch + index);
            break;
            
        default:
            break;
        }
    }
    
    // And voila, all GPRs are free to reuse.

    // Save all state from FPRs into the scratch buffer.
    
    for (size_t index = 0; index < operands.size(); ++index) {
        const ValueRecovery& recovery = operands[index];
        
        switch (recovery.technique()) {
        case UnboxedDoubleInFPR:
        case InFPR:
            m_jit.move(AssemblyHelpers::TrustedImmPtr(scratch + index), GPRInfo::regT0);
            m_jit.storeDouble(recovery.fpr(), MacroAssembler::Address(GPRInfo::regT0));
            break;
            
        default:
            break;
        }
    }
    
    // Now, all FPRs are also free.
    
    // Save all state from the stack into the scratch buffer. For simplicity we
    // do this even for state that's already in the right place on the stack.
    // It makes things simpler later.

    for (size_t index = 0; index < operands.size(); ++index) {
        const ValueRecovery& recovery = operands[index];
        
        switch (recovery.technique()) {
        case DisplacedInJSStack:
        case CellDisplacedInJSStack:
        case BooleanDisplacedInJSStack:
        case Int32DisplacedInJSStack:
        case DoubleDisplacedInJSStack:
        case Int52DisplacedInJSStack:
        case StrictInt52DisplacedInJSStack:
            m_jit.load64(AssemblyHelpers::addressFor(recovery.virtualRegister()), GPRInfo::regT0);
            m_jit.store64(GPRInfo::regT0, scratch + index);
            break;
            
        default:
            break;
        }
    }

    // Need to ensure that the stack pointer accounts for the worst-case stack usage at exit. This
    // could toast some stack that the DFG used. We need to do it before storing to stack offsets
    // used by baseline.
    m_jit.addPtr(
        CCallHelpers::TrustedImm32(
            -m_jit.codeBlock()->jitCode()->dfgCommon()->requiredRegisterCountForExit * sizeof(Register)),
        CCallHelpers::framePointerRegister, CCallHelpers::stackPointerRegister);
    
    // Restore the DFG callee saves and then save the ones the baseline JIT uses.
    m_jit.emitRestoreCalleeSaves();
    m_jit.emitSaveCalleeSavesFor(m_jit.baselineCodeBlock());

    // The tag registers are needed to materialize recoveries below.
    m_jit.emitMaterializeTagCheckRegisters();

    if (exit.isExceptionHandler())
        m_jit.copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();

    // Do all data format conversions and store the results into the stack.
    
    for (size_t index = 0; index < operands.size(); ++index) {
        const ValueRecovery& recovery = operands[index];
        VirtualRegister reg = operands.virtualRegisterForIndex(index);

        if (reg.isLocal() && reg.toLocal() < static_cast<int>(m_jit.baselineCodeBlock()->calleeSaveSpaceAsVirtualRegisters()))
            continue;

        int operand = reg.offset();

        switch (recovery.technique()) {
        case InGPR:
        case UnboxedCellInGPR:
        case DisplacedInJSStack:
        case CellDisplacedInJSStack:
        case BooleanDisplacedInJSStack:
        case InFPR:
            m_jit.load64(scratch + index, GPRInfo::regT0);
            m_jit.store64(GPRInfo::regT0, AssemblyHelpers::addressFor(operand));
            break;
            
        case UnboxedInt32InGPR:
        case Int32DisplacedInJSStack:
            m_jit.load64(scratch + index, GPRInfo::regT0);
            m_jit.zeroExtend32ToPtr(GPRInfo::regT0, GPRInfo::regT0);
            m_jit.or64(GPRInfo::tagTypeNumberRegister, GPRInfo::regT0);
            m_jit.store64(GPRInfo::regT0, AssemblyHelpers::addressFor(operand));
            break;
            
        case UnboxedInt52InGPR:
        case Int52DisplacedInJSStack:
            m_jit.load64(scratch + index, GPRInfo::regT0);
            m_jit.rshift64(
                AssemblyHelpers::TrustedImm32(JSValue::int52ShiftAmount), GPRInfo::regT0);
            m_jit.boxInt52(GPRInfo::regT0, GPRInfo::regT0, GPRInfo::regT1, FPRInfo::fpRegT0);
            m_jit.store64(GPRInfo::regT0, AssemblyHelpers::addressFor(operand));
            break;
            
        case UnboxedStrictInt52InGPR:
        case StrictInt52DisplacedInJSStack:
            m_jit.load64(scratch + index, GPRInfo::regT0);
            m_jit.boxInt52(GPRInfo::regT0, GPRInfo::regT0, GPRInfo::regT1, FPRInfo::fpRegT0);
            m_jit.store64(GPRInfo::regT0, AssemblyHelpers::addressFor(operand));
            break;
            
        case UnboxedDoubleInFPR:
        case DoubleDisplacedInJSStack:
            m_jit.move(AssemblyHelpers::TrustedImmPtr(scratch + index), GPRInfo::regT0);
            m_jit.loadDouble(MacroAssembler::Address(GPRInfo::regT0), FPRInfo::fpRegT0);
            m_jit.purifyNaN(FPRInfo::fpRegT0);
            m_jit.boxDouble(FPRInfo::fpRegT0, GPRInfo::regT0);
            m_jit.store64(GPRInfo::regT0, AssemblyHelpers::addressFor(operand));
            break;
            
        case Constant:
            m_jit.store64(
                AssemblyHelpers::TrustedImm64(JSValue::encode(recovery.constant())),
                AssemblyHelpers::addressFor(operand));
            break;
            
        case DirectArgumentsThatWereNotCreated:
        case ClonedArgumentsThatWereNotCreated:
            // Don't do this, yet.
            break;
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
    }

    // Now that things on the stack are recovered, do the arguments recovery. We assume that arguments
    // recoveries don't recursively refer to each other. But, we don't try to assume that they only
    // refer to certain ranges of locals. Hence why we need to do this here, once the stack is sensible.
    // Note that we also roughly assume that the arguments might still be materialized outside of its
    // inline call frame scope - but for now the DFG wouldn't do that.
    
    emitRestoreArguments(operands);
    
    // Adjust the old JIT's execute counter. Since we are exiting OSR, we know
    // that all new calls into this code will go to the new JIT, so the execute
    // counter only affects call frames that performed OSR exit and call frames
    // that were still executing the old JIT at the time of another call frame's
    // OSR exit. We want to ensure that the following is true:
    //
    // (a) Code the performs an OSR exit gets a chance to reenter optimized
    //     code eventually, since optimized code is faster. But we don't
    //     want to do such reentery too aggressively (see (c) below).
    //
    // (b) If there is code on the call stack that is still running the old
    //     JIT's code and has never OSR'd, then it should get a chance to
    //     perform OSR entry despite the fact that we've exited.
    //
    // (c) Code the performs an OSR exit should not immediately retry OSR
    //     entry, since both forms of OSR are expensive. OSR entry is
    //     particularly expensive.
    //
    // (d) Frequent OSR failures, even those that do not result in the code
    //     running in a hot loop, result in recompilation getting triggered.
    //
    // To ensure (c), we'd like to set the execute counter to
    // counterValueForOptimizeAfterWarmUp(). This seems like it would endanger
    // (a) and (b), since then every OSR exit would delay the opportunity for
    // every call frame to perform OSR entry. Essentially, if OSR exit happens
    // frequently and the function has few loops, then the counter will never
    // become non-negative and OSR entry will never be triggered. OSR entry
    // will only happen if a loop gets hot in the old JIT, which does a pretty
    // good job of ensuring (a) and (b). But that doesn't take care of (d),
    // since each speculation failure would reset the execute counter.
    // So we check here if the number of speculation failures is significantly
    // larger than the number of successes (we want 90% success rate), and if
    // there have been a large enough number of failures. If so, we set the
    // counter to 0; otherwise we set the counter to
    // counterValueForOptimizeAfterWarmUp().
    
    handleExitCounts(m_jit, exit);
    
    // Reify inlined call frames.
    
    reifyInlinedCallFrames(m_jit, exit);

    // And finish.
    adjustAndJumpToTarget(m_jit, exit);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT) && USE(JSVALUE64)
