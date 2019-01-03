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

#include "MacroAssembler.h"
#include "RegisterSet.h"
#include "TempRegisterSet.h"

namespace JSC {

struct ScratchBuffer;

// This class provides a low-level register allocator for use in stubs.

class ScratchRegisterAllocator {
public:
    ScratchRegisterAllocator() { }
    ScratchRegisterAllocator(const RegisterSet& usedRegisters);
    ~ScratchRegisterAllocator();

    void lock(GPRReg);
    void lock(FPRReg);
    void lock(JSValueRegs);
    
    template<typename BankInfo>
    typename BankInfo::RegisterType allocateScratch();
    
    GPRReg allocateScratchGPR();
    FPRReg allocateScratchFPR();
    
    bool didReuseRegisters() const
    {
        return !!m_numberOfReusedRegisters;
    }
    
    unsigned numberOfReusedRegisters() const
    {
        return m_numberOfReusedRegisters;
    }

    RegisterSet usedRegisters() const { return m_usedRegisters; }
    
    enum class ExtraStackSpace { SpaceForCCall, NoExtraSpace };

    struct PreservedState {
        PreservedState()
            : numberOfBytesPreserved(std::numeric_limits<unsigned>::max())
            , extraStackSpaceRequirement(ExtraStackSpace::SpaceForCCall)
        { }

        PreservedState(unsigned numberOfBytes, ExtraStackSpace extraStackSpace)
            : numberOfBytesPreserved(numberOfBytes)
            , extraStackSpaceRequirement(extraStackSpace)
        { }

        explicit operator bool() const { return numberOfBytesPreserved != std::numeric_limits<unsigned>::max(); }

        unsigned numberOfBytesPreserved;
        ExtraStackSpace extraStackSpaceRequirement;
    };

    PreservedState preserveReusedRegistersByPushing(MacroAssembler& jit, ExtraStackSpace);
    void restoreReusedRegistersByPopping(MacroAssembler& jit, const PreservedState&);
    
    RegisterSet usedRegistersForCall() const;
    
    unsigned desiredScratchBufferSizeForCall() const;
    
    void preserveUsedRegistersToScratchBufferForCall(MacroAssembler& jit, ScratchBuffer* scratchBuffer, GPRReg scratchGPR = InvalidGPRReg);
    void restoreUsedRegistersFromScratchBufferForCall(MacroAssembler& jit, ScratchBuffer* scratchBuffer, GPRReg scratchGPR = InvalidGPRReg);

    static unsigned preserveRegistersToStackForCall(MacroAssembler& jit, const RegisterSet& usedRegisters, unsigned extraPaddingInBytes);
    static void restoreRegistersFromStackForCall(MacroAssembler& jit, const RegisterSet& usedRegisters, const RegisterSet& ignore, unsigned numberOfStackBytesUsedForRegisterPreservation, unsigned extraPaddingInBytes);

private:
    RegisterSet m_usedRegisters;
    TempRegisterSet m_lockedRegisters;
    TempRegisterSet m_scratchRegisters;
    unsigned m_numberOfReusedRegisters;
};

} // namespace JSC

#endif // ENABLE(JIT)
