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

#include "core/data_render/vnode/android/conversion.h"
#include "android/utils/params_utils.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/object.h"
#include "core/manager/weex_core_manager.h"
#include "wson/wson.h"
#include "wson/wson_parser.h"

namespace weex {
namespace core {
namespace data_render {

json11::Json Conversion::GenJSON(const Value* v) {
  switch (v->type) {
    case Value::Type::NUMBER:
      return v->n;
    case Value::Type::STRING:
      return v->str->c_str();
    case Value::Type::INT:
      return static_cast<int>(v->i);
    case Value::Type::BOOL:
      return v->b;
    case Value::Type::ARRAY: {
      Array* array = ValueTo<Array>(v);
      std::vector<json11::Json> json_array;
      for (auto it = array->items.begin(); it != array->items.end(); it++) {
        json_array.push_back(GenJSON(&*it));
      }
      return json_array;
    }
    case Value::Type::TABLE: {
      Table* table = ValueTo<Table>(v);
      std::map<std::string, json11::Json> json_table;
      for (auto it = table->map.begin(); it != table->map.end(); it++) {
        json_table.insert({it->first, GenJSON(&it->second)});
      }
      return json_table;
    }
    case Value::Type::NIL:
      return nullptr;
  }
}

VALUE_WITH_TYPE* Conversion::GenValueWithType(const char* event_str) {
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

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif