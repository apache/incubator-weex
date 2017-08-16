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

#if ENABLE(DFG_JIT) && USE(JSVALUE32_64)

#include "DFGOperations.h"
#include "DFGOSRExitCompilerCommon.h"
#include "DFGSpeculativeJIT.h"
#include "JSCInlines.h"
#include <wtf/DataLog.h>

namespace JSC { namespace DFG {

void OSRExitCompiler::compileExit(const OSRExit& exit, const Operands<ValueRecovery>& operands, SpeculationRecovery* recovery)
{
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
            break;

        case SpeculativeAddImmediate:
            m_jit.sub32(AssemblyHelpers::Imm32(recovery->immediate()), recovery->dest());
            break;
            
        case BooleanSpeculationCheck:
            break;
            
        default:
            break;
        }
    }

    // Refine some value profile, if appropriate.
    
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
            
            // Note: We are free to assume that the jsValueSource is already known to
            // be a cell since both BadCache and BadIndexingType exits occur after
            // the cell check would have already happened.
            
            CodeOrigin codeOrigin = exit.m_codeOriginForExitProfile;
            if (ArrayProfile* arrayProfile = m_jit.baselineCodeBlockFor(codeOrigin)->getArrayProfile(codeOrigin.bytecodeIndex)) {
                GPRReg usedRegister1;
                GPRReg usedRegister2;
                if (exit.m_jsValueSource.isAddress()) {
                    usedRegister1 = exit.m_jsValueSource.base();
                    usedRegister2 = InvalidGPRReg;
                } else {
                    usedRegister1 = exit.m_jsValueSource.payloadGPR();
                    if (exit.m_jsValueSource.hasKnownTag())
                        usedRegister2 = InvalidGPRReg;
                    else
                        usedRegister2 = exit.m_jsValueSource.tagGPR();
                }
                
                GPRReg scratch1;
                GPRReg scratch2;
                scratch1 = AssemblyHelpers::selectScratchGPR(usedRegister1, usedRegister2);
                scratch2 = AssemblyHelpers::selectScratchGPR(usedRegister1, usedRegister2, scratch1);
                
                m_jit.push(scratch1);
                m_jit.push(scratch2);
                
                GPRReg value;
                if (exit.m_jsValueSource.isAddress()) {
                    value = scratch1;
                    m_jit.loadPtr(AssemblyHelpers::Address(exit.m_jsValueSource.asAddress()), value);
                } else
                    value = exit.m_jsValueSource.payloadGPR();
                
                m_jit.loadPtr(AssemblyHelpers::Address(value, JSCell::structureIDOffset()), scratch1);
                m_jit.storePtr(scratch1, arrayProfile->addressOfLastSeenStructureID());
                m_jit.load8(AssemblyHelpers::Address(scratch1, Structure::indexingTypeIncludingHistoryOffset()), scratch1);
                m_jit.move(AssemblyHelpers::TrustedImm32(1), scratch2);
                m_jit.lshift32(scratch1, scratch2);
                m_jit.or32(scratch2, AssemblyHelpers::AbsoluteAddress(arrayProfile->addressOfArrayModes()));
                
                m_jit.pop(scratch2);
                m_jit.pop(scratch1);
            }
        }
        
        if (MethodOfGettingAValueProfile profile = exit.m_valueProfile) {
            if (exit.m_jsValueSource.isAddress()) {
                // Save a register so we can use it.
                GPRReg scratchPayload = AssemblyHelpers::selectScratchGPR(exit.m_jsValueSource.base());
                GPRReg scratchTag = AssemblyHelpers::selectScratchGPR(exit.m_jsValueSource.base(), scratchPayload);
                m_jit.pushToSave(scratchPayload);
                m_jit.pushToSave(scratchTag);

                JSValueRegs scratch(scratchTag, scratchPayload);
                
                m_jit.loadValue(exit.m_jsValueSource.asAddress(), scratch);
                profile.emitReportValue(m_jit, scratch);
                
                m_jit.popToRestore(scratchTag);
                m_jit.popToRestore(scratchPayload);
            } else if (exit.m_jsValueSource.hasKnownTag()) {
                GPRReg scratchTag = AssemblyHelpers::selectScratchGPR(exit.m_jsValueSource.payloadGPR());
                m_jit.pushToSave(scratchTag);
                m_jit.move(AssemblyHelpers::TrustedImm32(exit.m_jsValueSource.tag()), scratchTag);
                JSValueRegs value(scratchTag, exit.m_jsValueSource.payloadGPR());
                profile.emitReportValue(m_jit, value);
                m_jit.popToRestore(scratchTag);
            } else
                profile.emitReportValue(m_jit, exit.m_jsValueSource.regs());
        }
    }
    
    // Do a simplified OSR exit. See DFGOSRExitCompiler64.cpp's comment regarding how and wny we
    // do this simple approach.

    // Save all state from GPRs into the scratch buffer.
    
    ScratchBuffer* scratchBuffer = m_jit.vm()->scratchBufferForSize(sizeof(EncodedJSValue) * operands.size());
    EncodedJSValue* scratch = scratchBuffer ? static_cast<EncodedJSValue*>(scratchBuffer->dataBuffer()) : 0;
    
    for (size_t index = 0; index < operands.size(); ++index) {
        const ValueRecovery& recovery = operands[index];
        
        switch (recovery.technique()) {
        case UnboxedInt32InGPR:
        case UnboxedBooleanInGPR:
        case UnboxedCellInGPR:
            m_jit.store32(
                recovery.gpr(),
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.payload);
            break;
            
        case InPair:
            m_jit.store32(
                recovery.tagGPR(),
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.tag);
            m_jit.store32(
                recovery.payloadGPR(),
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.payload);
            break;
            
        default:
            break;
        }
    }
    
    // Now all GPRs are free to reuse.
    
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
    
    // Now all FPRs are free to reuse.
    
    // Save all state from the stack into the scratch buffer. For simplicity we
    // do this even for state that's already in the right place on the stack.
    // It makes things simpler later.
    
    for (size_t index = 0; index < operands.size(); ++index) {
        const ValueRecovery& recovery = operands[index];
        
        switch (recovery.technique()) {
        case DisplacedInJSStack:
        case Int32DisplacedInJSStack:
        case DoubleDisplacedInJSStack:
        case CellDisplacedInJSStack:
        case BooleanDisplacedInJSStack:
            m_jit.load32(
                AssemblyHelpers::tagFor(recovery.virtualRegister()),
                GPRInfo::regT0);
            m_jit.load32(
                AssemblyHelpers::payloadFor(recovery.virtualRegister()),
                GPRInfo::regT1);
            m_jit.store32(
                GPRInfo::regT0,
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.tag);
            m_jit.store32(
                GPRInfo::regT1,
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.payload);
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
        case InPair:
        case DisplacedInJSStack:
        case InFPR:
            m_jit.load32(
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.tag,
                GPRInfo::regT0);
            m_jit.load32(
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.payload,
                GPRInfo::regT1);
            m_jit.store32(
                GPRInfo::regT0,
                AssemblyHelpers::tagFor(operand));
            m_jit.store32(
                GPRInfo::regT1,
                AssemblyHelpers::payloadFor(operand));
            break;
            
        case UnboxedDoubleInFPR:
        case DoubleDisplacedInJSStack:
            m_jit.move(AssemblyHelpers::TrustedImmPtr(scratch + index), GPRInfo::regT0);
            m_jit.loadDouble(MacroAssembler::Address(GPRInfo::regT0), FPRInfo::fpRegT0);
            m_jit.purifyNaN(FPRInfo::fpRegT0);
            m_jit.storeDouble(FPRInfo::fpRegT0, AssemblyHelpers::addressFor(operand));
            break;

        case UnboxedInt32InGPR:
        case Int32DisplacedInJSStack:
            m_jit.load32(
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.payload,
                GPRInfo::regT0);
            m_jit.store32(
                AssemblyHelpers::TrustedImm32(JSValue::Int32Tag),
                AssemblyHelpers::tagFor(operand));
            m_jit.store32(
                GPRInfo::regT0,
                AssemblyHelpers::payloadFor(operand));
            break;
            
        case UnboxedCellInGPR:
        case CellDisplacedInJSStack:
            m_jit.load32(
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.payload,
                GPRInfo::regT0);
            m_jit.store32(
                AssemblyHelpers::TrustedImm32(JSValue::CellTag),
                AssemblyHelpers::tagFor(operand));
            m_jit.store32(
                GPRInfo::regT0,
                AssemblyHelpers::payloadFor(operand));
            break;
            
        case UnboxedBooleanInGPR:
        case BooleanDisplacedInJSStack:
            m_jit.load32(
                &bitwise_cast<EncodedValueDescriptor*>(scratch + index)->asBits.payload,
                GPRInfo::regT0);
            m_jit.store32(
                AssemblyHelpers::TrustedImm32(JSValue::BooleanTag),
                AssemblyHelpers::tagFor(operand));
            m_jit.store32(
                GPRInfo::regT0,
                AssemblyHelpers::payloadFor(operand));
            break;
            
        case Constant:
            m_jit.store32(
                AssemblyHelpers::TrustedImm32(recovery.constant().tag()),
                AssemblyHelpers::tagFor(operand));
            m_jit.store32(
                AssemblyHelpers::TrustedImm32(recovery.constant().payload()),
                AssemblyHelpers::payloadFor(operand));
            break;
            
        case DirectArgumentsThatWereNotCreated:
        case ClonedArgumentsThatWereNotCreated:
            // Don't do this, yet.
            break;
            
        default:
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

#endif // ENABLE(DFG_JIT) && USE(JSVALUE32_64)
