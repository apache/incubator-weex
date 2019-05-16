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
//
// Created by Darin on 11/02/2018.
//

#ifndef WEEXV8_WEEXGLOBALOBJECT_H
#define WEEXV8_WEEXGLOBALOBJECT_H

#include <map>
#include "android/jsengine/weex_ipc_server.h"
#include "android/jsengine/weex_jsc_utils.h"

#include "include/WeexApiHeader.h"
#include "APICast.h"
#include "JSStringRef.h"
#include "JSContextRef.h"

using namespace JSC;

class TimerQueue;

namespace weex {
    class GlobalObjectDelegate;
}
namespace WeexCore {
    class ScriptBridge;
}

class WeexGlobalObject: public JSGlobalObject {
private:
    WeexGlobalObject(VM&, Structure*);
    WeexCore::ScriptBridge *script_bridge_;

public:
    typedef JSGlobalObject Base;
    std::vector<INIT_FRAMEWORK_PARAMS *> m_initFrameworkParams;
    std::string id = "";
    TimerQueue* timeQueue = nullptr;
    static WeexGlobalObject* create(VM& vm, Structure* structure)
    {
        WeexGlobalObject* object = new (NotNull, allocateCell<WeexGlobalObject>(vm.heap)) WeexGlobalObject(vm, structure);
        //object->finishCreation(vm);
        return object;
    }

    static const bool needsDestruction = false;

    DECLARE_INFO;
    static const GlobalObjectMethodTable s_globalObjectMethodTable;

    static Structure* createStructure(VM& vm, JSValue prototype)
    {
        return Structure::create(vm, 0, prototype, TypeInfo(GlobalObjectType, StructureFlags), info());
    }

    static RuntimeFlags javaScriptRuntimeFlags(const JSGlobalObject*) { return RuntimeFlags::createAllEnabled(); }

//    void initWXEnvironment(IPCArguments* arguments);

    void addExtraOptions(std::vector<INIT_FRAMEWORK_PARAMS *> &params);
    void initWxEnvironment(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool forAppContext, bool isSave);
    void initFunctionForContext();
    void initFunctionForAppContext();
    void initFunction();

    inline WeexCore::ScriptBridge* js_bridge() { return script_bridge_; }
    void SetScriptBridge(WeexCore::ScriptBridge *script_bridge);

    // store js timer function
    void addTimer(uint32_t function_id, JSC::Strong<JSC::Unknown>&& function);
    void removeTimer(uint32_t function_id);
    uint32_t genFunctionID();
    JSValue getTimerFunction(uint32_t function_id);

    void updateInitFrameworkParams(const std::string& key, const std::string& value);



protected:
    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        //addFunction(vm, "gc", functionGCAndSweep, 0);
    }

    void addFunction(VM& vm, JSObject* object, const char* name, NativeFunction function, unsigned arguments)
    {
        Identifier identifier = Identifier::fromString(&vm, name);
        object->putDirect(vm, identifier, JSFunction::create(vm, this, arguments, identifier.string(), function));
    }

    void addFunction(VM& vm, const char* name, NativeFunction function, unsigned arguments)
    {
        addFunction(vm, this, name, function, arguments);
    }

    void addConstructableFunction(VM& vm, const char* name, NativeFunction function, unsigned arguments)
    {
        Identifier identifier = Identifier::fromString(&vm, name);
        putDirect(vm, identifier, JSFunction::create(vm, this, arguments, identifier.string(), function, NoIntrinsic, function));
    }

    void addString(VM& vm, JSObject* object, const char* name, String&& value)
    {
        Identifier identifier = Identifier::fromString(&vm, name);
        JSString* jsString = jsNontrivialString(&vm, WTFMove(value));
        object->putDirect(vm, identifier, jsString);
    }

    void addValue(VM& vm, JSObject* object, const char* name, JSValue value)
    {
        Identifier identifier = Identifier::fromString(&vm, name);
        object->putDirect(vm, identifier, value);
    }

    void addValue(VM& vm, const char* name, JSValue value)
    {
        addValue(vm, this, name, value);
    }


    // use map to store timer js function, avoid the object be gc
    std::map<uint32_t, JSC::Strong<JSC::Unknown>> function_maps_;
    typedef std::map<uint32_t, JSC::Strong<JSC::Unknown>>::iterator MapIterator;

    uint32_t function_id_;
};


#endif //WEEXV8_WEEXGLOBALOBJECT_H
