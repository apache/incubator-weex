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
#include "core/data_render/rax_jsx_ast.h"

namespace weex {
namespace core {
namespace data_render {

ClassDescriptor *NewClassDescriptor(ClassDescriptor *p_super) {
    return new ClassDescriptor(p_super);
}
    
void AddClassStaticCFunc(ClassDescriptor *p_desc, const std::string& name, CFunction function) {
    Value func;
    func.type = Value::Type::CFUNC;
    func.cf = reinterpret_cast<void *>(function);
    p_desc->statics_->Add(name, func);
}

void AddClassCFunc(ClassDescriptor *p_desc, const std::string& name, CFunction function) {
    Value func;
    func.type = Value::Type::CFUNC;
    func.cf = reinterpret_cast<void *>(function);
    p_desc->funcs_->Add(name, func);
}

ClassInstance *NewClassInstance(ClassDescriptor *p_desc) {
    return new ClassInstance(p_desc);
}
    
bool FindConstructor(ClassInstance *p_inst, Value *caller, Value *caller_inst) {
    bool constructor = false;
    while (p_inst) {
        int index = p_inst->p_desc_->funcs_->IndexOf(JS_GLOBAL_CONSTRUCTOR);
        if (index >= 0) {
            *caller = *p_inst->p_desc_->funcs_->Find(index);
            SetCIValue(caller_inst, reinterpret_cast<GCObject *>(p_inst));
            constructor = true;
            break;
        }
        p_inst = p_inst->p_super_;
    }
    return constructor;
}
    
Value *GetClassMember(ClassInstance *inst,const std::string &name) {
    Value *ret = nullptr;
    do {
        ClassInstance *inst_current = inst;
        while (inst_current) {
            Variables *funcs = inst_current->p_desc_->funcs_.get();
            int index = funcs->IndexOf(name);
            if (index < 0) {
                Variables *vars = inst_current->vars_.get();
                index = vars->IndexOf(name);
                if (index >= 0) {
                    ret = vars->Find(index);
                }
            }
            else {
                ret = funcs->Find(index);
            }
            if (ret) {
                break;
            }
            // 构造函数不找super
            if (name == "constructor") {
                break;
            }
            inst_current = inst_current->p_super_;
        }
        
    } while (0);
    
    return ret;
}

Value *GetClassMemberVar(ClassInstance *inst,const std::string &name) {
    Value *ret = nullptr;
    do {
        Variables *funcs = inst->p_desc_->funcs_.get();
        int index = funcs->IndexOf(name);
        if (index < 0) {
            Variables *vars = inst->vars_.get();
            index = vars->IndexOf(name);
            if (index < 0) {
                Value var;
                SetNil(&var);
                index = vars->Add(name, var);
            }
            ret = vars->Find(index);
        }
        else {
            ret = funcs->Find(index);
        }
        
    } while (0);
    
    return ret;
}
    
Value *GetClassStaticMemberVar(ClassDescriptor *desc, const std::string &name) {
    Value *ret = nullptr;
    do {
        Variables *static_vars = desc->statics_.get();
        int index = static_vars->IndexOf(name);
        if (index < 0) {
            Value var;
            index = static_vars->Add(name, var);
            ret = static_vars->Find(index);
        }
        else {
            ret = static_vars->Find(index);
        }
        
    } while (0);
    
    return ret;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
