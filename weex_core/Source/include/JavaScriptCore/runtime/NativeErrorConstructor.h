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
#include "NativeErrorPrototype.h"

namespace JSC {

class ErrorInstance;
class FunctionPrototype;
class NativeErrorPrototype;

class NativeErrorConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;

    static NativeErrorConstructor* create(VM& vm, JSGlobalObject* globalObject, Structure* structure, Structure* prototypeStructure, const String& name)
    {
        NativeErrorConstructor* constructor = new (NotNull, allocateCell<NativeErrorConstructor>(vm.heap)) NativeErrorConstructor(vm, structure);
        constructor->finishCreation(vm, globalObject, prototypeStructure, name);
        return constructor;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    Structure* errorStructure() { return m_errorStructure.get(); }

protected:
    void finishCreation(VM&, JSGlobalObject*, Structure* prototypeStructure, const String& name);

private:
    NativeErrorConstructor(VM&, Structure*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
    static void visitChildren(JSCell*, SlotVisitor&);

    WriteBarrier<Structure> m_errorStructure;
};

} // namespace JSC
