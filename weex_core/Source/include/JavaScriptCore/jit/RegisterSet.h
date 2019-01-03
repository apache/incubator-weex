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

#if ENABLE(JIT)

#include "FPRInfo.h"
#include "GPRInfo.h"
#include "MacroAssembler.h"
#include "Reg.h"
#include "TempRegisterSet.h"
#include <wtf/Bitmap.h>

namespace JSC {

class RegisterSet {
public:
    template<typename... Regs>
    explicit RegisterSet(Regs... regs)
    {
        setMany(regs...);
    }
    
    JS_EXPORT_PRIVATE static RegisterSet stackRegisters();
    JS_EXPORT_PRIVATE static RegisterSet reservedHardwareRegisters();
    static RegisterSet runtimeRegisters();
    static RegisterSet specialRegisters(); // The union of stack, reserved hardware, and runtime registers.
    JS_EXPORT_PRIVATE static RegisterSet calleeSaveRegisters();
    static RegisterSet vmCalleeSaveRegisters(); // Callee save registers that might be saved and used by any tier.
    static RegisterSet llintBaselineCalleeSaveRegisters(); // Registers saved and used by the LLInt.
    static RegisterSet dfgCalleeSaveRegisters(); // Registers saved and used by the DFG JIT.
    static RegisterSet ftlCalleeSaveRegisters(); // Registers that might be saved and used by the FTL JIT.
#if ENABLE(WEBASSEMBLY)
    static RegisterSet webAssemblyCalleeSaveRegisters(); // Registers saved and used by the WebAssembly JIT.
#endif
    static RegisterSet volatileRegistersForJSCall();
    static RegisterSet stubUnavailableRegisters(); // The union of callee saves and special registers.
    JS_EXPORT_PRIVATE static RegisterSet macroScratchRegisters();
    JS_EXPORT_PRIVATE static RegisterSet allGPRs();
    JS_EXPORT_PRIVATE static RegisterSet allFPRs();
    static RegisterSet allRegisters();
    static RegisterSet argumentGPRS();

    static RegisterSet registersToNotSaveForJSCall();
    static RegisterSet registersToNotSaveForCCall();
    
    void set(Reg reg, bool value = true)
    {
        ASSERT(!!reg);
        m_bits.set(reg.index(), value);
    }
    
    void set(JSValueRegs regs, bool value = true)
    {
        if (regs.tagGPR() != InvalidGPRReg)
            set(regs.tagGPR(), value);
        set(regs.payloadGPR(), value);
    }
    
    void clear(Reg reg)
    {
        ASSERT(!!reg);
        set(reg, false);
    }
    
    bool get(Reg reg) const
    {
        ASSERT(!!reg);
        return m_bits.get(reg.index());
    }
    
    template<typename Iterable>
    void setAll(const Iterable& iterable)
    {
        for (Reg reg : iterable)
            set(reg);
    }
    
    void merge(const RegisterSet& other) { m_bits.merge(other.m_bits); }
    void filter(const RegisterSet& other) { m_bits.filter(other.m_bits); }
    void exclude(const RegisterSet& other) { m_bits.exclude(other.m_bits); }
    
    size_t numberOfSetGPRs() const;
    size_t numberOfSetFPRs() const;
    size_t numberOfSetRegisters() const { return m_bits.count(); }
    
    bool isEmpty() const { return m_bits.isEmpty(); }
    
    JS_EXPORT_PRIVATE void dump(PrintStream&) const;
    
    enum EmptyValueTag { EmptyValue };
    enum DeletedValueTag { DeletedValue };
    
    RegisterSet(EmptyValueTag)
    {
        m_bits.set(hashSpecialBitIndex);
    }
    
    RegisterSet(DeletedValueTag)
    {
        m_bits.set(hashSpecialBitIndex);
        m_bits.set(deletedBitIndex);
    }
    
    bool isEmptyValue() const
    {
        return m_bits.get(hashSpecialBitIndex) && !m_bits.get(deletedBitIndex);
    }
    
    bool isDeletedValue() const
    {
        return m_bits.get(hashSpecialBitIndex) && m_bits.get(deletedBitIndex);
    }
    
    bool operator==(const RegisterSet& other) const { return m_bits == other.m_bits; }
    bool operator!=(const RegisterSet& other) const { return m_bits != other.m_bits; }
    
    unsigned hash() const { return m_bits.hash(); }
    
    template<typename Func>
    void forEach(const Func& func) const
    {
        m_bits.forEachSetBit(
            [&] (size_t index) {
                func(Reg::fromIndex(index));
            });
    }
    
private:
    void setAny(Reg reg) { set(reg); }
    void setAny(const RegisterSet& set) { merge(set); }
    void setMany() { }
    template<typename RegType, typename... Regs>
    void setMany(RegType reg, Regs... regs)
    {
        setAny(reg);
        setMany(regs...);
    }

    // These offsets mirror the logic in Reg.h.
    static const unsigned gprOffset = 0;
    static const unsigned fprOffset = gprOffset + MacroAssembler::numGPRs;
    static const unsigned hashSpecialBitIndex = fprOffset + MacroAssembler::numFPRs;
    static const unsigned deletedBitIndex = 0;
    
    Bitmap<MacroAssembler::numGPRs + MacroAssembler::numFPRs + 1> m_bits;
};

struct RegisterSetHash {
    static unsigned hash(const RegisterSet& set) { return set.hash(); }
    static bool equal(const RegisterSet& a, const RegisterSet& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = false;
};

} // namespace JSC

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::RegisterSet> {
    typedef JSC::RegisterSetHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::RegisterSet> : public CustomHashTraits<JSC::RegisterSet> { };

} // namespace WTF

#endif // ENABLE(JIT)
