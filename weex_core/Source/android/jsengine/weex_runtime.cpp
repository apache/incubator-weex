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
// Created by Darin on 28/04/2018.
//

#include "android/jsengine/weex_runtime.h"

#include "android/jsengine/bridge/script/script_bridge_in_multi_so.h"
#include "android/jsengine/bridge/script/core_side_in_multi_process.h"
#include "android/jsengine/object/weex_env.h"
#include "include/JavaScriptCore/runtime/Exception.h"
#include "include/JavaScriptCore/heap/HeapSnapshotBuilder.h"

using namespace JSC;
using namespace WTF;
using namespace WEEXICU;


WeexRuntime::WeexRuntime(TimerQueue* timeQueue,bool isMultiProgress) : is_multi_process_(isMultiProgress), script_bridge_(nullptr) {
    WeexEnv::getEnv()->initJSC(isMultiProgress);
    m_globalVM = std::move(VM::create(LargeHeap));

    static bool wson_init_flag = false;
    if(!wson_init_flag) {
      wson_init_flag = true;
      wson::init(m_globalVM.get());
    }

    WeexEnv::getEnv()->jsc_init_finished();
    WeexEnv::getEnv()->locker()->signal();

    weexObjectHolder.reset(new WeexObjectHolder(m_globalVM.get(), timeQueue, isMultiProgress));
    LOGE("WeexRuntime is running and mode is %s", isMultiProgress ? "multiProcess" : "singleProcess");
}

WeexRuntime::WeexRuntime(TimerQueue* timeQueue, WeexCore::ScriptBridge *script_bridge, bool isMultiProgress) : WeexRuntime(timeQueue, isMultiProgress) {
    this->script_bridge_ = script_bridge;
}


int WeexRuntime::initFramework(IPCArguments *arguments) {
    base::debug::TraceEvent::StartATrace(nullptr);

    base::debug::TraceScope traceScope("weex", "initFramework");
    weexObjectHolder->initFromIPCArguments(arguments, 1, false);
    const IPCString *ipcSource = arguments->getString(0);
    const String &source = jString2String(ipcSource->content, ipcSource->length);

    return _initFramework(source);
}

int WeexRuntime::initFramework(const String &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    base::debug::TraceEvent::StartATrace(nullptr);
    base::debug::TraceScope traceScope("weex", "initFramework");
    weexObjectHolder->initFromParams(params, false);
    return _initFramework(script);
}

int WeexRuntime::initAppFrameworkMultiProcess(const String &instanceId, const String &appFramework,
                                              IPCArguments *arguments) {
    auto pHolder = getLightAppObjectHolder(instanceId);
    if (pHolder == nullptr) {
        auto holder = new WeexObjectHolder(this->m_globalVM.get(), weexObjectHolder->timeQueue, true);
        holder->initFromIPCArguments(arguments, 2, true);
        appWorkerContextHolderMap[instanceId.utf8().data()] = holder;
    }

    return _initAppFramework(instanceId, appFramework);
}

int WeexRuntime::initAppFramework(const String &instanceId, const String &appFramework,
                                  std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    // Create Worker Context
    auto pHolder = getLightAppObjectHolder(instanceId);
    LOGE("Weex jsserver initAppFramework %s",instanceId.utf8().data());
    if (pHolder == nullptr) {
        auto holder = new WeexObjectHolder(this->m_globalVM.get(), weexObjectHolder->timeQueue, is_multi_process_);
        holder->initFromParams(params, true);
        LOGE("Weex jsserver initAppFramework pHolder == null and id %s",instanceId.utf8().data());
        appWorkerContextHolderMap[instanceId.utf8().data()] = holder;
    }
    return _initAppFramework(instanceId, appFramework);
}

int WeexRuntime::_initAppFramework(const String &instanceId, const String &appFramework) {
  JSLockHolder locker_global(this->m_globalVM.get());

  auto appWorkerObjectHolder = getLightAppObjectHolder(instanceId);
  if (appWorkerObjectHolder == nullptr) {
    return static_cast<int32_t>(false);
  }
  WeexGlobalObject *worker_globalObject = appWorkerObjectHolder->m_globalObject.get();
  worker_globalObject->SetScriptBridge(script_bridge_);

  worker_globalObject->id = instanceId.utf8().data();
  return static_cast<int32_t>(
      ExecuteJavaScript(worker_globalObject,
                        appFramework,
                        "(app framework)",
                        true,
                        "initAppFramework",
                        instanceId.utf8().data()));
}

