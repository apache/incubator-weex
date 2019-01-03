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

#include "BasicBlockLocation.h"
#include "MacroAssembler.h"
#include "PutByIdFlags.h"
#include "SymbolTable.h"
#include "TypeLocation.h"
#include "PropertySlot.h"
#include "SpecialPointer.h"
#include "Structure.h"
#include "StructureChain.h"
#include "ToThisStatus.h"
#include "VirtualRegister.h"
#include <wtf/VectorTraits.h>

namespace JSC {

class ArrayAllocationProfile;
class ArrayProfile;
class ObjectAllocationProfile;
class WatchpointSet;
struct LLIntCallLinkInfo;
struct ValueProfile;

#if ENABLE(COMPUTED_GOTO_OPCODES)
typedef void* Opcode;
#else
typedef OpcodeID Opcode;
#endif

struct Instruction {
    Instruction()
    {
        u.jsCell.clear();
    }
        
    Instruction(Opcode opcode)
    {
#if !ENABLE(COMPUTED_GOTO_OPCODES)
        // We have to initialize one of the pointer members to ensure that
        // the entire struct is initialized, when opcode is not a pointer.
        u.jsCell.clear();
#endif
        u.opcode = opcode;
    }

    Instruction(int operand)
    {
        // We have to initialize one of the pointer members to ensure that
        // the entire struct is initialized in 64-bit.
        u.jsCell.clear();
        u.operand = operand;
    }
    Instruction(unsigned unsignedValue)
    {
        // We have to initialize one of the pointer members to ensure that
        // the entire struct is initialized in 64-bit.
        u.jsCell.clear();
        u.unsignedValue = unsignedValue;
    }

    Instruction(PutByIdFlags flags)
    {
        u.putByIdFlags = flags;
    }

    Instruction(VM& vm, JSCell* owner, Structure* structure)
    {
        u.structure.clear();
        u.structure.set(vm, owner, structure);
    }
    Instruction(VM& vm, JSCell* owner, StructureChain* structureChain)
    {
        u.structureChain.clear();
        u.structureChain.set(vm, owner, structureChain);
    }
    Instruction(VM& vm, JSCell* owner, JSCell* jsCell)
    {
        u.jsCell.clear();
        u.jsCell.set(vm, owner, jsCell);
    }

    Instruction(PropertySlot::GetValueFunc getterFunc) { u.getterFunc = getterFunc; }
        
    Instruction(LLIntCallLinkInfo* callLinkInfo) { u.callLinkInfo = callLinkInfo; }
    Instruction(ValueProfile* profile) { u.profile = profile; }
    Instruction(ArrayProfile* profile) { u.arrayProfile = profile; }
    Instruction(ArrayAllocationProfile* profile) { u.arrayAllocationProfile = profile; }
    Instruction(ObjectAllocationProfile* profile) { u.objectAllocationProfile = profile; }
    Instruction(WriteBarrier<Unknown>* variablePointer) { u.variablePointer = variablePointer; }
    Instruction(Special::Pointer pointer) { u.specialPointer = pointer; }
    Instruction(UniquedStringImpl* uid) { u.uid = uid; }
    Instruction(bool* predicatePointer) { u.predicatePointer = predicatePointer; }

    union {
        Opcode opcode;
        int operand;
        unsigned unsignedValue;
        WriteBarrierBase<Structure> structure;
        StructureID structureID;
        WriteBarrierBase<SymbolTable> symbolTable;
        WriteBarrierBase<StructureChain> structureChain;
        WriteBarrierBase<JSCell> jsCell;
        WriteBarrier<Unknown>* variablePointer;
        Special::Pointer specialPointer;
        PropertySlot::GetValueFunc getterFunc;
        LLIntCallLinkInfo* callLinkInfo;
        UniquedStringImpl* uid;
        ValueProfile* profile;
        ArrayProfile* arrayProfile;
        ArrayAllocationProfile* arrayAllocationProfile;
        ObjectAllocationProfile* objectAllocationProfile;
        WatchpointSet* watchpointSet;
        void* pointer;
        bool* predicatePointer;
        ToThisStatus toThisStatus;
        TypeLocation* location;
        BasicBlockLocation* basicBlockLocation;
        PutByIdFlags putByIdFlags;
    } u;
        
private:
    Instruction(StructureChain*);
    Instruction(Structure*);
};

} // namespace JSC

namespace WTF {

template<> struct VectorTraits<JSC::Instruction> : VectorTraitsBase<true, JSC::Instruction> { };

} // namespace WTF
