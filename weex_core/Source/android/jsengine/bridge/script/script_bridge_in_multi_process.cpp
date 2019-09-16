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
// Created by yxp on 2018/6/15.
//

#include "android/jsengine/bridge/script/script_bridge_in_multi_process.h"

#include "android/jsengine/bridge/script/core_side_in_multi_process.h"
#include "android/jsengine/bridge/script/core_side_in_simple.h"
#include "android/jsengine/bridge/script/script_side_in_simple.h"
#include "android/jsengine/bridge/script/script_side_in_queue.h"
#include "android/jsengine/object/weex_env.h"
#include "android/jsengine/weex_ipc_server.h"
#include "android/jsengine/weex_jsc_utils.h"
#include "android/jsengine/weex_runtime.h"
#include "third_party/IPC/IPCArguments.h"
#include "third_party/IPC/IPCException.h"
#include "third_party/IPC/IPCHandler.h"
#include "third_party/IPC/IPCMessageJS.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/IPCString.h"

static WeexJSServer *server = nullptr;

static unsigned long parseUL(const char *s) {
    unsigned long val;
    errno = 0;
    val = strtoul(s, nullptr, 10);
    if (errno) {
        LOGE("failed to parse ul: %s %s", s, strerror(errno));
        _exit(1);
    }
    return val;
}

struct ThreadData {
    int fd;
    int fd_client;
    bool enableTrace;
    char* crashFileName;
};

static void *threadEntry(void *_td) {
    ThreadData *td = static_cast<ThreadData *>(_td);
    //  server = new weex::IPCServer(static_cast<int>(td->fd),
    //  static_cast<bool>(td->enableTrace));
    server = new WeexJSServer(static_cast<int>(td->fd), static_cast<int>(td->fd_client),
                              static_cast<bool>(td->enableTrace), td->crashFileName);
    // Register handler for bridge
    weex::bridge::js::ScriptBridgeInMultiProcess::RegisterIPCCallback(server->getHandler());
    nice(6);
    try {
        server->loop();
    } catch (IPCException &e) {
        LOGE("%s", e.msg());
        _exit(1);
    }
    return static_cast<void **>(nullptr);
}

__attribute__((visibility("default"))) extern "C" int serverMain(int argc, char **argv) {
    unsigned long fd;
    unsigned long fd_client = 0;
    unsigned long enableTrace;
    if (argc < 4) {
        LOGE("argc is not correct");
        _exit(1);
    }
    fd = parseUL(argv[1]);
    fd_client = parseUL(argv[2]);
    enableTrace = parseUL(argv[3]);
    char* fileName = argv[4];
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setstacksize(&threadAttr, 10 * 1024 * 1024);
    pthread_t thread;
    ThreadData td = {static_cast<int>(fd), static_cast<int>(fd_client), static_cast<bool>(enableTrace),fileName};
    pthread_create(&thread, &threadAttr, threadEntry, &td);
    void *rdata;
    pthread_join(thread, &rdata);
    return 0;
}

namespace weex {
    namespace bridge {
        namespace js {
            bool ScriptBridgeInMultiProcess::has_read_alarm_config = false;

            static inline const char* GetUTF8StringFromIPCArg(IPCArguments* arguments, size_t index) {
                return arguments->getByteArray(index)->length == 0 ? nullptr : arguments->getByteArray(index)->content;
            }

            ScriptBridgeInMultiProcess *ScriptBridgeInMultiProcess::g_instance = NULL;

            ScriptBridgeInMultiProcess::ScriptBridgeInMultiProcess() {
                set_script_side(new ScriptSideInQueue());
                set_core_side(new CoreSideInMultiProcess());
            }

            ScriptBridgeInMultiProcess::~ScriptBridgeInMultiProcess() {}

