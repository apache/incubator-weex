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
#include "core/data_render/class_json.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "core/data_render/table.h"

#include "core/data_render/vnode/vnode_exec_env.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

static Value stringify(ExecState *exec_state);
static Value parse(ExecState *exec_state);

ClassDescriptor *NewClassJSON() {
    ClassDescriptor *array_desc = new ClassDescriptor(nullptr);
    AddClassStaticCFunc(array_desc, "stringify", stringify);
    AddClassStaticCFunc(array_desc, "parse", parse);
    return array_desc;
}
    
static Value stringify(ExecState *exec_state) {
    Value ret;
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 1) {
            break;
        }
        Value *value = exec_state->GetArgument(0);
        if (!IsArray(value) && !IsTable(value)) {
            throw VMExecError("stringify caller isn't a Array or Table");
        }
        std::string json_string = "";
        if (IsTable(value)) {
            json_string = TableToString(ValueTo<Table>(value));
        }
        else if (IsArray(value)) {
            json_string = ArrayToString(ValueTo<Array>(value));
        }
        ret = exec_state->string_table()->StringFromUTF8(json_string);
        
    } while (0);
    
    return ret;
}
    
static Value parse(ExecState *exec_state) {
    Value ret;
    do {
        size_t length = exec_state->GetArgumentCount();
        if (length < 1) {
            break;
        }
        Value *value = exec_state->GetArgument(0);
        if (!IsString(value)) {
            throw VMExecError("json parse caller isn't a String");
        }
        std::string json_string = CStringValue(value);
        ret = StringToValue(exec_state, json_string);        
        
    } while (0);
    
    return ret;

}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