int WeexRuntime::createAppContext(const String &instanceId, const String &jsBundle) {
    if (instanceId == "") {
        return static_cast<int32_t>(false);
    } else {
        String pre = "";
        if (instanceId.length() > 6) {
            pre = instanceId.substring(0, 7);
        }

        String get_context_fun_name = "";
        String final_instanceId = "";
        if (pre == "plugin_") {
            LOGE("createAppContext __get_plugin_context__");
            get_context_fun_name = "__get_plugin_context__";
            final_instanceId = instanceId.substring(7, instanceId.length()-7);
        } else {
            LOGE("createAppContext __get_app_context__");
            get_context_fun_name = "__get_app_context__";
            final_instanceId = instanceId;
        }

        // new a global object
        // --------------------------------------------------
        auto appWorkerObjectHolder = getLightAppObjectHolder(final_instanceId);
        if (appWorkerObjectHolder == nullptr) {
          return static_cast<int32_t>(false);
        }

        JSGlobalObject *worker_globalObject = appWorkerObjectHolder->m_globalObject.get();
        if (worker_globalObject == nullptr) {
          return static_cast<int32_t>(false);
        }

        JSLockHolder locker_global(this->m_globalVM.get());

        WeexGlobalObject *app_globalObject = appWorkerObjectHolder->cloneWeexObject(final_instanceId.utf8().data(),true, true);
        weex::GlobalObjectDelegate *delegate = NULL;
        app_globalObject->SetScriptBridge(script_bridge_);
//        VM &vm = worker_globalObject->vm();
//
//        JSLockHolder locker_1(&vm);
//
//        VM &thisVm = app_globalObject->vm();
//        JSLockHolder locker_2(&thisVm);
//
        PropertyName createInstanceContextProperty(Identifier::fromString(this->m_globalVM.get(), get_context_fun_name));
        ExecState *state = worker_globalObject->globalExec();
        JSValue createInstanceContextFunction = worker_globalObject->get(state, createInstanceContextProperty);
        MarkedArgumentBuffer args;

        CallData callData;
        CallType callType = getCallData(createInstanceContextFunction, callData);
        NakedPtr<Exception> returnedException;
        JSValue ret = call(state, createInstanceContextFunction, callType, callData,
                           worker_globalObject, args, returnedException);
        if (returnedException) {
            String exceptionInfo = exceptionToString(worker_globalObject, returnedException->value());
            return static_cast<int32_t>(false);
        }
        app_globalObject->resetPrototype(*(this->m_globalVM.get()), ret);
        app_globalObject->id = final_instanceId.utf8().data();
        // --------------------------------------------------


        if (!ExecuteJavaScript(app_globalObject, jsBundle, ("weex createAppContext"), true, "createAppContext",
                               final_instanceId.utf8().data())) {
            return static_cast<int32_t>(false);
        }
    }
    return static_cast<int32_t>(true);
}

std::unique_ptr<WeexJSResult> WeexRuntime::exeJSOnAppWithResult(const String &instanceId, const String &jsBundle) {

  std::unique_ptr<WeexJSResult> returnResult;
  return returnResult;
}

int
WeexRuntime::callJSOnAppContext(const String &instanceId, const String &func, std::vector<VALUE_WITH_TYPE *> &params) {
//    LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT instanceId:%s, func:%s", instanceId.utf8().data(), func.utf8().data());
    if (instanceId == "") {
        return static_cast<int32_t>(false);
    } else {
        auto appWorkerObjectHolder = getLightAppObjectHolder(instanceId);
        if (appWorkerObjectHolder == nullptr) {
//            LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT weexLiteAppObjectHolder is null");
            return static_cast<int32_t>(false);
        }

        JSGlobalObject *worker_globalObject = appWorkerObjectHolder->m_globalObject.get();
        if (worker_globalObject == NULL) {
//            LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT worker_globalObject is null");
            return static_cast<int32_t>(false);
        }
//        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT1");
        JSLockHolder locker_global(this->m_globalVM.get());

//        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT2");
        MarkedArgumentBuffer obj;
        ExecState *state = worker_globalObject->globalExec();
        _getArgListFromJSParams(&obj, state, params);
//        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT3");
        Identifier funcIdentifier = Identifier::fromString(this->m_globalVM.get(), func);

        JSValue function;
        JSValue result;
        function = worker_globalObject->get(state, funcIdentifier);
        CallData callData;
        CallType callType = getCallData(function, callData);
        NakedPtr<Exception> returnedException;
        // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT start call js runtime funtion");
        if (function.isEmpty()) {
            LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT js funtion is empty");
        }
        JSValue ret = call(state, function, callType, callData, worker_globalObject, obj, returnedException);
        // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT end");
        if (returnedException) {
            ReportException(worker_globalObject, returnedException.get(), instanceId.utf8().data(), func.utf8().data());
            return static_cast<int32_t>(false);
        }
        worker_globalObject->vm().drainMicrotasks();
        return static_cast<int32_t>(true);
    }
}

