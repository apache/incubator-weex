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

#include "platform_side_multi_process.h"

#include "android/jsengine/weex_ipc_server.h"
#include "core/layout/layout.h"
#include "third_party/IPC/Buffering/IPCBuffer.h"
#include "third_party/IPC/IPCResult.h"
#include "third_party/IPC/IPCSender.h"
#include "third_party/IPC/Serializing/IPCSerializer.h"
#include "third_party/IPC/IPCMessageJS.h"
#include "wson/wson.h"

namespace weex {

inline static void wson_push_type_char_array(wson_buffer *buffer,
                                             const char *src, int32_t length) {
  wson_push_type_uint8_string(buffer, (const uint8_t *)src, length);
}

WXCoreSize PlatformSideInMultiProcess::InvokeMeasureFunction(
    const char *page_id, long render_ptr, float width, int width_measure_mode,
    float height, int height_measure_mode) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::INVOKE_MEASURE_FUNCTION));
  serializer->add(page_id, strlen(page_id));
  serializer->add(static_cast<int64_t>(render_ptr));
  serializer->add(width);
  serializer->add(width_measure_mode);
  serializer->add(height);
  serializer->add(height_measure_mode);
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());

  const char *result_str = result->getByteArrayContent();
  WXCoreSize size;
  size.width = reinterpret_cast<const float *>(result_str)[0];
  size.height = reinterpret_cast<const float *>(result_str)[1];
  return size;
}
void PlatformSideInMultiProcess::InvokeLayoutBefore(const char *page_id,
                                                    long render_ptr) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::INVOKE_LAYOUT_BEFORE));
  serializer->add(page_id, strlen(page_id));
  serializer->add(static_cast<int64_t>(render_ptr));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}
void PlatformSideInMultiProcess::InvokeLayoutAfter(const char *page_id,
                                                   long render_ptr, float width,
                                                   float height) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::INVOKE_LAYOUT_AFTER));
  serializer->add(page_id, strlen(page_id));
  serializer->add(static_cast<int64_t>(render_ptr));
  serializer->add(width);
  serializer->add(height);
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}

void PlatformSideInMultiProcess::SetJSVersion(const char *version) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::SET_JS_VERSION));
  serializer->add(version, strlen(version));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}

void PlatformSideInMultiProcess::ReportException(const char *page_id,
                                                 const char *func,
                                                 const char *exception_string) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REPORT_EXCEPTION));
  serializer->add(page_id, strlen(page_id));
  serializer->add(func, strlen(func));
  serializer->add(exception_string, strlen(exception_string));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}

void PlatformSideInMultiProcess::ReportServerCrash(const char *instance_id) {
  // TODO
}

void PlatformSideInMultiProcess::ReportNativeInitStatus(const char *status_code,
                                         const char *error_msg) {
  // TODO
}

int PlatformSideInMultiProcess::CallNative(const char *page_id,
                                           const char *task,
                                           const char *callback) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CALL_NATIVE));
  serializer->add(page_id, strlen(page_id));
  serializer->add(task, strlen(task));
  serializer->add(callback, strlen(callback));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

std::unique_ptr<IPCResult> PlatformSideInMultiProcess::CallNativeModule(
    const char *page_id, const char *module, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  LOGE("CallNativeModule ipc");
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CALL_NATIVE_MODULE));
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
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result;
}

void PlatformSideInMultiProcess::CallNativeComponent(
    const char *page_id, const char *ref, const char *method,
    const char *arguments, int arguments_length, const char *options,
    int options_length) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CALL_NATIVE_COMPONENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(method, strlen(method));
  serializer->add(arguments, arguments_length);
  serializer->add(arguments_length);
  serializer->add(options, options_length);
  serializer->add(options_length);
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}

void PlatformSideInMultiProcess::SetTimeout(const char *callback_id,
                                            const char *time) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::SET_TIMEOUT));
  serializer->add(callback_id, strlen(callback_id));
  serializer->add(time, strlen(time));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}

void PlatformSideInMultiProcess::NativeLog(const char *str_array) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::NATIVE_LOG));
  serializer->add(str_array, strlen(str_array));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}

