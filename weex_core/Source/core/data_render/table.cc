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
#include <algorithm>
#include "core/data_render/table.h"
#include "core/data_render/vm_mem.h"

namespace weex {
namespace core {
namespace data_render {

int IndexOf(const std::vector<Value> *arr, const Value *val) {
  auto it = std::find(arr->begin(), arr->end(), val);
  if (it != arr->end()) {
    return std::distance(arr->begin(), it);
  } else {
    return -1;
  }
}

int SetTabIntValue(Table *t, Value *key, const Value &val) {
  if (IsNil(&val)) {
    return 0;
  }
  int index = IndexOf(t->array, &val);
  if (index < 0) {
    t->array->emplace_back(val);
    index = t->array->size() - 1;
  }
  if (nullptr != key) {
    SetIValue(key, index);
  }
  return 1;
}

int SetTabStringValue(Table *t, const Value *key, const Value &val) {
  if (IsNil(key)) {
    return 0;
  }
  std::string keyStr = CStringValue(key);
  if (keyStr.empty()) {
    return 0;
  }
  auto it = t->map->find(keyStr);
  if (it != t->map->end()) {
    t->map->erase(it);
  }
  t->map->insert(std::make_pair(keyStr, val));
  return 1;
}

Value *GetTabIntValue(const Table *t, const Value *key) {
  int index = IntValue(key);
  if (index < t->array->size()) {
    return &(t->array->at(index));
  }
  return nullptr;
}

Value *GetTabStringValue(const Table *t, const Value *key) {
  std::string str = CStringValue(key);
  if (!str.empty()) {
    auto it = t->map->find(str);
    if (it != t->map->end()) {
      return &(it->second);
    }
  }
  return nullptr;
}

Table *NewTable() {
  Table *t = reinterpret_cast<Table *>(reallocMem(nullptr, sizeof(Table)));
  if (nullptr == t) {
    return nullptr;
  }
  t->array = new std::vector<Value>();
  t->map = new std::unordered_map<std::string, Value>();
  return t;
}

int ResizeTab(Table *t, size_t nasize, size_t nhsize) {

  return 1;
}

Value *GetTabValue(const Table *t, const Value &key) {
  if (IsInt(&key)) {
    return GetTabIntValue(t, &key);
  } else if (IsString(&key)) {
    return GetTabStringValue(t, &key);
  }
  return nullptr;
}

int SetTabValue(Table *t, Value *key, const Value &val) {
  if (IsInt(key)) {
    return SetTabIntValue(t, key, val);
  } else if (IsString(key)) {
    return SetTabStringValue(t, key, val);
  }
  return 0;
}

int GetTableSize(Table *t) {
  if (!t) {
    return -1;
  }
  int size = t->array->size();
  if (size > 0) {
    return size;
  } else {
    return t->map->size();
  }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex