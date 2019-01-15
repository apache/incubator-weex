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
#include "core_side_in_multi_process.h"

#include "android/base/log_utils.h"
#include "android/utils/params_utils.h"
#include "core_side_in_multi_process.h"
#include "include/WeexApiHeader.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "third_party/IPC/IPCMessageJS.h"
#include "third_party/IPC/IPCSender.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/Buffering/IPCBuffer.h"
#include "third_party/IPC/IPCException.h"

namespace WeexCore {

    void CoreSideInMultiProcess::SetDefaultHeightAndWidthIntoRootDom(
            const std::string &instance_id, float default_width, float default_height,
            bool is_width_wrap_content, bool is_height_wrap_content) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(static_cast<uint32_t>(
                                   IPCMsgFromPlatformToCore::SET_DEFAULT_HEIGHT_AND_WIDTH_INTO_ROOT_DOM));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(default_width);
        serializer->add(default_height);
        serializer->add(is_width_wrap_content);
        serializer->add(is_height_wrap_content);
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::OnInstanceClose(const std::string &instance_id) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::ON_INSTANCE_CLOSE));
        serializer->add(instance_id.c_str(), instance_id.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::SetStyleWidth(const std::string &instance_id,
                                         const std::string &render_ref,
                                         float width) {
        // TODO 无法 call on main thread
    }

    void CoreSideInMultiProcess::SetStyleHeight(const std::string &instance_id,
                                          const std::string &render_ref,
                                          float height) {
        // TODO 无法 call on main thread
    }

    void CoreSideInMultiProcess::SetMargin(const std::string &instance_id,
                                     const std::string &render_ref, int edge,
                                     float value) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_MARGIN));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(render_ref.c_str(), render_ref.size());
        serializer->add(edge);
        serializer->add(value);
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::SetPadding(const std::string &instance_id,
                                      const std::string &render_ref, int edge,
                                      float value) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_PADDING));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(render_ref.c_str(), render_ref.size());
        serializer->add(edge);
        serializer->add(value);
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::SetPosition(const std::string &instance_id,
                                       const std::string &render_ref, int edge,
                                       float value) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_POSITION));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(render_ref.c_str(), render_ref.size());
        serializer->add(edge);
        serializer->add(value);
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::MarkDirty(const std::string &instance_id,
                                     const std::string &render_ref) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::MARK_DIRTY));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(render_ref.c_str(), render_ref.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::SetViewPortWidth(const std::string &instance_id,
                                            float width) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_VIEWPORT_WIDTH));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(width);
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::ForceLayout(const std::string &instance_id) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::FORCE_LAYOUT));
        serializer->add(instance_id.c_str(), instance_id.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    bool CoreSideInMultiProcess::NotifyLayout(const std::string &instance_id) {
        // TODO 为什么需要 notfiylayout，直接一次 layout 就可以，layout 可以给 async 标签，不需要等待写入
        return true;
    }
    std::vector<int64_t> CoreSideInMultiProcess::GetFirstScreenRenderTime(
            const std::string &instance_id) {
        // TODO 无法 call on main thread
        return std::vector<int64_t>();
    }
    std::vector<int64_t> CoreSideInMultiProcess::GetRenderFinishTime(
            const std::string &instance_id) {
        // TODO 无法 call on main thread
        return std::vector<int64_t>();
    }
    void CoreSideInMultiProcess::SetRenderContainerWrapContent(
            const std::string &instance_id, bool wrap) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(static_cast<uint32_t>(
                                   IPCMsgFromPlatformToCore::SET_RENDER_CONTAINER_WRAP_CONTENT));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(wrap);
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::BindMeasurementToRenderObject(long ptr) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(static_cast<uint32_t>(
                                   IPCMsgFromPlatformToCore::BIND_MEASUREMENT_TO_RENDER_OBJECT));
        serializer->add(static_cast<int64_t>(ptr));
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::RegisterCoreEnv(const std::string &key,
                                           const std::string &value) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::REGISTER_CORE_ENV));
        serializer->add(key.c_str(), key.size());
        serializer->add(value.c_str(), value.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    long CoreSideInMultiProcess::GetRenderObject(const std::string &instance_id,
                                           const std::string &render_ref) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::GET_RENDER_OBJECT));
        serializer->add(instance_id.c_str(), instance_id.size());
        serializer->add(render_ref.c_str(), render_ref.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
        return result->get<long>();
    }

    void CoreSideInMultiProcess::UpdateRenderObjectStyle(long render_ptr,
                                                   const std::string &key,
                                                   const std::string &value) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(static_cast<uint32_t>(
                                   IPCMsgFromPlatformToCore::UPDATE_RENDER_OBJECT_STYLE));
        serializer->add(static_cast<int64_t>(render_ptr));
        serializer->add(key.c_str(), key.size());
        serializer->add(value.c_str(), value.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::UpdateRenderObjectAttr(long render_ptr,
                                                  const std::string &key,
                                                  const std::string &value) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(static_cast<uint32_t>(
                                   IPCMsgFromPlatformToCore::UPDATE_RENDER_OBJECT_ATTR));
        serializer->add(static_cast<int64_t>(render_ptr));
        serializer->add(key.c_str(), key.size());
        serializer->add(value.c_str(), value.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    long CoreSideInMultiProcess::CopyRenderObject(long render_ptr) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::COPY_RENDER_OBJECT));
        serializer->add(static_cast<int64_t>(render_ptr));
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
        return result->get<long>();
    }

    void CoreSideInMultiProcess::SetMeasureFunctionAdapter() {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(static_cast<uint32_t>(
                                   IPCMsgFromPlatformToCore::SET_MEASURE_FUNCTION_ADAPTER));
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::SetPlatform(const std::string& platform) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_PLATFORM));
        serializer->add(platform.c_str(), platform.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::SetDeviceWidthAndHeight(float width, float height) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(static_cast<uint32_t>(
                                   IPCMsgFromPlatformToCore::SET_DEVICE_WIDTH_AND_HEIGHT));
        serializer->add(width);
        serializer->add(height);
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }

    void CoreSideInMultiProcess::AddOption(const std::string& key, const std::string& value) {
        std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
        serializer->setMsg(
                static_cast<uint32_t>(IPCMsgFromPlatformToCore::ADD_OPTION));
        serializer->add(key.c_str(), key.size());
        serializer->add(value.c_str(), value.size());
        std::unique_ptr<IPCBuffer> buffer = serializer->finish();
        std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    }


    int CoreSideInMultiProcess::RefreshInstance(const char* instanceId, const char* nameSpace,
                        const char* func,
                        std::vector<VALUE_WITH_TYPE *> &params) {
        return 1;
    }

    int CoreSideInMultiProcess::InitFramework(const char *script,
                                          std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::INIT_FRAMEWORK));
            serializer->add(script, strlen(script));
            for (auto it = params.begin(); it != params.end(); ++it) {
                serializer->add((*it)->type->content, (*it)->type->length);
                serializer->add((*it)->value->content, (*it)->value->length);
            }
            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::INT32) {
                LOGE("initFramework Unexpected result type");
                bridge()->platform_side()->ReportException("", "initFramework",
                                                           "error, initFramework Unexpected result type");
                return false;
            }
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            return false;
        }
        return true;
    }

    int CoreSideInMultiProcess::InitAppFramework(
            const char *instanceId, const char *appFramework,
            std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::INIT_APP_FRAMEWORK));
            serializer->add(instanceId, strlen(instanceId));
            serializer->add(appFramework, strlen(appFramework));

            for (auto it = params.begin(); it != params.end(); ++it) {
                serializer->add((*it)->type->content, (*it)->type->length);
                serializer->add((*it)->value->content, (*it)->value->length);
            }

            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            return result->get<jint>();
        } catch (IPCException &e) {
            LOGE("initAppFramework error %s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return false;
        }
    }

    int CoreSideInMultiProcess::CreateAppContext(const char *instanceId,
                                             const char *jsBundle) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::CREATE_APP_CONTEXT));
            serializer->add(instanceId, strlen(instanceId));
            serializer->add(jsBundle, strlen(jsBundle));
            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            return result->get<jint>();
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return false;
        }
    }

    std::unique_ptr<WeexJSResult> CoreSideInMultiProcess::ExecJSOnAppWithResult(const char *instanceId,
                                                    const char *jsBundle) {
        std::unique_ptr<WeexJSResult> ret;
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS_ON_APP_WITH_RESULT));
            serializer->add(instanceId, strlen(instanceId));
            serializer->add(jsBundle, strlen(jsBundle));
            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::BYTEARRAY) {
                return ret;
            }
            if (result->getByteArrayLength() == 0) {
                return ret;
            }

            ret.reset(new WeexJSResult);
            ret->length = result->getByteArrayLength();
            char *string = new char[ret->length + 1];
            ret->data.reset(string);
            memset(string, 0, ret->length);
            memcpy(string, result->getByteArrayContent(), result->getByteArrayLength());
            string[ret->length] = '\0';
            return ret;
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return ret;
        }
    }

    int CoreSideInMultiProcess::CallJSOnAppContext(const char *instanceId, const char *func,
                                               std::vector<VALUE_WITH_TYPE *> &params) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::CALL_JS_ON_APP_CONTEXT));
            serializer->add(instanceId, strlen(instanceId));
            serializer->add(func, strlen(func));


            for (int i = 0; i < params.size(); i++) {
                VALUE_WITH_TYPE *param = params[i];

                if (param->type == ParamsType::DOUBLE) {
                    serializer->add(param->value.doubleValue);
                } else if (param->type == ParamsType::STRING) {
                    serializer->add(param->value.string->content,
                                    param->value.string->length);
                } else if(param->type == ParamsType::JSONSTRING) {
                    serializer->addJSON(param->value.string->content,
                                        param->value.string->length);
                } else {
                    serializer->addJSUndefined();
                }
            }
            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::INT32) {
                LOGE("execJS Unexpected result type");
                return false;
            }
            return result->get<jint>();
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return false;
        }
        return true;
    }

    int CoreSideInMultiProcess::DestroyAppContext(const char *instanceId) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::DESTROY_APP_CONTEXT));
            serializer->add(instanceId, strlen(instanceId));
            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            return true;
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return false;
        }
    }

    int CoreSideInMultiProcess::ExecJsService(const char *source) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS_SERVICE));
            serializer->add(source, strlen(source));
            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::INT32) {
                LOGE("execJSService Unexpected result type");
                return false;
            }
            return result->get<jint>();
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            return false;
        }
    }

    int CoreSideInMultiProcess::ExecTimeCallback(const char *source) {
        return 0;
    }

    int CoreSideInMultiProcess::ExecJS(const char *instanceId, const char *nameSpace,
                                   const char *func,
                                   std::vector<VALUE_WITH_TYPE *> &params) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS));
            serializer->add(instanceId, strlen(instanceId));
            if (nameSpace)
                serializer->add(nameSpace, strlen(nameSpace));
            else {
                uint16_t tmp = 0;
                serializer->add(&tmp, 0);
            }
            LOGE("dd function is %s", func);
            serializer->add(func, strlen(func));

            for (int i = 0; i < params.size(); i++) {
                VALUE_WITH_TYPE *param = params[i];
                addParamsToIPCSerializer(serializer.get(), param);
//                if (param->type == ParamsType::DOUBLE) {
//                    serializer->add(param->value.doubleValue);
//                } else if (param->type == ParamsType::STRING) {
//                    serializer->add(param->value.string->content,
//                                    param->value.string->length);
//                } else if(param->type == ParamsType::JSONSTRING) {
//                    serializer->addJSON(param->value.string->content,
//                                        param->value.string->length);
//                }else if (param->type == ParamsType::BYTEARRAY) {
//                    serializer->add(param->value.byteArray->content,
//                                    param->value.byteArray->length);
//                } else {
//                    serializer->addJSUndefined();
//                }
            }

            std::unique_ptr<IPCBuffer> buffer = serializer->finish();

            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::INT32) {
                LOGE("execJS Unexpected result type");
                return false;
            }

            return result->get<jint>();

        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return false;
        }
    }

    std::unique_ptr<WeexJSResult> CoreSideInMultiProcess::ExecJSWithResult(
            const char *instanceId, const char *nameSpace, const char *func,
            std::vector<VALUE_WITH_TYPE *> &params) {
        std::unique_ptr<WeexJSResult> ret;
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS_WITH_RESULT));
            serializer->add(instanceId, strlen(instanceId));
            if (nameSpace)
                serializer->add(nameSpace, strlen(nameSpace));
            else {
                uint16_t tmp = 0;
                serializer->add(&tmp, 0);
            }
            serializer->add(func, strlen(func));

            for (int i = 0; i < params.size(); i++) {
                VALUE_WITH_TYPE *param = params[i];
                addParamsToIPCSerializer(serializer.get(), param);
//                if (param->type == ParamsType::DOUBLE) {
//                    serializer->add(param->value.doubleValue);
//                }  else if (param->type == ParamsType::STRING) {
//                    serializer->add(param->value.string->content,
//                                    param->value.string->length);
//                } else if(param->type == ParamsType::JSONSTRING) {
//                    serializer->addJSON(param->value.string->content,
//                                        param->value.string->length);
//                } else if (param->type == ParamsType::BYTEARRAY) {
//                    serializer->add(param->value.byteArray->content,
//                                    param->value.byteArray->length);
//                } else {
//                    serializer->addJSUndefined();
//                }
            }

            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::BYTEARRAY) {
                return ret;
            }
            if (result->getByteArrayLength() == 0) {
                return ret;
            }
            ret.reset(new WeexJSResult);
            ret->length = result->getByteArrayLength();
            char *string = new char[ret->length + 1];
            ret->data.reset(string);
            memset(string, 0, ret->length);
            memcpy(string, result->getByteArrayContent(), result->getByteArrayLength());
            string[ret->length] = '\0';

            return ret;

        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return ret;
        }
    }

    int CoreSideInMultiProcess::CreateInstance(const char *instanceId, const char *func,
                                           const char *script, const char *opts,
                                           const char *initData, const char *extendsApi,
                                           std::vector<INIT_FRAMEWORK_PARAMS*>& params,
                                           const char* render_strategy) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::CREATE_INSTANCE));
            serializer->add(instanceId, strlen(instanceId));
            serializer->add(func, strlen(func));
            serializer->add(script, strlen(script));
            serializer->add(opts, strlen(opts));
            serializer->add(initData, strlen(initData));
            serializer->add(extendsApi, strlen(extendsApi));
            for (auto it = params.begin(); it != params.end(); ++it) {
                serializer->add((*it)->type->content, (*it)->type->length);
                serializer->add((*it)->value->content, (*it)->value->length);
            }

            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::INT32) {
                LOGE("createInstanceContext Unexpected result type");
                return false;
            }
            return result->get<jint>();
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return false;
        }
    }

    std::unique_ptr<WeexJSResult> CoreSideInMultiProcess::ExecJSOnInstance(const char *instanceId,
                                               const char *script) {
        std::unique_ptr<WeexJSResult> ret;
        try {
            // base::debug::TraceScope traceScope("weex", "native_execJSOnInstance");
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS_ON_INSTANCE));

            serializer->add(instanceId, strlen(instanceId));
            serializer->add(script, strlen(script));

            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::BYTEARRAY) {
                // LOGE("native_execJSOnInstance return type error");
                return nullptr;
            }
            ret.reset(new WeexJSResult);
            ret->length = result->getByteArrayLength();
            ret->data.reset(new char[ret->length + 1]);
            void *dst = ret->data.get();
            memset(dst, 0, ret->length);
            memcpy(dst, result->getByteArrayContent(), result->getByteArrayLength());
            dst[ret->length] = '\0';
            return ret;
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return ret;
        }
    }

    int CoreSideInMultiProcess::DestroyInstance(const char *instanceId) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::DESTROY_INSTANCE));
            serializer->add(instanceId, strlen(instanceId));

            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
            if (result->getType() != IPCType::INT32) {
                LOGE("destoryInstance Unexpected result type");
                return false;
            }
            return result->get<jint>();
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
            // report crash here
            bridge()->platform_side()->ReportServerCrash(instanceId);
            return false;
        }
    }

    int CoreSideInMultiProcess::UpdateGlobalConfig(const char *config) {
        try {
            std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
            serializer->setMsg(static_cast<uint32_t>(IPCMsgFromPlatformToCore::UPDATE_GLOBAL_CONFIG));
            serializer->add(config, strlen(config));
            std::unique_ptr<IPCBuffer> buffer = serializer->finish();
            std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
        } catch (IPCException &e) {
            LOGE("%s", e.msg());
        }
        return true;
    }
}
