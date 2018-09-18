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
#include "core/data_render/object.h"
#include "core/data_render/table.h"
#include "third_party/json11/json11.hpp"
#include "core/data_render/common_error.h"

namespace weex {
namespace core {
namespace data_render {

int IndexOf(const std::vector<Value> *arr, const Value *val) {
  auto it = std::find(arr->begin(), arr->end(), val);
  if (it != arr->end()) {
    return static_cast<int>(std::distance(arr->begin(), it));
  } else {
    return -1;
  }
}

int SetTableForTable(Table *table, const Value *src, const Value &val) {
    int ret = 0;
    do {
        if (!IsTable(src)) {
            break;
        }
        Table *src_table = ValueTo<Table>(src);
        for (auto iter = src_table->map.begin(); iter != src_table->map.end(); iter++) {
            auto find = table->map.find(iter->first);
            if (find != table->map.end()) {
                GCRelease(&find->second);
                table->map.erase(find);
            }
            table->map.insert(std::make_pair(iter->first, iter->second));
            GCRetain(&iter->second);
        }
        ret = 1;
        
    } while (0);
    
    return ret;
}

int SetTableForKey(Table *t, const Value *key, const Value &val) {
    if (IsNil(key)) {
        return 0;
    }
    std::string keyStr = CStringValue(key);
    if (keyStr.empty()) {
        return 0;
    }
    auto iter = t->map.find(keyStr);
    if (iter != t->map.end()) {
        GCRelease(&iter->second);
        t->map.erase(iter);
    }
    t->map.insert(std::make_pair(keyStr, val));
    GCRetain((Value *)&val);
    return 1;
}

Value *GetTableForKey(Table *t, const Value *key) {
    std::string str = CStringValue(key);
    if (!str.empty()) {
        auto it = t->map.find(str);
        if (it != t->map.end()) {
            return &(it->second);
        }
    }
    return nullptr;
}

Value *GetTableValue(Table *t, const Value &key) {
    if (IsString(&key)) {
        return GetTableForKey(t, &key);
    }
    return nullptr;
}
    
Value *GetTableVar(Table *table, const Value &key) {
    Value *ret = nullptr;
    do {
        if (!IsString(&key)) {
            throw VMExecError("can't get table var when the key isn't string");
        }
        ret = GetTableForKey(table, &key);
        if (!ret) {
            std::string keystr = CStringValue(&key);
            table->map.insert(std::make_pair(keystr, Value()));
            ret = &table->map.find(keystr)->second;
        }
        
    } while (0);
    
    return ret;
}

int SetTableValue(Table *t, Value *key, const Value &val) {
    if (IsString(key)) {
        return SetTableForKey(t, key, val);
    }
    else if (IsTable(key)) {
        return SetTableForTable(t, key, val);
    }
    return 0;
}

size_t GetTableSize(Table *table) {
    size_t size = 0;
    if (table) {
        size = table->map.size();
    }
    return size;
}
    
json11::Json TableToJson(Table *table);
    
json11::Json ArrayToJson(Array *array) {
    json11::Json::array json;
    for (int i = 0; i < array->items.size(); i++) {
        Value item = array->items[i];
        if (item.type == Value::Type::TABLE) {
            json.push_back(TableToJson(ValueTo<Table>(&item)));
        }
        else if (item.type == Value::Type::STRING) {
            json.push_back(CStringValue(&item));
        }
        else if (item.type == Value::Type::INT) {
            json.push_back(to_string(IntValue(&item)));
        }
        else if (item.type == Value::Type::ARRAY) {
            json.push_back(ArrayToJson(ValueTo<Array>(&item)));
        }
    }
    return json;
}
    
std::string ArrayToString(Array *array) {
    json11::Json json = ArrayToJson(array);
    std::string json_string;
    json.dump(json_string);
    return json_string;
}
    
json11::Json TableToJson(Table *table) {
    json11::Json::object obj;
    for (auto iter = table->map.begin(); iter != table->map.end(); iter++) {
        if (iter->second.type == Value::Type::TABLE) {
            obj.insert(std::make_pair(iter->first, TableToJson(ValueTo<Table>(&iter->second))));
        }
        else if (iter->second.type == Value::Type::STRING) {
            obj.insert(std::make_pair(iter->first, CStringValue(&iter->second)));
        }
        else if (iter->second.type == Value::Type::INT) {
            obj.insert(std::make_pair(iter->first, to_string(IntValue(&iter->second))));
        }
        else if (iter->second.type == Value::Type::ARRAY) {
            obj.insert(std::make_pair(iter->first, ArrayToJson(ValueTo<Array>(&iter->second))));
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
    
int TableInKey(StringTable *string_table,Table *table, Value *condition, Value *var) {
    int ret = 1;
    do {
        if (!table->map.size()) {
            SetBValue(condition, false);
            break;
        }
        if (!IsString(var)) {
            String *key = string_table->StringFromUTF8(table->map.begin()->first);
            SetBValue(condition, true);
            SetSValue(var, key);
            break;
        }
        bool exist = false;
        auto iter = table->map.begin();
        for (; iter != table->map.end(); iter++) {
            if (strcmp(CStringValue(var), iter->first.c_str()) == 0) {
                exist = true;
                iter++;
                break;
            }
        }
        if (!exist) {
            String *key = string_table->StringFromUTF8(table->map.begin()->first);
            SetBValue(condition, true);
            SetSValue(var, key);
            break;
        }
        if (iter == table->map.end()) {
            SetBValue(condition, false);
            SetNil(var);
            break;
        }
        else {
            String *key = string_table->StringFromUTF8(iter->first);
            SetBValue(condition, true);
            SetSValue(var, key);
        }
        
    } while (0);
    
    return ret;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