            static void FillVectorOfValueWithType(std::vector<VALUE_WITH_TYPE *> &params,
                                                  IPCArguments *arguments, size_t start,
                                                  size_t end) {
                for (size_t i = start; i < end; ++i) {
                    auto value = new ValueWithType;
                    switch (arguments->getType(i)) {
                        case IPCType::DOUBLE: {
                            value->type = ParamsType::DOUBLE;
                            value->value.doubleValue = arguments->get<double>(i);
                        }
                            break;
                        case IPCType::STRING: {
                            const IPCString *ipcstr = arguments->getString(i);
                            size_t size = ipcstr->length * sizeof(uint16_t);
                            auto temp = (WeexString *) malloc(size + sizeof(WeexString));
                            memset(temp, 0, size + sizeof(WeexString));
                            temp->length = ipcstr->length;
                            memcpy(temp->content, ipcstr->content, size);
                            value->type = ParamsType::STRING;
                            value->value.string = temp;
                        }
                            break;
                        case IPCType::JSONSTRING: {
                            const IPCString *ipcstr = arguments->getString(i);
                            size_t size = ipcstr->length * sizeof(uint16_t);
                            auto temp = (WeexString *) malloc(size + sizeof(WeexString));
                            memset(temp, 0, size + sizeof(WeexString));
                            temp->length = ipcstr->length;
                            memcpy(temp->content, ipcstr->content, size);
                            value->type = ParamsType::JSONSTRING;
                            value->value.string = temp;
                        }
                            break;
                        case IPCType::BYTEARRAY: {
                            const IPCByteArray *array = arguments->getByteArray(i);
                            size_t size = array->length * sizeof(char);
                            auto temp = (WeexByteArray *) malloc(size + sizeof(WeexByteArray));
                            memset(temp, 0, size + sizeof(WeexByteArray));
                            temp->length = array->length;
                            memcpy(temp->content, array->content, size);
                            value->type = ParamsType::BYTEARRAY;
                            value->value.byteArray = temp;
                        }
                            break;
                        default: {
                            value->type = ParamsType::JSUNDEFINED;
                        }
                            break;
                    }
                    params.push_back(value);
                }
            }

            static void ClearVectorOfValueWithType(std::vector<VALUE_WITH_TYPE *> &params) {
                for (auto it = params.begin(); it != params.end(); ++it) {
                    switch ((*it)->type) {
                        case ParamsType::STRING:
                        case ParamsType::JSONSTRING:
                            free((*it)->value.string);
                            break;
                        case ParamsType::BYTEARRAY:
                            free((*it)->value.byteArray);
                            break;
                        default:
                            break;
                    }
                    delete *it;
                }
            }

