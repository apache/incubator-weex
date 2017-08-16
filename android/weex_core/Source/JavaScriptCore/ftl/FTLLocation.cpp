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
#include "FTLLocation.h"

#if ENABLE(FTL_JIT)

#include "B3ValueRep.h"
#include "FTLSaveRestore.h"
#include "RegisterSet.h"
#include <wtf/CommaPrinter.h>
#include <wtf/DataLog.h>
#include <wtf/ListDump.h>

namespace JSC { namespace FTL {

using namespace B3;

Location Location::forValueRep(const ValueRep& rep)
{
    switch (rep.kind()) {
    case ValueRep::Register:
        return forRegister(rep.reg(), 0);
    case ValueRep::Stack:
        return forIndirect(GPRInfo::callFrameRegister, rep.offsetFromFP());
    case ValueRep::Constant:
        return forConstant(rep.value());
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return Location();
    }
}

void Location::dump(PrintStream& out) const
{
    out.print("(", kind());
    if (hasReg())
        out.print(", ", reg());
    if (hasOffset())
        out.print(", ", offset());
    if (hasAddend())
        out.print(", ", addend());
    if (hasConstant())
        out.print(", ", constant());
    out.print(")");
}

bool Location::involvesGPR() const
{
    return isGPR() || kind() == Indirect;
}

bool Location::isGPR() const
{
    return kind() == Register && reg().isGPR();
}

GPRReg Location::gpr() const
{
    return reg().gpr();
}

bool Location::isFPR() const
{
    return kind() == Register && reg().isFPR();
}

FPRReg Location::fpr() const
{
    return reg().fpr();
}

void Location::restoreInto(MacroAssembler& jit, char* savedRegisters, GPRReg result, unsigned numFramesToPop) const
{
    if (involvesGPR() && RegisterSet::stackRegisters().get(gpr())) {
        // Make the result GPR contain the appropriate stack register.
        if (numFramesToPop) {
            jit.move(MacroAssembler::framePointerRegister, result);
            
            for (unsigned i = numFramesToPop - 1; i--;)
                jit.loadPtr(result, result);
            
            if (gpr() == MacroAssembler::framePointerRegister)
                jit.loadPtr(result, result);
            else
                jit.addPtr(MacroAssembler::TrustedImmPtr(sizeof(void*) * 2), result);
        } else
            jit.move(gpr(), result);
    }
    
    if (isGPR()) {
        if (RegisterSet::stackRegisters().get(gpr())) {
            // Already restored into result.
        } else
            jit.load64(savedRegisters + offsetOfGPR(gpr()), result);
        
        if (addend())
            jit.add64(MacroAssembler::TrustedImm32(addend()), result);
        return;
    }
    
    if (isFPR()) {
        jit.load64(savedRegisters + offsetOfFPR(fpr()), result);
        ASSERT(!addend());
        return;
    }
    
    switch (kind()) {
    case Register:
        // B3 used some register that we don't know about!
        dataLog("Unrecognized location: ", *this, "\n");
        RELEASE_ASSERT_NOT_REACHED();
        return;
        
    case Indirect:
        if (RegisterSet::stackRegisters().get(gpr())) {
            // The stack register is already recovered into result.
            jit.load64(MacroAssembler::Address(result, offset()), result);
            return;
        }
        
        jit.load64(savedRegisters + offsetOfGPR(gpr()), result);
        jit.load64(MacroAssembler::Address(result, offset()), result);
        return;
        
    case Constant:
        jit.move(MacroAssembler::TrustedImm64(constant()), result);
        return;
        
    case Unprocessed:
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

GPRReg Location::directGPR() const
{
    RELEASE_ASSERT(!addend());
    return gpr();
}

} } // namespace JSC::FTL

namespace WTF {

using namespace JSC::FTL;

void printInternal(PrintStream& out, JSC::FTL::Location::Kind kind)
{
    switch (kind) {
    case Location::Unprocessed:
        out.print("Unprocessed");
        return;
    case Location::Register:
        out.print("Register");
        return;
    case Location::Indirect:
        out.print("Indirect");
        return;
    case Location::Constant:
        out.print("Constant");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(FTL_JIT)

