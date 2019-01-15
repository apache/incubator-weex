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

#include "JSFunction.h"

namespace JSC {

EncodedJSValue JSC_HOST_CALL boundThisNoArgsFunctionCall(ExecState*);
EncodedJSValue JSC_HOST_CALL boundFunctionCall(ExecState*);
EncodedJSValue JSC_HOST_CALL boundThisNoArgsFunctionConstruct(ExecState*);
EncodedJSValue JSC_HOST_CALL boundFunctionConstruct(ExecState*);
EncodedJSValue JSC_HOST_CALL isBoundFunction(ExecState*);
EncodedJSValue JSC_HOST_CALL hasInstanceBoundFunction(ExecState*);

class JSBoundFunction : public JSFunction {
public:
    typedef JSFunction Base;
    const static unsigned StructureFlags = ~ImplementsDefaultHasInstance & Base::StructureFlags;

    static JSBoundFunction* create(VM&, ExecState*, JSGlobalObject*, JSObject* targetFunction, JSValue boundThis, JSArray* boundArgs, int, const String& name);
    
    static bool customHasInstance(JSObject*, ExecState*, JSValue);

    JSObject* targetFunction() { return m_targetFunction.get(); }
    JSValue boundThis() { return m_boundThis.get(); }
    JSArray* boundArgs() { return m_boundArgs.get(); }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info()); 
    }
    
    static ptrdiff_t offsetOfTargetFunction() { return OBJECT_OFFSETOF(JSBoundFunction, m_targetFunction); }
    static ptrdiff_t offsetOfBoundThis() { return OBJECT_OFFSETOF(JSBoundFunction, m_boundThis); }

    DECLARE_INFO;

protected:
    static void visitChildren(JSCell*, SlotVisitor&);

private:
    JSBoundFunction(VM&, JSGlobalObject*, Structure*, JSObject* targetFunction, JSValue boundThis, JSArray* boundArgs);
    
    void finishCreation(VM&, NativeExecutable*, int length);

    WriteBarrier<JSObject> m_targetFunction;
    WriteBarrier<Unknown> m_boundThis;
    WriteBarrier<JSArray> m_boundArgs;
};

} // namespace JSC
