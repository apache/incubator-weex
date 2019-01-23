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

#include "JSWrapperObject.h"
#include "Symbol.h"

namespace JSC {

class SymbolObject : public JSWrapperObject {
public:
    typedef JSWrapperObject Base;

    static SymbolObject* create(VM& vm, Structure* structure)
    {
        Symbol* symbol = Symbol::create(vm);
        SymbolObject* object = new (NotNull, allocateCell<SymbolObject>(vm.heap)) SymbolObject(vm, structure);
        object->finishCreation(vm, symbol);
        return object;
    }
    static SymbolObject* create(VM& vm, Structure* structure, Symbol* symbol)
    {
        SymbolObject* object = new (NotNull, allocateCell<SymbolObject>(vm.heap)) SymbolObject(vm, structure);
        object->finishCreation(vm, symbol);
        return object;
    }
    static SymbolObject* create(VM&, JSGlobalObject*, Symbol*);

    DECLARE_EXPORT_INFO;

    Symbol* internalValue() const { return asSymbol(JSWrapperObject::internalValue()); }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static JSValue defaultValue(const JSObject*, ExecState*, PreferredPrimitiveType);

    static String toStringName(const JSObject*, ExecState*);

protected:
    JS_EXPORT_PRIVATE void finishCreation(VM&, Symbol*);
    JS_EXPORT_PRIVATE SymbolObject(VM&, Structure*);
};

} // namespace JSC
