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

#include "DFGCommon.h"
#include "DFGMinifiedID.h"
#include "DataFormat.h"
#include "MacroAssembler.h"
#include <stdio.h>

namespace JSC { namespace DFG {

enum VariableEventKind {
    // Marks the beginning of a checkpoint. If you interpret the variable
    // events starting at a Reset point then you'll get everything you need.
    Reset,
    
    // Node births. Points in the code where a node becomes relevant for OSR.
    // It may be the point where it is actually born (i.e. assigned) or it may
    // be a later point, if it's only later in the sequence of instructions
    // that we start to care about this node.
    BirthToFill,
    BirthToSpill,
    Birth,
    
    // Events related to how a node is represented.
    Fill,
    Spill,
    
    // Death of a node - after this we no longer care about this node.
    Death,
    
    // A MovHintEvent means that a node is being associated with a bytecode operand,
    // but that it has not been stored into that operand.
    MovHintEvent,
    
    // A SetLocalEvent means that a node's value has been stored into the stack.
    SetLocalEvent,
    
    // Used to indicate an uninitialized VariableEvent. Don't use for other
    // purposes.
    InvalidEventKind
};

union VariableRepresentation {
    MacroAssembler::RegisterID gpr;
    MacroAssembler::FPRegisterID fpr;
#if USE(JSVALUE32_64)
    struct {
        MacroAssembler::RegisterID tagGPR;
        MacroAssembler::RegisterID payloadGPR;
    } pair;
#endif
    int32_t virtualReg;
};

class VariableEvent {
public:
    VariableEvent()
        : m_kind(InvalidEventKind)
    {
    }
    
    static VariableEvent reset()
    {
        VariableEvent event;
        event.m_kind = Reset;
        return event;
    }
    
    static VariableEvent fillGPR(VariableEventKind kind, MinifiedID id, MacroAssembler::RegisterID gpr, DataFormat dataFormat)
    {
        ASSERT(kind == BirthToFill || kind == Fill);
        ASSERT(dataFormat != DataFormatDouble);
#if USE(JSVALUE32_64)
        ASSERT(!(dataFormat & DataFormatJS));
#endif
        VariableEvent event;
        event.m_which.id = id.bits();
        event.m_representation.gpr = gpr;
        event.m_kind = kind;
        event.m_dataFormat = dataFormat;
        return event;
    }
    
#if USE(JSVALUE32_64)
    static VariableEvent fillPair(VariableEventKind kind, MinifiedID id, MacroAssembler::RegisterID tagGPR, MacroAssembler::RegisterID payloadGPR)
    {
        ASSERT(kind == BirthToFill || kind == Fill);
        VariableEvent event;
        event.m_which.id = id.bits();
        event.m_representation.pair.tagGPR = tagGPR;
        event.m_representation.pair.payloadGPR = payloadGPR;
        event.m_kind = kind;
        event.m_dataFormat = DataFormatJS;
        return event;
    }
#endif // USE(JSVALUE32_64)
    
    static VariableEvent fillFPR(VariableEventKind kind, MinifiedID id, MacroAssembler::FPRegisterID fpr)
    {
        ASSERT(kind == BirthToFill || kind == Fill);
        VariableEvent event;
        event.m_which.id = id.bits();
        event.m_representation.fpr = fpr;
        event.m_kind = kind;
        event.m_dataFormat = DataFormatDouble;
        return event;
    }
    
    static VariableEvent birth(MinifiedID id)
    {
        VariableEvent event;
        event.m_which.id = id.bits();
        event.m_kind = Birth;
        return event;
    }
    
    static VariableEvent spill(VariableEventKind kind, MinifiedID id, VirtualRegister virtualRegister, DataFormat format)
    {
        ASSERT(kind == BirthToSpill || kind == Spill);
        VariableEvent event;
        event.m_which.id = id.bits();
        event.m_representation.virtualReg = virtualRegister.offset();
        event.m_kind = kind;
        event.m_dataFormat = format;
        return event;
    }
    
