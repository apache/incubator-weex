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

#include "JSArray.h"
#include "Lookup.h"

namespace JSC {

class ArrayPrototypeAdaptiveInferredPropertyWatchpoint;

class ArrayPrototype : public JSArray {
private:
    ArrayPrototype(VM&, Structure*);

public:
    typedef JSArray Base;

    enum class SpeciesWatchpointStatus {
        Uninitialized,
        Initialized,
        Fired
    };

    static ArrayPrototype* create(VM&, JSGlobalObject*, Structure*);
        
    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(DerivedArrayType, StructureFlags), info(), ArrayClass);
    }

    void tryInitializeSpeciesWatchpoint(ExecState*);

    static const bool needsDestruction = false;
    // We don't need destruction since we use a finalizer.
    static void destroy(JSC::JSCell*);

protected:
    void finishCreation(VM&, JSGlobalObject*);

private:
    // This bit is set if any user modifies the constructor property Array.prototype. This is used to optimize species creation for JSArrays.
    friend ArrayPrototypeAdaptiveInferredPropertyWatchpoint;
    std::unique_ptr<ArrayPrototypeAdaptiveInferredPropertyWatchpoint> m_constructorWatchpoint;
    std::unique_ptr<ArrayPrototypeAdaptiveInferredPropertyWatchpoint> m_constructorSpeciesWatchpoint;
};

EncodedJSValue JSC_HOST_CALL arrayProtoFuncToString(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoFuncValues(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoPrivateFuncConcatMemcpy(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayProtoPrivateFuncAppendMemcpy(ExecState*);

} // namespace JSC
