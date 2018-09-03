//
// Created by chad on 2018/7/19.
//

#include "core/data_render/class_factory.h"
#include "core/data_render/class.h"
#include "core/data_render/class_array.h"
#include "core/data_render/class_string.h"
#include "core/data_render/class_json.h"
#include "core/data_render/vm_mem.h"

namespace weex {
namespace core {
namespace data_render {

Value ClassFactory::CreateClassDescriptor(ClassDescriptor *p_super) {
    ClassDescriptor *desc = NewClassDescriptor(p_super);
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    descs_.emplace_back(desc);
    return value;
}
    
Value ClassFactory::CreateArray() {
    Array *array = new Array();
    Value value;
    SetAValue(&value, reinterpret_cast<GCObject *>(array));
    arrays_.emplace_back(array);
    return value;
}
    
Value ClassFactory::ClassString() {
    ClassDescriptor *desc = NewClassString();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    descs_.emplace_back(desc);
    return value;
}
    
Value ClassFactory::ClassJSON() {
    ClassDescriptor *desc = NewClassJSON();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    descs_.emplace_back(desc);
    return value;
}
    
Value ClassFactory::ClassArray() {
    ClassDescriptor *desc = NewClassArray();
    Value value;
    SetCDValue(&value, reinterpret_cast<GCObject *>(desc));
    descs_.emplace_back(desc);
    return value;
}
    
ClassInstance *ClassFactory::CreateClassInstanceFromSuper(ClassDescriptor *p_desc) {
    ClassInstance *p_super = nullptr;
    ClassInstance *inst = NewClassInstance(p_desc);
    insts_.emplace_back(inst);
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
    for (auto iter = descs_.begin(); iter != descs_.end(); iter++) {
        delete *iter;
    }
    descs_.clear();
    for (auto iter = insts_.begin(); iter != insts_.end(); iter++) {
        delete *iter;
    }
    insts_.clear();
    for (auto iter = arrays_.begin(); iter != arrays_.end(); iter++) {
        delete *iter;
    }
    arrays_.clear();
}

}
}
}
