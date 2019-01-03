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

namespace JSC {

class JSInternalPromise;
class JSModuleNamespaceObject;
class SourceCode;

class JSModuleLoader : public JSNonFinalObject {
private:
    JSModuleLoader(VM&, Structure*);
public:
    typedef JSNonFinalObject Base;

    enum Status {
        Fetch = 1,
        Instantiate,
        Satisfy,
        Link,
        Ready,
    };

    static JSModuleLoader* create(ExecState* exec, VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        JSModuleLoader* object = new (NotNull, allocateCell<JSModuleLoader>(vm.heap)) JSModuleLoader(vm, structure);
        object->finishCreation(exec, vm, globalObject);
        return object;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    // APIs to control the module loader.
    JSValue provide(ExecState*, JSValue key, Status, const SourceCode&);
    JSInternalPromise* loadAndEvaluateModule(ExecState*, JSValue moduleName, JSValue referrer, JSValue scriptFetcher);
    JSInternalPromise* loadModule(ExecState*, JSValue moduleName, JSValue referrer, JSValue scriptFetcher);
    JSValue linkAndEvaluateModule(ExecState*, JSValue moduleKey, JSValue scriptFetcher);
    JSInternalPromise* requestImportModule(ExecState*, const Identifier&, JSValue scriptFetcher);

    // Platform dependent hooked APIs.
    JSInternalPromise* importModule(ExecState*, JSString* moduleName, const SourceOrigin& referrer);
    JSInternalPromise* resolve(ExecState*, JSValue name, JSValue referrer, JSValue scriptFetcher);
    JSInternalPromise* fetch(ExecState*, JSValue key, JSValue scriptFetcher);
    JSInternalPromise* instantiate(ExecState*, JSValue key, JSValue source, JSValue scriptFetcher);

    // Additional platform dependent hooked APIs.
    JSValue evaluate(ExecState*, JSValue key, JSValue moduleRecord, JSValue scriptFetcher);

    // Utility functions.
    JSModuleNamespaceObject* getModuleNamespaceObject(ExecState*, JSValue moduleRecord);

protected:
    void finishCreation(ExecState*, VM&, JSGlobalObject*);
};

} // namespace JSC
