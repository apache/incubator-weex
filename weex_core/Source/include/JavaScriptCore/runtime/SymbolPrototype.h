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

#include "Symbol.h"
#include "SymbolObject.h"

namespace JSC {

// In the ES6 spec, Symbol.prototype object is an ordinary JS object, not one of the symbol wrapper object instance.
class SymbolPrototype : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | HasStaticPropertyTable;

    static SymbolPrototype* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        SymbolPrototype* prototype = new (NotNull, allocateCell<SymbolPrototype>(vm.heap)) SymbolPrototype(vm, structure);
        prototype->finishCreation(vm, globalObject);
        return prototype;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    SymbolPrototype(VM&, Structure*);
    void finishCreation(VM&, JSGlobalObject*);
};
STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(SymbolPrototype);

} // namespace JSC
