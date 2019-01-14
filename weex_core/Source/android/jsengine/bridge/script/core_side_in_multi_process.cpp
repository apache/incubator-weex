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

#include "core_side_in_multi_process.h"
#include <unistd.h>
#include <string>
#include "android/jsengine/weex_ipc_server.h"
#include "android/jsengine/weex_jsc_utils.h"
#include "android/jsengine/object/weex_env.h"
#include "base/utils/log_utils.h"
#include "third_party/IPC/Buffering/IPCBuffer.h"
#include "third_party/IPC/IPCException.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/IPCSender.h"
#include "third_party/IPC/IPCType.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "third_party/IPC/IPCMessageJS.h"

namespace weex {
    namespace bridge {
        namespace js {
            CoreSideInMultiProcess::CoreSideInMultiProcess(WeexIPCClient *client)
                    : client_(client) {}

            CoreSideInMultiProcess::CoreSideInMultiProcess() : client_(NULL) {}

            CoreSideInMultiProcess::~CoreSideInMultiProcess() {}

            void CoreSideInMultiProcess::CallNative(const char *page_id, const char *task,
                                                    const char *callback) {
                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLNATIVE);
                ipc_task->addParams(page_id);
                ipc_task->addParams(task);
                ipc_task->addParams(callback);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            std::unique_ptr<ValueWithType> CoreSideInMultiProcess::CallNativeModule(
                    const char *page_id, const char *module, const char *method,
                    const char *arguments, int arguments_length, const char *options,
                    int options_length) {
                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLNATIVEMODULE);
                ipc_task->addParams(page_id);
                ipc_task->addParams(module);
                ipc_task->addParams(method);
                ipc_task->addParams(arguments, arguments_length);
                ipc_task->addParams(options, options_length);
                auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
                        new BackToWeexCoreQueue::Future());
                ipc_task->set_future(future.get());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);

                std::unique_ptr<IPCResult> ipc_result = future.get()->waitResult();

                std::unique_ptr<ValueWithType> ret(new ValueWithType);

                switch (ipc_result->getType()) {
                    case IPCType::DOUBLE:
                        ret->type = ParamsType::DOUBLE;
                        ret->value.doubleValue = ipc_result->get<double>();
                        break;
                    case IPCType::STRING:
                        ret->type = ParamsType::STRING;
                        ret->value.string = genWeexStringSS(ipc_result->getStringContent(),
                                                            ipc_result->getStringLength());
                        break;
                    case IPCType::JSONSTRING:
                        ret->type = ParamsType::JSONSTRING;
                        ret->value.string = genWeexStringSS(ipc_result->getStringContent(),
                                                            ipc_result->getStringLength());
                        break;
                    case IPCType::BYTEARRAY: {
                        ret->type = ParamsType::BYTEARRAY;
                        ret->value.byteArray = genWeexByteArraySS(ipc_result->getByteArrayContent(),
                                                                  ipc_result->getByteArrayLength());
                    }
                        break;
                    default:
                        ret->type = ParamsType::JSUNDEFINED;
                }
                return ret;
            }

            void CoreSideInMultiProcess::CallNativeComponent(
                    const char *page_id, const char *ref, const char *method,
                    const char *arguments, int arguments_length, const char *options,
                    int options_length) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLNATIVECOMPONENT);
                ipc_task->addParams(page_id);
                ipc_task->addParams(ref);
                ipc_task->addParams(method);
                ipc_task->addParams(arguments, arguments_length);
                ipc_task->addParams(options, options_length);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
