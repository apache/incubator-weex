/*
 * Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
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
