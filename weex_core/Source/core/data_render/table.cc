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

#include <string.h>
#include <algorithm>
#include "core/data_render/object.h"
#include "core/data_render/table.h"
#include "third_party/json11/json11.hpp"
#include "core/data_render/common_error.h"

namespace weex {
namespace core {
namespace data_render {

int SetTableForTable(Table *table, const Value *src) {
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
    
Value *GetTableForKey(Table *table, const std::string &key) {
    auto iter = table->map.find(key);
    if (iter != table->map.end()) {
        return &(iter->second);
    }
    return nullptr;
}

Value *GetTableForKey(Table *table, const Value *key) {
    std::string keystr = CStringValue(key);
    return GetTableForKey(table, keystr);
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
    
Value *GetTableValue(Table *table, std::string key)
{
    return GetTableForKey(table, key);
}
    
std::vector<std::string> GetTableKeys(Table *table)
{
    std::vector<std::string>keys;
    do {
        if (!table->map.size()) {
            break;
        }
        for (auto iter = table->map.begin(); iter != table->map.end(); iter++) {
            keys.push_back(iter->first);
        }
        
    } while (0);
    
    return keys;
}

int SetTableValue(Table *t, Value *key, const Value &val) {
    if (IsString(key)) {
        return SetTableForKey(t, key, val);
    }
    else if (IsTable(key)) {
        return SetTableForTable(t, key);
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
    
json11::Json ArrayToJson(Array *p_array) {
    json11::Json::array array;
    for (auto it = p_array->items.begin(); it != p_array->items.end(); it++) {
        if (it->type == Value::STRING) {
            array.push_back(json11::Json(it->str->c_str()));
            continue;
        }
        if (it->type == Value::BOOL) {
            array.push_back(json11::Json(it->b));
            continue;
        }
        if (it->type == Value::Type::INT) {
            array.push_back(json11::Json(static_cast<double>(it->i)));
            continue;
        }
        if (it->type == Value::Type::NUMBER) {
            array.push_back(json11::Json(it->n));
            continue;
        }

        if (it->type == Value::TABLE) {
            array.push_back(TableToJson(ValueTo<Table>(&*it)));
            continue;
        }

        if (it->type == Value::ARRAY) {
            array.push_back(ArrayToJson(ValueTo<Array>(&*it)));
            continue;
        }
    }
    return json11::Json(array);
}
    
std::string ArrayToString(Array *array) {
    json11::Json json = ArrayToJson(array);
    std::string json_string;
    json.dump(json_string);
    return json_string;
}
    
json11::Json TableToJson(Table *table) {
    json11::Json::object object;
    for (auto it = table->map.begin(); it != table->map.end(); it++) {
        if (it->second.type == Value::STRING) {
            object.insert({it->first, json11::Json(it->second.str->c_str())});
            continue;
        }
        if (it->second.type == Value::BOOL) {
            object.insert({it->first, json11::Json(it->second.b)});
            continue;
        }
        if (it->second.type == Value::Type::INT) {
            object.insert(
                std::make_pair(it->first, json11::Json(static_cast<double>(it->second.i))));
            continue;
        }
        if (it->second.type == Value::Type::NUMBER) {
            object.insert(
                std::make_pair(it->first, json11::Json(NumValue(&it->second))));
            continue;
        }
        if (it->second.type == Value::TABLE) {
            object.insert({it->first, TableToJson(ValueTo<Table>(&it->second))});
            continue;
        }
        if (it->second.type == Value::ARRAY) {
            object.insert({it->first, ArrayToJson(ValueTo<Array>(&it->second))});
            continue;
        }
        if (it->second.type == Value::FUNC) {
            object.insert({it->first, json11::Json("FUNC")});
        }
        if (it->second.type == Value::FUNC_INST) {
            object.insert({it->first, json11::Json("FUNC_INST")});
        }
    }
    return json11::Json(object);
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