int WeexRuntime::callJSOnAppContext(IPCArguments *arguments) {
    const IPCString *ipcInstanceId = arguments->getString(0);
    const IPCString *ipcFunc = arguments->getString(1);
    String instanceId = jString2String(ipcInstanceId->content, ipcInstanceId->length);
    String func = jString2String(ipcFunc->content, ipcFunc->length);
    // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT instanceId:%s, func:%s", instanceId.utf8().data(), func.utf8().data());

    if (instanceId == "") {
        return static_cast<int32_t>(false);
    } else {
        std::map<std::string, WeexGlobalObject *>::iterator it_find;
        auto appWorkerObjectHolder = getLightAppObjectHolder(instanceId);
        if (appWorkerObjectHolder == nullptr) {
//            LOGE("callJSOnAppContext is running and id is %s and weexLiteAppObjectHolder is null", instanceId.utf8().data());
            return static_cast<int32_t>(false);
        }

        JSGlobalObject *worker_globalObject = appWorkerObjectHolder->m_globalObject.get();
        if (worker_globalObject == NULL) {
//            LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT worker_globalObject is null");
            return static_cast<int32_t>(false);
        }
//        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT1");

        JSLockHolder locker(this->m_globalVM.get());
        // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT2");
        MarkedArgumentBuffer obj;
        ExecState *state = worker_globalObject->globalExec();
        _getArgListFromIPCArguments(&obj, state, arguments, 2);
        // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT3");
        Identifier funcIdentifier = Identifier::fromString(this->m_globalVM.get(), func);

        JSValue function;
        JSValue result;
        function = worker_globalObject->get(state, funcIdentifier);
        CallData callData;
        CallType callType = getCallData(function, callData);
        NakedPtr<Exception> returnedException;
        // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT start call js runtime funtion");
        if (function.isEmpty()) {
            LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT js funtion is empty");
        }
        JSValue ret = call(state, function, callType, callData, worker_globalObject, obj, returnedException);
        // LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT end");
        if (returnedException) {
            ReportException(worker_globalObject, returnedException.get(), instanceId.utf8().data(), func.utf8().data());
            return static_cast<int32_t>(false);
        }
        worker_globalObject->vm().drainMicrotasks();
        return static_cast<int32_t>(true);
    }
}

int WeexRuntime::destroyAppContext(const String &instanceId) {
    auto appWorkerObjectHolder = getLightAppObjectHolder(instanceId);
    if (appWorkerObjectHolder == nullptr) {
        return static_cast<int32_t>(false);
    }

    LOGE("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT end1 %s",instanceId.utf8().data());
    std::map<std::string, WeexGlobalObject *>::iterator it_find_instance;
    auto objectMap = appWorkerObjectHolder->m_jsInstanceGlobalObjectMap;
    it_find_instance = objectMap.find(instanceId.utf8().data());
    if (it_find_instance != objectMap.end()) {
        // LOGE("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT mAppInstanceGlobalObjectMap donnot contain and return");
        objectMap.erase(instanceId.utf8().data());
    }

    if(appWorkerObjectHolder->timeQueue != nullptr)
        appWorkerObjectHolder->timeQueue->destroyPageTimer(instanceId.utf8().data());

    // GC on VM
//    WeexGlobalObject* instanceGlobalObject = mAppInstanceGlobalObjectMap[instanceId.utf8().data()];
//    if (instanceGlobalObject == NULL) {
//      return static_cast<int32_t>(true);
//    }
//    LOGE("Weex jsserver IPCJSMsg::DESTORYAPPCONTEXT start GC");
//    VM& vm_global = *globalVM.get();
//    JSLockHolder locker_global(&vm_global);
//
//    ExecState* exec_ = instanceGlobalObject->globalExec();
//    JSLockHolder locker_(exec_);
//    VM& vm_ = exec_->vm();
//    vm_.heap.collectAllGarbage();
//    instanceGlobalObject->m_server = nullptr;
//    instanceGlobalObject = NULL;

    appWorkerContextHolderMap.erase(instanceId.utf8().data());
    delete appWorkerObjectHolder;
    appWorkerObjectHolder = nullptr;
    // LOGE("mAppInstanceGlobalObjectMap size:%d mAppGlobalObjectMap size:%d",
    //      mAppInstanceGlobalObjectMap.size(), mAppGlobalObjectMap.size());
    return static_cast<int32_t>(true);
}

int WeexRuntime::exeJsService(const String &source) {
    base::debug::TraceScope traceScope("weex", "exeJSService");
//    if (weexLiteAppObjectHolder.get() != nullptr) {
//        VM & vm_global = *weexLiteAppObjectHolder->m_globalVM.get();
//        JSLockHolder locker_global(&vm_global);
//    }
    JSGlobalObject *globalObject = weexObjectHolder->m_globalObject.get();
    JSLockHolder locker(this->m_globalVM.get());
    if (!ExecuteJavaScript(globalObject, source, ("weex service"), true, "execjsservice")) {
        LOGE("jsLog JNI_Error >>> scriptStr :%s", source.utf8().data());
        return static_cast<int32_t>(false);
    }
    return static_cast<int32_t>(true);
}

