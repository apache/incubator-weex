/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "APICast.h"
#include "Completion.h"
#include "Exception.h"
#include "JSBasePrivate.h"
#include "VM.h"
#include "JSScriptRefPrivate.h"
#include "OpaqueJSString.h"
#include "JSCInlines.h"
#include "Parser.h"
#include "SourceCode.h"
#include "SourceProvider.h"

using namespace JSC;

struct OpaqueJSScript : public SourceProvider {
public:
    static WTF::Ref<OpaqueJSScript> create(VM& vm, const SourceOrigin& sourceOrigin, const String& url, int startingLineNumber, const String& source)
    {
        return WTF::adoptRef(*new OpaqueJSScript(vm, sourceOrigin, url, startingLineNumber, source));
    }

    unsigned hash() const override
    {
        return m_source.get().hash();
    }

    StringView source() const override
    {
        return m_source.get();
    }

    VM& vm() const { return m_vm; }

private:
    OpaqueJSScript(VM& vm, const SourceOrigin& sourceOrigin, const String& url, int startingLineNumber, const String& source)
        : SourceProvider(sourceOrigin, url, TextPosition(OrdinalNumber::fromOneBasedInt(startingLineNumber), OrdinalNumber()), SourceProviderSourceType::Program)
        , m_vm(vm)
        , m_source(source.isNull() ? *StringImpl::empty() : *source.impl())
    {
    }

    virtual ~OpaqueJSScript() { }

    VM& m_vm;
    Ref<StringImpl> m_source;
};

static bool parseScript(VM& vm, const SourceCode& source, ParserError& error)
{
    return !!JSC::parse<JSC::ProgramNode>(
        &vm, source, Identifier(), JSParserBuiltinMode::NotBuiltin,
        JSParserStrictMode::NotStrict, JSParserScriptMode::Classic, SourceParseMode::ProgramMode, SuperBinding::NotNeeded,
        error);
}

extern "C" {

JSScriptRef JSScriptCreateReferencingImmortalASCIIText(JSContextGroupRef contextGroup, JSStringRef url, int startingLineNumber, const char* source, size_t length, JSStringRef* errorMessage, int* errorLine)
{
    auto& vm = *toJS(contextGroup);
    JSLockHolder locker(&vm);
    for (size_t i = 0; i < length; i++) {
        if (!isASCII(source[i]))
            return 0;
    }

    startingLineNumber = std::max(1, startingLineNumber);

    auto sourceURLString = url ? url->string() : String();
    auto result = OpaqueJSScript::create(vm, SourceOrigin { sourceURLString }, sourceURLString, startingLineNumber, String(StringImpl::createFromLiteral(source, length)));

    ParserError error;
    if (!parseScript(vm, SourceCode(result.copyRef()), error)) {
        if (errorMessage)
            *errorMessage = OpaqueJSString::create(error.message()).leakRef();
        if (errorLine)
            *errorLine = error.line();
        return nullptr;
    }

    return &result.leakRef();
}

JSScriptRef JSScriptCreateFromString(JSContextGroupRef contextGroup, JSStringRef url, int startingLineNumber, JSStringRef source, JSStringRef* errorMessage, int* errorLine)
{
    auto& vm = *toJS(contextGroup);
    JSLockHolder locker(&vm);

    startingLineNumber = std::max(1, startingLineNumber);

    auto sourceURLString = url ? url->string() : String();
    auto result = OpaqueJSScript::create(vm, SourceOrigin { sourceURLString }, sourceURLString, startingLineNumber, source->string());

    ParserError error;
    if (!parseScript(vm, SourceCode(result.copyRef()), error)) {
        if (errorMessage)
            *errorMessage = OpaqueJSString::create(error.message()).leakRef();
        if (errorLine)
            *errorLine = error.line();
        return nullptr;
    }

    return &result.leakRef();
}

void JSScriptRetain(JSScriptRef script)
{
    JSLockHolder locker(&script->vm());
    script->ref();
}

void JSScriptRelease(JSScriptRef script)
{
    JSLockHolder locker(&script->vm());
    script->deref();
}

JSValueRef JSScriptEvaluate(JSContextRef context, JSScriptRef script, JSValueRef thisValueRef, JSValueRef* exception)
{
    ExecState* exec = toJS(context);
    JSLockHolder locker(exec);
    if (&script->vm() != &exec->vm()) {
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
    NakedPtr<Exception> internalException;
    JSValue thisValue = thisValueRef ? toJS(exec, thisValueRef) : jsUndefined();
    JSValue result = evaluate(exec, SourceCode(*script), thisValue, internalException);
    if (internalException) {
        if (exception)
            *exception = toRef(exec, internalException->value());
        return 0;
    }
    ASSERT(result);
    return toRef(exec, result);
}

}
