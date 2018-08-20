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
    
ClassDescriptor *NewClassArray() {
    ClassDescriptor *array_desc = new ClassDescriptor(nullptr);
    AddClassStaticCFunc(array_desc, "isArray", isArray);
    return array_desc;
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