int WeexRuntime::exeCTimeCallback(const String &source) {
    base::debug::TraceScope traceScope("weex", "EXECTIMERCALLBACK");
//    LOGE("IPC EXECTIMERCALLBACK and ExecuteJavaScript");
    JSGlobalObject *globalObject = weexObjectHolder->m_globalObject.get();
    JSLockHolder locker(this->m_globalVM.get());
    if (!ExecuteJavaScript(globalObject, source, ("weex service"), false, "timercallback")) {
        LOGE("jsLog EXECTIMERCALLBACK >>> scriptStr :%s", source.utf8().data());
        return static_cast<int32_t>(false);
    }

    return static_cast<int32_t>(true);
}

int WeexRuntime::exeJS(const String &instanceId, const String &nameSpace, const String &func,
                       std::vector<VALUE_WITH_TYPE *> &params) {
//    LOGE("dyyLog EXECJS func:%s and params size is %d", func.utf8().data(), params.size());

    String runFunc = func;
    JSGlobalObject *globalObject;
    // fix instanceof Object error
    // if function is callJs on instance, should us Instance object to call __WEEX_CALL_JAVASCRIPT__

    if (std::strcmp("callJS", runFunc.utf8().data()) == 0) {

      auto iterator = weexObjectHolder->m_jsInstanceGlobalObjectMap.find(instanceId.utf8().data());
      if (iterator != weexObjectHolder->m_jsInstanceGlobalObjectMap.end()) {
        globalObject = weexObjectHolder->m_jsInstanceGlobalObjectMap[instanceId.utf8().data()];
      }

        if (globalObject == NULL) {
            globalObject = weexObjectHolder->m_globalObject.get();
        } else {
            WTF::String funcWString("__WEEX_CALL_JAVASCRIPT__");
            runFunc = funcWString;
        }
    } else {
        globalObject = weexObjectHolder->m_globalObject.get();
    }
    JSLockHolder locker(this->m_globalVM.get());
//    if (weexLiteAppObjectHolder.get() != nullptr) {
//        VM & vm_global = *weexLiteAppObjectHolder->m_globalVM.get();
//        JSLockHolder locker_global(&vm_global);
//    }

    MarkedArgumentBuffer obj;
    base::debug::TraceScope traceScope("weex", "exeJS", "function", runFunc.utf8().data());
    ExecState *state = globalObject->globalExec();
    _getArgListFromJSParams(&obj, state, params);

    Identifier funcIdentifier = Identifier::fromString(this->m_globalVM.get(), runFunc);

    JSValue function;
    JSValue result;
    if (nameSpace.isEmpty()) {
        function = globalObject->get(state, funcIdentifier);
    } else {
        Identifier namespaceIdentifier = Identifier::fromString(this->m_globalVM.get(), nameSpace);
        JSValue master = globalObject->get(state, namespaceIdentifier);
        if (!master.isObject()) {
            return static_cast<int32_t>(false);
        }
        function = master.toObject(state)->get(state, funcIdentifier);
    }
    CallData callData;
    CallType callType = getCallData(function, callData);
    NakedPtr<Exception> returnedException;
    JSValue ret = call(state, function, callType, callData, globalObject, obj, returnedException);

    this->m_globalVM.get()->drainMicrotasks();


    if (returnedException) {
        ReportException(globalObject, returnedException.get(), instanceId.utf8().data(), func.utf8().data());
        return static_cast<int32_t>(false);
    }
    return static_cast<int32_t>(true);
}

inline void convertJSArrayToWeexJSResult(ExecState *state, JSValue &ret, WeexJSResult *jsResult) {
    if (ret.isUndefined() || ret.isNull() || !isJSArray(ret)) {
        // createInstance return whole source object, which is big, only accept array result
        return;
    }
    //
    /** most scene, return result is array of null */
    JSArray *array = asArray(ret);
    uint32_t length = array->length();
    bool isAllNull = true;
    for (uint32_t i = 0; i < length; i++) {
        JSValue ele = array->getIndex(state, i);
        if (!ele.isUndefinedOrNull()) {
            isAllNull = false;
            break;
        }
    }
    if (isAllNull) {
        return;
    }
    const char *data;
    if (WeexEnv::getEnv()->useWson()) {
        wson_buffer *buffer = wson::toWson(state, ret);
        data = (char *) buffer->data;
        jsResult->length = buffer->position;
        buffer->data = nullptr;
        wson_buffer_free(buffer);
    } else {
        String string = JSONStringify(state, ret, 0);
        CString cstring = string.utf8();
        char *buf = new char[cstring.length() + 1];
        memcpy(buf, cstring.data(), cstring.length());
        buf[cstring.length()] = '\0';
        jsResult->length = cstring.length();
        data = cstring.data();
    }
    char *buf = new char[jsResult->length + 1];
    memcpy(buf, data, jsResult->length);
    buf[jsResult->length] = '\0';
    jsResult->data.reset(buf);
}