    static VariableEvent death(MinifiedID id)
    {
        VariableEvent event;
        event.m_which.id = id.bits();
        event.m_kind = Death;
        return event;
    }
    
    static VariableEvent setLocal(
        VirtualRegister bytecodeReg, VirtualRegister machineReg, DataFormat format)
    {
        VariableEvent event;
        event.m_which.virtualReg = machineReg.offset();
        event.m_representation.virtualReg = bytecodeReg.offset();
        event.m_kind = SetLocalEvent;
        event.m_dataFormat = format;
        return event;
    }
    
    static VariableEvent movHint(MinifiedID id, VirtualRegister bytecodeReg)
    {
        VariableEvent event;
        event.m_which.id = id.bits();
        event.m_representation.virtualReg = bytecodeReg.offset();
        event.m_kind = MovHintEvent;
        return event;
    }
    
    VariableEventKind kind() const
    {
        return static_cast<VariableEventKind>(m_kind);
    }
    
    MinifiedID id() const
    {
        ASSERT(
            m_kind == BirthToFill || m_kind == Fill || m_kind == BirthToSpill || m_kind == Spill
            || m_kind == Death || m_kind == MovHintEvent || m_kind == Birth);
        return MinifiedID::fromBits(m_which.id);
    }
    
    DataFormat dataFormat() const
    {
        ASSERT(
            m_kind == BirthToFill || m_kind == Fill || m_kind == BirthToSpill || m_kind == Spill
            || m_kind == SetLocalEvent);
        return static_cast<DataFormat>(m_dataFormat);
    }
    
    MacroAssembler::RegisterID gpr() const
    {
        ASSERT(m_kind == BirthToFill || m_kind == Fill);
        ASSERT(m_dataFormat);
        ASSERT(m_dataFormat != DataFormatDouble);
#if USE(JSVALUE32_64)
        ASSERT(!(m_dataFormat & DataFormatJS));
#endif
        return m_representation.gpr;
    }
    
#if USE(JSVALUE32_64)
    MacroAssembler::RegisterID tagGPR() const
    {
        ASSERT(m_kind == BirthToFill || m_kind == Fill);
        ASSERT(m_dataFormat & DataFormatJS);
        return m_representation.pair.tagGPR;
    }
    MacroAssembler::RegisterID payloadGPR() const
    {
        ASSERT(m_kind == BirthToFill || m_kind == Fill);
        ASSERT(m_dataFormat & DataFormatJS);
        return m_representation.pair.payloadGPR;
    }
#endif // USE(JSVALUE32_64)
    
    MacroAssembler::FPRegisterID fpr() const
    {
        ASSERT(m_kind == BirthToFill || m_kind == Fill);
        ASSERT(m_dataFormat == DataFormatDouble);
        return m_representation.fpr;
    }
    
    VirtualRegister spillRegister() const
    {
        ASSERT(m_kind == BirthToSpill || m_kind == Spill);
        return VirtualRegister(m_representation.virtualReg);
    }
    
    VirtualRegister bytecodeRegister() const
    {
        ASSERT(m_kind == SetLocalEvent || m_kind == MovHintEvent);
        return VirtualRegister(m_representation.virtualReg);
    }
    
    VirtualRegister machineRegister() const
    {
        ASSERT(m_kind == SetLocalEvent);
        return VirtualRegister(m_which.virtualReg);
    }
    
    const VariableRepresentation& variableRepresentation() const { return m_representation; }
    
    void dump(PrintStream&) const;
    
private:
    void dumpFillInfo(const char* name, PrintStream&) const;
    void dumpSpillInfo(const char* name, PrintStream&) const;
    
    union {
        int virtualReg;
        uintptr_t id;
    } m_which;
    
    // For BirthToFill, Fill:
    //   - The GPR or FPR, or a GPR pair.
    // For BirthToSpill, Spill:
    //   - The virtual register.
    // For MovHintEvent, SetLocalEvent:
    //   - The bytecode operand.
    // For Death:
    //   - Unused.
    VariableRepresentation m_representation;
    
    int8_t m_kind;
    int8_t m_dataFormat;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
