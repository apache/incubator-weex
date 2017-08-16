/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

namespace Inspector {

class InjectedScriptHost;

class JSInjectedScriptHost : public JSC::JSDestructibleObject {
public:
    typedef JSC::JSDestructibleObject Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    DECLARE_INFO;

    static JSC::Structure* createStructure(JSC::VM& vm, JSC::JSGlobalObject* globalObject, JSC::JSValue prototype)
    {
        return JSC::Structure::create(vm, globalObject, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), info());
    }

    static JSInjectedScriptHost* create(JSC::VM& vm, JSC::Structure* structure, Ref<InjectedScriptHost>&& impl)
    {
        JSInjectedScriptHost* instance = new (NotNull, JSC::allocateCell<JSInjectedScriptHost>(vm.heap)) JSInjectedScriptHost(vm, structure, WTFMove(impl));
        instance->finishCreation(vm);
        return instance;
    }

    static JSC::JSObject* createPrototype(JSC::VM&, JSC::JSGlobalObject*);
    static void destroy(JSC::JSCell*);

    InjectedScriptHost& impl() const { return m_wrapped; }

    // Attributes.
    JSC::JSValue evaluate(JSC::ExecState*) const;

    // Functions.
    JSC::JSValue evaluateWithScopeExtension(JSC::ExecState*);
    JSC::JSValue internalConstructorName(JSC::ExecState*);
    JSC::JSValue isHTMLAllCollection(JSC::ExecState*);
    JSC::JSValue subtype(JSC::ExecState*);
    JSC::JSValue functionDetails(JSC::ExecState*);
    JSC::JSValue getInternalProperties(JSC::ExecState*);
    JSC::JSValue proxyTargetValue(JSC::ExecState*);
    JSC::JSValue weakMapSize(JSC::ExecState*);
    JSC::JSValue weakMapEntries(JSC::ExecState*);
    JSC::JSValue weakSetSize(JSC::ExecState*);
    JSC::JSValue weakSetEntries(JSC::ExecState*);
    JSC::JSValue iteratorEntries(JSC::ExecState*);

protected:
    void finishCreation(JSC::VM&);

private:
    JSInjectedScriptHost(JSC::VM&, JSC::Structure*, Ref<InjectedScriptHost>&&);

    Ref<InjectedScriptHost> m_wrapped;
};

} // namespace Inspector
