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
#ifndef JSCallbackConstructor_h
#define JSCallbackConstructor_h

#include "JSObjectRef.h"
#include "runtime/JSDestructibleObject.h"

namespace JSC {

class JSCallbackConstructor : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | ImplementsHasInstance | ImplementsDefaultHasInstance;

    static JSCallbackConstructor* create(ExecState* exec, JSGlobalObject* globalObject, Structure* structure, JSClassRef classRef, JSObjectCallAsConstructorCallback callback) 
    {
        JSCallbackConstructor* constructor = new (NotNull, allocateCell<JSCallbackConstructor>(*exec->heap())) JSCallbackConstructor(globalObject, structure, classRef, callback);
        constructor->finishCreation(globalObject, classRef);
        return constructor;
    }
    
    ~JSCallbackConstructor();
    static void destroy(JSCell*);
    JSClassRef classRef() const { return m_class; }
    JSObjectCallAsConstructorCallback callback() const { return m_callback; }
    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto) 
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    JSCallbackConstructor(JSGlobalObject*, Structure*, JSClassRef, JSObjectCallAsConstructorCallback);
    void finishCreation(JSGlobalObject*, JSClassRef);

private:
    friend struct APICallbackFunction;

    static ConstructType getConstructData(JSCell*, ConstructData&);

    JSObjectCallAsConstructorCallback constructCallback() { return m_callback; }

    JSClassRef m_class;
    JSObjectCallAsConstructorCallback m_callback;
};

} // namespace JSC

#endif // JSCallbackConstructor_h
