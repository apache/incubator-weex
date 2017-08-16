/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "Reg.h"
#include <wtf/HashMap.h>

namespace JSC { namespace B3 { namespace Air {

class Arg;

// A Tmp is a generalization of a register. It can be used to refer to any GPR or FPR. It can also
// be used to refer to an unallocated register (i.e. a temporary). Like many Air classes, we use
// deliberately terse naming since we will have to use this name a lot.

class Tmp {
public:
    Tmp()
        : m_value(0)
    {
    }

    explicit Tmp(Reg reg)
    {
        if (reg) {
            if (reg.isGPR())
                m_value = encodeGPR(reg.gpr());
            else
                m_value = encodeFPR(reg.fpr());
        } else
            m_value = 0;
    }

    explicit Tmp(const Arg&);

    static Tmp gpTmpForIndex(unsigned index)
    {
        Tmp result;
        result.m_value = encodeGPTmp(index);
        return result;
    }

    static Tmp fpTmpForIndex(unsigned index)
    {
        Tmp result;
        result.m_value = encodeFPTmp(index);
        return result;
    }

    explicit operator bool() const { return !!m_value; }

    bool isGP() const
    {
        return isEncodedGP(m_value);
    }

    bool isFP() const
    {
        return isEncodedFP(m_value);
    }

    bool isGPR() const
    {
        return isEncodedGPR(m_value);
    }

    bool isFPR() const
    {
        return isEncodedFPR(m_value);
    }

    bool isReg() const
    {
        return isGPR() || isFPR();
    }

    GPRReg gpr() const
    {
        return decodeGPR(m_value);
    }

    FPRReg fpr() const
    {
        return decodeFPR(m_value);
    }

    Reg reg() const
    {
        if (isGP())
            return gpr();
        return fpr();
    }

    bool hasTmpIndex() const
    {
        return !isReg();
    }

    unsigned gpTmpIndex() const
    {
        return decodeGPTmp(m_value);
    }

    unsigned fpTmpIndex() const
    {
        return decodeFPTmp(m_value);
    }

    unsigned tmpIndex() const
    {
        if (isGP())
            return gpTmpIndex();
        return fpTmpIndex();
    }

    bool isAlive() const
    {
        return !!*this;
    }

    bool operator==(const Tmp& other) const
    {
        return m_value == other.m_value;
    }

    bool operator!=(const Tmp& other) const
    {
        return !(*this == other);
    }

    void dump(PrintStream& out) const;

    Tmp(WTF::HashTableDeletedValueType)
        : m_value(std::numeric_limits<int>::max())
    {
    }

    bool isHashTableDeletedValue() const
    {
        return *this == Tmp(WTF::HashTableDeletedValue);
    }

    unsigned hash() const
    {
        return WTF::IntHash<int>::hash(m_value);
    }

    unsigned internalValue() const { return static_cast<unsigned>(m_value); }

    static Tmp tmpForInternalValue(unsigned index)
    {
        Tmp result;
        result.m_value = static_cast<int>(index);
        return result;
    }

private:
    static int encodeGP(unsigned index)
    {
        return 1 + index;
    }

    static int encodeFP(unsigned index)
    {
        return -1 - index;
    }

    static int encodeGPR(GPRReg gpr)
    {
        return encodeGP(gpr - MacroAssembler::firstRegister());
    }

    static int encodeFPR(FPRReg fpr)
    {
        return encodeFP(fpr - MacroAssembler::firstFPRegister());
    }

    static int encodeGPTmp(unsigned index)
    {
        return encodeGPR(MacroAssembler::lastRegister()) + 1 + index;
    }

    static int encodeFPTmp(unsigned index)
    {
        return encodeFPR(MacroAssembler::lastFPRegister()) - 1 - index;
    }

    static bool isEncodedGP(int value)
    {
        return value > 0;
    }

    static bool isEncodedFP(int value)
    {
        return value < 0;
    }

    static bool isEncodedGPR(int value)
    {
        return isEncodedGP(value) && value <= encodeGPR(MacroAssembler::lastRegister());
    }

    static bool isEncodedFPR(int value)
    {
        return isEncodedFP(value) && value >= encodeFPR(MacroAssembler::lastFPRegister());
    }

    static bool isEncodedGPTmp(int value)
    {
        return isEncodedGP(value) && !isEncodedGPR(value);
    }

    static bool isEncodedFPTmp(int value)
    {
        return isEncodedFP(value) && !isEncodedFPR(value);
    }

    static GPRReg decodeGPR(int value)
    {
        ASSERT(isEncodedGPR(value));
        return static_cast<GPRReg>(
            (value - encodeGPR(MacroAssembler::firstRegister())) + MacroAssembler::firstRegister());
    }

    static FPRReg decodeFPR(int value)
    {
        ASSERT(isEncodedFPR(value));
        return static_cast<FPRReg>(
            (encodeFPR(MacroAssembler::firstFPRegister()) - value) +
            MacroAssembler::firstFPRegister());
    }

    static unsigned decodeGPTmp(int value)
    {
        ASSERT(isEncodedGPTmp(value));
        return value - (encodeGPR(MacroAssembler::lastRegister()) + 1);
    }

    static unsigned decodeFPTmp(int value)
    {
        ASSERT(isEncodedFPTmp(value));
        return (encodeFPR(MacroAssembler::lastFPRegister()) - 1) - value;
    }

    // 0: empty Tmp
    // positive: GPRs and then GP temps.
    // negative: FPRs and then FP temps.
    int m_value;
};

struct TmpHash {
    static unsigned hash(const Tmp& key) { return key.hash(); }
    static bool equal(const Tmp& a, const Tmp& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } } // namespace JSC::B3::Air

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::B3::Air::Tmp> {
    typedef JSC::B3::Air::TmpHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::B3::Air::Tmp> : SimpleClassHashTraits<JSC::B3::Air::Tmp> { };

} // namespace WTF

#endif // ENABLE(B3_JIT)