int PlatformSideInMultiProcess::UpdateFinish(const char *page_id,
                                             const char *task, int taskLen,
                                             const char *callback, int callbackLen) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::UPDATE_FINISH));
  serializer->add(page_id, strlen(page_id));
  serializer->add(task, strlen(task));
  serializer->add(callback, strlen(callback));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::RefreshFinish(const char *page_id,
                                              const char *task,
                                              const char *callback) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REFRESH_FINISH));
  serializer->add(page_id, strlen(page_id));
  serializer->add(task, strlen(task));
  serializer->add(callback, strlen(callback));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::AddEvent(const char *page_id, const char *ref,
                                         const char *event) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::ADD_EVENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(event, strlen(event));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::RemoveEvent(const char *page_id,
                                            const char *ref,
                                            const char *event) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REMOVE_EVENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(event, strlen(event));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::CreateBody(
    const char *page_id, const char *component_type, const char *ref,
    std::map<std::string, std::string> *styles,
    std::map<std::string, std::string> *attributes,
    std::set<std::string> *events, const WXCoreMargin &margins,
    const WXCorePadding &paddings, const WXCoreBorderWidth &borders) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CREATE_BODY));
  serializer->add(page_id, strlen(page_id));
  serializer->add(component_type, strlen(component_type));
  serializer->add(ref, strlen(ref));

  if (styles != nullptr) {
    wson_buffer *styles_buffer = wson_buffer_new();
    wson_push_type_map(styles_buffer, styles->size());
    for (auto it = styles->begin(); it != styles->end(); ++it) {
      wson_push_type_char_array(styles_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(styles_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(styles_buffer->data),
                    styles_buffer->position);
    wson_buffer_free(styles_buffer);
  } else {
    serializer->addVoid();
  }

  if (attributes != nullptr) {
    wson_buffer *attrs_buffer = wson_buffer_new();
    wson_push_type_map(attrs_buffer, attributes->size());
    for (auto it = attributes->begin(); it != attributes->end(); ++it) {
      wson_push_type_char_array(attrs_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(attrs_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(attrs_buffer->data),
                    attrs_buffer->position);
    wson_buffer_free(attrs_buffer);
  } else {
    serializer->addVoid();
  }

  if (events != nullptr) {
    wson_buffer *events_buffer = wson_buffer_new();
    wson_push_type_array(events_buffer, events->size());
    for (auto it = events->begin(); it != events->end(); ++it) {
      wson_push_type_char_array(events_buffer, (*it).c_str(), (*it).size());
    }
    serializer->add(static_cast<char *>(events_buffer->data),
                    events_buffer->position);
    wson_buffer_free(events_buffer);
  } else {
    serializer->addVoid();
  }

  float margins_buffer[]{
      margins.getMargin(kMarginTop), margins.getMargin(kMarginBottom),
      margins.getMargin(kMarginLeft), margins.getMargin(kMarginRight)};
  serializer->add(reinterpret_cast<uint16_t *>(margins_buffer),
                  4 * sizeof(float));
  float paddings_buffer[]{
      paddings.getPadding(kPaddingTop), paddings.getPadding(kPaddingBottom),
      paddings.getPadding(kPaddingLeft), paddings.getPadding(kPaddingRight)};
  serializer->add(reinterpret_cast<uint16_t *>(paddings_buffer),
                  4 * sizeof(float));
  float borders_buffer[]{borders.getBorderWidth(kBorderWidthTop),
                         borders.getBorderWidth(kBorderWidthBottom),
                         borders.getBorderWidth(kBorderWidthLeft),
                         borders.getBorderWidth(kBorderWidthRight)};
  serializer->add(reinterpret_cast<uint16_t *>(borders_buffer),
                  4 * sizeof(float));

  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());

  return result->get<int>();
}

int PlatformSideInMultiProcess::AddElement(
    const char *page_id, const char *component_type, const char *ref,
    int &index, const char *parentRef,
    std::map<std::string, std::string> *styles,
    std::map<std::string, std::string> *attributes,
    std::set<std::string> *events, const WXCoreMargin &margins,
    const WXCorePadding &paddings, const WXCoreBorderWidth &borders,
    bool willLayout) {

  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::ADD_ELEMENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(component_type, strlen(component_type));
  serializer->add(ref, strlen(ref));
  serializer->add(index);
  serializer->add(parentRef, strlen(parentRef));

  if (styles != nullptr) {
    wson_buffer *styles_buffer = wson_buffer_new();
    wson_push_type_map(styles_buffer, styles->size());
    for (auto it = styles->begin(); it != styles->end(); ++it) {
      wson_push_type_char_array(styles_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(styles_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(styles_buffer->data),
                    styles_buffer->position);
    wson_buffer_free(styles_buffer);
  } else {
    serializer->addVoid();
  }

  if (attributes != nullptr) {
    wson_buffer *attrs_buffer = wson_buffer_new();
    wson_push_type_map(attrs_buffer, attributes->size());
    for (auto it = attributes->begin(); it != attributes->end(); ++it) {
      wson_push_type_char_array(attrs_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(attrs_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(attrs_buffer->data),
                    attrs_buffer->position);
    wson_buffer_free(attrs_buffer);
  } else {
    serializer->addVoid();
  }

  if (events != nullptr) {
    wson_buffer *events_buffer = wson_buffer_new();
    wson_push_type_array(events_buffer, events->size());
    for (auto it = events->begin(); it != events->end(); ++it) {
      wson_push_type_char_array(events_buffer, (*it).c_str(), (*it).size());
    }
    serializer->add(static_cast<char *>(events_buffer->data),
                    events_buffer->position);
    wson_buffer_free(events_buffer);
  } else {
    serializer->addVoid();
  }

  float margins_buffer[]{
      margins.getMargin(kMarginTop), margins.getMargin(kMarginBottom),
      margins.getMargin(kMarginLeft), margins.getMargin(kMarginRight)};
  serializer->add(reinterpret_cast<const char *>(margins_buffer),
                  4 * sizeof(float));
  float paddings_buffer[]{
      paddings.getPadding(kPaddingTop), paddings.getPadding(kPaddingBottom),
      paddings.getPadding(kPaddingLeft), paddings.getPadding(kPaddingRight)};
  serializer->add(reinterpret_cast<const char *>(paddings_buffer),
                  4 * sizeof(float));
  float borders_buffer[]{borders.getBorderWidth(kBorderWidthTop),
                         borders.getBorderWidth(kBorderWidthBottom),
                         borders.getBorderWidth(kBorderWidthLeft),
                         borders.getBorderWidth(kBorderWidthRight)};
  serializer->add(reinterpret_cast<const char *>(borders_buffer),
                  4 * sizeof(float));
  serializer->add(willLayout);

  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());

  return result->get<int>();
}

int PlatformSideInMultiProcess::Layout(const char *page_id, const char *ref,
                                       int top, int bottom, int left, int right,
                                       int height, int width, bool isRTL,
                                       int index) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(IPCMsgFromCoreToPlatform::LAYOUT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(top);
  serializer->add(bottom);
  serializer->add(left);
  serializer->add(right);
  serializer->add(height);
  serializer->add(width);
  serializer->add(isRTL);
  serializer->add(index);
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::UpdateStyle(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style,
    std::vector<std::pair<std::string, std::string>> *margin,
    std::vector<std::pair<std::string, std::string>> *padding,
    std::vector<std::pair<std::string, std::string>> *border) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::UPDATE_STYLE));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));

  if (style != nullptr) {
    wson_buffer *style_buffer = wson_buffer_new();
    wson_push_type_map(style_buffer, style->size());
    for (auto it = style->begin(); it != style->end(); ++it) {
      wson_push_type_char_array(style_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(style_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(style_buffer->data),
                    style_buffer->position);
    wson_buffer_free(style_buffer);
  } else {
    serializer->addVoid();
  }

  if (margin != nullptr) {
    wson_buffer *margin_buffer = wson_buffer_new();
    wson_push_type_map(margin_buffer, margin->size());
    for (auto it = margin->begin(); it != margin->end(); ++it) {
      wson_push_type_char_array(margin_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(margin_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(margin_buffer->data),
                    margin_buffer->position);
    wson_buffer_free(margin_buffer);
  } else {
    serializer->addVoid();
  }

  if (padding != nullptr) {
    wson_buffer *padding_buffer = wson_buffer_new();
    wson_push_type_map(padding_buffer, padding->size());
    for (auto it = padding->begin(); it != padding->end(); ++it) {
      wson_push_type_char_array(padding_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(padding_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(padding_buffer->data),
                    padding_buffer->position);
    wson_buffer_free(padding_buffer);
  } else {
    serializer->addVoid();
  }

  if (border != nullptr) {
    wson_buffer *border_buffer = wson_buffer_new();
    wson_push_type_map(border_buffer, border->size());
    for (auto it = border->begin(); it != border->end(); ++it) {
      wson_push_type_char_array(border_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(border_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(border_buffer->data),
                    border_buffer->position);
    wson_buffer_free(border_buffer);
  } else {
    serializer->addVoid();
  }

  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());

  return result->get<int>();
}

int PlatformSideInMultiProcess::UpdateAttr(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::UPDATE_ATTR));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));

  if (attrs != nullptr) {
    wson_buffer *attrs_buffer = wson_buffer_new();
    wson_push_type_map(attrs_buffer, attrs->size());
    for (auto it = attrs->begin(); it != attrs->end(); ++it) {
      wson_push_type_char_array(attrs_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(attrs_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(attrs_buffer->data),
                    attrs_buffer->position);
    wson_buffer_free(attrs_buffer);
  } else {
    serializer->addVoid();
  }

  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());

  return result->get<int>();
}

int PlatformSideInMultiProcess::CreateFinish(const char *page_id) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::CREATE_FINISH));
  serializer->add(page_id, strlen(page_id));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::RenderSuccess(const char *page_id) {
  WeexIPCClient *pClient = client_;

  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::RENDER_SUCCESS));
  serializer->add(page_id, strlen(page_id));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::RemoveElement(const char *page_id,
                                              const char *ref) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::REMOVE_ELEMENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::MoveElement(const char *page_id,
                                            const char *ref,
                                            const char *parent_ref, int index) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::MOVE_ELEMENT));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  serializer->add(parent_ref, strlen(parent_ref));
  serializer->add(index);
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::AppendTreeCreateFinish(const char *page_id,
                                                       const char *ref) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(static_cast<uint32_t>(
      IPCMsgFromCoreToPlatform::APPEND_TREE_CREATE_FINISH));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
  return result->get<int>();
}