std::unique_ptr<WeexJSResult> WeexRuntime::exeJSWithResult(const String &instanceId, const String &nameSpace, const String &func,
                                          std::vector<VALUE_WITH_TYPE *> &params) {


    std::unique_ptr<WeexJSResult> returnResult;
    returnResult.reset(new WeexJSResult);

    JSGlobalObject *globalObject;
    String runFunc = func;
    // fix instanceof Object error
    // if function is callJs should us Instance object to call __WEEX_CALL_JAVASCRIPT__
    if (std::strcmp("callJS", runFunc.utf8().data()) == 0) {
        globalObject = weexObjectHolder->m_jsInstanceGlobalObjectMap[instanceId.utf8().data()];
        if (globalObject == NULL) {
            globalObject = weexObjectHolder->m_globalObject.get();
        } else {
            WTF::String funcWString("__WEEX_CALL_JAVASCRIPT__");
            runFunc = funcWString;
        }
    } else {
        globalObject = weexObjectHolder->m_globalObject.get();
    }
    VM &vm = globalObject->vm();
    JSLockHolder locker(&vm);
//    if (weexLiteAppObjectHolder.get() != nullptr) {
//        VM & vm_global = *weexLiteAppObjectHolder->m_globalVM.get();
//        JSLockHolder locker_global(&vm_global);
//    }

    base::debug::TraceScope traceScope("weex", "exeJSWithResult", "function", runFunc.utf8().data());

    MarkedArgumentBuffer obj;
    ExecState *state = globalObject->globalExec();

    _getArgListFromJSParams(&obj, state, params);

    Identifier funcIdentifier = Identifier::fromString(&vm, runFunc);
    JSValue function;
    JSValue result;
    if (nameSpace.isEmpty()) {
        function = globalObject->get(state, funcIdentifier);
    } else {
        Identifier namespaceIdentifier = Identifier::fromString(&vm, nameSpace);
        JSValue master = globalObject->get(state, namespaceIdentifier);
        if (!master.isObject()) {
            return returnResult;
        }
        function = master.toObject(state)->get(state, funcIdentifier);
    }
    CallData callData;
    CallType callType = getCallData(function, callData);
    NakedPtr<Exception> returnedException;
    JSValue ret = call(state, function, callType, callData, globalObject, obj, returnedException);
    globalObject->vm().drainMicrotasks();

    if (returnedException) {
        ReportException(globalObject, returnedException.get(), instanceId.utf8().data(), func.utf8().data());
        return returnResult;
    }
    convertJSArrayToWeexJSResult(state, ret, returnResult.get());
    return returnResult;
}

void WeexRuntime::exeJSWithCallback(const String &instanceId, const String &nameSpace, const String &func,
                                          std::vector<VALUE_WITH_TYPE *> &params,  long callback_id) {
    auto result = exeJSWithResult(instanceId, nameSpace, func, params);
    script_bridge_->core_side()->OnReceivedResult(callback_id, result);
}

std::unique_ptr<WeexJSResult> WeexRuntime::exeJSOnInstance(const String &instanceId, const String &script) {
    LOGE("test-> [runtime] beofore exeJSOnInstance");
    std::unique_ptr<WeexJSResult> returnResult;
    returnResult.reset(new WeexJSResult);

    JSGlobalObject *globalObject = weexObjectHolder->m_jsInstanceGlobalObjectMap[instanceId.utf8().data()];
    if (globalObject == NULL) {
        globalObject = weexObjectHolder->m_globalObject.get();
    }
    VM &vm = globalObject->vm();
    JSLockHolder locker(&vm);
//    if (weexLiteAppObjectHolder.get() != nullptr) {
//        VM & vm_global = *weexLiteAppObjectHolder->m_globalVM.get();
//        JSLockHolder locker_global(&vm_global);
//    }

    SourceOrigin sourceOrigin(String::fromUTF8("(weex)"));
    NakedPtr<Exception> evaluationException;
    JSValue returnValue = evaluate(globalObject->globalExec(),
                                   makeSource(script, sourceOrigin, "execjs on instance context"), JSValue(),
                                   evaluationException);
    globalObject->vm().drainMicrotasks();
    if (evaluationException) {
        // String exceptionInfo = exceptionToString(globalObject, evaluationException.get()->value());
        // LOGE("EXECJSONINSTANCE exception:%s", exceptionInfo.utf8().data());
        ReportException(globalObject, evaluationException.get(), instanceId.utf8().data(), "execJSOnInstance");
        return nullptr;
    }
    // WTF::String str = returnValue.toWTFString(globalObject->globalExec());
    const char *data = returnValue.toWTFString(globalObject->globalExec()).utf8().data();
    returnResult->length = strlen(data);
    char *buf = new char[returnResult->length + 1];
    strcpy(buf, data);
    returnResult->data.reset(buf);
    LOGE("test-> [runtime] end exeJSOnInstance");
    return returnResult;
}

