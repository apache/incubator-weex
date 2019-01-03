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

#include "InternalFunction.h"

namespace JSC {

class WeakMapPrototype;
class GetterSetter;

class WeakMapConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;

    static WeakMapConstructor* create(VM& vm, Structure* structure, WeakMapPrototype* prototype, GetterSetter*)
    {
        WeakMapConstructor* constructor = new (NotNull, allocateCell<WeakMapConstructor>(vm.heap)) WeakMapConstructor(vm, structure);
        constructor->finishCreation(vm, prototype);
        return constructor;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

private:
    WeakMapConstructor(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }
    void finishCreation(VM&, WeakMapPrototype*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
};

} // namespace JSC
