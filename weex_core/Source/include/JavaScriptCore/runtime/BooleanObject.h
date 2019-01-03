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

namespace JSC {

class BooleanObject : public JSWrapperObject {
protected:
    JS_EXPORT_PRIVATE BooleanObject(VM&, Structure*);
    JS_EXPORT_PRIVATE void finishCreation(VM&);

public:
    typedef JSWrapperObject Base;

    static BooleanObject* create(VM& vm, Structure* structure)
    {
        BooleanObject* boolean = new (NotNull, allocateCell<BooleanObject>(vm.heap)) BooleanObject(vm, structure);
        boolean->finishCreation(vm);
        return boolean;
    }
        
    DECLARE_EXPORT_INFO;
        
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }
};

BooleanObject* asBooleanObject(JSValue);

inline BooleanObject* asBooleanObject(JSValue value)
{
    ASSERT(asObject(value)->inherits(*value.getObject()->vm(), BooleanObject::info()));
    return static_cast<BooleanObject*>(asObject(value));
}

} // namespace JSC
