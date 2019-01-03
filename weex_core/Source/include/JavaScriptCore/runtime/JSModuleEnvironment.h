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

#include "JSLexicalEnvironment.h"

namespace JSC {

class AbstractModuleRecord;
class Register;

class JSModuleEnvironment : public JSLexicalEnvironment {
    friend class JIT;
    friend class LLIntOffsetsExtractor;
public:
    typedef JSLexicalEnvironment Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | OverridesGetPropertyNames;

    static JSModuleEnvironment* create(VM&, Structure*, JSScope*, SymbolTable*, JSValue initialValue, AbstractModuleRecord*);

    static JSModuleEnvironment* create(VM& vm, JSGlobalObject* globalObject, JSScope* currentScope, SymbolTable* symbolTable, JSValue initialValue, AbstractModuleRecord* moduleRecord)
    {
        Structure* structure = globalObject->moduleEnvironmentStructure();
        return create(vm, structure, currentScope, symbolTable, initialValue, moduleRecord);
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject)
    {
        return Structure::create(vm, globalObject, jsNull(), TypeInfo(ModuleEnvironmentType, StructureFlags), info());
    }

    static size_t offsetOfModuleRecord(SymbolTable* symbolTable)
    {
        size_t offset = Base::allocationSize(symbolTable);
        ASSERT(WTF::roundUpToMultipleOf<sizeof(WriteBarrier<AbstractModuleRecord>)>(offset) == offset);
        return offset;
    }

    static size_t allocationSize(SymbolTable* symbolTable)
    {
        return offsetOfModuleRecord(symbolTable) + sizeof(WriteBarrier<AbstractModuleRecord>);
    }

    AbstractModuleRecord* moduleRecord()
    {
        return moduleRecordSlot().get();
    }

    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    static bool deleteProperty(JSCell*, ExecState*, PropertyName);

private:
    JSModuleEnvironment(VM&, Structure*, JSScope*, SymbolTable*);

    void finishCreation(VM&, JSValue initialValue, AbstractModuleRecord*);

    WriteBarrierBase<AbstractModuleRecord>& moduleRecordSlot()
    {
        return *bitwise_cast<WriteBarrierBase<AbstractModuleRecord>*>(bitwise_cast<char*>(this) + offsetOfModuleRecord(symbolTable()));
    }

    static void visitChildren(JSCell*, SlotVisitor&);
};

inline JSModuleEnvironment::JSModuleEnvironment(VM& vm, Structure* structure, JSScope* currentScope, SymbolTable* symbolTable)
    : Base(vm, structure, currentScope, symbolTable)
{
}

} // namespace JSC