int WeexRuntime::destroyInstance(const String &instanceId) {

    // LOGE("IPCJSMsg::DESTORYINSTANCE");
    auto *globalObject = weexObjectHolder->m_jsInstanceGlobalObjectMap[instanceId.utf8().data()];
    if (globalObject == NULL) {
        return static_cast<int32_t>(true);
    }
    // LOGE("DestoryInstance map 11 length:%d", weexObjectHolder->m_jsGlobalObjectMap.size());
    weexObjectHolder->m_jsInstanceGlobalObjectMap.erase(instanceId.utf8().data());
    // LOGE("DestoryInstance map 22 length:%d", weexObjectHolder->m_jsGlobalObjectMap.size());
    if(weexObjectHolder->timeQueue != nullptr)
    weexObjectHolder->timeQueue->destroyPageTimer(instanceId.utf8().data());
    // release JSGlobalContextRelease
    // when instanceId % 20 == 0 GC
    bool needGc = false;
    if (instanceId.length() > 0) {
        int index = atoi(instanceId.utf8().data());
        if (index > 0 && index % 20 == 0) {
            // LOGE("needGc is true, instanceId.utf8().data():%s index:%d", instanceId.utf8().data(), index);
            needGc = true;
        }
    }
    if (needGc) {
      ExecState *exec = globalObject->globalExec();
      JSLockHolder locker(exec);
      VM &vm = exec->vm();
      globalObject->resetPrototype(vm, jsNull());
      bool protectCountIsZero = Heap::heap(exec->vmEntryGlobalObject())->unprotect(exec->vmEntryGlobalObject());
      vm.heap.reportAbandonedObjectGraph();
      //vm.heap.collectSync(CollectionScope::Full);//collectAllGarbage();//collectAsync(CollectionScope::Full);
      //TODO disable gc temporary
      vm.heap.collectAllGarbage();
    }

//    globalObject->m_server = nullptr;
    globalObject = NULL;

    return static_cast<int32_t>(true);
}

int WeexRuntime::updateGlobalConfig(const String &config) {
    JSGlobalObject *globalObject = weexObjectHolder->m_globalObject.get();
    VM &vm = globalObject->vm();
    JSLockHolder locker(&vm);




    //FIXME, Heron



//    if (weexLiteAppObjectHolder.get() != nullptr) {
//        VM & vm_global = *weexLiteAppObjectHolder->m_globalVM.get();
//        JSLockHolder locker_global(&vm_global);
//    }


    const char *configChar = config.utf8().data();
    doUpdateGlobalSwitchConfig(configChar);
    return static_cast<int32_t>(true);
}

int WeexRuntime::UpdateInitFrameworkParams(const std::string &key, const std::string &value,
                                           const std::string &desc) {
    JSGlobalObject *globalObject = weexObjectHolder->m_globalObject.get();
    VM &vm = globalObject->vm();
    JSLockHolder locker(&vm);
    weexObjectHolder->m_globalObject->updateInitFrameworkParams(key, value);
    return static_cast<int32_t>(true);
}

