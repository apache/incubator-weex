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
#include "core/data_render/class_function.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/common_error.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

void AddClosure(ExecState *exec_state, Value *func) {
    do {
        if (!IsFuncInstance(func)) {
            throw VMExecError("add function closure type error");
            break;
        }
        FuncInstance *inst = ValueTo<FuncInstance>(func);
        FuncState *func_state = inst->func_;
        for (int i = 0; i < func_state->in_closure().size(); i++) {
            ValueRef *ref = func_state->in_closure()[i];
            FuncClosure *closure = new FuncClosure(ref);
            ref->SetClosure(closure);
            inst->closures_.push_back(closure);
        }
        
    } while (0);
}
    
Value *LoadClosure(Value *func, ValueRef *ref) {
    Value *ret = nullptr;
    do {
        if (!IsFuncInstance(func)) {
            throw VMExecError("load function closure type error");
            break;
        }
        FuncInstance *inst = ValueTo<FuncInstance>(func);
        for (int i = 0; i < inst->closures_.size(); i++) {
            FuncClosure *closure = inst->closures_[i];
            if (closure->func_state() == ref->func_state() && closure->register_id() == ref->register_id()) {
                ret = &closure->value();
                break;
            }
        }
        
    } while (0);
    
    return ret;
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
