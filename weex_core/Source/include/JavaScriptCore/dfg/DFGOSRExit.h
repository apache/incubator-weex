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

#if ENABLE(DFG_JIT)

#include "CodeOrigin.h"
#include "DFGCommon.h"
#include "DFGExitProfile.h"
#include "DFGOSRExitBase.h"
#include "GPRInfo.h"
#include "MacroAssembler.h"
#include "MethodOfGettingAValueProfile.h"
#include "Operands.h"
#include "ValueProfile.h"
#include "ValueRecovery.h"

namespace JSC { namespace DFG {

class SpeculativeJIT;
struct BasicBlock;
struct Node;

// This enum describes the types of additional recovery that
// may need be performed should a speculation check fail.
enum SpeculationRecoveryType : uint8_t {
    SpeculativeAdd,
    SpeculativeAddImmediate,
    BooleanSpeculationCheck
};

// === SpeculationRecovery ===
//
// This class provides additional information that may be associated with a
// speculation check - for example 
class SpeculationRecovery {
public:
    SpeculationRecovery(SpeculationRecoveryType type, GPRReg dest, GPRReg src)
        : m_src(src)
        , m_dest(dest)
        , m_type(type)
    {
        ASSERT(m_type == SpeculativeAdd || m_type == BooleanSpeculationCheck);
    }

    SpeculationRecovery(SpeculationRecoveryType type, GPRReg dest, int32_t immediate)
        : m_immediate(immediate)
        , m_dest(dest)
        , m_type(type)
    {
        ASSERT(m_type == SpeculativeAddImmediate);
    }

    SpeculationRecoveryType type() { return m_type; }
    GPRReg dest() { return m_dest; }
    GPRReg src() { return m_src; }
    int32_t immediate() { return m_immediate; }

private:
    // different recovery types may required different additional information here.
    union {
        GPRReg m_src;
        int32_t m_immediate;
    };
    GPRReg m_dest;

    // Indicates the type of additional recovery to be performed.
    SpeculationRecoveryType m_type;
};

// === OSRExit ===
//
// This structure describes how to exit the speculative path by
// going into baseline code.
struct OSRExit : public OSRExitBase {
    OSRExit(ExitKind, JSValueSource, MethodOfGettingAValueProfile, SpeculativeJIT*, unsigned streamIndex, unsigned recoveryIndex = UINT_MAX);

    unsigned m_patchableCodeOffset { 0 };
    
    MacroAssemblerCodeRef m_code;
    
    JSValueSource m_jsValueSource;
    MethodOfGettingAValueProfile m_valueProfile;
    
    unsigned m_recoveryIndex;

    void setPatchableCodeOffset(MacroAssembler::PatchableJump);
    MacroAssembler::Jump getPatchableCodeOffsetAsJump() const;
    CodeLocationJump codeLocationForRepatch(CodeBlock*) const;
    void correctJump(LinkBuffer&);

    unsigned m_streamIndex;
    void considerAddingAsFrequentExitSite(CodeBlock* profiledCodeBlock)
    {
        OSRExitBase::considerAddingAsFrequentExitSite(profiledCodeBlock, ExitFromDFG);
    }
};

struct SpeculationFailureDebugInfo {
    CodeBlock* codeBlock;
    ExitKind kind;
    unsigned bytecodeOffset;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
