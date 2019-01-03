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
// Created by Darin on 03/08/2018.
//

#ifndef WEEXV8_WEEXCONSOLEOBJECT_H
#define WEEXV8_WEEXCONSOLEOBJECT_H

#include "android/jsengine/weex_jsc_utils.h"

using namespace JSC;

class WeexConsoleObject : public JSNonFinalObject {
private:
    WeexConsoleObject(VM&, Structure*);

public:
    typedef JSNonFinalObject Base;

    static WeexConsoleObject* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        WeexConsoleObject* object = new (NotNull, allocateCell<WeexConsoleObject>(vm.heap)) WeexConsoleObject(vm, structure);
        object->finishCreation(vm, globalObject);
        return object;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    void finishCreation(VM&, JSGlobalObject*);
};


#endif //WEEXV8_WEEXCONSOLEOBJECT_H
