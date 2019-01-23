/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "GPRInfo.h"
#include "JSCJSValue.h"
#include "ResultType.h"
#include "TagRegistersMode.h"

namespace JSC {

class CCallHelpers;

struct ObservedType {
    ObservedType(uint8_t bits = TypeEmpty)
        : m_bits(bits)
    { }

    bool sawInt32() const { return m_bits & TypeInt32; }
    bool isOnlyInt32() const { return m_bits == TypeInt32; }
    bool sawNumber() const { return m_bits & TypeNumber; }
    bool isOnlyNumber() const { return m_bits == TypeNumber; }
    bool sawNonNumber() const { return m_bits & TypeNonNumber; }
    bool isOnlyNonNumber() const { return m_bits == TypeNonNumber; }
    bool isEmpty() const { return !m_bits; }
    uint8_t bits() const { return m_bits; }

    ObservedType withInt32() const { return ObservedType(m_bits | TypeInt32); }
    ObservedType withNumber() const { return ObservedType(m_bits | TypeNumber); }
    ObservedType withNonNumber() const { return ObservedType(m_bits | TypeNonNumber); }
    ObservedType withoutNonNumber() const { return ObservedType(m_bits & ~TypeNonNumber); }

    bool operator==(const ObservedType& other) const { return m_bits == other.m_bits; }

    static const uint8_t TypeEmpty = 0x0;
    static const uint8_t TypeInt32 = 0x1;
    static const uint8_t TypeNumber = 0x02;
    static const uint8_t TypeNonNumber = 0x04;

    static const uint32_t numBitsNeeded = 3;

private:
    uint8_t m_bits;
};

struct ArithProfile {
private:
    static const uint32_t numberOfFlagBits = 5;
    static const uint32_t rhsResultTypeShift = numberOfFlagBits;
    static const uint32_t lhsResultTypeShift = rhsResultTypeShift + ResultType::numBitsNeeded;
    static const uint32_t rhsObservedTypeShift = lhsResultTypeShift + ResultType::numBitsNeeded;
    static const uint32_t lhsObservedTypeShift = rhsObservedTypeShift + ObservedType::numBitsNeeded;

    static_assert(ObservedType::numBitsNeeded == 3, "We make a hard assumption about that here.");
    static const uint32_t clearRhsObservedTypeBitMask = static_cast<uint32_t>(~((1 << rhsObservedTypeShift) | (1 << (rhsObservedTypeShift + 1)) | (1 << (rhsObservedTypeShift + 2))));
    static const uint32_t clearLhsObservedTypeBitMask = static_cast<uint32_t>(~((1 << lhsObservedTypeShift) | (1 << (lhsObservedTypeShift + 1)) | (1 << (lhsObservedTypeShift + 2))));

    static const uint32_t resultTypeMask = (1 << ResultType::numBitsNeeded) - 1;
    static const uint32_t observedTypeMask = (1 << ObservedType::numBitsNeeded) - 1;
public:
    static const uint32_t specialFastPathBit = 1 << (lhsObservedTypeShift + ObservedType::numBitsNeeded);
    static_assert((lhsObservedTypeShift + ObservedType::numBitsNeeded) <= (sizeof(uint32_t) * 8) - 1, "Should fit in a uint32_t.");
    static_assert(!(specialFastPathBit & ~clearLhsObservedTypeBitMask), "These bits should not intersect.");
    static_assert(specialFastPathBit & clearLhsObservedTypeBitMask, "These bits should intersect.");
    static_assert(specialFastPathBit > ~clearLhsObservedTypeBitMask, "These bits should not intersect and specialFastPathBit should be a higher bit.");

    ArithProfile(ResultType arg)
    {
        m_bits = (arg.bits() << lhsResultTypeShift);
        ASSERT(lhsResultType().bits() == arg.bits());
        ASSERT(lhsObservedType().isEmpty());
        ASSERT(rhsObservedType().isEmpty());
    }

    ArithProfile(ResultType lhs, ResultType rhs)
    {
        m_bits = (lhs.bits() << lhsResultTypeShift) | (rhs.bits() << rhsResultTypeShift);
        ASSERT(lhsResultType().bits() == lhs.bits() && rhsResultType().bits() == rhs.bits());
        ASSERT(lhsObservedType().isEmpty());
        ASSERT(rhsObservedType().isEmpty());
    }
    ArithProfile() = default;

    static ArithProfile fromInt(uint32_t bits)
    {
        ArithProfile result;
        result.m_bits = bits;
        return result;
    }

    enum ObservedResults {
        NonNegZeroDouble = 1 << 0,
        NegZeroDouble    = 1 << 1,
        NonNumber        = 1 << 2,
        Int32Overflow    = 1 << 3,
        Int52Overflow    = 1 << 4,
    };

    ResultType lhsResultType() const { return ResultType((m_bits >> lhsResultTypeShift) & resultTypeMask); }
    ResultType rhsResultType() const { return ResultType((m_bits >> rhsResultTypeShift) & resultTypeMask); }

