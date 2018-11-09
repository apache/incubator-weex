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
#ifdef OS_ANDROID

#include "core/data_render/vnode/vcomponent_lifecycle_listener.h"
#include "android/utils/params_utils.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/vnode/android/conversion.h"
#include "core/manager/weex_core_manager.h"
#include "wson/wson.h"

namespace weex {
namespace core {
namespace data_render {

static const char* kMethodOnComponentEvent = "callJS";
static const char* kComponentHook = "componentHook";
static const char* kLifecycle = "lifecycle";
static const char* kKeyMethod = "method";
static const char* kKeyArgs = "args";
static const char* kEventOnCreated = "created";
static const char* kEventOnUpdated = "updated";
static const char* kEventOnDestroyed = "destroyed";
static int kNonParentId = -1;

void GenWSONValue(wson_buffer* buffer, Value* value);
void GenWSONValue(wson_buffer* buffer, Table* t);
void GenWSONValue(wson_buffer* buffer, int32_t i);
void GenWSONArray(wson_buffer* buffer, Array* array);
void GenWSONTable(wson_buffer* buffer, Table* table);

void GenWSONValue(wson_buffer* buffer, const std::string& str);
template <typename Type>
void GenWSONValue(wson_buffer* buffer, const std::vector<Type>& vector);
template <typename Type>
void GenWSONValue(wson_buffer* buffer,
                  const std::unordered_map<std::string, Type>& ref_map);

void GenWSONValue(wson_buffer* buffer, Value* value) {
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

void GenWSONArray(wson_buffer* buffer, Array* array) {
  wson_push_type_array(buffer, array->items.size());
  for (auto it = array->items.begin(); it != array->items.end(); it++) {
    GenWSONValue(buffer, &*it);
  }
}

void GenWSONTable(wson_buffer* buffer, Table* table) {
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

void GenWSONValue(wson_buffer* buffer, Table* t) { GenWSONTable(buffer, t); }

void GenWSONValue(wson_buffer* buffer, int32_t i) {
  wson_push_type_int(buffer, i);
}

void GenWSONValue(wson_buffer* buffer, const std::string& str) {
  wson_push_type_uint8_string(
      buffer, reinterpret_cast<const uint8_t*>(str.c_str()), str.size());
}

template <typename Type>
void GenWSONValue(wson_buffer* buffer, const std::vector<Type>& vector) {
  wson_push_type_array(buffer, vector.size());
  for (auto it = vector.begin(); it != vector.end(); it++) {
    GenWSONValue(buffer, *it);
  }
}

template <typename Type>
void GenWSONValue(wson_buffer* buffer,
                  const std::unordered_map<std::string, Type>& ref_map) {
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

static void UnpackAndGenWSONValue(wson_buffer* buffer) {}

template <typename T>
static void UnpackAndGenWSONValue(wson_buffer* buffer, T arg) {
  GenWSONValue(buffer, std::forward<T>(arg));
}

template <typename T, typename... Args>
static void UnpackAndGenWSONValue(wson_buffer* buffer, T arg, Args... args) {
  GenWSONValue(buffer, std::forward<T>(arg));
  UnpackAndGenWSONValue(buffer, args...);
}

template <typename... Args>
static void GenParamsForCallJS(std::vector<VALUE_WITH_TYPE*>& params,
                               VComponent* component, const char* event,
                               int32_t id, size_t argc_inside,
                               Args... args_inside) {
  auto page_id = component->exec_state()->context()->page_id();
  // page_id
  params.push_back(Conversion::GenValueWithType(page_id.c_str()));

  // args -> { method: 'componentHook', args: [ id, 'lifecycle',
  // event, [args_inside ] }
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
  // value -> [ id, 'lifecycle', event, [ args_inside ] ]
  {
    wson_push_type_array(buffer, 4);
    // templateId
    wson_push_type_int(buffer, id);
    // 'lifecycle'
    wson_push_type_uint8_string(buffer,
                                reinterpret_cast<const uint8_t*>(kLifecycle),
                                strlen(kLifecycle));
    // lifecycle
    wson_push_type_uint8_string(buffer, reinterpret_cast<const uint8_t*>(event),
                                strlen(event));
    // array -> [ args_inside ]
    {
      wson_push_type_array(buffer, argc_inside);
      for (int i = 0; i < argc_inside; i++) {
        UnpackAndGenWSONValue(buffer, args_inside...);
      }
    }
  }

  args->value.byteArray = genWeexByteArray(
      static_cast<const char*>(buffer->data), buffer->position);
  buffer->data = nullptr;
  wson_buffer_free(buffer);
  params.push_back(args);
}

void VComponentLifecycleListener::OnCreated(
    VComponent* component, Table* data, Table* props,
    const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map) {
  std::vector<VALUE_WITH_TYPE*> params;
  // parentId
  int parentId = kNonParentId;
  if (component->parent() != NULL) {
    parentId = component->parent()->component()->id();
  }
  // [pageId, args]
  //
  // args -> { method: 'componentHook', args: [ templateId, 'lifecycle',
  // lifecycle, [componentId, parentId, data, props, refList] ] }
  GenParamsForCallJS(params, component, kEventOnCreated,
                     component->template_id(), 5, component->id(), parentId,
                     data, props, ref_map);
  auto page_id = component->exec_state()->context()->page_id();
  WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      page_id.c_str(), "", kMethodOnComponentEvent, params);
  freeParams(params);
}

void VComponentLifecycleListener::OnUpdated(
    VComponent* component, Table* props,
    const std::unordered_map<std::string, VComponent::VNodeRefs>& ref_map) {
  auto page_id = component->exec_state()->context()->page_id();

  std::vector<VALUE_WITH_TYPE*> params;
  // [pageId, args]
  //
  // args -> { method: 'componentHook', args: [ componentId, 'lifecycle',
  // lifecycle, [props, refList] ] }
  GenParamsForCallJS(params, component, kEventOnUpdated, component->id(), 2,
                     props, ref_map);
  WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      page_id.c_str(), "", kMethodOnComponentEvent, params);
  freeParams(params);
}

void VComponentLifecycleListener::OnDestroyed(VComponent* component) {
  auto page_id = component->exec_state()->context()->page_id();
  std::vector<VALUE_WITH_TYPE*> params;
  // [pageId, args]
  //
  // args -> { method: 'componentHook', args: [ componentId, 'lifecycle',
  // lifecycle, [] ] }
  GenParamsForCallJS(params, component, kEventOnDestroyed, component->id(), 0);
  WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      page_id.c_str(), "", kMethodOnComponentEvent, params);
  freeParams(params);
}
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif