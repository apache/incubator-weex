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
// Created by yxp on 2018/6/12.
//

#include "platform_bridge_in_multi_process.h"
#include <cstring>
#include "android/jsengine/bridge/platform/core_side_in_simple.h"
#include "android/jsengine/bridge/platform/platform_side_multi_process.h"
#include "android/jsengine/bridge/script/script_bridge_in_simple.h"
#include "android/jsengine/utils/Utils.h"
#include "base/android/log_utils.h"
#include "core/config/core_environment.h"
#include "core/manager/weex_core_manager.h"
#include "third_party/IPC/IPCArguments.h"
#include "third_party/IPC/IPCByteArray.h"
#include "third_party/IPC/IPCHandler.h"
#include "third_party/IPC/IPCMessageJS.h"
#include "third_party/IPC/IPCResult.h"

namespace weex {
PlatformBridgeInMultiProcess* PlatformBridgeInMultiProcess::g_instance = NULL;

PlatformBridgeInMultiProcess::PlatformBridgeInMultiProcess() {
  set_core_side(new CoreSideInSimple);
  set_platform_side(new PlatformSideInMultiProcess);
}

PlatformBridgeInMultiProcess::~PlatformBridgeInMultiProcess() {}

void PlatformBridgeInMultiProcess::RegisterIPCCallback(IPCHandler* handler) {
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::SET_DEFAULT_HEIGHT_AND_WIDTH_INTO_ROOT_DOM),
      SetDefaultHeightAndWidthIntoRootDom);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::ON_INSTANCE_CLOSE),
      OnInstanceClose);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_STYLE_WIDTH),
      SetStyleWidth);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_STYLE_HEIGHT),
      SetStyleHeight);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_MARGIN), SetMargin);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_PADDING), SetPadding);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_POSITION),
      SetPosition);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::MARK_DIRTY), MarkDirty);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_VIEWPORT_WIDTH),
      SetViewPortWidth);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::FORCE_LAYOUT),
      ForceLayout);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::NOTIFY_LAYOUT),
      NotifyLayout);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::GET_FIRST_SCREEN_RENDER_TIME),
      GetFirstScreenRenderTime);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::GET_RENDER_FINISH_TIME),
      GetRenderFinishTime);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::SET_RENDER_CONTAINER_WRAP_CONTENT),
      SetRenderContainerWrapContent);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::BIND_MEASUREMENT_TO_RENDER_OBJECT),
      BindMeasurementToRenderObject);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::REGISTER_CORE_ENV),
      RegisterCoreEnv);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::GET_RENDER_OBJECT),
      GetRenderObject);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::UPDATE_RENDER_OBJECT_STYLE),
      UpdateRenderObjectStyle);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::UPDATE_RENDER_OBJECT_ATTR),
      UpdateRenderObjectAttr);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::COPY_RENDER_OBJECT),
      CopyRenderObject);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::SET_MEASURE_FUNCTION_ADAPTER),
      SetMeasureFunctionAdapter);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::SET_PLATFORM),
      SetPlatform);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::SET_DEVICE_WIDTH_AND_HEIGHT),
      SetDeviceWidthAndHeight);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::ADD_OPTION), AddOption);

  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::INIT_FRAMEWORK),
      InitFramework);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::INIT_APP_FRAMEWORK),
      InitAppFramework);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::CREATE_APP_CONTEXT),
      CreateAppContext);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromPlatformToCore::EXEC_JS_ON_APP_WITH_RESULT),
      ExecJSOnAppWithResult);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::CALL_JS_ON_APP_CONTEXT),
      CallJSOnAppContext);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::DESTROY_APP_CONTEXT),
      DestroyAppContext);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS_SERVICE),
      ExecJSService);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_TIME_CALLBACK),
      ExecTimerCallback);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS), ExecJS);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS_WITH_RESULT),
      ExecJSWithResult);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::CREATE_INSTANCE),
      CreateInstance);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::DESTROY_INSTANCE),
      DestroyInstance);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::EXEC_JS_ON_INSTANCE),
      ExecJSOnInstance);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromPlatformToCore::UPDATE_GLOBAL_CONFIG),
      UpdateGlobalConfig);

  handler->registerHandler(
        static_cast<uint32_t>(IPCMsgFromPlatformToCore::UpdateInitFrameworkParams),
        UpdateInitFrameworkParams);
}

