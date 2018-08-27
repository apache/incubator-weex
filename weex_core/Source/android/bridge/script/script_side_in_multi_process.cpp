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

#include "script_side_in_multi_process.h"
#include "IPC/Buffering/IPCBuffer.h"
#include "IPC/IPCException.h"
#include "IPC/IPCMessageJS.h"
#include "IPC/IPCSender.h"
#include "IPC/Serializing/IPCSerializer.h"
#include "android/base/log_utils.h"
#include "android/bridge/multi_process_and_so_initializer.h"
#include "core/manager/weex_core_manager.h"

namespace WeexCore {
namespace bridge {
namespace script {
ScriptSideInMultiProcess::ScriptSideInMultiProcess():sender_(nullptr){}

ScriptSideInMultiProcess::~ScriptSideInMultiProcess() {}

int ScriptSideInMultiProcess::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  try {
    if(sender_ == nullptr) {
      LOGE("InitFramework sender is null");
      return false;
    }


    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::INITFRAMEWORK));
    serializer->add(script, strlen(script));
    for (auto it = params.begin(); it != params.end(); ++it) {
      serializer->add((*it)->type->content, (*it)->type->length);
      serializer->add((*it)->value->content, (*it)->value->length);
    }
    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    if (result->getType() != IPCType::INT32) {
      LOGE("initFramework Unexpected result type");
      bridge()->core_side()->ReportException(
          "", "initFramework", "error, initFramework Unexpected result type");
      return false;
    }
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    return false;
  }
  return true;
}

int ScriptSideInMultiProcess::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  try {
    if(sender_ == nullptr) {
      LOGE("InitAppFramework sender is null");
      return false;
    }

    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::INITAPPFRAMEWORK));
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
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return false;
  }
}

int ScriptSideInMultiProcess::CreateAppContext(const char *instanceId,
                                               const char *jsBundle) {
  try {

    if(sender_ == nullptr) {
      LOGE("CreateAppContext sender is null");
      return false;
    }

    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::CREATEAPPCONTEXT));
    serializer->add(instanceId, strlen(instanceId));
    serializer->add(jsBundle, strlen(jsBundle));
    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    return result->get<jint>();
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    // report crash here
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return false;
  }
}

char *ScriptSideInMultiProcess::ExecJSOnAppWithResult(const char *instanceId,
                                                      const char *jsBundle) {
  try {
    if(sender_ == nullptr) {
      LOGE("ExecJSOnAppWithResult sender is null");
      return false;
    }
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSONAPPWITHRESULT));
    serializer->add(instanceId, strlen(instanceId));
    serializer->add(jsBundle, strlen(jsBundle));
    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    if (result->getType() != IPCType::BYTEARRAY) {
      return NULL;
    }
    if (result->getByteArrayLength() == 0) {
      return NULL;
    }
    return const_cast<char *>(result->getByteArrayContent());
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    // report crash here
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return nullptr;
  }
}

int ScriptSideInMultiProcess::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  try {

    if(sender_ == nullptr) {
      LOGE("CallJSOnAppContext sender is null");
      return false;
    }

    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::CALLJSONAPPCONTEXT));
    serializer->add(instanceId, strlen(instanceId));
    serializer->add(func, strlen(func));

    for (int i = 0; i < params.size(); i++) {
      VALUE_WITH_TYPE *param = params[i];

      if (param->type == ParamsType::DOUBLE) {
        serializer->add(param->value.doubleValue);
      } else if (param->type == ParamsType::STRING) {
        serializer->add(param->value.string->content,
                        param->value.string->length);
      } else if (param->type == ParamsType::JSONSTRING) {
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
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return false;
  }
  return true;
}

int ScriptSideInMultiProcess::DestroyAppContext(const char *instanceId) {
  try {
    if(sender_ == nullptr) {
      LOGE("DestroyAppContext sender is null");
      return false;
    }
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::DESTORYAPPCONTEXT));
    serializer->add(instanceId, strlen(instanceId));
    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    return true;
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    // report crash here
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return false;
  }
}

int ScriptSideInMultiProcess::ExecJsService(const char *source) {
  try {
    if(sender_ == nullptr) {
      LOGE("ExecJsService sender is null");
      return false;
    }
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSSERVICE));
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

int ScriptSideInMultiProcess::ExecTimeCallback(const char *source) { return 0; }

