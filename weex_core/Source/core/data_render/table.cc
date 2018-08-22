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
#include "third_party/json11/json11.hpp"

namespace weex {
namespace core {
namespace data_render {


template <typename T>
std::string to_string(T value)
{
  std::ostringstream os ;
  os << value ;
  return os.str() ;
}

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
//  int index = IndexOf(&t->array, &val);
//  if (index < 0) {
//    t->array.emplace_back(val);
//    index = t->array.size() - 1;
//  }
  if (nullptr != key) {
    //SetIValue(key, index);
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
  auto it = t->map.find(keyStr);
  if (it != t->map.end()) {
    t->map.erase(it);
  }
  t->map.insert(std::make_pair(keyStr, val));
    if (Value::Type::STRING == val.type) {
        printf("SetTabStringValue: key:%s => value:%s\n", CStringValue(key), val.str->c_str());
    }
    else {
        printf("SetTabStringValue: key:%s =>\n", CStringValue(key));
    }
  return 1;
}

Value *GetTabIntValue(Table *t, const Value *key) {
//  int index = IntValue(key);
//  if (index < t->array.size()) {
//    return &(t->array.at(index));
//  }
  return nullptr;
}

Value *GetTabStringValue(Table *t, const Value *key) {
  std::string str = CStringValue(key);
  if (!str.empty()) {
    auto it = t->map.find(str);
    if (it != t->map.end()) {
      return &(it->second);
    }
  }
  return nullptr;
}

Table *NewTable() {
//  Table *t = reinterpret_cast<Table *>(reallocMem(nullptr, sizeof(Table)));
//  if (nullptr == t) {
//    return nullptr;
//  }
//  t->array = new std::vector<Value>();
//  t->map = new std::unordered_map<std::string, Value>();
  Table *t = new Table();
  return t;
}

int ResizeTab(Table *t, size_t nasize, size_t nhsize) {

  return 1;
}

Value *GetTabValue(Table *t, const Value &key) {
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

size_t GetTableSize(Table *t) {
  size_t m_size = GetMapSize(t);
  return m_size;
}

size_t GetArraySize(Array *t) {
  if (nullptr == t) {
    return 0;
  }
  return t->items.size();
}

size_t GetMapSize(Table *t) {
  if (nullptr == t) {
    return -1;
  }
  return t->map.size();
}

size_t GetValueArraySize(Value &o) {
  return GetArraySize(ObjectValue<Array>(&o));
}

size_t GetValueMapSize(Value &o) {
  return GetMapSize(ObjectValue<Table>(&o));
}
    
#ifdef DEBUG
json11::Json TableToJson(Table *table);
    
json11::Json ArrayToJson(Array *array) {
    json11::Json::array json;
    for (int i = 0; i < array->items.size(); i++) {
        Value item = array->items[i];
        if (item.type == Value::Type::TABLE) {
            json.push_back(TableToJson(ObjectValue<Table>(&item)));
        }
        else if (item.type == Value::Type::STRING) {
            json.push_back(CStringValue(&item));
        }
        else if (item.type == Value::Type::INT) {
            json.push_back(to_string(IntValue(&item)));
        }
        else if (item.type == Value::Type::ARRAY) {
            json.push_back(ArrayToJson(ObjectValue<Array>(&item)));
        }
    }
    return json;
}
    
json11::Json TableToJson(Table *table) {
    json11::Json::object obj;
    for (auto iter = table->map.begin(); iter != table->map.end(); iter++) {
        if (iter->second.type == Value::Type::TABLE) {
            obj.insert(std::make_pair(iter->first, TableToJson(ObjectValue<Table>(&iter->second))));
        }
        else if (iter->second.type == Value::Type::STRING) {
            obj.insert(std::make_pair(iter->first, CStringValue(&iter->second)));
        }
        else if (iter->second.type == Value::Type::INT) {
            obj.insert(std::make_pair(iter->first, to_string(IntValue(&iter->second))));
        }
        else if (iter->second.type == Value::Type::ARRAY) {
            obj.insert(std::make_pair(iter->first, ArrayToJson(ObjectValue<Array>(&iter->second))));
        }
    }
    return json11::Json(obj);
}
    
std::string TableToString(Table *table) {
    json11::Json json = TableToJson(table);
    std::string json_string;
    json.dump(json_string);
    return json_string;
}
    
#endif

}  // namespace data_render
}  // namespace core
}  // namespace weex
