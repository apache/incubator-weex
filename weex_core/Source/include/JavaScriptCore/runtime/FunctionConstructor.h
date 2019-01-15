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

namespace WTF {
class TextPosition;
}

namespace JSC {

class FunctionPrototype;

class FunctionConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;

    static FunctionConstructor* create(VM& vm, Structure* structure, FunctionPrototype* functionPrototype)
    {
        FunctionConstructor* constructor = new (NotNull, allocateCell<FunctionConstructor>(vm.heap)) FunctionConstructor(vm, structure);
        constructor->finishCreation(vm, functionPrototype);
        return constructor;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype) 
    { 
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info()); 
    }

private:
    FunctionConstructor(VM&, Structure*);
    void finishCreation(VM&, FunctionPrototype*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
};

enum class FunctionConstructionMode {
    Function,
    Generator,
    Async,
};

JSObject* constructFunction(ExecState*, JSGlobalObject*, const ArgList&, const Identifier& functionName, const SourceOrigin&, const String& sourceURL, const WTF::TextPosition&, FunctionConstructionMode = FunctionConstructionMode::Function, JSValue newTarget = JSValue());
JSObject* constructFunction(ExecState*, JSGlobalObject*, const ArgList&, FunctionConstructionMode = FunctionConstructionMode::Function, JSValue newTarget = JSValue());

JS_EXPORT_PRIVATE JSObject* constructFunctionSkippingEvalEnabledCheck(
    ExecState*, JSGlobalObject*, const ArgList&, const Identifier&, const SourceOrigin&,
    const String&, const WTF::TextPosition&, int overrideLineNumber = -1,
    FunctionConstructionMode = FunctionConstructionMode::Function, JSValue newTarget = JSValue());

} // namespace JSC