//                if (result->getType() != IPCType::INT32) {
//                    LOGE("functionCallNativeComponent: unexpected result: %d",
//                         result->getType());
//                }
            }

            void CoreSideInMultiProcess::AddElement(const char *page_id,
                                                    const char *parent_ref,
                                                    const char *dom_str, int dom_str_length,
                                                    const char *index_str) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLADDELEMENT);
                ipc_task->addParams(page_id);
                ipc_task->addParams(parent_ref);
                ipc_task->addParams(dom_str, dom_str_length);
                ipc_task->addParams(index_str);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::SetTimeout(const char *callback_id,
                                                    const char *time) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::SETTIMEOUT);
                ipc_task->addParams(callback_id);
                ipc_task->addParams(time);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::NativeLog(const char *str_array) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::NATIVELOG);
                ipc_task->addParams(str_array);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::CreateBody(const char *page_id,
                                                    const char *dom_str,
                                                    int dom_str_length) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLCREATEBODY);
                ipc_task->addParams(page_id);
                ipc_task->addParams(dom_str, dom_str_length);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            int CoreSideInMultiProcess::UpdateFinish(const char *page_id, const char *task,
                                                     int task_length, const char *callback,
                                                     int callback_length) {


                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLUPDATEFINISH);
                ipc_task->addParams(page_id);
                ipc_task->addParams(task, task_length);
                ipc_task->addParams(callback, callback_length);
                auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
                        new BackToWeexCoreQueue::Future());
                ipc_task->set_future(future.get());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);

                std::unique_ptr<IPCResult> ipc_result = future.get()->waitResult();

                if (ipc_result->getType() != IPCType::INT32) {
                    LOGE("functionCallUpdateFinish: unexpected result: %d", ipc_result->getType());
                    return 0;
                }
                return ipc_result->get<int32_t>();
            }

            void CoreSideInMultiProcess::CreateFinish(const char *page_id) {


                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLCREATEFINISH);
                ipc_task->addParams(page_id);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            int CoreSideInMultiProcess::RefreshFinish(const char *page_id, const char *task,
                                                      const char *callback) {


                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLREFRESHFINISH);
                ipc_task->addParams(page_id);
                ipc_task->addParams(task);
                ipc_task->addParams(callback);
                auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
                        new BackToWeexCoreQueue::Future());
                ipc_task->set_future(future.get());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);

                std::unique_ptr<IPCResult> ipc_result = future.get()->waitResult();

                if (ipc_result->getType() != IPCType::INT32) {
                    LOGE("functionCallRefreshFinish: unexpected result: %d", ipc_result->getType());
                    return 0;
                }
                return ipc_result->get<int32_t>();
            }

            void CoreSideInMultiProcess::UpdateAttrs(const char *page_id, const char *ref,
                                                     const char *data, int data_length) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLUPDATEATTRS);
                ipc_task->addParams(page_id);
                ipc_task->addParams(ref);
                ipc_task->addParams(data, data_length);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::UpdateStyle(const char *page_id, const char *ref,
                                                     const char *data, int data_length) {


                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLUPDATESTYLE);
                ipc_task->addParams(page_id);
                ipc_task->addParams(ref);
                ipc_task->addParams(data, data_length);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::RemoveElement(const char *page_id,
                                                       const char *ref) {


                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLREMOVEELEMENT);
                ipc_task->addParams(page_id);
                ipc_task->addParams(ref);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::MoveElement(const char *page_id, const char *ref,
                                                     const char *parent_ref, int index) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLMOVEELEMENT);
                ipc_task->addParams(page_id);
                ipc_task->addParams(ref);
                ipc_task->addParams(parent_ref);
                auto temp = std::to_string(index);
                ipc_task->addParams(temp.c_str(), temp.length());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::AddEvent(const char *page_id, const char *ref,
                                                  const char *event) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLADDEVENT);
                ipc_task->addParams(page_id);
                ipc_task->addParams(ref);
                ipc_task->addParams(event);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::RemoveEvent(const char *page_id, const char *ref,
                                                     const char *event) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLREMOVEEVENT);
                ipc_task->addParams(page_id);
                ipc_task->addParams(ref);
                ipc_task->addParams(event);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            const char *CoreSideInMultiProcess::CallGCanvasLinkNative(
                    const char *context_id, int type, const char *arg) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLGCANVASLINK);
                ipc_task->addParams(context_id);
                auto temp = std::to_string(type);
                ipc_task->addParams(temp.c_str(), temp.length());
                ipc_task->addParams(arg);
                auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
                        new BackToWeexCoreQueue::Future());
                ipc_task->set_future(future.get());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);

                std::unique_ptr<IPCResult> result = future.get()->waitResult();


                try {
                    // LOGE("weexjsc functionGCanvasLinkNative");
                    // if (result->getType() == IPCType::VOID) {
                    //     return JSValue::encode(ret);
                    // } else if (result->getStringLength() > 0) {
                    //     WTF::String retWString = jString2String(result->getStringContent(),
                    //     result->getStringLength()); LOGE("weexjsc functionGCanvasLinkNative
                    //     result length > 1 retWString:%s", retWString.utf8().data()); ret =
                    //     String2JSValue(state, retWString);

                    // }
                    if (result->getType() != IPCType::VOID) {
                        if (result->getStringLength() > 0) {
                            return jString2String(result->getStringContent(),
                                                  result->getStringLength())
                                    .utf8()
                                    .data();
                        }
                    }
                } catch (IPCException &e) {
                    LOGE("functionGCanvasLinkNative exception: %s", e.msg());
                    _exit(1);
                }
                return NULL;
            }

            int CoreSideInMultiProcess::SetInterval(const char *page_id,
                                                    const char *callback_id,
                                                    const char *time) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::SETINTERVAL);
                ipc_task->addParams(page_id);
                ipc_task->addParams(callback_id);
                ipc_task->addParams(time);
                auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
                        new BackToWeexCoreQueue::Future());
                ipc_task->set_future(future.get());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);

                std::unique_ptr<IPCResult> result = future.get()->waitResult();
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionSetIntervalWeex: unexpected result: %d", result->getType());
                    return 0;
                }
                return result->get<int32_t>();
            }

            void CoreSideInMultiProcess::ClearInterval(const char *page_id,
                                                       const char *callback_id) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CLEARINTERVAL);
                ipc_task->addParams(page_id);
                ipc_task->addParams(callback_id);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            const char *CoreSideInMultiProcess::CallT3DLinkNative(int type,
                                                                  const char *arg) {


                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::CALLT3DLINK);
                auto temp = std::to_string(type);
                ipc_task->addParams(temp.c_str(), temp.length());
                ipc_task->addParams(arg);
                auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
                        new BackToWeexCoreQueue::Future());
                ipc_task->set_future(future.get());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);

                std::unique_ptr<IPCResult> result = future.get()->waitResult();
                // LOGE("weexjsc functionT3DLinkNative");
                // if (result->getType() == IPCType::VOID) {
                //     return JSValue::encode(ret);
                // } else if (result->getStringLength() > 0) {
                //     WTF::String retWString = jString2String(result->getStringContent(),
                //     result->getStringLength()); LOGE("weexjsc functionT3DLinkNative
                //     result length > 1 retWString:%s", retWString.utf8().data()); ret =
                //     String2JSValue(state, retWString);

                // }
                if (result->getType() != IPCType::VOID) {
                    if (result->getStringLength() > 0) {
                        return jString2String(result->getStringContent(),
                                              result->getStringLength())
                                .utf8()
                                .data();
                    }
                }
                return nullptr;
            }

            void CoreSideInMultiProcess::PostMessage(const char *vim_id, const char *data,
                                                     int dataLength) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::POSTMESSAGE);
                ipc_task->addParams(data, dataLength);
                ipc_task->addParams(vim_id);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::DispatchMessage(const char *client_id,
                                                         const char *data,
                                                         int dataLength,
                                                         const char *callback,
                                                         const char *vm_id) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::DISPATCHMESSAGE);
                ipc_task->addParams(client_id);
                ipc_task->addParams(data, dataLength);
                ipc_task->addParams(callback);
                ipc_task->addParams(vm_id);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            std::unique_ptr<WeexJSResult>
            CoreSideInMultiProcess::DispatchMessageSync(const char *client_id,
                                                        const char *data,
                                                        int dataLength,
                                                        const char *vm_id) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::DISPATCHMESSAGESYNC);
                ipc_task->addParams(client_id);
                ipc_task->addParams(data, dataLength);
                ipc_task->addParams(vm_id);
                auto future = std::unique_ptr<BackToWeexCoreQueue::Future>(
                        new BackToWeexCoreQueue::Future());
                ipc_task->set_future(future.get());
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);

                std::unique_ptr<IPCResult> result = future.get()->waitResult();
                char *copy = nullptr;
                int length = 0;
                if (result->getType() == IPCType::BYTEARRAY) {
                    length = result->getByteArrayLength();
                    copy = new char[length];
                    strcpy(copy, result->getByteArrayContent());
                }
                return std::unique_ptr<WeexJSResult>(
                        new WeexJSResult(std::unique_ptr<char[]>(copy), length));
            }

            void CoreSideInMultiProcess::ReportException(const char *page_id,
                                                         const char *func,
                                                         const char *exception_string) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::REPORTEXCEPTION);
                ipc_task->addParams(page_id);
                ipc_task->addParams(func);
                ipc_task->addParams(exception_string);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::SetJSVersion(const char *js_version) {

                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::SETJSFVERSION);
                ipc_task->addParams(js_version);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::OnReceivedResult(long callback_id,
                                                          std::unique_ptr<WeexJSResult> &result) {
                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::ONRECEIVEDRESULT);

                auto temp = std::to_string(callback_id);
                ipc_task->addParams(temp.c_str(), temp.length());
                if (result != nullptr) {
                    ipc_task->addParams(result->data.get(), result->length);
                }
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }

            void CoreSideInMultiProcess::UpdateComponentData(const char *page_id, const char *cid,
                                                             const char *json_data) {


                BackToWeexCoreQueue::IPCTask *ipc_task = new BackToWeexCoreQueue::IPCTask(
                        IPCProxyMsg::UPDATECOMPONENTDATA);
                ipc_task->addParams(page_id);
                ipc_task->addParams(cid);
                ipc_task->addParams(json_data);
                WeexEnv::getEnv()->m_back_to_weex_core_thread.get()->addTask(ipc_task);
            }
        }  // namespace js
    }  // namespace bridge
}  // namespace weex