int ScriptSideInMultiProcess::ExecJS(const char *instanceId,
                                     const char *nameSpace, const char *func,
                                     std::vector<VALUE_WITH_TYPE *> &params) {
  try {
    if(sender_ == nullptr) {
      LOGE("ExecJS sender is null");
      return false;
    }
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJS));
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

      if (param->type == ParamsType::DOUBLE) {
        serializer->add(param->value.doubleValue);
      } else if (param->type == ParamsType::STRING) {
        serializer->add(param->value.string->content,
                        param->value.string->length);
      } else if (param->type == ParamsType::JSONSTRING) {
        serializer->addJSON(param->value.string->content,
                            param->value.string->length);
      } else if (param->type == ParamsType::BYTEARRAY) {
        serializer->add(param->value.byteArray->content,
                        param->value.byteArray->length);
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
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return false;
  }
}

WeexJSResult ScriptSideInMultiProcess::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  try {
    if(sender_ == nullptr) {
      LOGE("ExecJSWithResult sender is null");
      return WeexJSResult();
    }
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    //    std::vector<VALUE_WITH_TYPE *> params;
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSWITHRESULT));
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

      if (param->type == ParamsType::DOUBLE) {
        serializer->add(param->value.doubleValue);
      } else if (param->type == ParamsType::STRING) {
        serializer->add(param->value.string->content,
                        param->value.string->length);
      } else if (param->type == ParamsType::JSONSTRING) {
        serializer->addJSON(param->value.string->content,
                            param->value.string->length);
      } else if (param->type == ParamsType::BYTEARRAY) {
        serializer->add(param->value.byteArray->content,
                        param->value.byteArray->length);
      } else {
        serializer->addJSUndefined();
      }
    }

    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    if (result->getType() != IPCType::BYTEARRAY) {
      return WeexJSResult();
    }
    if (result->getByteArrayLength() == 0) {
      return WeexJSResult();
    }
    WeexJSResult weex_js_result;
    weex_js_result.data = const_cast<char *>(result->getByteArrayContent());
    weex_js_result.length = result->getByteArrayLength();
    return weex_js_result;
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    // report crash here
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return WeexJSResult();
  }
}

int ScriptSideInMultiProcess::CreateInstance(
    const char *instanceId, const char *func, const char *script,
    const char *opts, const char *initData, const char *extendsApi) {
  try {

    if(sender_ == nullptr) {
      LOGE("CreateInstance sender is null");
      return false;
    }

    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::CREATEINSTANCE));
    serializer->add(instanceId, strlen(instanceId));
    serializer->add(func, strlen(func));
    serializer->add(script, strlen(script));
    serializer->add(opts, strlen(opts));
    serializer->add(initData, strlen(initData));
    serializer->add(extendsApi, strlen(extendsApi));

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
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return false;
  }
}

char *ScriptSideInMultiProcess::ExecJSOnInstance(const char *instanceId,
                                                 const char *script) {
  try {
    // base::debug::TraceScope traceScope("weex", "native_execJSOnInstance");
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::EXECJSONINSTANCE));

    serializer->add(instanceId, strlen(instanceId));
    serializer->add(script, strlen(script));

    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
    if (result->getType() != IPCType::BYTEARRAY) {
      // LOGE("native_execJSOnInstance return type error");
      return nullptr;
    }
    return const_cast<char *>(result->getByteArrayContent());
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
    // report crash here
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return nullptr;
  }
}

int ScriptSideInMultiProcess::DestroyInstance(const char *instanceId) {
  try {
    if(sender_ == nullptr) {
      LOGE("DestroyInstance sender is null");
      return false;
    }
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::DESTORYINSTANCE));
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
    WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->ReportServerCrash(instanceId);
    return false;
  }
}

int ScriptSideInMultiProcess::UpdateGlobalConfig(const char *config) {
  try {
    if(sender_ == nullptr) {
      LOGE("UpdateGlobalConfig sender is null");
      return false;
    }
    std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
    serializer->setMsg(static_cast<uint32_t>(IPCJSMsg::UPDATEGLOBALCONFIG));
    serializer->add(config, strlen(config));
    std::unique_ptr<IPCBuffer> buffer = serializer->finish();
    std::unique_ptr<IPCResult> result = sender_->send(buffer.get());
  } catch (IPCException &e) {
    LOGE("%s", e.msg());
  }
  return true;
}
}  // namespace script
}  // namespace bridge
}  // namespace WeexCore