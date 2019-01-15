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
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVE));
                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // task args[1]
                serializer->add(task, strlen(task));
                // callback args[2]
                serializer->add(callback, strlen(callback));
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
            }

            std::unique_ptr<ValueWithType> CoreSideInMultiProcess::CallNativeModule(
                    const char *page_id, const char *module, const char *method,
                    const char *arguments, int arguments_length, const char *options,
                    int options_length) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVEMODULE));

                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // module args[1]
                serializer->add(module, strlen(module));
                // method args[2]
                serializer->add(method, strlen(method));
                // arguments args[3]
                serializer->add(arguments, arguments_length);
                // options args[4]
                serializer->add(options, options_length);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();

                auto ipc_result = sender->send(buffer.get());

                std::unique_ptr<ValueWithType> ret(new ValueWithType);

                switch (ipc_result->getType()) {
                    case IPCType::DOUBLE:
                        ret->type = ParamsType::DOUBLE;
                        ret->value.doubleValue = ipc_result->get<double>();
                        break;
                    case IPCType::STRING:
                        ret->type = ParamsType::STRING;
                        ret->value.string = genWeexStringSS(ipc_result->getStringContent(), ipc_result->getStringLength());
                        break;
                    case IPCType::JSONSTRING:
                        ret->type = ParamsType::JSONSTRING;
                        ret->value.string = genWeexStringSS(ipc_result->getStringContent(), ipc_result->getStringLength());
                        break;
                    case IPCType::BYTEARRAY: {
                        ret->type = ParamsType::BYTEARRAY;
                        ret->value.byteArray = genWeexByteArraySS(ipc_result->getByteArrayContent(), ipc_result->getByteArrayLength());
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
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLNATIVECOMPONENT));

                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // module args[1]
                serializer->add(ref, strlen(ref));
                // method args[2]
                serializer->add(method, strlen(method));
                // arguments args[3]
                serializer->add(arguments, arguments_length);
                // options args[4]
                serializer->add(options, options_length);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallNativeComponent: unexpected result: %d",
                         result->getType());
                }
            }

            void CoreSideInMultiProcess::AddElement(const char *page_id,
                                                    const char *parent_ref,
                                                    const char *dom_str, int dom_str_length,
                                                    const char *index_str) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLADDELEMENT) |
                                   MSG_FLAG_ASYNC);

                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // instacneID args[1]
                serializer->add(parent_ref, strlen(parent_ref));
                // dom node args[2]
                serializer->add(dom_str, dom_str_length);
                // index  args[3]
                serializer->add(index_str, strlen(index_str));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
            }

            void CoreSideInMultiProcess::SetTimeout(const char *callback_id,
                                                    const char *time) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::SETTIMEOUT));
                // callbackId
                serializer->add(callback_id, strlen(callback_id));
                // time
                serializer->add(time, strlen(time));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallNativeComponent: unexpected result: %d",
                         result->getType());
                }
            }

            void CoreSideInMultiProcess::NativeLog(const char *str_array) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::NATIVELOG) |
                                   MSG_FLAG_ASYNC);
                serializer->add(str_array, strlen(str_array));
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                sender->send(buffer.get());
            }

            void CoreSideInMultiProcess::CreateBody(const char *page_id,
                                                    const char *dom_str,
                                                    int dom_str_length) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEBODY));

                // page id
                serializer->add(page_id, strlen(page_id));
                // dom node args[2]
                serializer->add(dom_str, dom_str_length);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallNative: unexpected result: %d", result->getType());
                }
            }

            int CoreSideInMultiProcess::UpdateFinish(const char *page_id, const char *task,
                                                     int task_length, const char *callback,
                                                     int callback_length) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEFINISH));
                serializer->add(page_id, strlen(page_id));
                serializer->add(task, task_length);
                serializer->add(callback, callback_length);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallUpdateFinish: unexpected result: %d", result->getType());
                    return 0;
                }
                return result->get<int32_t>();
            }

            void CoreSideInMultiProcess::CreateFinish(const char *page_id) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLCREATEFINISH));

                serializer->add(page_id, strlen(page_id));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallCreateFinish: unexpected result: %d", result->getType());
                }
            }

            int CoreSideInMultiProcess::RefreshFinish(const char *page_id, const char *task,
                                                      const char *callback) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLREFRESHFINISH));
                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // task args[1]
                serializer->add(task, strlen(task));
                // callback args[2]
                serializer->add(callback, strlen(callback));
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallRefreshFinish: unexpected result: %d", result->getType());
                    return 0;
                }
                return result->get<int32_t>();
            }

            void CoreSideInMultiProcess::UpdateAttrs(const char *page_id, const char *ref,
                                                     const char *data, int data_length) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATEATTRS));
                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // ref args[1]
                serializer->add(ref, strlen(ref));
                // dom node args[2] TODO
                serializer->add(data, data_length);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallUpdateAttrs: unexpected result: %d", result->getType());
                }
            }

            void CoreSideInMultiProcess::UpdateStyle(const char *page_id, const char *ref,
                                                     const char *data, int data_length) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLUPDATESTYLE));
                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // ref args[1]
                serializer->add(ref, strlen(ref));
                // dom node styles args[2]
                serializer->add(data, data_length);

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallUpdateStyle: unexpected result: %d", result->getType());
                }
            }

            void CoreSideInMultiProcess::RemoveElement(const char *page_id,
                                                       const char *ref) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEELEMENT));
                serializer->add(page_id, strlen(page_id));
                serializer->add(ref, strlen(ref));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallRemoveElement: unexpected result: %d", result->getType());
                }
            }

            void CoreSideInMultiProcess::MoveElement(const char *page_id, const char *ref,
                                                     const char *parent_ref, int index) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLMOVEELEMENT));
                serializer->add(page_id, strlen(page_id));
                serializer->add(ref, strlen(ref));
                serializer->add(parent_ref, strlen(parent_ref));
                // Client need string
                auto temp = std::to_string(index);
                serializer->add(temp.c_str(), temp.size());

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallMoveElement: unexpected result: %d", result->getType());
                }
            }

            void CoreSideInMultiProcess::AddEvent(const char *page_id, const char *ref,
                                                  const char *event) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLADDEVENT));
                serializer->add(page_id, strlen(page_id));
                serializer->add(ref, strlen(ref));
                serializer->add(event, strlen(event));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallAddEvent: unexpected result: %d", result->getType());
                }
            }

            void CoreSideInMultiProcess::RemoveEvent(const char *page_id, const char *ref,
                                                     const char *event) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLREMOVEEVENT));
                serializer->add(page_id, strlen(page_id));
                serializer->add(ref, strlen(ref));
                serializer->add(event, strlen(event));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::INT32) {
                    LOGE("functionCallRemoveEvent: unexpected result: %d", result->getType());
                }
            }

            const char *CoreSideInMultiProcess::CallGCanvasLinkNative(
                    const char *context_id, int type, const char *arg) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLGCANVASLINK));

                // contextId args[0]
                serializer->add(context_id, strlen(context_id));
                // type args[1]
                serializer->add(type);
                // arg args[2]
                serializer->add(arg, strlen(arg));

                try {
                    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                    std::unique_ptr<IPCResult> result = sender->send(buffer.get());
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
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::SETINTERVAL));
                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // task args[1]
                serializer->add(callback_id, strlen(callback_id));
                // callback args[2]
                serializer->add(time, strlen(time));
                try {
                    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                    std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                    if (result->getType() != IPCType::INT32) {
                        LOGE("functionSetIntervalWeex: unexpected result: %d", result->getType());
                        return 0;
                    }
                    return result->get<int32_t>();
                } catch (IPCException &e) {
                    LOGE("functionSetIntervalWeex exception %s", e.msg());
                }
                return 0;
            }

            void CoreSideInMultiProcess::ClearInterval(const char *page_id,
                                                       const char *callback_id) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CLEARINTERVAL));
                // instacneID args[0]
                serializer->add(page_id, strlen(page_id));
                // task args[1]
                serializer->add(callback_id, strlen(callback_id));
                try {
                    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                    std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                } catch (IPCException &e) {
                    LOGE("functionClearIntervalWeex exception %s", e.msg());
                }
            }

            const char *CoreSideInMultiProcess::CallT3DLinkNative(int type,
                                                                  const char *arg) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::CALLT3DLINK));
                // type args[1]
                serializer->add(type);
                // arg args[2]
                serializer->add(arg, strlen(arg));
                try {
                    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                    std::unique_ptr<IPCResult> result = sender->send(buffer.get());
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
                } catch (IPCException &e) {
                    LOGE("functionT3DLinkNative exception: %s", e.msg());
                    _exit(1);
                }
                return NULL;
            }

            void CoreSideInMultiProcess::PostMessage(const char *vim_id, const char *data, int dataLength) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::POSTMESSAGE));
                serializer->add(data, dataLength);
                serializer->add(vim_id, strlen(vim_id));
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
            }

            void CoreSideInMultiProcess::DispatchMessage(const char *client_id,
                                                         const char *data,
                                                         int dataLength,
                                                         const char *callback,
                                                         const char *vm_id) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::DISPATCHMESSAGE));
                // clientid
                serializer->add(client_id, strlen(client_id));
                // data
                serializer->add(data, dataLength);
                // callback
                serializer->add(callback, strlen(callback));
                serializer->add(vm_id, strlen(vm_id));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
            }

            std::unique_ptr<WeexJSResult>
            CoreSideInMultiProcess::DispatchMessageSync(const char *client_id,
                                                        const char *data,
                                                        int dataLength,
                                                        const char *vm_id) {
              IPCSender *sender = client_->getSender();
              std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
              serializer->setMsg(
                  static_cast<uint32_t>(IPCProxyMsg::DISPATCHMESSAGESYNC));
              // clientid
              serializer->add(client_id, strlen(client_id));
              // data
              serializer->add(data, dataLength);
              serializer->add(vm_id, strlen(vm_id));

              std::unique_ptr<IPCBuffer> buffer = serializer->finish();
              std::unique_ptr<IPCResult> result = sender->send(buffer.get());

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
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::REPORTEXCEPTION));
                serializer->add(page_id, strlen(page_id));
                serializer->add(func, strlen(func));
                serializer->add(exception_string, strlen(exception_string));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::VOID) {
                    LOGE("REPORTEXCEPTION: unexpected result: %d", result->getType());
                }
            }

            void CoreSideInMultiProcess::SetJSVersion(const char *js_version) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::SETJSFVERSION));
                serializer->add(js_version, strlen(js_version));
                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                std::unique_ptr<IPCResult> result = sender->send(buffer.get());
                if (result->getType() != IPCType::VOID) {
                    LOGE("setJSFVersion: unexpected result: %d", result->getType());
                }
            }

            void CoreSideInMultiProcess::OnReceivedResult(long callback_id, std::unique_ptr<WeexJSResult>& result) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());;
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::ONRECEIVEDRESULT));
                serializer->add(static_cast<int64_t>(callback_id));
                if (result != nullptr) {
                    serializer->add(result->data.get(), result->length);
                } else {
                    serializer->addVoid();
                }

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                sender->send(buffer.get());
            }

            void CoreSideInMultiProcess::UpdateComponentData(const char* page_id, const char* cid, const char* json_data) {
                IPCSender *sender = client_->getSender();
                std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
                serializer->setMsg(static_cast<uint32_t>(IPCProxyMsg::UPDATECOMPONENTDATA));
                serializer->add(page_id, strlen(page_id));
                serializer->add(cid, strlen(cid));
                serializer->add(json_data, strlen(json_data));

                std::unique_ptr<IPCBuffer> buffer = serializer->finish();
                sender->send(buffer.get());
            }
        }  // namespace js
    }  // namespace bridge
}  // namespace weex
