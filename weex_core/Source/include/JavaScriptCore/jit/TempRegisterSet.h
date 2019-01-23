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

namespace JSC {

class RegisterSet;

class TempRegisterSet {
public:
    TempRegisterSet()
    {
        clearAll();
    }
    
    TempRegisterSet(const RegisterSet&);
    
    void set(GPRReg reg)
    {
        setBit(GPRInfo::toIndex(reg));
    }
    
    void set(JSValueRegs regs)
    {
        if (regs.tagGPR() != InvalidGPRReg)
            set(regs.tagGPR());
        set(regs.payloadGPR());
    }
    
    void setGPRByIndex(unsigned index)
    {
        ASSERT(index < GPRInfo::numberOfRegisters);
        setBit(index);
    }
    
    void clear(GPRReg reg)
    {
        clearBit(GPRInfo::toIndex(reg));
    }
    
    bool get(GPRReg reg) const
    {
        return getBit(GPRInfo::toIndex(reg));
    }
    
    bool getGPRByIndex(unsigned index) const
    {
        ASSERT(index < GPRInfo::numberOfRegisters);
        return getBit(index);
    }
    
    // Return the index'th free GPR.
    GPRReg getFreeGPR(unsigned index = 0) const
    {
        for (unsigned i = GPRInfo::numberOfRegisters; i--;) {
            if (!getGPRByIndex(i) && !index--)
                return GPRInfo::toRegister(i);
        }
        return InvalidGPRReg;
    }
    
    void set(FPRReg reg)
    {
        setBit(GPRInfo::numberOfRegisters + FPRInfo::toIndex(reg));
    }
    
    void setFPRByIndex(unsigned index)
    {
        ASSERT(index < FPRInfo::numberOfRegisters);
        setBit(GPRInfo::numberOfRegisters + index);
    }
    
    void clear(FPRReg reg)
    {
        clearBit(GPRInfo::numberOfRegisters + FPRInfo::toIndex(reg));
    }
    
    bool get(FPRReg reg) const
    {
        return getBit(GPRInfo::numberOfRegisters + FPRInfo::toIndex(reg));
    }
    
    bool getFPRByIndex(unsigned index) const
    {
        ASSERT(index < FPRInfo::numberOfRegisters);
        return getBit(GPRInfo::numberOfRegisters + index);
    }
    
    // Return the index'th free FPR.
    FPRReg getFreeFPR(unsigned index = 0) const
    {
        for (unsigned i = FPRInfo::numberOfRegisters; i--;) {
            if (!getFPRByIndex(i) && !index--)
                return FPRInfo::toRegister(i);
        }
        return InvalidFPRReg;
    }

    template<typename BankInfo>
    void setByIndex(unsigned index)
    {
        set(BankInfo::toRegister(index));
    }
    
    template<typename BankInfo>
    bool getByIndex(unsigned index)
    {
        return get(BankInfo::toRegister(index));
    }
    
    unsigned numberOfSetGPRs() const
    {
        unsigned result = 0;
        for (unsigned i = GPRInfo::numberOfRegisters; i--;) {
            if (!getBit(i))
                continue;
            result++;
        }
        return result;
    }
    
    unsigned numberOfSetFPRs() const
    {
        unsigned result = 0;
        for (unsigned i = FPRInfo::numberOfRegisters; i--;) {
            if (!getBit(GPRInfo::numberOfRegisters + i))
                continue;
            result++;
        }
        return result;
    }
    
    unsigned numberOfSetRegisters() const
    {
        unsigned result = 0;
        for (unsigned i = totalNumberOfRegisters; i--;) {
            if (!getBit(i))
                continue;
            result++;
        }
        return result;
    }
    
private:
    void clearAll()
    {
        for (unsigned i = numberOfBytesInTempRegisterSet; i--;)
            m_set[i] = 0;
    }

    void setBit(unsigned i)
    {
        ASSERT(i < totalNumberOfRegisters);
        m_set[i >> 3] |= (1 << (i & 7));
    }
    
    void clearBit(unsigned i)
    {
        ASSERT(i < totalNumberOfRegisters);
        m_set[i >> 3] &= ~(1 << (i & 7));
    }
    
    bool getBit(unsigned i) const
    {
        ASSERT(i < totalNumberOfRegisters);
        return !!(m_set[i >> 3] & (1 << (i & 7)));
    }
    
    static const unsigned totalNumberOfRegisters =
        GPRInfo::numberOfRegisters + FPRInfo::numberOfRegisters;
    
    static const unsigned numberOfBytesInTempRegisterSet =
        (totalNumberOfRegisters + 7) >> 3;

    uint8_t m_set[numberOfBytesInTempRegisterSet];
};

} // namespace JSC

#else // ENABLE(JIT) -> so if JIT is disabled

namespace JSC {

// Define TempRegisterSet to something, to make it easier to refer to this type in code that
// make be compiled when the JIT is disabled.

struct TempRegisterSet { };

} // namespace JSC

#endif // ENABLE(JIT)
