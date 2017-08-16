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

#pragma once

#if ENABLE(B3_JIT)

#include "FPRInfo.h"
#include "GPRInfo.h"
#include "JSCJSValue.h"
#include "Reg.h"
#include "RegisterSet.h"
#include "ValueRecovery.h"
#include <wtf/PrintStream.h>

namespace JSC {

class AssemblyHelpers;

namespace B3 {

// We use this class to describe value representations at stackmaps. It's used both to force a
// representation and to get the representation. When the B3 client forces a representation, we say
// that it's an input. When B3 tells the client what representation it picked, we say that it's an
// output.

class ValueRep {
public:
    enum Kind {
        // As an input representation, this means that B3 can pick any representation. As an output
        // representation, this means that we don't know. This will only arise as an output
        // representation for the active arguments of Check/CheckAdd/CheckSub/CheckMul.
        WarmAny,

        // Same as WarmAny, but implies that the use is cold. A cold use is not counted as a use for
        // computing the priority of the used temporary.
        ColdAny,

        // Same as ColdAny, but also implies that the use occurs after all other effects of the stackmap
        // value.
        LateColdAny,

        // As an input representation, this means that B3 should pick some register. It could be a
        // register that this claims to clobber!
        SomeRegister,

        // As an input representation, this tells us that B3 should pick some register, but implies
        // that the def happens before any of the effects of the stackmap. This is only valid for
        // the result constraint of a Patchpoint.
        SomeEarlyRegister,
        
        // As an input representation, this forces a particular register. As an output
        // representation, this tells us what register B3 picked.
        Register,

        // As an input representation, this forces a particular register and states that
        // the register is used late. This means that the register is used after the result
        // is defined (i.e, the result will interfere with this as an input).
        // It's not a valid output representation.
        LateRegister,

        // As an output representation, this tells us what stack slot B3 picked. It's not a valid
        // input representation.
        Stack,

        // As an input representation, this forces the value to end up in the argument area at some
        // offset.
        StackArgument,

        // As an output representation, this tells us that B3 constant-folded the value.
        Constant
    };
    
    ValueRep()
        : m_kind(WarmAny)
    {
    }

    explicit ValueRep(Reg reg)
        : m_kind(Register)
    {
        u.reg = reg;
    }

    ValueRep(Kind kind)
        : m_kind(kind)
    {
        ASSERT(kind == WarmAny || kind == ColdAny || kind == LateColdAny || kind == SomeRegister || kind == SomeEarlyRegister);
    }

    static ValueRep reg(Reg reg)
    {
        return ValueRep(reg);
    }

    static ValueRep lateReg(Reg reg)
    {
        ValueRep result(reg);
        result.m_kind = LateRegister;
        return result;
    }

    static ValueRep stack(intptr_t offsetFromFP)
    {
        ValueRep result;
        result.m_kind = Stack;
        result.u.offsetFromFP = offsetFromFP;
        return result;
    }

    static ValueRep stackArgument(intptr_t offsetFromSP)
    {
        ValueRep result;
        result.m_kind = StackArgument;
        result.u.offsetFromSP = offsetFromSP;
        return result;
    }

    static ValueRep constant(int64_t value)
    {
        ValueRep result;
        result.m_kind = Constant;
        result.u.value = value;
        return result;
    }

    static ValueRep constantDouble(double value)
    {
        return ValueRep::constant(bitwise_cast<int64_t>(value));
    }

    Kind kind() const { return m_kind; }

    bool operator==(const ValueRep& other) const
    {
        if (kind() != other.kind())
            return false;
        switch (kind()) {
        case LateRegister:
        case Register:
            return u.reg == other.u.reg;
        case Stack:
            return u.offsetFromFP == other.u.offsetFromFP;
        case StackArgument:
            return u.offsetFromSP == other.u.offsetFromSP;
        case Constant:
            return u.value == other.u.value;
        default:
            return true;
        }
    }

    bool operator!=(const ValueRep& other) const
    {
        return !(*this == other);
    }

    explicit operator bool() const { return kind() != WarmAny; }

    bool isAny() const { return kind() == WarmAny || kind() == ColdAny || kind() == LateColdAny; }

    bool isReg() const { return kind() == Register || kind() == LateRegister; }
    
    Reg reg() const
    {
        ASSERT(isReg());
        return u.reg;
    }

    bool isGPR() const { return isReg() && reg().isGPR(); }
    bool isFPR() const { return isReg() && reg().isFPR(); }

    GPRReg gpr() const { return reg().gpr(); }
    FPRReg fpr() const { return reg().fpr(); }

    bool isStack() const { return kind() == Stack; }

    intptr_t offsetFromFP() const
    {
        ASSERT(isStack());
        return u.offsetFromFP;
    }

    bool isStackArgument() const { return kind() == StackArgument; }

    intptr_t offsetFromSP() const
    {
        ASSERT(isStackArgument());
        return u.offsetFromSP;
    }

    bool isConstant() const { return kind() == Constant; }

    int64_t value() const
    {
        ASSERT(isConstant());
        return u.value;
    }

    double doubleValue() const
    {
        return bitwise_cast<double>(value());
    }

    ValueRep withOffset(intptr_t offset) const
    {
        switch (kind()) {
        case Stack:
            return stack(offsetFromFP() + offset);
        case StackArgument:
            return stackArgument(offsetFromSP() + offset);
        default:
            return *this;
        }
    }

    void addUsedRegistersTo(RegisterSet&) const;
    
    RegisterSet usedRegisters() const;

    // Get the used registers for a vector of ValueReps.
    template<typename VectorType>
    static RegisterSet usedRegisters(const VectorType& vector)
    {
        RegisterSet result;
        for (const ValueRep& value : vector)
            value.addUsedRegistersTo(result);
        return result;
    }

    JS_EXPORT_PRIVATE void dump(PrintStream&) const;

    // This has a simple contract: it emits code to restore the value into the given register. This
    // will work even if it requires moving between bits a GPR and a FPR.
    void emitRestore(AssemblyHelpers&, Reg) const;

    // Computes the ValueRecovery assuming that the Value* was for a JSValue (i.e. Int64).
    // NOTE: We should avoid putting JSValue-related methods in B3, but this was hard to avoid
    // because some parts of JSC use ValueRecovery like a general "where my bits at" object, almost
    // exactly like ValueRep.
    ValueRecovery recoveryForJSValue() const;

private:
    Kind m_kind;
    union U {
        Reg reg;
        intptr_t offsetFromFP;
        intptr_t offsetFromSP;
        int64_t value;

        U()
        {
            memset(this, 0, sizeof(*this));
        }
    } u;
};

} } // namespace JSC::B3

namespace WTF {

void printInternal(PrintStream&, JSC::B3::ValueRep::Kind);

} // namespace WTF

#endif // ENABLE(B3_JIT)
