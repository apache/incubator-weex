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

#include "JSObject.h"
#include "JSSymbolTableObject.h"
#include "SymbolTable.h"

namespace JSC {

class LLIntOffsetsExtractor;

class JSEnvironmentRecord : public JSSymbolTableObject {
    friend class JIT;
    friend class LLIntOffsetsExtractor;

public:
    typedef JSSymbolTableObject Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    WriteBarrierBase<Unknown>* variables()
    {
        return bitwise_cast<WriteBarrierBase<Unknown>*>(bitwise_cast<char*>(this) + offsetOfVariables());
    }
    
    bool isValidScopeOffset(ScopeOffset offset)
    {
        return !!offset && offset.offset() < symbolTable()->scopeSize();
    }
    
    WriteBarrierBase<Unknown>& variableAt(ScopeOffset offset)
    {
        ASSERT(isValidScopeOffset(offset));
        return variables()[offset.offset()];
    }

    static size_t offsetOfVariables()
    {
        return WTF::roundUpToMultipleOf<sizeof(WriteBarrier<Unknown>)>(sizeof(JSEnvironmentRecord));
    }
    
    static ptrdiff_t offsetOfVariable(ScopeOffset offset)
    {
        return offsetOfVariables() + offset.offset() * sizeof(WriteBarrier<Unknown>);
    }

    DECLARE_INFO;

    static size_t allocationSizeForScopeSize(unsigned scopeSize)
    {
        return offsetOfVariables() + scopeSize * sizeof(WriteBarrier<Unknown>);
    }
    
    static size_t allocationSize(SymbolTable* symbolTable)
    {
        return allocationSizeForScopeSize(symbolTable->scopeSize());
    }
    
protected:
    JSEnvironmentRecord(
        VM& vm,
        Structure* structure,
        JSScope* scope,
        SymbolTable* symbolTable)
        : Base(vm, structure, scope, symbolTable)
    {
    }
    
    void finishCreationUninitialized(VM& vm)
    {
        Base::finishCreation(vm);
    }
    
    void finishCreation(VM& vm, JSValue value)
    {
        finishCreationUninitialized(vm);
        ASSERT(value == jsUndefined() || value == jsTDZValue());
        for (unsigned i = symbolTable()->scopeSize(); i--;) {
            // Filling this with undefined/TDZEmptyValue is useful because that's what variables start out as.
            variableAt(ScopeOffset(i)).setStartingValue(value);
        }
    }

    static void visitChildren(JSCell*, SlotVisitor&);
    static void heapSnapshot(JSCell*, HeapSnapshotBuilder&);
};

} // namespace JSC