    ObservedType lhsObservedType() const { return ObservedType((m_bits >> lhsObservedTypeShift) & observedTypeMask); }
    ObservedType rhsObservedType() const { return ObservedType((m_bits >> rhsObservedTypeShift) & observedTypeMask); }
    void setLhsObservedType(ObservedType type)
    {
        uint32_t bits = m_bits;
        bits &= clearLhsObservedTypeBitMask;
        bits |= type.bits() << lhsObservedTypeShift;
        m_bits = bits;
        ASSERT(lhsObservedType() == type);
    }

    void setRhsObservedType(ObservedType type)
    { 
        uint32_t bits = m_bits;
        bits &= clearRhsObservedTypeBitMask;
        bits |= type.bits() << rhsObservedTypeShift;
        m_bits = bits;
        ASSERT(rhsObservedType() == type);
    }

    bool tookSpecialFastPath() const { return m_bits & specialFastPathBit; }

    bool didObserveNonInt32() const { return hasBits(NonNegZeroDouble | NegZeroDouble | NonNumber); }
    bool didObserveDouble() const { return hasBits(NonNegZeroDouble | NegZeroDouble); }
    bool didObserveNonNegZeroDouble() const { return hasBits(NonNegZeroDouble); }
    bool didObserveNegZeroDouble() const { return hasBits(NegZeroDouble); }
    bool didObserveNonNumber() const { return hasBits(NonNumber); }
    bool didObserveInt32Overflow() const { return hasBits(Int32Overflow); }
    bool didObserveInt52Overflow() const { return hasBits(Int52Overflow); }

    void setObservedNonNegZeroDouble() { setBit(NonNegZeroDouble); }
    void setObservedNegZeroDouble() { setBit(NegZeroDouble); }
    void setObservedNonNumber() { setBit(NonNumber); }
    void setObservedInt32Overflow() { setBit(Int32Overflow); }
    void setObservedInt52Overflow() { setBit(Int52Overflow); }

    const void* addressOfBits() const { return &m_bits; }

    void observeResult(JSValue value)
    {
        if (value.isInt32())
            return;
        if (value.isNumber()) {
            m_bits |= Int32Overflow | Int52Overflow | NonNegZeroDouble | NegZeroDouble;
            return;
        }
        m_bits |= NonNumber;
    }

    void lhsSawInt32() { setLhsObservedType(lhsObservedType().withInt32()); }
    void lhsSawNumber() { setLhsObservedType(lhsObservedType().withNumber()); }
    void lhsSawNonNumber() { setLhsObservedType(lhsObservedType().withNonNumber()); }
    void rhsSawInt32() { setRhsObservedType(rhsObservedType().withInt32()); }
    void rhsSawNumber() { setRhsObservedType(rhsObservedType().withNumber()); }
    void rhsSawNonNumber() { setRhsObservedType(rhsObservedType().withNonNumber()); }

    void observeLHS(JSValue lhs)
    {
        ArithProfile newProfile = *this;
        if (lhs.isNumber()) {
            if (lhs.isInt32())
                newProfile.lhsSawInt32();
            else
                newProfile.lhsSawNumber();
        } else
            newProfile.lhsSawNonNumber();

        m_bits = newProfile.bits();
    }

    void observeLHSAndRHS(JSValue lhs, JSValue rhs)
    {
        observeLHS(lhs);

        ArithProfile newProfile = *this;
        if (rhs.isNumber()) {
            if (rhs.isInt32())
                newProfile.rhsSawInt32();
            else
                newProfile.rhsSawNumber();
        } else
            newProfile.rhsSawNonNumber();

        m_bits = newProfile.bits();
    }

#if ENABLE(JIT)    
    // Sets (Int32Overflow | Int52Overflow | NonNegZeroDouble | NegZeroDouble) if it sees a
    // double. Sets NonNumber if it sees a non-number.
    void emitObserveResult(CCallHelpers&, JSValueRegs, TagRegistersMode = HaveTagRegisters);
    
    // Sets (Int32Overflow | Int52Overflow | NonNegZeroDouble | NegZeroDouble).
    bool shouldEmitSetDouble() const;
    void emitSetDouble(CCallHelpers&) const;
    
    // Sets NonNumber.
    void emitSetNonNumber(CCallHelpers&) const;
    bool shouldEmitSetNonNumber() const;
#endif // ENABLE(JIT)

    uint32_t bits() const { return m_bits; }

private:
    bool hasBits(int mask) const { return m_bits & mask; }
    void setBit(int mask) { m_bits |= mask; }

    uint32_t m_bits { 0 }; // We take care to update m_bits only in a single operation. We don't ever store an inconsistent bit representation to it.
};

} // namespace JSC

namespace WTF {

void printInternal(PrintStream&, const JSC::ArithProfile&);
void printInternal(PrintStream&, const JSC::ObservedType&);

} // namespace WTF
