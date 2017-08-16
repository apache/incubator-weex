/*
 * Copyright (C) 2014, 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSJavaScriptCallFrame.h"

#include "DebuggerScope.h"
#include "Error.h"
#include "IdentifierInlines.h"
#include "JSCInlines.h"
#include "JSJavaScriptCallFramePrototype.h"
#include "ObjectConstructor.h"

using namespace JSC;

namespace Inspector {

const ClassInfo JSJavaScriptCallFrame::s_info = { "JavaScriptCallFrame", &Base::s_info, 0, CREATE_METHOD_TABLE(JSJavaScriptCallFrame) };

JSJavaScriptCallFrame::JSJavaScriptCallFrame(VM& vm, Structure* structure, Ref<JavaScriptCallFrame>&& impl)
    : JSDestructibleObject(vm, structure)
    , m_impl(&impl.leakRef())
{
}

void JSJavaScriptCallFrame::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
}

JSObject* JSJavaScriptCallFrame::createPrototype(VM& vm, JSGlobalObject* globalObject)
{
    return JSJavaScriptCallFramePrototype::create(vm, globalObject, JSJavaScriptCallFramePrototype::createStructure(vm, globalObject, globalObject->objectPrototype()));
}

void JSJavaScriptCallFrame::destroy(JSC::JSCell* cell)
{
    JSJavaScriptCallFrame* thisObject = static_cast<JSJavaScriptCallFrame*>(cell);
    thisObject->JSJavaScriptCallFrame::~JSJavaScriptCallFrame();
}

void JSJavaScriptCallFrame::releaseImpl()
{
    if (auto impl = std::exchange(m_impl, nullptr))
        impl->deref();
}

JSJavaScriptCallFrame::~JSJavaScriptCallFrame()
{
    releaseImpl();
}

JSValue JSJavaScriptCallFrame::evaluateWithScopeExtension(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue scriptValue = exec->argument(0);
    if (!scriptValue.isString())
        return throwTypeError(exec, scope, ASCIILiteral("JSJavaScriptCallFrame.evaluateWithScopeExtension first argument must be a string."));

    String script = asString(scriptValue)->value(exec);
    RETURN_IF_EXCEPTION(scope, JSValue());

    NakedPtr<Exception> exception;
    JSObject* scopeExtension = exec->argument(1).getObject();
    JSValue result = impl().evaluateWithScopeExtension(script, scopeExtension, exception);
    if (exception)
        throwException(exec, scope, exception);

    return result;
}

static JSValue valueForScopeType(DebuggerScope* scope)
{
    if (scope->isCatchScope())
        return jsNumber(JSJavaScriptCallFrame::CATCH_SCOPE);
    if (scope->isFunctionNameScope())
        return jsNumber(JSJavaScriptCallFrame::FUNCTION_NAME_SCOPE);
    if (scope->isWithScope())
        return jsNumber(JSJavaScriptCallFrame::WITH_SCOPE);
    if (scope->isNestedLexicalScope())
        return jsNumber(JSJavaScriptCallFrame::NESTED_LEXICAL_SCOPE);
    if (scope->isGlobalLexicalEnvironment())
        return jsNumber(JSJavaScriptCallFrame::GLOBAL_LEXICAL_ENVIRONMENT_SCOPE);
    if (scope->isGlobalScope())
        return jsNumber(JSJavaScriptCallFrame::GLOBAL_SCOPE);

    ASSERT(scope->isClosureScope());
    return jsNumber(JSJavaScriptCallFrame::CLOSURE_SCOPE);
}

static JSValue valueForScopeLocation(ExecState* exec, const DebuggerLocation& location)
{
    if (location.sourceID == noSourceID)
        return jsNull();

    // Debugger.Location protocol object.
    JSObject* result = constructEmptyObject(exec);
    result->putDirect(exec->vm(), Identifier::fromString(exec, "scriptId"), jsString(exec, String::number(location.sourceID)));
    result->putDirect(exec->vm(), Identifier::fromString(exec, "lineNumber"), jsNumber(location.line));
    result->putDirect(exec->vm(), Identifier::fromString(exec, "columnNumber"), jsNumber(location.column));
    return result;
}

JSValue JSJavaScriptCallFrame::scopeDescriptions(ExecState* exec)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    DebuggerScope* scopeChain = impl().scopeChain();
    if (!scopeChain)
        return jsUndefined();

    int index = 0;
    JSArray* array = constructEmptyArray(exec, nullptr);

    DebuggerScope::iterator end = scopeChain->end();
    for (DebuggerScope::iterator iter = scopeChain->begin(); iter != end; ++iter) {
        DebuggerScope* scope = iter.get();
        JSObject* description = constructEmptyObject(exec);
        description->putDirect(exec->vm(), Identifier::fromString(exec, "type"), valueForScopeType(scope));
        description->putDirect(exec->vm(), Identifier::fromString(exec, "name"), jsString(exec, scope->name()));
        description->putDirect(exec->vm(), Identifier::fromString(exec, "location"), valueForScopeLocation(exec, scope->location()));
        array->putDirectIndex(exec, index++, description);
        RETURN_IF_EXCEPTION(throwScope, JSValue());
    }

    return array;
}

JSValue JSJavaScriptCallFrame::caller(ExecState* exec) const
{
    return toJS(exec, globalObject(), impl().caller());
}

JSValue JSJavaScriptCallFrame::sourceID(ExecState*) const
{
    return jsNumber(impl().sourceID());
}

JSValue JSJavaScriptCallFrame::line(ExecState*) const
{
    return jsNumber(impl().line());
}

JSValue JSJavaScriptCallFrame::column(ExecState*) const
{
    return jsNumber(impl().column());
}

JSValue JSJavaScriptCallFrame::functionName(ExecState* exec) const
{
    return jsString(exec, impl().functionName());
}

JSValue JSJavaScriptCallFrame::scopeChain(ExecState* exec) const
{
    if (!impl().scopeChain())
        return jsNull();

    DebuggerScope* scopeChain = impl().scopeChain();
    DebuggerScope::iterator iter = scopeChain->begin();
    DebuggerScope::iterator end = scopeChain->end();

    // We must always have something in the scope chain.
    ASSERT(iter != end);

    MarkedArgumentBuffer list;
    do {
        list.append(iter.get());
        ++iter;
    } while (iter != end);

    return constructArray(exec, nullptr, globalObject(), list);
}

JSValue JSJavaScriptCallFrame::thisObject(ExecState*) const
{
    return impl().thisValue();
}

JSValue JSJavaScriptCallFrame::isTailDeleted(JSC::ExecState*) const
{
    return jsBoolean(impl().isTailDeleted());
}

JSValue JSJavaScriptCallFrame::type(ExecState* exec) const
{
    switch (impl().type()) {
    case DebuggerCallFrame::FunctionType:
        return jsNontrivialString(exec, ASCIILiteral("function"));
    case DebuggerCallFrame::ProgramType:
        return jsNontrivialString(exec, ASCIILiteral("program"));
    }

    ASSERT_NOT_REACHED();
    return jsNull();
}

JSValue toJS(ExecState* exec, JSGlobalObject* globalObject, JavaScriptCallFrame* impl)
{
    if (!impl)
        return jsNull();

    JSObject* prototype = JSJavaScriptCallFrame::createPrototype(exec->vm(), globalObject);
    Structure* structure = JSJavaScriptCallFrame::createStructure(exec->vm(), globalObject, prototype);
    JSJavaScriptCallFrame* javaScriptCallFrame = JSJavaScriptCallFrame::create(exec->vm(), structure, *impl);

    return javaScriptCallFrame;
}

} // namespace Inspector

