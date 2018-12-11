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
#include "core/data_render/class_object.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/table.h"
#include "core/data_render/class_array.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

static Value keys(ExecState *exec_state);

ClassDescriptor *NewClassOject() {
    ClassDescriptor *object_desc = new ClassDescriptor(nullptr);
    AddClassStaticCFunc(object_desc, "keys", keys);
    return object_desc;
}
    
static Value keys(ExecState *exec_state) {
    Value ret = exec_state->class_factory()->CreateArray();
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 1) {
            break;
        }
        Value *value = exec_state->GetArgument(0);
        if (!IsArray(value) && !IsTable(value)) {
            throw VMExecError("Object.keys caller isn't a Array or Table");
        }
        if (IsTable(value)) {
            std::vector<std::string> keys = GetTableKeys(ValueTo<Table>(value));
            Array *array = ValueTo<Array>(&ret);
            for (int i = 0; i < keys.size(); i++) {
                SetArray(array, i, exec_state->string_table()->StringFromUTF8(keys[i]));
            }
        }
        
    } while (0);
    
    return ret;
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
