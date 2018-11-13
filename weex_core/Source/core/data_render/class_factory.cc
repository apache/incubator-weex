//
// Created by chad on 2018/7/19.
//

#include "core/data_render/class_factory.h"
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/class_string.h"
#include "core/data_render/class_json.h"
#include "core/data_render/class_object.h"
#include "core/data_render/table.h"

namespace weex {
namespace core {
namespace data_render {

Value ClassFactory::CreateClassDescriptor(ClassDescriptor *p_super) {
    ClassDescriptor *desc = NewClassDescriptor(p_super);
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    descs_.push_back(desc);
    return value;
}
    
Value ClassFactory::CreateArray() {
    Array *array = new Array();
    Value value;
    SetAValue(&value, reinterpret_cast<GCObject *>(array));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(array), value.type));
    arrays_.push_back(array);
    return value;
}
    
Value ClassFactory::CreateTable() {
    Table *table = new Table();
    Value value;
    SetTValue(&value, reinterpret_cast<GCObject *>(table));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(table), value.type));
    tables_.push_back(table);
    return value;
}
    
Value ClassFactory::ClassString() {
    ClassDescriptor *desc = NewClassString();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    descs_.push_back(desc);
    return value;
}
    
Value ClassFactory::ClassJSON() {
    ClassDescriptor *desc = NewClassJSON();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    descs_.push_back(desc);
    return value;
}
    
Value ClassFactory::ClassArray() {
    ClassDescriptor *desc = NewClassArray();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    descs_.push_back(desc);
    return value;
}
    
Value ClassFactory::ClassObject() {
    ClassDescriptor *desc = NewClassOject();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(desc), value.type));
    descs_.push_back(desc);
    return value;
}
    
int ClassFactory::findDesc(const ClassDescriptor *desc) {
    int index = 0;
    for (auto d : descs_) {
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
    stores_.push_back(std::make_pair(reinterpret_cast<GCObject *>(inst), Value::Type::CLASS_INST));
    if (p_desc->p_super_) {
        p_super = CreateClassInstanceFromSuper(p_desc->p_super_);
        inst->p_super_ = p_super;
    }
    return inst;
}
    
Value ClassFactory::CreateClassInstance(ClassDescriptor *p_desc) {
    ClassInstance *inst = CreateClassInstanceFromSuper(p_desc);
    Value value;
    SetCIValue(&value, reinterpret_cast<GCObject *>(inst));
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
            default:
                break;
        }
    }
    stores_.clear();
}

}
}
}