int WeexRuntime::createInstance(const String &instanceId, const String &func, const String &script, const String &opts,
                                const String &initData,
                                const String &extendsApi,
                                std::vector<INIT_FRAMEWORK_PARAMS*>& params) {
    LOGE("test-> : start createInstance");

    JSGlobalObject *impl_globalObject = weexObjectHolder->m_globalObject.get();
    JSGlobalObject *globalObject;
    if (instanceId == "") {
        globalObject = impl_globalObject;
    } else {

      WeexGlobalObject * temp_object = nullptr;

      auto iterator = weexObjectHolder->m_jsInstanceGlobalObjectMap.find(instanceId.utf8().data());
      if (iterator != weexObjectHolder->m_jsInstanceGlobalObjectMap.end()) {
          temp_object = weexObjectHolder->m_jsInstanceGlobalObjectMap[instanceId.utf8().data()];
      }

        if (temp_object == nullptr) {
            // new a global object
            // --------------------------------------------------
//            if (weexLiteAppObjectHolder.get() != nullptr) {
//                VM &vm_global = *weexLiteAppObjectHolder->m_globalVM.get();
//                JSLockHolder locker_global(&vm_global);
//            }

            temp_object = weexObjectHolder->cloneWeexObject(instanceId.utf8().data(), true, false);
            VM &vm = temp_object->vm();
            JSLockHolder locker(&vm);
            temp_object->addExtraOptions(params);
            temp_object->SetScriptBridge(script_bridge_);


            // --------------------------------------------------

            // use impl global object run createInstanceContext
            // --------------------------------------------------
            // RAx or vue
            JSGlobalObject *globalObject_local = impl_globalObject;
            PropertyName createInstanceContextProperty(Identifier::fromString(&vm, "createInstanceContext"));
            ExecState *state = globalObject_local->globalExec();
            JSValue createInstanceContextFunction = globalObject_local->get(state, createInstanceContextProperty);
            MarkedArgumentBuffer args;
            args.append(String2JSValue(state, instanceId));
            JSValue optsObject = parseToObject(state, opts);
            args.append(optsObject);
            JSValue initDataObject = parseToObject(state, initData);
            args.append(initDataObject);
            // args.append(String2JSValue(state, ""));
            CallData callData;
            CallType callType = getCallData(createInstanceContextFunction, callData);
            NakedPtr<Exception> returnedException;
            JSValue ret = call(state, createInstanceContextFunction, callType, callData,
                               globalObject_local, args, returnedException);
            if (returnedException) {
                // ReportException(globalObject, returnedException.get(), nullptr, "");
                String exceptionInfo = exceptionToString(globalObject_local, returnedException->value());
                LOGE("getJSGlobalObject returnedException %s", exceptionInfo.utf8().data());
            }
            // --------------------------------------------------

            // String str = getArgumentAsString(state, ret);
            //(ret.toWTFString(state));

            // use it to set Vue prototype to instance context
            JSObject *object = ret.toObject(state, temp_object);
            JSObjectRef ref = toRef(object);
            JSGlobalContextRef globalContextRef = toGlobalRef(state);
            JSGlobalContextRef instanceContextRef = toGlobalRef(temp_object->globalExec());
            auto instanceGlobalObject = JSContextGetGlobalObject(instanceContextRef);
            auto pArray = JSObjectCopyPropertyNames(globalContextRef, ref);
            size_t keyCount = JSPropertyNameArrayGetCount(pArray);
//            LOGE("dyyLog instance create and id is %s, and time is %lld, currentThread is %u", instanceId.utf8().data(), microTime(), pthread_self());
            for (size_t i = 0; i < keyCount; ++i) {
                auto propertyName_ = JSPropertyNameArrayGetNameAtIndex(pArray, i);
                auto propertyValue_ = JSObjectGetProperty(globalContextRef, ref, propertyName_, NULL);
                if(propertyValue_ == nullptr) {
                    LOGE("dyy create instance propertyValue_ == null");
                    continue;
                }

                if(propertyName_ == nullptr) {
                    LOGE("dyy create instance propertyName_ == null");
                    continue;
                }

                JSObjectSetProperty(instanceContextRef, instanceGlobalObject, propertyName_, propertyValue_, 0, NULL);
            }



            JSValueRef vueRef = JSObjectGetProperty(globalContextRef, ref, JSStringCreateWithUTF8CString("Vue"), nullptr);
            if (vueRef != nullptr) {
                JSObjectRef vueObject = JSValueToObject(globalContextRef, vueRef, nullptr);
                if (vueObject != nullptr) {

                    JSObjectSetPrototype(instanceContextRef, vueObject,
                                         JSObjectGetPrototype(instanceContextRef,
                                                              JSContextGetGlobalObject(instanceContextRef)));
                }
            }
            //-------------------------------------------------

//            temp_object->resetPrototype(vm, ret);
            weexObjectHolder->m_jsInstanceGlobalObjectMap[instanceId.utf8().data()] = temp_object;
//            LOGE("create Instance instanceId.utf8().data() %s",instanceId.utf8().data());
            // -----------------------------------------
            // ExecState* exec =temp_object->globalExec();
            // JSLockHolder temp_locker(exec);
            // VM& temp_vm = exec->vm();
            // gcProtect(exec->vmEntryGlobalObject());
            // temp_vm.ref();
            // ------------------------------------------
        }
        globalObject = temp_object;
    }

//    if (weexLiteAppObjectHolder.get() != nullptr) {
//        VM &vm_global = *weexLiteAppObjectHolder->m_globalVM.get();
//        JSLockHolder locker_global(&vm_global);
//    }

    VM &vm = globalObject->vm();
    JSLockHolder locker(&vm);

    // if extend api is not null should exec befor createInstanceContext, such as rax-api
    if (!extendsApi.isEmpty() && extendsApi.length() > 0) {
        if (!ExecuteJavaScript(globalObject, extendsApi, ("weex run raxApi"), true,
                               "runRaxApi", instanceId.utf8().data())) {
            LOGE("before createInstanceContext run rax api Error");
            return static_cast<int32_t>(false);
        }
    }

    LOGE("test-> : after run extendsApi");

    if (!ExecuteJavaScript(globalObject, script, ("weex createInstanceContext"), true,
                           "createInstanceContext", instanceId.utf8().data())) {
        LOGE("createInstanceContext and ExecuteJavaScript Error");
        return static_cast<int32_t>(false);
    }
    LOGE("test-> : after ExecuteJavaScript");
    return static_cast<int32_t>(true);
}

int WeexRuntime::_initFramework(const String &source) {
    JSLockHolder locker(this->m_globalVM.get());

    auto globalObject = weexObjectHolder->m_globalObject.get();
    globalObject->SetScriptBridge(script_bridge_);

    if (!ExecuteJavaScript(globalObject, source, "(weex framework)", true, "initFramework", "")) {
        return false;
    }

    setJSFVersion(globalObject);
    return true;
}

