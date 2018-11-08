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

#include "core/data_render/vnode/vcomponent_lifecycle_listener.h"
#include <android/utils/params_utils.h>
#include <wson.h>
#include <wson_parser.h>
#include "core/data_render/exec_state.h"
#include "core/manager/weex_core_manager.h"

namespace weex {
namespace core {
namespace data_render {

//static const char* kMethodOnComponentEvent = "__onComponentEvent";
 static const char* kMethodOnComponentEvent = "callJS";
static const char* kComponentHook = "componentHook";
static const char* kLifecycle = "lifecycle";
static const char* kKeyMethod = "method";
static const char* kKeyArgs = "args";
static const char* kEventOnCreated = "created";
static const char* kEventOnUpdated = "updated";
static const char* kEventOnDestroyed = "destroyed";

static VALUE_WITH_TYPE* GenValueWithType(const char* event_str) {
  VALUE_WITH_TYPE* event = getValueWithTypePtr();
  event->type = ParamsType::BYTEARRAY;
  auto buffer = wson_buffer_new();
  wson_push_type_uint8_string(
      buffer, reinterpret_cast<const uint8_t*>(event_str), strlen(event_str));
  event->value.byteArray = genWeexByteArray(
      static_cast<const char*>(buffer->data), buffer->position);
  buffer->data = nullptr;
  wson_buffer_free(buffer);
  return event;
}

inline static VALUE_WITH_TYPE* GenValueWithType(int i) {
  VALUE_WITH_TYPE* event = getValueWithTypePtr();
  event->type = ParamsType::INT32;
  event->value.int32Value = i;
  return event;
}
static void GenWSONArray(wson_buffer* buffer, Array* array);
static void GenWSONTable(wson_buffer* buffer, Table* table);
static void GenWSONValue(wson_buffer* buffer, Value* value) {
  switch (value->type) {
    case Value::Type::ARRAY:
      return GenWSONArray(buffer, ValueTo<Array>(value));
    case Value::Type::TABLE:
      return GenWSONTable(buffer, ValueTo<Table>(value));
    case Value::Type::INT:
      wson_push_type_long(buffer, value->i);
      break;
    case Value::Type::NUMBER:
      wson_push_type_double(buffer, value->n);
      break;
    case Value::Type::STRING:
      wson_push_type_uint8_string(
          buffer, reinterpret_cast<const uint8_t*>(value->str->c_str()),
          strlen(value->str->c_str()));
      break;
    case Value::Type::BOOL:
      wson_push_type_boolean(buffer, value->b);
      break;
    default:
      wson_push_type_null(buffer);
      break;
  }
}

static void GenWSONArray(wson_buffer* buffer, Array* array) {
  wson_push_type_array(buffer, array->items.size());
  for (auto it = array->items.begin(); it != array->items.end(); it++) {
    GenWSONValue(buffer, &*it);
  }
}

static void GenWSONTable(wson_buffer* buffer, Table* table) {
  wson_push_type_map(buffer, table->map.size());
  for (auto it = table->map.begin(); it != table->map.end(); it++) {
    if (it->first.empty()) continue;
    // Use utf16 as wson only support utf16 as key in map
    auto utf16_key = weex::base::to_utf16(const_cast<char*>(it->first.c_str()),
                                          it->first.size());
    wson_push_uint(buffer, utf16_key.size() * sizeof(uint16_t));
    wson_push_bytes(buffer, utf16_key.c_str(),
                    utf16_key.size() * sizeof(uint16_t));
    GenWSONValue(buffer, &it->second);
  }
}
static void GenWSONValue(wson_buffer* buffer, const std::string& str);
template <typename Type>
static void GenWSONValue(wson_buffer* buffer, const std::vector<Type>& vector);
template <typename Value>
static void GenWSONValue(wson_buffer* buffer,
                         const std::unordered_map<std::string, Value>& ref_map);

static void GenWSONValue(wson_buffer* buffer, Table* t) {
  GenWSONTable(buffer, t);
}

static void GenWSONValue(wson_buffer* buffer, int32_t i) {
  wson_push_type_int(buffer, i);
}

static void GenWSONValue(wson_buffer* buffer, const std::string& str) {
  wson_push_type_uint8_string(
      buffer, reinterpret_cast<const uint8_t*>(str.c_str()), str.size());
}

template <typename Type>
static void GenWSONValue(wson_buffer* buffer, const std::vector<Type>& vector) {
  wson_push_type_array(buffer, vector.size());
  for (auto it = vector.begin(); it != vector.end(); it++) {
    GenWSONValue(buffer, *it);
  }
}

template <typename Value>
static void GenWSONValue(
    wson_buffer* buffer,
    const std::unordered_map<std::string, Value>& ref_map) {
  wson_push_type_map(buffer, ref_map.size());
  for (auto it = ref_map.begin(); it != ref_map.end(); it++) {
    if (it->first.empty()) continue;
    // Use utf16 as wson only support utf16 as key in map
    auto utf16_key = weex::base::to_utf16(const_cast<char*>(it->first.c_str()),
                                          it->first.size());
    wson_push_uint(buffer, utf16_key.size() * sizeof(uint16_t));
    wson_push_bytes(buffer, utf16_key.c_str(),
                    utf16_key.size() * sizeof(uint16_t));
    GenWSONValue(buffer, it->second);
  }
}

//template <typename... Args>
//static void GenParamsForCallJS(std::vector<VALUE_WITH_TYPE*>& params, VComponent* component,
//                               const char* event, int32_t id, size_t argc_inside, Args... args_inside) {
//  auto page_id = component->exec_state()->context()->page_id();
//  // page_id
//  params.push_back(GenValueWithType(page_id.c_str()));
//  // args -> { method: 'componentHook', args: [ templateId, 'lifecycle',
//  // lifecycle, [componentId, parentId, data, props, refList] ] }
//
//  VALUE_WITH_TYPE* args = getValueWithTypePtr();
//  args->type = ParamsType::BYTEARRAY;
//  auto buffer = wson_buffer_new();
//  wson_push_type_array(buffer, 1);
//  wson_push_type_map(buffer, 2);
//  // key -> method
//  auto utf16_key_method =
//          weex::base::to_utf16(const_cast<char*>(kKeyMethod), strlen(kKeyMethod));
//  wson_push_uint(buffer, utf16_key_method.size() * sizeof(uint16_t));
//  wson_push_bytes(buffer, utf16_key_method.c_str(),
//                  utf16_key_method.size() * sizeof(uint16_t));
//  // value -> 'componentHook'
//  wson_push_type_uint8_string(buffer,
//                              reinterpret_cast<const uint8_t*>(kComponentHook),
//                              strlen(kComponentHook));
//  // key -> args
//  auto utf16_key_args =
//          weex::base::to_utf16(const_cast<char*>(kKeyArgs), strlen(kKeyArgs));
//  wson_push_uint(buffer, utf16_key_args.size() * sizeof(uint16_t));
//  wson_push_bytes(buffer, utf16_key_args.c_str(),
//                  utf16_key_args.size() * sizeof(uint16_t));
//  // value -> [ templateId, 'lifecycle', lifecycle, [componentId, parentId,
//  // data, props, refList] ]
//  {
//    wson_push_type_array(buffer, 4);
//    // templateId
//    wson_push_type_int(buffer, component->template_id());
//    // 'lifecycle'
//    wson_push_type_uint8_string(buffer,
//                                reinterpret_cast<const uint8_t*>(kLifecycle),
//                                strlen(kLifecycle));
//    // lifecycle
//    wson_push_type_uint8_string(
//            buffer, reinterpret_cast<const uint8_t*>(event),
//            strlen(event));
//    // array -> [componentId, parentId, data, props, refList]
//    {
//      wson_push_type_array(buffer, argc_inside);
//      for (int i = 0; i < argc_inside; i++) {
//          (GenWSONValue(std::forward< Args>(args_inside)),0)...);
//      }
//    }
//  }
//
//  args->value.byteArray = genWeexByteArray(
//          static_cast<const char*>(buffer->data), buffer->position);
//  buffer->data = nullptr;
//  wson_buffer_free(buffer);
//  params.push_back(args);
//}



void VComponentLifecycleListener::OnCreated(
    VComponent* component, Table* data, Table* props,
    const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map) {
  auto page_id = component->exec_state()->context()->page_id();

  std::vector<VALUE_WITH_TYPE*> params;
  // page_id
  params.push_back(GenValueWithType(page_id.c_str()));
  // args -> { method: 'componentHook', args: [ templateId, 'lifecycle',
  // lifecycle, [componentId, parentId, data, props, refList] ] }

  VALUE_WITH_TYPE* args = getValueWithTypePtr();
  args->type = ParamsType::BYTEARRAY;
  auto buffer = wson_buffer_new();
  wson_push_type_array(buffer, 1);
  wson_push_type_map(buffer, 2);
  // key -> method
  auto utf16_key_method =
      weex::base::to_utf16(const_cast<char*>(kKeyMethod), strlen(kKeyMethod));
  wson_push_uint(buffer, utf16_key_method.size() * sizeof(uint16_t));
  wson_push_bytes(buffer, utf16_key_method.c_str(),
                  utf16_key_method.size() * sizeof(uint16_t));
  // value -> 'componentHook'
  wson_push_type_uint8_string(buffer,
                              reinterpret_cast<const uint8_t*>(kComponentHook),
                              strlen(kComponentHook));
  // key -> args
  auto utf16_key_args =
      weex::base::to_utf16(const_cast<char*>(kKeyArgs), strlen(kKeyArgs));
  wson_push_uint(buffer, utf16_key_args.size() * sizeof(uint16_t));
  wson_push_bytes(buffer, utf16_key_args.c_str(),
                  utf16_key_args.size() * sizeof(uint16_t));
  // value -> [ templateId, 'lifecycle', lifecycle, [componentId, parentId,
  // data, props, refList] ]
  {
    wson_push_type_array(buffer, 4);
    // templateId
    wson_push_type_int(buffer, component->template_id());
    // 'lifecycle'
    wson_push_type_uint8_string(buffer,
                                reinterpret_cast<const uint8_t*>(kLifecycle),
                                strlen(kLifecycle));
    // lifecycle
    wson_push_type_uint8_string(
        buffer, reinterpret_cast<const uint8_t*>(kEventOnCreated),
        strlen(kEventOnCreated));
    // array -> [componentId, parentId, data, props, refList]
    {
      wson_push_type_array(buffer, 5);
      // componentId
      wson_push_type_int(buffer, component->id());
      // parentId
      if (component->parent() != NULL) {
        wson_push_type_int(buffer, component->parent()->component()->id());
      } else {
        wson_push_type_int(buffer, INT32_MAX);
      }
      // data
      GenWSONTable(buffer, data);
      // props
      GenWSONTable(buffer, props);
      // refList
      GenWSONValue(buffer, ref_map);
    }
  }

  args->value.byteArray = genWeexByteArray(
      static_cast<const char*>(buffer->data), buffer->position);
  buffer->data = nullptr;
  wson_buffer_free(buffer);
  params.push_back(args);

  WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      page_id.c_str(), "", kMethodOnComponentEvent, params);
  freeParams(params);
}

void VComponentLifecycleListener::OnUpdated(
    VComponent* component, Table* props,
    const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map) {
  auto page_id = component->exec_state()->context()->page_id();

  std::vector<VALUE_WITH_TYPE*> params;
  // page_id
  params.push_back(GenValueWithType(page_id.c_str()));
  // args -> { method: 'componentHook', args: [ componentId, 'lifecycle',
  // lifecycle, [props, refList] ] }

  VALUE_WITH_TYPE* args = getValueWithTypePtr();
  args->type = ParamsType::BYTEARRAY;
  auto buffer = wson_buffer_new();
    wson_push_type_array(buffer, 1);
  wson_push_type_map(buffer, 2);
  // key -> method
  auto utf16_key_method =
      weex::base::to_utf16(const_cast<char*>(kKeyMethod), strlen(kKeyMethod));
  wson_push_uint(buffer, utf16_key_method.size() * sizeof(uint16_t));
  wson_push_bytes(buffer, utf16_key_method.c_str(),
                  utf16_key_method.size() * sizeof(uint16_t));
  // value -> 'componentHook'
  wson_push_type_uint8_string(buffer,
                              reinterpret_cast<const uint8_t*>(kComponentHook),
                              strlen(kComponentHook));
  // key -> args
  auto utf16_key_args =
      weex::base::to_utf16(const_cast<char*>(kKeyArgs), strlen(kKeyArgs));
  wson_push_uint(buffer, utf16_key_args.size() * sizeof(uint16_t));
  wson_push_bytes(buffer, utf16_key_args.c_str(),
                  utf16_key_args.size() * sizeof(uint16_t));
  // value -> [ componentId, 'lifecycle', lifecycle, [props, refList] ]
  {
    wson_push_type_array(buffer, 4);
    // componentId
    wson_push_type_int(buffer, component->id());
    // 'lifecycle'
    wson_push_type_uint8_string(buffer,
                                reinterpret_cast<const uint8_t*>(kLifecycle),
                                strlen(kLifecycle));
    // lifecycle
    wson_push_type_uint8_string(
        buffer, reinterpret_cast<const uint8_t*>(kEventOnUpdated),
        strlen(kEventOnUpdated));
    // array -> [props, refList]
    {
      wson_push_type_array(buffer, 2);
      // props
      GenWSONTable(buffer, props);
      // refList
      GenWSONValue(buffer, ref_map);
    }
  }
  args->value.byteArray = genWeexByteArray(
      static_cast<const char*>(buffer->data), buffer->position);
  buffer->data = nullptr;
  wson_buffer_free(buffer);
  params.push_back(args);

  WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      page_id.c_str(), "", kMethodOnComponentEvent, params);
  freeParams(params);
}

void VComponentLifecycleListener::OnDestroyed(VComponent* component) {
  auto page_id = component->exec_state()->context()->page_id();
  std::vector<VALUE_WITH_TYPE*> params;
  // page_id
  params.push_back(GenValueWithType(page_id.c_str()));
  // args -> { method: 'componentHook', args: [ componentId, 'lifecycle',
  // lifecycle, [] ] }

  VALUE_WITH_TYPE* args = getValueWithTypePtr();
  args->type = ParamsType::BYTEARRAY;
  auto buffer = wson_buffer_new();
    wson_push_type_array(buffer, 1);
  wson_push_type_map(buffer, 2);
  // key -> method
  auto utf16_key_method =
      weex::base::to_utf16(const_cast<char*>(kKeyMethod), strlen(kKeyMethod));
  wson_push_uint(buffer, utf16_key_method.size() * sizeof(uint16_t));
  wson_push_bytes(buffer, utf16_key_method.c_str(),
                  utf16_key_method.size() * sizeof(uint16_t));
  // value -> 'componentHook'
  wson_push_type_uint8_string(buffer,
                              reinterpret_cast<const uint8_t*>(kComponentHook),
                              strlen(kComponentHook));
  // key -> args
  auto utf16_key_args =
      weex::base::to_utf16(const_cast<char*>(kKeyArgs), strlen(kKeyArgs));
  wson_push_uint(buffer, utf16_key_args.size() * sizeof(uint16_t));
  wson_push_bytes(buffer, utf16_key_args.c_str(),
                  utf16_key_args.size() * sizeof(uint16_t));
  // value -> [ componentId, 'lifecycle', lifecycle, [props, refList] ]
  {
    wson_push_type_array(buffer, 4);
    // componentId
    wson_push_type_int(buffer, component->id());
    // 'lifecycle'
    wson_push_type_uint8_string(buffer,
                                reinterpret_cast<const uint8_t*>(kLifecycle),
                                strlen(kLifecycle));
    // lifecycle
    wson_push_type_uint8_string(
        buffer, reinterpret_cast<const uint8_t*>(kEventOnDestroyed),
        strlen(kEventOnDestroyed));
    // array -> []
    { wson_push_type_array(buffer, 0); }
  }
  args->value.byteArray = genWeexByteArray(
      static_cast<const char*>(buffer->data), buffer->position);
  buffer->data = nullptr;
  wson_buffer_free(buffer);
  params.push_back(args);

  WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      page_id.c_str(), "", kMethodOnComponentEvent, params);
  freeParams(params);
}
}  // namespace data_render
}  // namespace core
}  // namespace weex