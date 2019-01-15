/*
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

#include "core/data_render/class_factory.h"
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/class_string.h"
#include "core/data_render/class_json.h"
#include "core/data_render/class_object.h"
#include "core/data_render/class_console.h"
#include "core/data_render/table.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/class_regex.h"
#include "core/data_render/class_window.h"
#include "class_math.h"

namespace weex {
namespace core {
namespace data_render {

Value ClassFactory::CreateClassDescriptor(ClassDescriptor *p_super) {
    ClassDescriptor *desc = NewClassDescriptor(p_super);
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::CreateArray() {
    Array *array = new Array();
    Value value;
    SetAValue(&value, reinterpret_cast<GCObject *>(array));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(array), value.type));
    return value;
}

Value ClassFactory::CreateTable() {
    Table *table = new Table();
    Value value;
    SetTValue(&value, reinterpret_cast<GCObject *>(table));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(table), value.type));
    return value;
}

Value ClassFactory::ClassString() {
    ClassDescriptor *desc = NewClassString();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::ClassJSON() {
    ClassDescriptor *desc = NewClassJSON();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::ClassArray() {
    ClassDescriptor *desc = NewClassArray();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::ClassObject() {
    ClassDescriptor *desc = NewClassOject();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::ClassRegExp() {
    ClassDescriptor *desc = NewClassRegex();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::ClassWindow() {
    ClassDescriptor *desc = NewClassWindow();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::ClassMath() {
    ClassDescriptor *desc = NewClassMath();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

Value ClassFactory::ClassConsole() {
    ClassDescriptor *desc = NewClassConsole();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    return value;
}

int ClassFactory::Find(const ClassDescriptor *desc) {
    std::vector<ClassDescriptor *> descs = this->descs();
    int index = 0;
    for (auto d : descs) {
        if (desc == d) {
            return index;
        }
        index++;
    }
    return -1;
}

ClassInstance *ClassFactory::CreateClassInstanceFromSuper(ClassDescriptor *p_desc) {
    ClassInstance *p_super = nullptr;
    ClassInstance *inst = NewClassInstance(p_desc);
    if (p_desc->p_super_) {
        p_super = CreateClassInstanceFromSuper(p_desc->p_super_);
        inst->p_super_ = p_super;
    }
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(inst), Value::Type::CLASS_INST));
    return inst;
}

Value ClassFactory::CreateClassInstance(ClassDescriptor *p_desc) {
    ClassInstance *inst = CreateClassInstanceFromSuper(p_desc);
    Value value;
    SetCIValue(&value, reinterpret_cast<GCObject *>(inst));
    return value;
}

Value ClassFactory::CreateFuncInstance(FuncState *func_state) {
    FuncInstance *func_inst = new FuncInstance(func_state);
    Value value;
    SetGCFuncValue(&value, reinterpret_cast<GCObject *>(func_inst));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(func_inst), Value::Type::FUNC_INST));
    return value;
}

ClassFactory::~ClassFactory() {
    for (auto iter = stores_.begin(); iter != stores_.end(); iter++) {
        switch (iter->second) {
            case Value::Type::ARRAY:
            {
                delete reinterpret_cast<Array *>(iter->first);
                break;
            }
            case Value::Type::TABLE:
            {
                delete reinterpret_cast<Table *>(iter->first);
                break;
            }
            case Value::Type::CLASS_DESC:
            {
                delete reinterpret_cast<ClassDescriptor *>(iter->first);
                break;
            }
            case Value::Type::CLASS_INST:
            {
                delete reinterpret_cast<ClassInstance *>(iter->first);
                break;
            }
            case Value::Type::FUNC_INST:
            {
                FuncInstance *inst = reinterpret_cast<FuncInstance *>(iter->first);
                for (int i = 0; i < inst->closures_.size(); i++) {
                    delete inst->closures_[i];
                }
                delete inst;
                break;
            }
            default:
                break;
        }
    }
    stores_.clear();
}
std::vector<ClassDescriptor *> ClassFactory::descs() {
    std::vector<ClassDescriptor *> descs;
    for (auto iter = stores_.begin(); iter != stores_.end(); iter++) {
        if (iter->second == Value::Type::CLASS_DESC) {
            descs.push_back(reinterpret_cast<ClassDescriptor *>(iter->first));
        }
    }
    return descs;
}

std::vector<Value> ClassFactory::constants() {
    std::vector<Value> constants;
    for (auto iter = stores_.begin(); iter != stores_.end(); iter++) {
        switch (iter->second) {
            case Value::Type::ARRAY:
            {
                Value array;
                SetAValue(&array, iter->first);
                constants.push_back(array);
                break;
            }
            case Value::Type::TABLE:
            {
                Value table;
                SetTValue(&table, iter->first);
                constants.push_back(table);
                break;
            }
            default:
                break;
        }
    }
    return constants;
}
    
}
}
}