void
WeexRuntime::_getArgListFromIPCArguments(MarkedArgumentBuffer *obj, ExecState *state, IPCArguments *arguments,
                                         size_t start) {
    size_t count = arguments->getCount();
    for (size_t i = start; i < count; ++i) {
        switch (arguments->getType(i)) {
            case IPCType::DOUBLE:
                obj->append(jsNumber(arguments->get<double>(i)));
                break;
            case IPCType::STRING: {
                const IPCString *ipcstr = arguments->getString(i);
                obj->append(jString2JSValue(state, ipcstr->content, ipcstr->length));
            }
                break;
            case IPCType::JSONSTRING: {
                const IPCString *ipcstr = arguments->getString(i);

                String str = jString2String(ipcstr->content, ipcstr->length);

                JSValue o = parseToObject(state, str);
                obj->append(o);
            }
                break;
            case IPCType::BYTEARRAY: {
                const IPCByteArray *array = arguments->getByteArray(i);
                JSValue o = wson::toJSValue(state, (void *) array->content, array->length);
                obj->append(o);
            }
                break;
            default:
                obj->append(jsUndefined());
                break;
        }
    }
}

void WeexRuntime::_getArgListFromJSParams(MarkedArgumentBuffer *obj, ExecState *state,
                                          std::vector<VALUE_WITH_TYPE *> &params) {

    //dyyLog delete
//    String msg = "exejs Args ";

    for (unsigned int i = 0; i < params.size(); i++) {
        VALUE_WITH_TYPE *paramsObject = params[i];
        switch (paramsObject->type) {
            case ParamsType::DOUBLE:
                obj->append(jsNumber(paramsObject->value.doubleValue));
//                msg.append(":");
//                msg.append(std::to_string(paramsObject->value.doubleValue).c_str());
                break;
            case ParamsType::STRING: {
                WeexString *ipcstr = paramsObject->value.string;
                const String &string2String = weexString2String(ipcstr);
                obj->append(jString2JSValue(state, ipcstr->content, ipcstr->length));

//                msg.append(":");
//                msg.append(string2String.utf8().data());
            }
                break;
            case ParamsType::JSONSTRING: {
                const WeexString *ipcstr = paramsObject->value.string;
                const String &string = weexString2String(ipcstr);
                String str = jString2String(ipcstr->content, ipcstr->length);
                JSValue o = parseToObject(state, str);
                obj->append(o);

//                msg.append(":");
//                msg.append(str.utf8().data());
            }
                break;
            case ParamsType::BYTEARRAY: {
                const WeexByteArray *array = paramsObject->value.byteArray;
                JSValue o = wson::toJSValue(state, (void *) array->content, array->length);

                obj->append(o);

//                msg.append(":");
//                msg.append(JSONStringify(state, o, 0).utf8().data());
            }
                break;
            default:
                obj->append(jsUndefined());
                break;
        }
    }

//    LOGE("dyyLog exejs Args is %s", msg.utf8().data());
}

WeexObjectHolder *WeexRuntime::getLightAppObjectHolder(const String &instanceId) {
    auto iterator = appWorkerContextHolderMap.find(instanceId.utf8().data());
    if (iterator == appWorkerContextHolderMap.end()) {
        return nullptr;
    }

    return appWorkerContextHolderMap.at(instanceId.utf8().data());
}

int WeexRuntime::exeTimerFunction(const String &instanceId, uint32_t timerFunction, JSGlobalObject *globalObject) {
    uint64_t begin = microTime();
    if (globalObject == nullptr) {
        LOGE("exeTimerFunction and object is null");
        return 0;
    }
    VM &vm = globalObject->vm();
    JSLockHolder locker(&vm);
    WeexGlobalObject* go = static_cast<WeexGlobalObject*>(globalObject);
    const JSValue& value = go->getTimerFunction(timerFunction);
    JSValue result;
    CallData callData;
    CallType callType = getCallData(value, callData);
    if (callType == CallType::None)
        return -1;

    NakedPtr<Exception> returnedException;
    if (value.isEmpty()) {
        LOGE("Weex jsserver IPCJSMsg::CALLJSONAPPCONTEXT js funtion is empty");
    }

    ArgList a;
    JSValue ret = call(globalObject->globalExec(), value, callType, callData, globalObject, a, returnedException);
    uint64_t end = microTime();

    return 0;
}

void WeexRuntime::removeTimerFunction(const uint32_t timerFunction, JSGlobalObject *globalObject) {
    WeexGlobalObject* go = static_cast<WeexGlobalObject*>(globalObject);
    if (go == nullptr)
        return;

    go->removeTimer(timerFunction);
}

bool WeexRuntime::hasInstanceId(String &id) {

    auto iterator = appWorkerContextHolderMap.find(id.utf8().data());
    if (iterator != appWorkerContextHolderMap.end()) {
        return true;
    }

    auto it = weexObjectHolder->m_jsInstanceGlobalObjectMap.find(id.utf8().data());
    if (it != weexObjectHolder->m_jsInstanceGlobalObjectMap.end()) {
        return true;
    }

    return false;
}











