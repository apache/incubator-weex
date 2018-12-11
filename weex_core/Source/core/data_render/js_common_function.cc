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

#include "core/data_render/js_common_function.h"
#include "core/data_render/common_error.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

static Value ParseInt(ExecState *state) {
    Value ret;
    do {
        if (state->GetArgumentCount() < 2) {
            throw VMExecError("parseInt must have 2 args");
            break;
        }
        ret = *state->GetArgument(1);
        Value *toInt = state->GetArgument(0);
        int64_t toIntVal = 0;
        if (ToInteger(toInt, 2, toIntVal)) {
            ret = Value(toIntVal);
        }
        
    } while (0);
    
    return ret;
}

static Value IsClass(ExecState *state) {
    Value ret(false);
    do {
        if (state->GetArgumentCount() < 1) {
            throw VMExecError("IsClass must have 1 args");
            break;
        }
        Value *arg = state->GetArgument(0);
        ret = IsClass(arg) ? Value(true) : Value(false);
        
    } while (0);
    
    return ret;
}

void RegisterJSCommonFunction(ExecState *state) {
    state->Register("parseInt", ParseInt);
    state->Register("isClass", IsClass);
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
