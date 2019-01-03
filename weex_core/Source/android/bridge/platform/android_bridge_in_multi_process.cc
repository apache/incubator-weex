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

#include "android_bridge_in_multi_process.h"

#include "android/base/string/string_utils.h"
#include "android/jsengine/bridge/script/core_side_in_multi_process.h"
#include "android/bridge/platform/android_side.h"
#include "android/bridge/multi_process_and_so_initializer.h"
#include "android/utils/ipc_string_result.h"
#include "core/layout/layout.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "include/WeexApiHeader.h"
#include "third_party/IPC/IPCArguments.h"
#include "third_party/IPC/IPCHandler.h"
#include "third_party/IPC/IPCMessageJS.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/IPCSender.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "wson/wson_parser.h"

namespace WeexCore {

AndroidBridgeInMultiProcess::AndroidBridgeInMultiProcess() {
  set_platform_side(new AndroidSide);
  set_core_side(new CoreSideInMultiProcess);

  std::unique_ptr<MultiProcessAndSoInitializer> initializer(
      new MultiProcessAndSoInitializer);
  bool passable = initializer->Init(
      [this](IPCHandler* handler) { RegisterIPCCallback(handler); },
      [this](std::unique_ptr<WeexJSConnection> connection,
             std::unique_ptr<IPCHandler> handler,
             std::unique_ptr<IPCHandler> server_handler) {
        static_cast<CoreSideInMultiProcess*>(core_side())
            ->set_sender(connection->sender());
        connection_ = std::move(connection);
        handler_ = std::move(handler);
        server_handler_ = std::move(server_handler);
        return true;
      },
      [this](const char* page_id, const char* func,
             const char* exception_string) {
        platform_side()->ReportException(page_id, func, exception_string);
      });
  set_is_passable(passable);
}

AndroidBridgeInMultiProcess::~AndroidBridgeInMultiProcess() {}

void AndroidBridgeInMultiProcess::RegisterIPCCallback(IPCHandler* handler) {
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::INVOKE_MEASURE_FUNCTION),
      InvokeMeasureFunction);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::INVOKE_LAYOUT_BEFORE),
      InvokeLayoutBefore);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::INVOKE_LAYOUT_AFTER),
      InvokeLayoutAfter);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::SET_JS_VERSION),
      SetJSVersion);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REPORT_EXCEPTION),
      ReportException);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CALL_NATIVE), CallNative);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CALL_NATIVE_MODULE),
      CallNativeModule);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CALL_NATIVE_COMPONENT),
      CallNativeComponent);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::SET_TIMEOUT), SetTimeout);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::NATIVE_LOG), NativeLog);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::UPDATE_FINISH),
      UpdateFinish);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REFRESH_FINISH),
      RefreshFinish);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::ADD_EVENT), AddEvent);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REMOVE_EVENT),
      RemoveEvent);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CREATE_BODY), CreateBody);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::ADD_ELEMENT), AddElement);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::LAYOUT), Layout);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::UPDATE_STYLE),
      UpdateStyle);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::UPDATE_ATTR), UpdateAttr);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CREATE_FINISH),
      CreateFinish);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::RENDER_SUCCESS),
      RenderSuccess);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REMOVE_ELEMENT),
      RemoveElement);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::MOVE_ELEMENT),
      MoveElement);
  handler->registerHandler(
      static_cast<uint32_t>(
          IPCMsgFromCoreToPlatform::APPEND_TREE_CREATE_FINISH),
      AppendTreeCreateFinish);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::HAS_TRANSITION_PROS),
      HasTransitionPros);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::POST_MESSAGE),
      PostMessage);
  handler->registerHandler(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::DISPATCH_MESSAGE),
      DispatchMessage);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::InvokeMeasureFunction(
    IPCArguments* arguments) {
  auto page_id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  int64_t render_ptr = getArgumentAsInt64(arguments, 1);
  float width = getArgumentAsFloat(arguments, 2);
  int32_t width_measure_mode = getArgumentAsInt32(arguments, 3);
  float height = getArgumentAsFloat(arguments, 4);
  int32_t height_measure_mode = getArgumentAsInt32(arguments, 5);

  WXCoreSize size = WeexCoreManager::Instance()
                        ->getPlatformBridge()
                        ->platform_side()
                        ->InvokeMeasureFunction(page_id.get(), render_ptr,
                                                width, width_measure_mode,
                                                height, height_measure_mode);

  float result[]{size.width, size.height};

  return createByteArrayResult(reinterpret_cast<const char*>(result),
                               2 * sizeof(float));
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::InvokeLayoutBefore(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  int64_t render_ptr = getArgumentAsInt64(arguments, 1);

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->InvokeLayoutBefore(page_id, render_ptr);

  delete[] page_id;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::InvokeLayoutAfter(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  int64_t render_ptr = getArgumentAsInt64(arguments, 1);
  float width = getArgumentAsFloat(arguments, 2);
  float height = getArgumentAsFloat(arguments, 3);

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->InvokeLayoutAfter(page_id, render_ptr, width, height);

  delete[] page_id;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::SetJSVersion(
    IPCArguments* arguments) {
  const char* version = getArumentAsCStr(arguments, 0);

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->SetJSVersion(version);

  delete[] version;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::ReportException(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* func = getArumentAsCStr(arguments, 1);
  const char* exception = getArumentAsCStr(arguments, 2);

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->ReportException(page_id, func, exception);

  delete[] page_id;
  delete[] func;
  delete[] exception;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::CallNative(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* task = getArumentAsCStr(arguments, 1);
  const char* callback = getArumentAsCStr(arguments, 2);

  int result = WeexCoreManager::Instance()
                   ->getPlatformBridge()
                   ->platform_side()
                   ->CallNative(page_id, task, callback);

  delete[] page_id;
  delete[] task;
  delete[] callback;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::CallNativeModule(
    IPCArguments* arguments) {
  auto page_id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto module = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto method = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  auto arguments_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
  int arguments_data_length = getArumentAsCStrLen(arguments, 3);
  auto options = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));
  int options_length = getArumentAsCStrLen(arguments, 4);

  auto result =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->platform_side()
          ->CallNativeModule(page_id.get(), module.get(), method.get(),
                             arguments_data.get(), arguments_data_length,
                             options.get(), options_length);

  return result;
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::CallNativeComponent(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);
  const char* method = getArumentAsCStr(arguments, 2);
  const char* arguments_data = getArumentAsCStr(arguments, 3);
  int arguments_data_length = getArgumentAsInt32(arguments, 4);
  const char* options = getArumentAsCStr(arguments, 5);
  int options_length = getArgumentAsInt32(arguments, 6);

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->CallNativeComponent(page_id, ref, method, arguments_data,
                            arguments_data_length, options, options_length);

  delete[] page_id;
  delete[] ref;
  delete[] method;
  delete[] arguments_data;
  delete[] options;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::SetTimeout(
    IPCArguments* arguments) {
  const char* callback_id = getArumentAsCStr(arguments, 0);
  const char* time = getArumentAsCStr(arguments, 1);

  WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->SetTimeout(
      callback_id, time);

  delete[] callback_id;
  delete[] time;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::NativeLog(
    IPCArguments* arguments) {
  const char* str_array = getArumentAsCStr(arguments, 0);

  WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->NativeLog(
      str_array);

  delete[] str_array;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::UpdateFinish(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* task = getArumentAsCStr(arguments, 1);
  int taskLen = getArumentAsCStrLen(arguments, 1);
  const char* callback = getArumentAsCStr(arguments, 2);
  int callbackLen = getArumentAsCStrLen(arguments, 2);

  int result =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->platform_side()
          ->UpdateFinish(page_id, task, taskLen, callback, callbackLen);

  delete[] page_id;
  delete[] task;
  delete[] callback;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::RefreshFinish(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* task = getArumentAsCStr(arguments, 1);
  const char* callback = getArumentAsCStr(arguments, 2);

  int result = WeexCoreManager::Instance()
                   ->getPlatformBridge()
                   ->platform_side()
                   ->RefreshFinish(page_id, task, callback);

  delete[] page_id;
  delete[] task;
  delete[] callback;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::AddEvent(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);
  const char* event = getArumentAsCStr(arguments, 2);

  int result = WeexCoreManager::Instance()
                   ->getPlatformBridge()
                   ->platform_side()
                   ->AddEvent(page_id, ref, event);

  delete[] page_id;
  delete[] ref;
  delete[] event;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::RemoveEvent(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);
  const char* event = getArumentAsCStr(arguments, 2);

  int result = WeexCoreManager::Instance()
                   ->getPlatformBridge()
                   ->platform_side()
                   ->RemoveEvent(page_id, ref, event);

  delete[] page_id;
  delete[] ref;
  delete[] event;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::CreateBody(
    IPCArguments* arguments) {
  auto page_id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto component_type = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto ref = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  // styles
  std::map<std::string, std::string> styles;
  if (arguments->getType(3) != IPCType::VOID) {
    auto styles_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
    wson_parser styles_parser = wson_parser(styles_data.get());
    styles_parser.nextType();
    int styles_length = styles_parser.nextMapSize();
    for (int i = 0; i < styles_length; ++i) {
      std::string key = styles_parser.nextStringUTF8(styles_parser.nextType());
      std::string value =
          styles_parser.nextStringUTF8(styles_parser.nextType());
      styles.insert(std::make_pair<std::string, std::string>(std::move(key),
                                                             std::move(value)));
    }
  }
  // attributes
  std::map<std::string, std::string> attributes;
  if (arguments->getType(4) != IPCType::VOID) {
    auto attributes_data =
        std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));
    wson_parser attributes_parser = wson_parser(attributes_data.get());
    attributes_parser.nextType();
    int attributes_length = attributes_parser.nextMapSize();
    for (int i = 0; i < attributes_length; ++i) {
      std::string key =
          attributes_parser.nextStringUTF8(attributes_parser.nextType());
      std::string value =
          attributes_parser.nextStringUTF8(attributes_parser.nextType());
      attributes.insert(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }
  // events
  std::set<std::string> events;
  if (arguments->getType(5) != IPCType::VOID) {
    auto events_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 5));
    wson_parser events_parser = wson_parser(events_data.get());
    events_parser.nextType();
    int events_length = events_parser.nextArraySize();
    for (int i = 0; i < events_length; ++i) {
      events.insert(events_parser.nextStringUTF8(events_parser.nextType()));
    }
  }
  // margin
  auto margins_buffer = const_cast<uint16_t*>(arguments->getString(6)->content);
  WXCoreMargin margin;
  margin.setMargin(kMarginTop,
                   reinterpret_cast<const float*>(margins_buffer)[0]);
  margin.setMargin(kMarginBottom,
                   reinterpret_cast<const float*>(margins_buffer)[1]);
  margin.setMargin(kMarginLeft,
                   reinterpret_cast<const float*>(margins_buffer)[2]);
  margin.setMargin(kMarginRight,
                   reinterpret_cast<const float*>(margins_buffer)[3]);
  // padding
  auto paddings_buffer =
      const_cast<uint16_t*>(arguments->getString(7)->content);
  WXCorePadding padding;
  padding.setPadding(kPaddingTop,
                     reinterpret_cast<const float*>(paddings_buffer)[0]);
  padding.setPadding(kPaddingBottom,
                     reinterpret_cast<const float*>(paddings_buffer)[1]);
  padding.setPadding(kPaddingLeft,
                     reinterpret_cast<const float*>(paddings_buffer)[2]);
  padding.setPadding(kPaddingRight,
                     reinterpret_cast<const float*>(paddings_buffer)[3]);
  // border
  auto borders_buffer = const_cast<uint16_t*>(arguments->getString(8)->content);
  WXCoreBorderWidth border;
  border.setBorderWidth(kBorderWidthTop,
                        reinterpret_cast<const float*>(borders_buffer)[0]);
  border.setBorderWidth(kBorderWidthBottom,
                        reinterpret_cast<const float*>(borders_buffer)[1]);
  border.setBorderWidth(kBorderWidthLeft,
                        reinterpret_cast<const float*>(borders_buffer)[2]);
  border.setBorderWidth(kBorderWidthRight,
                        reinterpret_cast<const float*>(borders_buffer)[3]);

  int result =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->platform_side()
          ->CreateBody(page_id.get(), component_type.get(), ref.get(), &styles,
                       &attributes, &events, margin, padding, border);

  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::AddElement(
    IPCArguments* arguments) {
  auto page_id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto component_type = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  auto ref = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
  int index = getArgumentAsInt32(arguments, 3);
  auto parent_ref = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));
  // styles
  std::map<std::string, std::string> styles;
  if (arguments->getType(5) != IPCType::VOID) {
    auto styles_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 5));
    wson_parser styles_parser = wson_parser(styles_data.get());
    styles_parser.nextType();
    int styles_length = styles_parser.nextMapSize();
    for (int i = 0; i < styles_length; ++i) {
      std::string key = styles_parser.nextStringUTF8(styles_parser.nextType());
      std::string value =
          styles_parser.nextStringUTF8(styles_parser.nextType());
      styles.insert(std::make_pair<std::string, std::string>(std::move(key),
                                                             std::move(value)));
    }
  }
  // attributes
  std::map<std::string, std::string> attributes;
  if (arguments->getType(6) != IPCType::VOID) {
    auto attributes_data =
        std::unique_ptr<char[]>(getArumentAsCStr(arguments, 6));
    wson_parser attributes_parser = wson_parser(attributes_data.get());
    attributes_parser.nextType();
    int attributes_length = attributes_parser.nextMapSize();
    for (int i = 0; i < attributes_length; ++i) {
      std::string key =
          attributes_parser.nextStringUTF8(attributes_parser.nextType());
      std::string value =
          attributes_parser.nextStringUTF8(attributes_parser.nextType());
      attributes.insert(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }
  // events
  std::set<std::string> events;
  if (arguments->getType(7) != IPCType::VOID) {
    auto events_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 7));
    wson_parser events_parser = wson_parser(events_data.get());
    events_parser.nextType();
    int events_length = events_parser.nextArraySize();
    for (int i = 0; i < events_length; ++i) {
      events.insert(events_parser.nextStringUTF8(events_parser.nextType()));
    }
  }
  // margin
  auto margins_buffer = const_cast<uint16_t*>(arguments->getString(8)->content);
  WXCoreMargin margin;
  margin.setMargin(kMarginTop,
                   reinterpret_cast<const float*>(margins_buffer)[0]);
  margin.setMargin(kMarginBottom,
                   reinterpret_cast<const float*>(margins_buffer)[1]);
  margin.setMargin(kMarginLeft,
                   reinterpret_cast<const float*>(margins_buffer)[2]);
  margin.setMargin(kMarginRight,
                   reinterpret_cast<const float*>(margins_buffer)[3]);
  // padding
  auto paddings_buffer =
      const_cast<uint16_t*>(arguments->getString(9)->content);
  WXCorePadding padding;
  padding.setPadding(kPaddingTop,
                     reinterpret_cast<const float*>(paddings_buffer)[0]);
  padding.setPadding(kPaddingBottom,
                     reinterpret_cast<const float*>(paddings_buffer)[1]);
  padding.setPadding(kPaddingLeft,
                     reinterpret_cast<const float*>(paddings_buffer)[2]);
  padding.setPadding(kPaddingRight,
                     reinterpret_cast<const float*>(paddings_buffer)[3]);
  // border
  auto borders_buffer =
      const_cast<uint16_t*>(arguments->getString(10)->content);
  WXCoreBorderWidth border;
  border.setBorderWidth(kBorderWidthTop,
                        reinterpret_cast<const float*>(borders_buffer)[0]);
  border.setBorderWidth(kBorderWidthBottom,
                        reinterpret_cast<const float*>(borders_buffer)[1]);
  border.setBorderWidth(kBorderWidthLeft,
                        reinterpret_cast<const float*>(borders_buffer)[2]);
  border.setBorderWidth(kBorderWidthRight,
                        reinterpret_cast<const float*>(borders_buffer)[3]);
  // will_layout
  bool will_layout = static_cast<bool>(getArgumentAsInt32(arguments, 11));
  int result = WeexCoreManager::Instance()
                   ->getPlatformBridge()
                   ->platform_side()
                   ->AddElement(page_id.get(), component_type.get(), ref.get(),
                                index, parent_ref.get(), &styles, &attributes,
                                &events, margin, padding, border, will_layout);

  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::Layout(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);
  int32_t top = getArgumentAsInt32(arguments, 2);
  int32_t bottom = getArgumentAsInt32(arguments, 3);
  int32_t left = getArgumentAsInt32(arguments, 4);
  int32_t right = getArgumentAsInt32(arguments, 5);
  int32_t height = getArgumentAsInt32(arguments, 6);
  int32_t width = getArgumentAsInt32(arguments, 7);
  int32_t index = getArgumentAsInt32(arguments, 8);

  auto result =
      WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->Layout(
          page_id, ref, top, bottom, left, right, height, width, index);
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::UpdateStyle(
    IPCArguments* arguments) {
  auto page_id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto ref = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  // styles
  std::vector<std::pair<std::string, std::string>> styles;
  if (arguments->getType(2) != IPCType::VOID) {
    auto styles_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
    wson_parser styles_parser = wson_parser(styles_data.get());
    styles_parser.nextType();
    int styles_length = styles_parser.nextMapSize();
    for (int i = 0; i < styles_length; ++i) {
      std::string key = styles_parser.nextStringUTF8(styles_parser.nextType());
      std::string value =
          styles_parser.nextStringUTF8(styles_parser.nextType());
      styles.push_back(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }

  // margin
  std::vector<std::pair<std::string, std::string>> margins;
  if (arguments->getType(3) != IPCType::VOID) {
    auto margins_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 3));
    wson_parser margins_parser = wson_parser(margins_data.get());
    margins_parser.nextType();
    int margins_length = margins_parser.nextMapSize();
    for (int i = 0; i < margins_length; ++i) {
      std::string key =
          margins_parser.nextStringUTF8(margins_parser.nextType());
      std::string value =
          margins_parser.nextStringUTF8(margins_parser.nextType());
      margins.push_back(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }

  // padding
  std::vector<std::pair<std::string, std::string>> paddings;
  if (arguments->getType(4) != IPCType::VOID) {
    auto paddings_data =
        std::unique_ptr<char[]>(getArumentAsCStr(arguments, 4));
    wson_parser paddings_parser = wson_parser(paddings_data.get());
    paddings_parser.nextType();
    int paddings_length = paddings_parser.nextMapSize();
    for (int i = 0; i < paddings_length; ++i) {
      std::string key =
          paddings_parser.nextStringUTF8(paddings_parser.nextType());
      std::string value =
          paddings_parser.nextStringUTF8(paddings_parser.nextType());
      paddings.push_back(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }

  // border
  std::vector<std::pair<std::string, std::string>> borders;
  if (arguments->getType(5) != IPCType::VOID) {
    auto borders_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 5));
    wson_parser borders_parser = wson_parser(borders_data.get());
    borders_parser.nextType();
    int borders_length = borders_parser.nextMapSize();
    for (int i = 0; i < borders_length; ++i) {
      std::string key =
          borders_parser.nextStringUTF8(borders_parser.nextType());
      std::string value =
          borders_parser.nextStringUTF8(borders_parser.nextType());
      borders.push_back(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }

  int result = WeexCoreManager::Instance()
                   ->getPlatformBridge()
                   ->platform_side()
                   ->UpdateStyle(page_id.get(), ref.get(), &styles, &margins,
                                 &paddings, &borders);

  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::UpdateAttr(
    IPCArguments* arguments) {
  auto page_id = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 0));
  auto ref = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 1));
  // attributes
  std::vector<std::pair<std::string, std::string>> attributes;
  if (arguments->getType(2) != IPCType::VOID) {
    auto attributes_data =
        std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
    wson_parser attributes_parser = wson_parser(attributes_data.get());
    attributes_parser.nextType();
    int attributes_length = attributes_parser.nextMapSize();
    for (int i = 0; i < attributes_length; ++i) {
      std::string key =
          attributes_parser.nextStringUTF8(attributes_parser.nextType());
      std::string value =
          attributes_parser.nextStringUTF8(attributes_parser.nextType());
      attributes.push_back(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }

  int result = WeexCoreManager::Instance()
                   ->getPlatformBridge()
                   ->platform_side()
                   ->UpdateAttr(page_id.get(), ref.get(), &attributes);

  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::CreateFinish(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);

  auto result = WeexCoreManager::Instance()
                    ->getPlatformBridge()
                    ->platform_side()
                    ->CreateFinish(page_id);
  delete[] page_id;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::RenderSuccess(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);

  auto result = WeexCoreManager::Instance()
                    ->getPlatformBridge()
                    ->platform_side()
                    ->RenderSuccess(page_id);
  delete[] page_id;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::RemoveElement(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);

  auto result = WeexCoreManager::Instance()
                    ->getPlatformBridge()
                    ->platform_side()
                    ->RemoveElement(page_id, ref);
  delete[] page_id;
  delete[] ref;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::MoveElement(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);
  const char* parent_ref = getArumentAsCStr(arguments, 2);
  int32_t index = getArgumentAsInt32(arguments, 3);

  auto result = WeexCoreManager::Instance()
                    ->getPlatformBridge()
                    ->platform_side()
                    ->MoveElement(page_id, ref, parent_ref, index);
  delete[] page_id;
  delete[] ref;
  delete[] parent_ref;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::AppendTreeCreateFinish(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);

  auto result = WeexCoreManager::Instance()
                    ->getPlatformBridge()
                    ->platform_side()
                    ->AppendTreeCreateFinish(page_id, ref);
  delete[] page_id;
  delete[] ref;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::HasTransitionPros(
    IPCArguments* arguments) {
  const char* page_id = getArumentAsCStr(arguments, 0);
  const char* ref = getArumentAsCStr(arguments, 1);

  std::vector<std::pair<std::string, std::string>> styles;
  if (arguments->getType(2) != IPCType::VOID) {
    auto styles_data = std::unique_ptr<char[]>(getArumentAsCStr(arguments, 2));
    wson_parser styles_parser = wson_parser(styles_data.get());
    styles_parser.nextType();
    int styles_length = styles_parser.nextMapSize();
    for (int i = 0; i < styles_length; ++i) {
      std::string key = styles_parser.nextStringUTF8(styles_parser.nextType());
      std::string value =
          styles_parser.nextStringUTF8(styles_parser.nextType());
      styles.push_back(std::make_pair<std::string, std::string>(
          std::move(key), std::move(value)));
    }
  }

  auto result = WeexCoreManager::Instance()
                    ->getPlatformBridge()
                    ->platform_side()
                    ->HasTransitionPros(page_id, ref, &styles);
  delete[] page_id;
  delete[] ref;
  return createInt32Result(result);
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::PostMessage(
    IPCArguments* arguments) {
  const char* vm_id = getArumentAsCStr(arguments, 0);
  const char* data = getArumentAsCStr(arguments, 1);
  int dataLength = getArumentAsCStrLen(arguments, 1);
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->PostMessage(vm_id, data,dataLength);
  delete[] vm_id;
  delete[] data;
  return createVoidResult();
}

std::unique_ptr<IPCResult> AndroidBridgeInMultiProcess::DispatchMessage(
    IPCArguments* arguments) {
  const char* client_id = getArumentAsCStr(arguments, 0);
  const char* vm_id = getArumentAsCStr(arguments, 1);
  const char* data = getArumentAsCStr(arguments, 2);
  int dataLength = getArumentAsCStrLen(arguments, 2);
  const char* callback = getArumentAsCStr(arguments, 3);

  

  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->DispatchMessage(client_id, data, dataLength, callback, vm_id);
  delete[] client_id;
  delete[] vm_id;
  delete[] data;
  delete[] callback;
  return createVoidResult();
}

}  // namespace WeexCore
