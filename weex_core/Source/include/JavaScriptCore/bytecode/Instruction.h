/*
 * Copyright (C) 2008, 2012-2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