std::unique_ptr<IPCResult>
PlatformBridgeInMultiProcess::SetDefaultHeightAndWidthIntoRootDom(
    IPCArguments* arguments) {
  LOGE("IPC SetDefaultHeightAndWidthIntoRootDom");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  float default_width = arguments->get<float>(1);
  float default_height = arguments->get<float>(2);
  bool is_width_wrap_content = arguments->get<bool>(3);
  bool is_height_wrap_content = arguments->get<bool>(4);
  Instance()->core_side()->SetDefaultHeightAndWidthIntoRootDom(
      instance_id, default_width, default_height, is_width_wrap_content,
      is_height_wrap_content);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::OnInstanceClose(
    IPCArguments* arguments) {
  LOGE("IPC OnInstanceClose");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  Instance()->core_side()->OnInstanceClose(instance_id);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::SetStyleWidth(
    IPCArguments* arguments) {
  LOGE("IPC SetStyleWidth");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  std::string render_ref(arguments->getByteArray(1)->content,
                         arguments->getByteArray(1)->length);
  float width = arguments->get<float>(2);
  Instance()->core_side()->SetStyleWidth(instance_id, render_ref, width);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::SetStyleHeight(
    IPCArguments* arguments) {
  LOGE("IPC SetStyleHeight");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  std::string render_ref(arguments->getByteArray(1)->content,
                         arguments->getByteArray(1)->length);
  float height = arguments->get<float>(2);
  Instance()->core_side()->SetStyleHeight(instance_id, render_ref, height);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::SetMargin(
    IPCArguments* arguments) {
  LOGE("IPC SetMargin");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  std::string render_ref(arguments->getByteArray(1)->content,
                         arguments->getByteArray(1)->length);
  int edge = arguments->get<int32_t>(2);
  float value = arguments->get<float>(2);
  Instance()->core_side()->SetMargin(instance_id, render_ref, edge, value);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::SetPadding(
    IPCArguments* arguments) {
  LOGE("IPC SetPadding");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  std::string render_ref(arguments->getByteArray(1)->content,
                         arguments->getByteArray(1)->length);
  int edge = arguments->get<int32_t>(2);
  float value = arguments->get<float>(2);
  Instance()->core_side()->SetPadding(instance_id, render_ref, edge, value);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::SetPosition(
    IPCArguments* arguments) {
  LOGE("IPC SetPosition");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  std::string render_ref(arguments->getByteArray(1)->content,
                         arguments->getByteArray(1)->length);
  int edge = arguments->get<int32_t>(2);
  float value = arguments->get<float>(3);
  Instance()->core_side()->SetPadding(instance_id, render_ref, edge, value);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::MarkDirty(
    IPCArguments* arguments) {
  LOGE("IPC MarkDirty");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  std::string render_ref(arguments->getByteArray(1)->content,
                         arguments->getByteArray(1)->length);
  Instance()->core_side()->MarkDirty(instance_id, render_ref);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::SetViewPortWidth(
    IPCArguments* arguments) {
  LOGE("IPC SetViewPortWidth");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  float width = arguments->get<float>(1);
  Instance()->core_side()->SetViewPortWidth(instance_id, width);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::ForceLayout(
    IPCArguments* arguments) {
  LOGE("IPC ForceLayout");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  Instance()->core_side()->ForceLayout(instance_id);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::NotifyLayout(
    IPCArguments* arguments) {
  LOGE("IPC NotifyLayout");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  auto result = Instance()->core_side()->NotifyLayout(instance_id);
  return createInt32Result(result);
}

std::unique_ptr<IPCResult>
PlatformBridgeInMultiProcess::GetFirstScreenRenderTime(
    IPCArguments* arguments) {
  LOGE("IPC GetFirstScreenRenderTime");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  auto result = Instance()->core_side()->GetFirstScreenRenderTime(instance_id);
  if (result.empty()) {
    return createVoidResult();
  } else {
    int64_t temp[]{result[0], result[1], result[2]};
    return createByteArrayResult(reinterpret_cast<const char*>(temp),
                                 3 * sizeof(int64_t) / sizeof(char));
  }
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::GetRenderFinishTime(
    IPCArguments* arguments) {
  LOGE("IPC GetRenderFinishTime");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  auto result = Instance()->core_side()->GetRenderFinishTime(instance_id);
  if (result.empty()) {
    return createVoidResult();
  } else {
    int64_t temp[]{result[0], result[1], result[2]};
    return createByteArrayResult(reinterpret_cast<const char*>(temp),
                                 3 * sizeof(int64_t) / sizeof(char));
  }
}

std::unique_ptr<IPCResult>
PlatformBridgeInMultiProcess::SetRenderContainerWrapContent(
    IPCArguments* arguments) {
  LOGE("IPC SetRenderContainerWrapContent");
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  bool wrap = arguments->get<bool>(1);
  Instance()->core_side()->SetRenderContainerWrapContent(instance_id, wrap);
  return createVoidResult();
}

std::unique_ptr<IPCResult>
PlatformBridgeInMultiProcess::BindMeasurementToRenderObject(
    IPCArguments* arguments) {
  LOGE("IPC BindMeasurementToRenderObject");
  long ptr = arguments->get<int64_t>(0);
  Instance()->core_side()->BindMeasurementToRenderObject(ptr);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::RegisterCoreEnv(
    IPCArguments* arguments) {
  LOGE("IPC RegisterCoreEnv");
  std::string key(arguments->getByteArray(0)->content,
                  arguments->getByteArray(0)->length);
  std::string value(arguments->getByteArray(1)->content,
                    arguments->getByteArray(1)->length);
  Instance()->core_side()->RegisterCoreEnv(key, value);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::GetRenderObject(
    IPCArguments* arguments) {
  std::string instance_id(arguments->getByteArray(0)->content,
                          arguments->getByteArray(0)->length);
  std::string render_ref(arguments->getByteArray(1)->content,
                         arguments->getByteArray(1)->length);
  auto result =
      Instance()->core_side()->GetRenderObject(instance_id, render_ref);
  LOGE("IPC GetRenderObject %d", result);
  return createInt64Result(result);
}
std::unique_ptr<IPCResult>
PlatformBridgeInMultiProcess::UpdateRenderObjectStyle(IPCArguments* arguments) {
  LOGE("IPC UpdateRenderObjectStyle");
  long ptr = arguments->get<int64_t>(0);
  std::string key(arguments->getByteArray(1)->content,
                  arguments->getByteArray(1)->length);
  std::string value(arguments->getByteArray(2)->content,
                    arguments->getByteArray(2)->length);
  Instance()->core_side()->UpdateRenderObjectStyle(ptr, key, value);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::UpdateRenderObjectAttr(
    IPCArguments* arguments) {
  LOGE("IPC UpdateRenderObjectAttr");
  long ptr = arguments->get<int64_t>(0);
  std::string key(arguments->getByteArray(1)->content,
                  arguments->getByteArray(1)->length);
  std::string value(arguments->getByteArray(2)->content,
                    arguments->getByteArray(2)->length);
  Instance()->core_side()->UpdateRenderObjectAttr(ptr, key, value);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::CopyRenderObject(
    IPCArguments* arguments) {
  LOGE("IPC CopyRenderObject");
  long ptr = arguments->get<int64_t>(0);
  auto result = Instance()->core_side()->CopyRenderObject(ptr);
  return createInt64Result(result);
}

std::unique_ptr<IPCResult>
PlatformBridgeInMultiProcess::SetMeasureFunctionAdapter(
    IPCArguments* arguments) {
  LOGE("IPC SetMeasureFunctionAdapter");
  Instance()->core_side()->SetMeasureFunctionAdapter();
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::SetPlatform(
    IPCArguments* arguments) {
  LOGE("IPC SetPlatform");
  std::string platform(arguments->getByteArray(0)->content,
                       arguments->getByteArray(0)->length);
  WeexCore::WXCoreEnvironment::getInstance()->SetPlatform(platform);
  return createVoidResult();
}

std::unique_ptr<IPCResult>
PlatformBridgeInMultiProcess::SetDeviceWidthAndHeight(IPCArguments* arguments) {
  LOGE("IPC SetDeviceWidthAndHeight");
  float width = arguments->get<float>(0);
  float height = arguments->get<float>(1);
  WeexCore::WXCoreEnvironment::getInstance()->set_device_width(width);
  WeexCore::WXCoreEnvironment::getInstance()->set_device_height(height);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::AddOption(
    IPCArguments* arguments) {
  LOGE("IPC AddOption");
  std::string key(arguments->getByteArray(0)->content,
                  arguments->getByteArray(0)->length);
  std::string value(arguments->getByteArray(1)->content,
                    arguments->getByteArray(1)->length);
  WeexCore::WXCoreEnvironment::getInstance()->AddOption(key, value);
  return createVoidResult();
}

static void FillVectorOfValueWithType(std::vector<VALUE_WITH_TYPE*>& params,
                                      IPCArguments* arguments, size_t start,
                                      size_t end) {
  for (size_t i = start; i < end; ++i) {
    auto value = new ValueWithType;
    switch (arguments->getType(i)) {
      case IPCType::DOUBLE: {
        value->type = ParamsType::DOUBLE;
        value->value.doubleValue = arguments->get<double>(i);
      } break;
      case IPCType::STRING: {
        const IPCString* ipcstr = arguments->getString(i);
        size_t size = ipcstr->length * sizeof(uint16_t);
        auto temp = (WeexString*)malloc(size + sizeof(WeexString));
        memset(temp, 0, size + sizeof(WeexString));
        temp->length = ipcstr->length;
        memcpy(temp->content, ipcstr->content, size);
        value->type = ParamsType::STRING;
        value->value.string = temp;
      } break;
      case IPCType::JSONSTRING: {
        const IPCString* ipcstr = arguments->getString(i);
        size_t size = ipcstr->length * sizeof(uint16_t);
        auto temp = (WeexString*)malloc(size + sizeof(WeexString));
        memset(temp, 0, size + sizeof(WeexString));
        temp->length = ipcstr->length;
        memcpy(temp->content, ipcstr->content, size);
        value->type = ParamsType::JSONSTRING;
        value->value.string = temp;
      } break;
      case IPCType::BYTEARRAY: {
        const IPCByteArray* array = arguments->getByteArray(i);
        size_t size = array->length * sizeof(char);
        auto temp = (WeexByteArray*)malloc(size + sizeof(WeexByteArray));
        memset(temp, 0, size + sizeof(WeexByteArray));
        temp->length = array->length;
        memcpy(temp->content, array->content, size);
        value->type = ParamsType::BYTEARRAY;
        value->value.byteArray = temp;
      } break;
      default: { value->type = ParamsType::JSUNDEFINED; } break;
    }
    params.push_back(value);
  }
}

static void ClearVectorOfValueWithType(std::vector<VALUE_WITH_TYPE*>& params) {
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

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::InitFramework(
    IPCArguments* arguments) {
  LOGE("IPC InitFramework");
  // Source
  const char* source = arguments->getByteArray(0)->content;
  // Params
  size_t startCount = 1;
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
  return createInt32Result(
      Instance()->core_side()->InitFramework(source, params));
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::InitAppFramework(
    IPCArguments* arguments) {
  const char* id = arguments->getByteArray(0)->content;
  const char* js = arguments->getByteArray(1)->content;
  LOGE("Weex jsserver IPCJSMsg::INITAPPFRAMEWORK id:%s", id);
  if (strcmp(id, "") == 0) {
    return createInt32Result(static_cast<int32_t>(false));
  }
  // Params
  size_t startCount = 2;
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
  return createInt32Result(
      Instance()->core_side()->InitAppFramework(id, js, params));
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::CreateAppContext(
    IPCArguments* arguments) {
  LOGE("IPC CreateAppContext");
  // LOGE("Weex jsserver IPCJSMsg::CREATEAPPCONTEXT start");
  const char* instanceID = arguments->getByteArray(0)->content;
  const char* js = arguments->getByteArray(1)->content;
  // LOGE("Weex jsserver IPCJSMsg::CREATEAPPCONTEXT end");
  return createInt32Result(
      Instance()->core_side()->CreateAppContext(instanceID, js));
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::ExecJSOnAppWithResult(
    IPCArguments* arguments) {
  LOGE("IPC ExecJSONAppWithResult");
  // LOGE("Weex jsserver IPCJSMsg::ExecJSONAppWithResult start");
  const char* instanceID = arguments->getByteArray(0)->content;
  const char* js = arguments->getByteArray(1)->content;
  char* result = const_cast<char*>(
      Instance()->core_side()->ExecJSOnAppWithResult(instanceID, js));

  if (result == nullptr)
    return createByteArrayResult(nullptr, 0);
  else
    return createCharArrayResult(result);
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::CallJSOnAppContext(
    IPCArguments* arguments) {
  LOGE("IPC CallJSONAppContext");
  // LOGE("Weex jsserver IPCJSMsg::CallJSONAppContext start");
  const char* instanceId = arguments->getByteArray(0)->content;
  const char* func = arguments->getByteArray(1)->content;

  std::vector<VALUE_WITH_TYPE*> params;
  FillVectorOfValueWithType(params, arguments, 2, arguments->getCount());
  auto result =
      Instance()->core_side()->CallJSOnAppContext(instanceId, func, params);
  ClearVectorOfValueWithType(params);
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::DestroyAppContext(
    IPCArguments* arguments) {
  LOGE("IPC DestroyAppContext");
  // LOGE("Weex jsserver IPCJSMsg::DestroyAppContext start");
  const char* instanceID = arguments->getByteArray(0)->content;
  if (strlen(instanceID) == 0) {
    return createInt32Result(static_cast<int32_t>(false));
  }

  return createInt32Result(
      Instance()->core_side()->DestroyAppContext(instanceID));
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::ExecJSService(
    IPCArguments* arguments) {
  LOGE("IPC ExecJSService");
  const char* source = arguments->getByteArray(0)->content;
  return createInt32Result(Instance()->core_side()->ExecJsService(source));
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::ExecTimerCallback(
    IPCArguments* arguments) {
  LOGE("IPC ExecTimerCallback");
  const char* source = arguments->getByteArray(0)->content;
  Instance()->core_side()->ExecTimeCallback(source);
  return createVoidResult();
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::ExecJS(
    IPCArguments* arguments) {
  LOGE("IPC ExecJS");
  const char* instanceId = arguments->getByteArray(0)->content;
  const char* namespaceStr = arguments->getByteArray(1)->content;
  const char* func = arguments->getByteArray(2)->content;

  std::vector<VALUE_WITH_TYPE*> params;
  FillVectorOfValueWithType(params, arguments, 3, arguments->getCount());
  auto result =
      Instance()->core_side()->ExecJS(instanceId, namespaceStr, func, params);
  ClearVectorOfValueWithType(params);
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::ExecJSWithResult(
    IPCArguments* arguments) {
  LOGE("IPC ExecJSWithResult");
  const char* instanceId = arguments->getByteArray(0)->content;
  const char* namespaceStr = arguments->getByteArray(1)->content;
  const char* func = arguments->getByteArray(2)->content;

  std::vector<VALUE_WITH_TYPE*> params;
  FillVectorOfValueWithType(params, arguments, 3, arguments->getCount());
  WeexJSResult jsResult = Instance()->core_side()->ExecJSWithResult(
      instanceId, namespaceStr, func, params);
  ClearVectorOfValueWithType(params);

  if (jsResult.length <= 0) {
    return createByteArrayResult(nullptr, 0);
  }
  std::unique_ptr<IPCResult> ipcResult =
      createByteArrayResult(jsResult.data, jsResult.length);
  WeexJSResultDataFree(jsResult);
  return ipcResult;
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::CreateInstance(
    IPCArguments* arguments) {
  LOGE("IPC CreateInstance");

  const char* instanceID = arguments->getByteArray(0)->content;
  const char* func = arguments->getByteArray(1)->content;
  const char* script = arguments->getByteArray(2)->content;
  const char* opts = arguments->getByteArray(3)->content;
  const char* initData = arguments->getByteArray(4)->content;
  const char* extendsApi = arguments->getByteArray(5)->content;

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
  return createInt32Result(Instance()->core_side()->CreateInstance(
      instanceID, func, script, opts, initData, extendsApi,params));
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::DestroyInstance(
    IPCArguments* arguments) {
  LOGE("IPC DestroyInstance");
  const char* instanceID = arguments->getByteArray(0)->content;
  if (strlen(instanceID) == 0) {
    LOGE("DestoryInstance instanceId is NULL");
    return createInt32Result(static_cast<int32_t>(false));
  }

  return createInt32Result(
      Instance()->core_side()->DestroyInstance(instanceID));
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::ExecJSOnInstance(
    IPCArguments* arguments) {
  LOGE("IPC ExecJSONInstance");
  const char* instanceID = arguments->getByteArray(0)->content;
  const char* script = arguments->getByteArray(1)->content;

  char* result = const_cast<char*>(
      Instance()->core_side()->ExecJSOnInstance(instanceID, script));

  if (result == nullptr)
    return createByteArrayResult(nullptr, 0);
  else
    return createCharArrayResult(result);
}

std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::UpdateGlobalConfig(
    IPCArguments* arguments) {
  LOGE("IPC UpdateGlobalConfig");
  const char* configString = arguments->getByteArray(0)->content;
  Instance()->core_side()->UpdateGlobalConfig(configString);
  return createVoidResult();
}


std::unique_ptr<IPCResult> PlatformBridgeInMultiProcess::UpdateInitFrameworkParams(
    IPCArguments* arguments) {
  LOGE("IPC UpdateInitFrameworkParams");
  const char* key = arguments->getByteArray(0)->content;
  const char* value = arguments->getByteArray(1)->content;
  const char* desc = arguments->getByteArray(2)->content;
  Instance()->core_side()->UpdateInitFrameworkParams(key, value, desc);
  return createVoidResult();
}

}  // namespace weex
