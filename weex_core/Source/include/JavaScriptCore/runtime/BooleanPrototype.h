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
#pragma once

#include "BooleanObject.h"

namespace JSC {

class BooleanPrototype : public BooleanObject {
public:
    typedef BooleanObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | HasStaticPropertyTable;

    static BooleanPrototype* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        BooleanPrototype* prototype = new (NotNull, allocateCell<BooleanPrototype>(vm.heap)) BooleanPrototype(vm, structure);
        prototype->finishCreation(vm, globalObject);
        return prototype;
    }
        
    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    void finishCreation(VM&, JSGlobalObject*);

private:
    BooleanPrototype(VM&, Structure*);
};

} // namespace JSC
