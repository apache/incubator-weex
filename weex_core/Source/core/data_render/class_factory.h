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
//
// Created by pentao.pt on 2018/7/25.
//

#ifndef DATA_RENDER_CLASSFACTORY_H
#define DATA_RENDER_CLASSFACTORY_H

#include <vector>
#include "core/data_render/object.h"

namespace weex {
namespace core {
namespace data_render {
        
class ClassFactory {
public:
    ClassFactory() {};
    virtual Value CreateClassDescriptor(ClassDescriptor *p_super);
    virtual Value CreateClassInstance(ClassDescriptor *p_desc);
    virtual Value ClassArray();
    virtual Value ClassString();
    virtual Value ClassJSON();
    virtual Value ClassObject();
    virtual Value CreateArray();
    virtual Value CreateTable();
    int findDesc(const ClassDescriptor *desc);
    inline const std::vector<std::pair<GCObject *, Value::Type>>& stores() {return stores_;}
    inline std::vector<Array *> arrays() {return arrays_;}
    inline std::vector<ClassDescriptor *> descs() {return descs_;}
    inline std::vector<Table *> tables() {return tables_;}

    ~ClassFactory();
    
private:
    ClassInstance *CreateClassInstanceFromSuper(ClassDescriptor *p_desc);
    std::vector<std::pair<GCObject *, Value::Type>> stores_;

    //for binary encode and decode
    std::vector<ClassDescriptor *> descs_;
    std::vector<Array *> arrays_;
    std::vector<Table *> tables_;
};

}
}
}

#endif //DATA_RENDER_CLASSFACTORY_H