int PlatformSideInMultiProcess::HasTransitionPros(
    const char *page_id, const char *ref,
    std::vector<std::pair<std::string, std::string>> *style) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::HAS_TRANSITION_PROS));
  serializer->add(page_id, strlen(page_id));
  serializer->add(ref, strlen(ref));

  if (style != nullptr) {
    wson_buffer *style_buffer = wson_buffer_new();
    wson_push_type_map(style_buffer, style->size());
    auto it = style->begin();
    for (; it != style->end(); ++it) {
      wson_push_type_char_array(style_buffer, it->first.c_str(),
                                it->first.size());
      wson_push_type_char_array(style_buffer, it->second.c_str(),
                                it->second.size());
    }
    serializer->add(static_cast<char *>(style_buffer->data),
                    style_buffer->position);
    wson_buffer_free(style_buffer);
  } else {
    serializer->addVoid();
  }

  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());

  return result->get<int>();
}

void PlatformSideInMultiProcess::PostMessage(const char *vm_id,
                                             const char *data) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::POST_MESSAGE));
  serializer->add(vm_id, strlen(vm_id));
  serializer->add(data, strlen(data));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}

void PlatformSideInMultiProcess::DispatchMessage(const char *client_id,
                                                 const char *vm_id,
                                                 const char *data,
                                                 const char *callback) {
  WeexIPCClient *pClient = client_;
  
  IPCSender *sender = pClient->getSender();
  std::unique_ptr<IPCSerializer> serializer(createIPCSerializer());
  serializer->setMsg(
      static_cast<uint32_t>(IPCMsgFromCoreToPlatform::DISPATCH_MESSAGE));
  serializer->add(client_id, strlen(client_id));
  serializer->add(vm_id, strlen(vm_id));
  serializer->add(data, strlen(data));
  serializer->add(callback, strlen(callback));
  std::unique_ptr<IPCBuffer> buffer = serializer->finish();
  std::unique_ptr<IPCResult> result = sender->send(buffer.get());
}
}  // namespace weex