            void ScriptBridgeInMultiProcess::RegisterIPCCallback(IPCHandler *handler) {
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::INITFRAMEWORK),
                                         InitFramework);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::INITAPPFRAMEWORK),
                                         InitAppFramework);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::CREATEAPPCONTEXT),
                                         CreateAppContext);
                handler->registerHandler(
                        static_cast<uint32_t>(IPCJSMsg::EXECJSONAPPWITHRESULT),
                        ExecJSOnAppWithResult);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::CALLJSONAPPCONTEXT),
                                         CallJSOnAppContext);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::DESTORYAPPCONTEXT),
                                         DestroyAppContext);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSSERVICE),
                                         ExecJSService);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::TAKEHEAPSNAPSHOT),
                                         TakeHeapSnapshot);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECTIMERCALLBACK),
                                         ExecTimerCallback);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJS), ExecJS);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSWITHRESULT),
                                         ExecJSWithResult);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSWITHCALLBACK),
                                         ExecJSWithCallback);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::CREATEINSTANCE),
                                         CreateInstance);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::DESTORYINSTANCE),
                                         DestroyInstance);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::EXECJSONINSTANCE),
                                         ExecJSOnInstance);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::UPDATEGLOBALCONFIG),
                                         UpdateGlobalConfig);
                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::UpdateInitFrameworkParams),
                                         UpdateInitFrameworkParams);

                handler->registerHandler(static_cast<uint32_t>(IPCJSMsg::SETLOGLEVEL),
                                         setLogType);
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::InitFramework(
                    IPCArguments *arguments) {
                //TODO change ScriptSide
                LOGD("ScriptBridgeInMultiProcess::InitFramework");
                static_cast<ScriptSideInQueue *>(Instance()->script_side())
                        ->setTaskQueue(new WeexTaskQueue());
                WeexEnv::getEnv()->setScriptBridge(Instance());

                // Source
                const char *source = GetUTF8StringFromIPCArg(arguments, 0);
                // Params
                size_t startCount = 1;
                size_t count = arguments->getCount();
                std::vector<INIT_FRAMEWORK_PARAMS *> params;
                for (size_t i = startCount; i < count; i += 2) {
                    if (arguments->getType(i) != IPCType::BYTEARRAY) {
                        continue;
                    }
                    if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
                        continue;
                    }
                    const IPCByteArray *ba = arguments->getByteArray(1 + i);
                    const IPCByteArray *ba_type = arguments->getByteArray(i);
                    auto init_framework_params =
                            (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));
                    if (init_framework_params == nullptr) {
                        break;
                    }
                    memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
                    init_framework_params->type = IPCByteArrayToWeexByteArray(ba_type);
                    init_framework_params->value = IPCByteArrayToWeexByteArray(ba);

                    if(!WeexEnv::getEnv()->enableBackupThread()) {
#ifdef USE_JS_RUNTIME
                        std::string type = init_framework_params->type->content;
                        std::string value = init_framework_params->value->content;
#else
                        auto type = String::fromUTF8(init_framework_params->type->content);
                        auto value = String::fromUTF8(init_framework_params->value->content);
#endif
                        if(type == "enableBackupThread") {
                            auto enable = value == "true";
                            LOGE("enable backupThread %d",enable);
                            WeexEnv::getEnv()->setEnableBackupThread(enable);
                        } else if(type == "enableBackupThreadCache") {
                            auto enable = value == "true";
                            LOGE("enable backupThreadCache %d",enable);
                            WeexEnv::getEnv()->set_m_cache_task_(enable);
                        }
                    }
                    if (!has_read_alarm_config){
#ifdef USE_JS_RUNTIME
                        std::string type = init_framework_params->type->content;
                        std::string value = init_framework_params->value->content;
#else
                        auto type = String::fromUTF8(init_framework_params->type->content);
                        auto value = String::fromUTF8(init_framework_params->value->content);
#endif
                        if (type == "enableAlarmSignal"){
                            has_read_alarm_config = true;
                            auto enable = value =="true";
                            WeexEnv::getEnv()->enableHandleAlarmSignal(enable);
                        }
                    }

                    params.push_back(init_framework_params);
                }
                int val = Instance()->script_side()->InitFramework(source, params);
                freeInitFrameworkParams(params);
                return createInt32Result(val);
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::InitAppFramework(
                    IPCArguments *arguments) {
                const char *id = GetUTF8StringFromIPCArg(arguments, 0);
                const char *js = GetUTF8StringFromIPCArg(arguments, 1);
                LOGD("Weex jsserver IPCJSMsg::INITAPPFRAMEWORK id:%s", id);
                if (strcmp(id, "") == 0) {
                    return createInt32Result(static_cast<int32_t>(false));
                }
                // Params
                size_t startCount = 2;
                size_t count = arguments->getCount();
                std::vector<INIT_FRAMEWORK_PARAMS *> params;
                for (size_t i = startCount; i < count; i += 2) {
                    if (arguments->getType(i) != IPCType::BYTEARRAY) {
                        continue;
                    }
                    if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
                        continue;
                    }
                    const IPCByteArray *ba = arguments->getByteArray(1 + i);
                    const IPCByteArray *ba_type = arguments->getByteArray(i);
                    auto init_framework_params =
                            (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));
                    if (init_framework_params == nullptr) {
                        break;
                    }
                    memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
                    init_framework_params->type = IPCByteArrayToWeexByteArray(ba_type);
                    init_framework_params->value = IPCByteArrayToWeexByteArray(ba);
                    params.push_back(init_framework_params);
                }
                return createInt32Result(
                        Instance()->script_side()->InitAppFramework(id, js, params));
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::CreateAppContext(
                    IPCArguments *arguments) {
                const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
                const char *js = GetUTF8StringFromIPCArg(arguments, 1);
                return createInt32Result(
                        Instance()->script_side()->CreateAppContext(instanceID, js));
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSOnAppWithResult(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::ExecJSONAppWithResult");
                const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
                const char *js = GetUTF8StringFromIPCArg(arguments, 1);
                const std::unique_ptr<WeexJSResult> &ptr = Instance()->script_side()->ExecJSOnAppWithResult(instanceID, js);

                return createByteArrayResult(ptr->data.get(), ptr->length);
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::CallJSOnAppContext(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::CallJSONAppContext");
                const char *instanceId = GetUTF8StringFromIPCArg(arguments, 0);
                const char *func = GetUTF8StringFromIPCArg(arguments, 1);

                std::vector<VALUE_WITH_TYPE *> params;
                FillVectorOfValueWithType(params, arguments, 2, arguments->getCount());
                auto result =
                        Instance()->script_side()->CallJSOnAppContext(instanceId, func, params);
                ClearVectorOfValueWithType(params);
                return createInt32Result(result);
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::DestroyAppContext(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::DestroyAppContext");
                const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
                if (strlen(instanceID) == 0) {
                    return createInt32Result(static_cast<int32_t>(false));
                }

                return createInt32Result(
                        Instance()->script_side()->DestroyAppContext(instanceID));
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSService(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::ExecJSService");
                const char *source = GetUTF8StringFromIPCArg(arguments, 0);
                return createInt32Result(Instance()->script_side()->ExecJsService(source));
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecTimerCallback(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::ExecTimerCallback");
                const char *source = GetUTF8StringFromIPCArg(arguments, 0);
                Instance()->script_side()->ExecTimeCallback(source);
                return createVoidResult();
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJS(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::ExecJS");
                const char *instanceId = GetUTF8StringFromIPCArg(arguments, 0);
                const char *namespaceStr = GetUTF8StringFromIPCArg(arguments, 1);
                const char *func = GetUTF8StringFromIPCArg(arguments, 2);

                std::vector<VALUE_WITH_TYPE *> params;
                FillVectorOfValueWithType(params, arguments, 3, arguments->getCount());
                auto result =
                        Instance()->script_side()->ExecJS(instanceId, namespaceStr, func, params);
                ClearVectorOfValueWithType(params);
                return createInt32Result(result);
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSWithResult(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::ExecJSWithResult");
                const char *instanceId = GetUTF8StringFromIPCArg(arguments, 0);
                const char *namespaceStr = GetUTF8StringFromIPCArg(arguments, 1);
                const char *func = GetUTF8StringFromIPCArg(arguments, 2);

                std::vector<VALUE_WITH_TYPE *> params;
                FillVectorOfValueWithType(params, arguments, 3, arguments->getCount());
                const std::unique_ptr<WeexJSResult> &ptr = Instance()->script_side()->ExecJSWithResult(
                                                        instanceId, namespaceStr, func, params);
                ClearVectorOfValueWithType(params);

                return createByteArrayResult(ptr->data.get(), ptr->length);
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSWithCallback(
                    IPCArguments *arguments) {
                const char *instanceId = GetUTF8StringFromIPCArg(arguments, 0);
                const char *namespaceStr = GetUTF8StringFromIPCArg(arguments, 1);
                const char *func = GetUTF8StringFromIPCArg(arguments, 2);
                // pass callback_id before params
                long id = arguments->get<int64_t>(3);

                std::vector<VALUE_WITH_TYPE *> params;
                FillVectorOfValueWithType(params, arguments, 4, arguments->getCount());
                Instance()->script_side()->ExecJSWithCallback(
                        instanceId, namespaceStr, func, params, id);
                ClearVectorOfValueWithType(params);

                return createVoidResult();
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::CreateInstance(
                    IPCArguments *arguments) {
                const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
                const char *func = GetUTF8StringFromIPCArg(arguments, 1);
                const char *script = GetUTF8StringFromIPCArg(arguments, 2);
                const char *opts = GetUTF8StringFromIPCArg(arguments, 3);
                const char *initData = GetUTF8StringFromIPCArg(arguments, 4);
                const char *extendsApi = GetUTF8StringFromIPCArg(arguments, 5);
                size_t startCount = 6;
                size_t count = arguments->getCount();
                std::vector<INIT_FRAMEWORK_PARAMS*> params;
                for (size_t i = startCount; i < count; i += 2) {
                    if (arguments->getType(i) != IPCType::BYTEARRAY) {
                        continue;
                    }
                    if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
                        continue;
                    }
                    const IPCByteArray* ba = arguments->getByteArray(1 + i);
                    const IPCByteArray* ba_type = arguments->getByteArray(i);
                    auto init_framework_params =
                            (INIT_FRAMEWORK_PARAMS*)malloc(sizeof(INIT_FRAMEWORK_PARAMS));
                    if (init_framework_params == nullptr) {
                        break;
                    }
                    memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
                    init_framework_params->type = IPCByteArrayToWeexByteArray(ba_type);
                    init_framework_params->value = IPCByteArrayToWeexByteArray(ba);
                    params.push_back(init_framework_params);
                }
                LOG_TLOG("jsEngine","ScriptBridgeInMultiProcess::CreateInstance and Id is : %s", instanceID);
                auto result = createInt32Result(Instance()->script_side()->CreateInstance(
                                        instanceID, func, script, opts, initData, extendsApi,params));
                freeInitFrameworkParams(params);
                return result;
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::DestroyInstance(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::DestroyInstance");
                const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
                if (strlen(instanceID) == 0) {
                    LOGE("DestoryInstance instanceId is NULL");
                    return createInt32Result(static_cast<int32_t>(false));
                }

                return createInt32Result(
                        Instance()->script_side()->DestroyInstance(instanceID));
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::ExecJSOnInstance(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::ExecJSONInstance");
                const char *instanceID = GetUTF8StringFromIPCArg(arguments, 0);
                const char *script = GetUTF8StringFromIPCArg(arguments, 1);
                int type = arguments->get<int32_t>(2);

                const std::unique_ptr<WeexJSResult> &ptr = Instance()->script_side()->ExecJSOnInstance(instanceID, script,type);

                return createByteArrayResult(ptr->data.get(), ptr->length);
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::UpdateGlobalConfig(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::UpdateGlobalConfig");
                const char *configString = GetUTF8StringFromIPCArg(arguments, 0);
                Instance()->script_side()->UpdateGlobalConfig(configString);
                return createVoidResult();
            }

            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::UpdateInitFrameworkParams(
                    IPCArguments *arguments) {
                LOGD("ScriptBridgeInMultiProcess::UpdateInitFrameworkParams");
                const char *key = GetUTF8StringFromIPCArg(arguments, 0);
                const char *value = GetUTF8StringFromIPCArg(arguments, 1);
                const char *desc = GetUTF8StringFromIPCArg(arguments, 2);
                Instance()->script_side()->UpdateInitFrameworkParams(key, value, desc);
                LOGD("ScriptBridgeInMultiProcess::UpdateInitFrameworkParams End");
                return createVoidResult();
            }

        std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::setLogType(
            IPCArguments *arguments) {
            LOGD("ScriptBridgeInMultiProcess::setLogType");
            int type = arguments->get<int32_t>(0);
            int perf = arguments->get<int32_t>(1);
            Instance()->script_side()->SetLogType(type, perf == 1);

            return createVoidResult();
        }


            std::unique_ptr<IPCResult> ScriptBridgeInMultiProcess::TakeHeapSnapshot(
                    IPCArguments *arguments) {
                return createVoidResult();
            }

        }  // namespace js
    }  // namespace bridge
}  // namespace weex
