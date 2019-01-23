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

#include "CodeBlock.h"
#include "JSEnvironmentRecord.h"
#include "SymbolTable.h"

namespace JSC {

class JSLexicalEnvironment : public JSEnvironmentRecord {
protected:
    JSLexicalEnvironment(VM&, Structure*, JSScope*, SymbolTable*);
    
public:
    typedef JSEnvironmentRecord Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | OverridesGetPropertyNames | OverridesToThis;

    static JSLexicalEnvironment* create(
        VM& vm, Structure* structure, JSScope* currentScope, SymbolTable* symbolTable, JSValue initialValue)
    {
        JSLexicalEnvironment* result = 
            new (
                NotNull,
                allocateCell<JSLexicalEnvironment>(vm.heap, allocationSize(symbolTable)))
            JSLexicalEnvironment(vm, structure, currentScope, symbolTable);
        result->finishCreation(vm, initialValue);
        return result;
    }

    static JSLexicalEnvironment* create(VM& vm, JSGlobalObject* globalObject, JSScope* currentScope, SymbolTable* symbolTable, JSValue initialValue)
    {
        Structure* structure = globalObject->activationStructure();
        return create(vm, structure, currentScope, symbolTable, initialValue);
    }
        
    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);

    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);

    static bool deleteProperty(JSCell*, ExecState*, PropertyName);

    static JSValue toThis(JSCell*, ExecState*, ECMAMode);

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject) { return Structure::create(vm, globalObject, jsNull(), TypeInfo(LexicalEnvironmentType, StructureFlags), info()); }
};

inline JSLexicalEnvironment::JSLexicalEnvironment(VM& vm, Structure* structure, JSScope* currentScope, SymbolTable* symbolTable)
    : Base(vm, structure, currentScope, symbolTable)
{
}

inline JSLexicalEnvironment* asActivation(JSValue value)
{
    ASSERT(asObject(value)->inherits(*value.getObject()->vm(), JSLexicalEnvironment::info()));
    return jsCast<JSLexicalEnvironment*>(asObject(value));
}
    
} // namespace JSC
