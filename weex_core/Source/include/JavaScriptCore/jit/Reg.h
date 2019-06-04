/*
 * Copyright (C) 2014, 2016 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)

#include "MacroAssembler.h"

namespace JSC {

// Reg is a polymorphic register class. It can refer to either integer or float registers.
// Here are some use cases:
//
// GPRReg gpr;
// Reg reg = gpr;
// reg.isSet() == true
// reg.isGPR() == true
// reg.isFPR() == false
//
// for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
//     if (reg.isGPR()) {
//     } else /* reg.isFPR() */ {
//     }
// }
//
// The above loop could have also used !!reg or reg.isSet() as a condition.

class Reg {
public:
    Reg()
        : m_index(invalid())
    {
    }

    Reg(WTF::HashTableDeletedValueType)
        : m_index(deleted())
    {
    }
    
    Reg(MacroAssembler::RegisterID reg)
        : m_index(MacroAssembler::registerIndex(reg))
    {
    }
    
    Reg(MacroAssembler::FPRegisterID reg)
        : m_index(MacroAssembler::registerIndex(reg))
    {
    }
    
    static Reg fromIndex(unsigned index)
    {
        Reg result;
        result.m_index = index;
        return result;
    }
    
    static Reg first()
    {
        Reg result;
        result.m_index = 0;
        return result;
    }
    
    static Reg last()
    {
        Reg result;
        result.m_index = MacroAssembler::numberOfRegisters() + MacroAssembler::numberOfFPRegisters() - 1;
        return result;
    }
    
    Reg next() const
    {
        ASSERT(!!*this);
        if (*this == last())
            return Reg();
        Reg result;
        result.m_index = m_index + 1;
        return result;
    }
    
    unsigned index() const { return m_index; }

    static unsigned maxIndex()
    {
        return last().index();
    }
    
    bool isSet() const { return m_index != invalid(); }
    explicit operator bool() const { return isSet(); }

    bool isHashTableDeletedValue() const { return m_index == deleted(); }
    
    bool isGPR() const
    {
        return m_index < MacroAssembler::numberOfRegisters();
    }
    
    bool isFPR() const
    {
        return (m_index - MacroAssembler::numberOfRegisters()) < MacroAssembler::numberOfFPRegisters();
    }
    
    MacroAssembler::RegisterID gpr() const
    {
        ASSERT(isGPR());
        return static_cast<MacroAssembler::RegisterID>(MacroAssembler::firstRegister() + m_index);
    }
    
    MacroAssembler::FPRegisterID fpr() const
    {
        ASSERT(isFPR());
        return static_cast<MacroAssembler::FPRegisterID>(
            MacroAssembler::firstFPRegister() + (m_index - MacroAssembler::numberOfRegisters()));
    }
    
    bool operator==(const Reg& other) const
    {
        return m_index == other.m_index;
    }
    
    bool operator!=(const Reg& other) const
    {
        return m_index != other.m_index;
    }
    
    bool operator<(const Reg& other) const
    {
        return m_index < other.m_index;
    }
    
    bool operator>(const Reg& other) const
    {
        return m_index > other.m_index;
    }
    
    bool operator<=(const Reg& other) const
    {
        return m_index <= other.m_index;
    }
    
    bool operator>=(const Reg& other) const
    {
        return m_index >= other.m_index;
    }
    
    unsigned hash() const
    {
        return m_index;
    }
    
    const char* debugName() const;
    
    void dump(PrintStream&) const;

    class AllRegsIterable {
    public:

        class iterator {
        public:
            iterator() { }

            explicit iterator(Reg reg)
                : m_regIndex(reg.index())
            {
            }

            Reg operator*() const { return Reg::fromIndex(m_regIndex); }

            iterator& operator++()
            {
                m_regIndex = Reg::fromIndex(m_regIndex).next().index();
                return *this;
            }

            bool operator==(const iterator& other) const
            {
                return m_regIndex == other.m_regIndex;
            }

            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }

        private:
            unsigned m_regIndex;
        };

        iterator begin() const { return iterator(Reg::first()); }
        iterator end() const { return iterator(Reg()); }
    };

    static AllRegsIterable all() { return AllRegsIterable(); }

private:
    static uint8_t invalid() { return 0xff; }

    static uint8_t deleted() { return 0xfe; }
    
    uint8_t m_index;
};

struct RegHash {
    static unsigned hash(const Reg& key) { return key.hash(); }
    static bool equal(const Reg& a, const Reg& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::Reg> {
    typedef JSC::RegHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::Reg> : SimpleClassHashTraits<JSC::Reg> {
    static const bool emptyValueIsZero = false;
 };

} // namespace WTF

#endif // ENABLE(JIT)
