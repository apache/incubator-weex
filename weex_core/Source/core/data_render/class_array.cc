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
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/vm_mem.h"
#include "core/data_render/common_error.h"

namespace weex {
namespace core {
namespace data_render {

static Value isArray(ExecState *exec_state);
static Value push(ExecState *exec_state);
    
ClassDescriptor *NewClassArray() {
    ClassDescriptor *array_desc = new ClassDescriptor(nullptr);
    AddClassStaticCFunc(array_desc, "isArray", isArray);
    AddClassCFunc(array_desc, "push", push);
    return array_desc;
}
    
int IndexOf(const std::vector<Value> &items, const Value *val) {
    auto iterator = std::find(items.begin(), items.end(), val);
    if (iterator != items.end()) {
        return (int)std::distance(items.begin(), iterator);
    }
    else {
        return -1;
    }
}

int SetArray(Array *array, Value *index, const Value &val) {
    int ret = 0;
    do {
        if (!IsInt(index)) {
            break;
        }
        int index = IndexOf(array->items, &val);
        if (index < 0) {
            break;
        }
        array->items.emplace_back(val);
        index = (int)array->items.size() - 1;
        ret = true;
        
    } while (0);
   
    return ret;
}
    
Value GetArray(Array *array, const Value &index) {
    Value ret = Value();
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
    
static Value push(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 2) {
        throw VMExecError("Argument Count Error For Array.push");
    }
    Value *array = exec_state->GetArgument(0);
    Value *item = exec_state->GetArgument(1);
    return Value();
}

static Value isArray(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 1) {
        throw VMExecError("Argument Count Error For Array.isArray");
    }
    Value *array = exec_state->GetArgument(0);
    if (array->type == Value::Type::ARRAY) {
        return Value(true);
    }
    else {
        return Value(false);
    }
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
