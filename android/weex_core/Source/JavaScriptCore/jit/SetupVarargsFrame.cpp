/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "SetupVarargsFrame.h"

#if ENABLE(JIT)

#include "Interpreter.h"
#include "JSCInlines.h"
#include "StackAlignment.h"

namespace JSC {

void emitSetVarargsFrame(CCallHelpers& jit, GPRReg lengthGPR, bool lengthIncludesThis, GPRReg numUsedSlotsGPR, GPRReg resultGPR)
{
    jit.move(numUsedSlotsGPR, resultGPR);
    // We really want to make sure the size of the new call frame is a multiple of
    // stackAlignmentRegisters(), however it is easier to accomplish this by
    // rounding numUsedSlotsGPR to the next multiple of stackAlignmentRegisters().
    // Together with the rounding below, we will assure that the new call frame is
    // located on a stackAlignmentRegisters() boundary and a multiple of
    // stackAlignmentRegisters() in size.
    jit.addPtr(CCallHelpers::TrustedImm32(stackAlignmentRegisters() - 1), resultGPR);
    jit.andPtr(CCallHelpers::TrustedImm32(~(stackAlignmentRegisters() - 1)), resultGPR);

    jit.addPtr(lengthGPR, resultGPR);
    jit.addPtr(CCallHelpers::TrustedImm32(CallFrame::headerSizeInRegisters + (lengthIncludesThis ? 0 : 1)), resultGPR);
    
    // resultGPR now has the required frame size in Register units
    // Round resultGPR to next multiple of stackAlignmentRegisters()
    jit.addPtr(CCallHelpers::TrustedImm32(stackAlignmentRegisters() - 1), resultGPR);
    jit.andPtr(CCallHelpers::TrustedImm32(~(stackAlignmentRegisters() - 1)), resultGPR);
    
    // Now resultGPR has the right stack frame offset in Register units.
    jit.negPtr(resultGPR);
    jit.lshiftPtr(CCallHelpers::Imm32(3), resultGPR);
    jit.addPtr(GPRInfo::callFrameRegister, resultGPR);
}

static void emitSetupVarargsFrameFastCase(CCallHelpers& jit, GPRReg numUsedSlotsGPR, GPRReg scratchGPR1, GPRReg scratchGPR2, GPRReg scratchGPR3, ValueRecovery argCountRecovery, VirtualRegister firstArgumentReg, unsigned firstVarArgOffset, CCallHelpers::JumpList& slowCase)
{
    CCallHelpers::JumpList end;
    
    if (argCountRecovery.isConstant()) {
        // FIXME: We could constant-fold a lot of the computation below in this case.
        // https://bugs.webkit.org/show_bug.cgi?id=141486
        jit.move(CCallHelpers::TrustedImm32(argCountRecovery.constant().asInt32()), scratchGPR1);
    } else
        jit.load32(CCallHelpers::payloadFor(argCountRecovery.virtualRegister()), scratchGPR1);
    if (firstVarArgOffset) {
        CCallHelpers::Jump sufficientArguments = jit.branch32(CCallHelpers::GreaterThan, scratchGPR1, CCallHelpers::TrustedImm32(firstVarArgOffset + 1));
        jit.move(CCallHelpers::TrustedImm32(1), scratchGPR1);
        CCallHelpers::Jump endVarArgs = jit.jump();
        sufficientArguments.link(&jit);
        jit.sub32(CCallHelpers::TrustedImm32(firstVarArgOffset), scratchGPR1);
        endVarArgs.link(&jit);
    }
    slowCase.append(jit.branch32(CCallHelpers::Above, scratchGPR1, CCallHelpers::TrustedImm32(JSC::maxArguments + 1)));
    
    emitSetVarargsFrame(jit, scratchGPR1, true, numUsedSlotsGPR, scratchGPR2);

    slowCase.append(jit.branchPtr(CCallHelpers::Above, CCallHelpers::AbsoluteAddress(jit.vm()->addressOfSoftStackLimit()), scratchGPR2));

    // Before touching stack values, we should update the stack pointer to protect them from signal stack.
    jit.addPtr(CCallHelpers::TrustedImm32(sizeof(CallerFrameAndPC)), scratchGPR2, CCallHelpers::stackPointerRegister);

    // Initialize ArgumentCount.
    jit.store32(scratchGPR1, CCallHelpers::Address(scratchGPR2, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset));

    // Copy arguments.
    jit.signExtend32ToPtr(scratchGPR1, scratchGPR1);
    CCallHelpers::Jump done = jit.branchSubPtr(CCallHelpers::Zero, CCallHelpers::TrustedImm32(1), scratchGPR1);
    // scratchGPR1: argumentCount

    CCallHelpers::Label copyLoop = jit.label();
    int argOffset = (firstArgumentReg.offset() - 1 + firstVarArgOffset) * static_cast<int>(sizeof(Register));
#if USE(JSVALUE64)
    jit.load64(CCallHelpers::BaseIndex(GPRInfo::callFrameRegister, scratchGPR1, CCallHelpers::TimesEight, argOffset), scratchGPR3);
    jit.store64(scratchGPR3, CCallHelpers::BaseIndex(scratchGPR2, scratchGPR1, CCallHelpers::TimesEight, CallFrame::thisArgumentOffset() * static_cast<int>(sizeof(Register))));
#else // USE(JSVALUE64), so this begins the 32-bit case
    jit.load32(CCallHelpers::BaseIndex(GPRInfo::callFrameRegister, scratchGPR1, CCallHelpers::TimesEight, argOffset + TagOffset), scratchGPR3);
    jit.store32(scratchGPR3, CCallHelpers::BaseIndex(scratchGPR2, scratchGPR1, CCallHelpers::TimesEight, CallFrame::thisArgumentOffset() * static_cast<int>(sizeof(Register)) + TagOffset));
    jit.load32(CCallHelpers::BaseIndex(GPRInfo::callFrameRegister, scratchGPR1, CCallHelpers::TimesEight, argOffset + PayloadOffset), scratchGPR3);
    jit.store32(scratchGPR3, CCallHelpers::BaseIndex(scratchGPR2, scratchGPR1, CCallHelpers::TimesEight, CallFrame::thisArgumentOffset() * static_cast<int>(sizeof(Register)) + PayloadOffset));
#endif // USE(JSVALUE64), end of 32-bit case
    jit.branchSubPtr(CCallHelpers::NonZero, CCallHelpers::TrustedImm32(1), scratchGPR1).linkTo(copyLoop, &jit);
    
    done.link(&jit);
}

void emitSetupVarargsFrameFastCase(CCallHelpers& jit, GPRReg numUsedSlotsGPR, GPRReg scratchGPR1, GPRReg scratchGPR2, GPRReg scratchGPR3, InlineCallFrame* inlineCallFrame, unsigned firstVarArgOffset, CCallHelpers::JumpList& slowCase)
{
    ValueRecovery argumentCountRecovery;
    VirtualRegister firstArgumentReg;
    if (inlineCallFrame) {
        if (inlineCallFrame->isVarargs()) {
            argumentCountRecovery = ValueRecovery::displacedInJSStack(
                inlineCallFrame->argumentCountRegister, DataFormatInt32);
        } else {
            argumentCountRecovery = ValueRecovery::constant(
                jsNumber(inlineCallFrame->arguments.size()));
        }
        if (inlineCallFrame->arguments.size() > 1)
            firstArgumentReg = inlineCallFrame->arguments[1].virtualRegister();
        else
            firstArgumentReg = VirtualRegister(0);
    } else {
        argumentCountRecovery = ValueRecovery::displacedInJSStack(
            VirtualRegister(CallFrameSlot::argumentCount), DataFormatInt32);
        firstArgumentReg = VirtualRegister(CallFrame::argumentOffset(0));
    }
    emitSetupVarargsFrameFastCase(jit, numUsedSlotsGPR, scratchGPR1, scratchGPR2, scratchGPR3, argumentCountRecovery, firstArgumentReg, firstVarArgOffset, slowCase);
}

} // namespace JSC

#endif // ENABLE(JIT)

