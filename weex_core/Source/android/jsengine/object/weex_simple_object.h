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
// Created by Darin on 11/02/2018.
//

#ifndef WEEXV8_SIMPLEOBJECT_H
#define WEEXV8_SIMPLEOBJECT_H
#include "include/JavaScriptCore/runtime/JSArray.h"
#include "include/JavaScriptCore/runtime/JSArrayBuffer.h"
#include "include/JavaScriptCore/runtime/JSFunction.h"
#include "include/JavaScriptCore/runtime/JSInternalPromise.h"
#include "include/JavaScriptCore/runtime/JSInternalPromiseDeferred.h"
#include "include/JavaScriptCore/runtime/JSLock.h"
#include "include/JavaScriptCore/runtime/JSModuleLoader.h"
#include "include/JavaScriptCore/runtime/JSNativeStdFunction.h"
#include "include/JavaScriptCore/runtime/JSONObject.h"
#include "include/JavaScriptCore/runtime/JSProxy.h"
#include "include/JavaScriptCore/runtime/JSSourceCode.h"
#include "include/JavaScriptCore/runtime/JSString.h"
#include "include/JavaScriptCore/runtime/JSTypedArrays.h"

using namespace JSC;

class SimpleObject : public JSNonFinalObject {
public:
    SimpleObject(VM& vm, Structure* structure)
            : Base(vm, structure)
    {
    }

    typedef JSNonFinalObject Base;
    static const bool needsDestruction = false;

    static SimpleObject* create(VM& vm, JSGlobalObject* globalObject)
    {
        Structure* structure = createStructure(vm, globalObject, jsNull());
        SimpleObject* simpleObject = new (NotNull, allocateCell<SimpleObject>(vm.heap, sizeof(SimpleObject))) SimpleObject(vm, structure);
        simpleObject->finishCreation(vm);
        return simpleObject;
    }

    static void visitChildren(JSCell* cell, SlotVisitor& visitor)
    {
        SimpleObject* thisObject = jsCast<SimpleObject*>(cell);
        ASSERT_GC_OBJECT_INHERITS(thisObject, info());
        Base::visitChildren(thisObject, visitor);
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    DECLARE_INFO;
};


#endif //WEEXV8_SIMPLEOBJECT_H
