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
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/table.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

static Value isArray(ExecState *exec_state);
static Value push(ExecState *exec_state);
static Value slice(ExecState *exec_state);
static Value forEach(ExecState *exec_state);
static Value indexOf(ExecState *exec_state);
static Value map(ExecState *exec_state);

ClassDescriptor *NewClassArray() {
    ClassDescriptor *array_desc = new ClassDescriptor(nullptr);
    AddClassStaticCFunc(array_desc, "isArray", isArray);
    AddClassCFunc(array_desc, "push", push);
    AddClassCFunc(array_desc, "slice", slice);
    AddClassCFunc(array_desc, "forEach", forEach);
    AddClassCFunc(array_desc, "indexOf", indexOf);
    AddClassCFunc(array_desc, "map", map);
    return array_desc;
}

int SetArray(Array *array, Value *index, const Value &val) {
    int ret = 0;
    do {
        if (!IsInt(index)) {
            break;
        }
        int64_t index_of = index->i;
        if (index_of > (int)array->items.size()) {
            break;
        }
        array->items.insert(array->items.begin()+ static_cast<int>(index_of), val);
        GCRetain((Value *)&val);
        ret = true;
        
    } while (0);
   
    return ret;
}
    
void ClearArray(Array *array) {
    array->items.clear();
}
    
void PushArray(Array *array, Value val) {
    array->items.push_back(val);
}
    
int SetArray(Array *array, int index, const Value &val) {
    int ret = 0;
    do {
        if (index >= (int)array->items.size()) {
            array->items.push_back(val);
        }
        else {
            array->items.insert(array->items.begin() + index, val);
        }
        GCRetain((Value *)&val);
        ret = true;

    } while (0);
    
    return ret;
}
    
Value* GetArrayVar(Array *array, const Value &index) {
    Value *ret = nullptr;
    do {
        if (!IsInt(&index)) {
            break;
        }
        int indexValue = (int)IntValue(&index);
        int size = (int)array->items.size();
        if (indexValue < size) {
            ret = &array->items.at(indexValue);
        }
        else {
            int inserts = indexValue - size + 1;
            for (int i = 0; i < inserts; i++) {
                array->items.push_back(Value());
            }
            ret = &array->items.at(indexValue);
        }
        
    } while (0);
    
    return ret;
}
    
Value GetArrayValue(Array *array, const Value &index) {
    Value ret;
    do {
        if (!IsInt(&index)) {
            break;
        }
        int indexValue = (int)IntValue(&index);
        if (indexValue < array->items.size()) {
            ret = array->items.at(indexValue);
        }
        
    } while (0);
    
    return ret;
}
    
Value GetArraySizeValue(Array *array) {
    return Value((int)array->items.size());
}
    
size_t GetArraySize(Array *array) {
    return array->items.size();
}
    
static Value forEach(ExecState *exec_state) {
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 2) {
            break;
        }
        Value *array = exec_state->GetArgument(0);
        if (!IsArray(array)) {
            throw VMExecError("forEach caller isn't a Array");
        }
        Value *func = exec_state->GetArgument(1);
        if (!IsFunction(func)) {
            throw VMExecError("forEach => isn't a function");
        }
        std::vector<Value> items = ValueTo<Array>(array)->items;
        for (int i = 0; i < items.size(); i++) {
            Value item = items[i];
            Value index = Value(i);
            std::vector<Value> args = { item, index };
            exec_state->Call(func, args);
        }
        
    } while (0);
    
    return Value();
}
    
static Value map(ExecState *exec_state) {
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 2) {
            break;
        }
        Value *array = exec_state->GetArgument(0);
        if (!IsArray(array)) {
            throw VMExecError("map caller isn't a Array");
        }
        Value *func = exec_state->GetArgument(1);
        if (!IsFunction(func)) {
            throw VMExecError("map callback isn't a function");
        }
        std::vector<Value> items = ValueTo<Array>(array)->items;
        for (int i = 0; i < items.size(); i++) {
            Value item = items[i];
            Value index = Value(i);
            std::vector<Value> args = { item, index };
            exec_state->Call(func, args);
        }
        
    } while (0);
    
    return Value();
}
    
static Value indexOf(ExecState *exec_state) {
    int index = -1;
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 2) {
            break;
        }
        Value *array = exec_state->GetArgument(0);
        if (!IsArray(array)) {
            throw VMExecError("indexOf caller isn't a Array");
        }
        Value *item = exec_state->GetArgument(1);
        std::vector<Value> items = ValueTo<Array>(array)->items;
        for (int i = 0; i < items.size(); i++) {
            if (ObjectEquals(item, &items[i])) {
                index = i;
                break;
            }
        }
        
    } while (0);

    return Value(index);
}
    
static Value push(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 2) {
        throw VMExecError("argument count error for Array.push");
    }
    Value *array = exec_state->GetArgument(0);
    Value *item = exec_state->GetArgument(1);
    if (!IsArray(array)) {
        throw VMExecError("caller isn't a Array");
    }
    if (IsNil(item)) {
        throw VMExecError("Array.push item can't be nil");
    }
    ValueTo<Array>(array)->items.push_back(*item);
    GCRetain(item);
    return Value();
}

static Value isArray(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 1) {
        throw VMExecError("argument count error for Array.isArray");
    }
    Value *array = exec_state->GetArgument(0);
    if (array->type == Value::Type::ARRAY) {
        return Value(true);
    }
    else {
        return Value(false);
    }
}
    
static Value slice(ExecState *exec_state) {
    Value ret;
    do {
        size_t argc = exec_state->GetArgumentCount();
        if (argc < 2) {
            throw VMExecError("argument count error for Array.slice");
            break;
        }
        Value *array = exec_state->GetArgument(0);
        if (!IsArray(array)) {
            throw VMExecError("Array.slice caller isn't a Array");
        }
        Value *start = exec_state->GetArgument(1);
        if (!IsInt(start)) {
            throw VMExecError("Array.slice start isn't a int");
        }
        int start_index = (int)IntValue(start);
        std::vector<Value> items = ValueTo<Array>(array)->items;
        int end_index = (int)items.size();
        Value *end = nullptr;
        if (argc > 2) {
            end = exec_state->GetArgument(2);
            if (!IsInt(end)) {
                throw VMExecError("Array.slice end isn't a int");
            }
            end_index = (int)IntValue(end);
            if (end_index < 0) {
                end_index = (int)items.size() + end_index;
            }
            if (end_index > items.size()) {
                end_index = (int)items.size();
            }
        }
        ret = exec_state->class_factory()->CreateArray();
        if (start_index >= 0 && start_index < items.size()) {
            for (int i = start_index; i < end_index; i++) {
                GCRetain(&items[i]);
                ValueTo<Array>(&ret)->items.push_back(items[i]);
            }
        }

    } while (0);
    
    return ret;
}
        
}  // namespace data_render
}  // namespace core
}  // namespace weex
