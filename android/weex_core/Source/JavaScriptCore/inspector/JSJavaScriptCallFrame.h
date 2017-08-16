/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#include "JSDestructibleObject.h"
#include "JavaScriptCallFrame.h"

namespace Inspector {

class JSJavaScriptCallFrame : public JSC::JSDestructibleObject {
public:
    typedef JSC::JSDestructibleObject Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    DECLARE_INFO;

    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
    }

    static JSJavaScriptCallFrame* create(JSC::VM& vm, JSC::Structure* structure, Ref<JavaScriptCallFrame>&& impl)
    {
        JSJavaScriptCallFrame* instance = new (NotNull, JSC::allocateCell<JSJavaScriptCallFrame>(vm.heap)) JSJavaScriptCallFrame(vm, structure, WTFMove(impl));
        instance->finishCreation(vm);
        return instance;
    }

    static JSC::JSObject* createPrototype(JSC::VM&, JSC::JSGlobalObject*);
    static void destroy(JSC::JSCell*);

    JavaScriptCallFrame& impl() const { return *m_impl; }
    void releaseImpl();

    // Functions.
    JSC::JSValue evaluateWithScopeExtension(JSC::ExecState*);
    JSC::JSValue scopeDescriptions(JSC::ExecState*);

    // Attributes.
    JSC::JSValue caller(JSC::ExecState*) const;
    JSC::JSValue sourceID(JSC::ExecState*) const;
    JSC::JSValue line(JSC::ExecState*) const;
    JSC::JSValue column(JSC::ExecState*) const;
    JSC::JSValue functionName(JSC::ExecState*) const;
    JSC::JSValue scopeChain(JSC::ExecState*) const;
    JSC::JSValue thisObject(JSC::ExecState*) const;
    JSC::JSValue type(JSC::ExecState*) const;
    JSC::JSValue isTailDeleted(JSC::ExecState*) const;

    // Constants.
    static const unsigned short GLOBAL_SCOPE = 0;
    static const unsigned short WITH_SCOPE = 1;
    static const unsigned short CLOSURE_SCOPE = 2;
    static const unsigned short CATCH_SCOPE = 3;
    static const unsigned short FUNCTION_NAME_SCOPE = 4;
    static const unsigned short GLOBAL_LEXICAL_ENVIRONMENT_SCOPE = 5;
    static const unsigned short NESTED_LEXICAL_SCOPE = 6;

protected:
    void finishCreation(JSC::VM&);

private:
    JSJavaScriptCallFrame(JSC::VM&, JSC::Structure*, Ref<JavaScriptCallFrame>&&);
    ~JSJavaScriptCallFrame();

    JavaScriptCallFrame* m_impl;
};

JSC::JSValue toJS(JSC::ExecState*, JSC::JSGlobalObject*, JavaScriptCallFrame*);

} // namespace Inspector
