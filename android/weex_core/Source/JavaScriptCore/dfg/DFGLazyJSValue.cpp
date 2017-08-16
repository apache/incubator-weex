/*
 * Copyright (C) 2013, 2014, 2016 Apple Inc. All rights reserved.
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
#include "DFGLazyJSValue.h"

#if ENABLE(DFG_JIT)

#include "CCallHelpers.h"
#include "DFGGraph.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"

namespace JSC { namespace DFG {

LazyJSValue LazyJSValue::newString(Graph& graph, const String& string)
{
    LazyJSValue result;
    result.m_kind = NewStringImpl;
    result.u.stringImpl = graph.m_localStrings.add(string).iterator->impl();
    return result;
}

JSValue LazyJSValue::getValue(VM& vm) const
{
    switch (m_kind) {
    case KnownValue:
        return value()->value();
    case SingleCharacterString:
        return jsSingleCharacterString(&vm, u.character);
    case KnownStringImpl:
        return jsString(&vm, u.stringImpl);
    case NewStringImpl:
        return jsString(&vm, AtomicStringImpl::add(u.stringImpl));
    }
    RELEASE_ASSERT_NOT_REACHED();
    return JSValue();
}

static TriState equalToSingleCharacter(JSValue value, UChar character)
{
    if (!value.isString())
        return FalseTriState;
    
    JSString* jsString = asString(value);
    if (jsString->length() != 1)
        return FalseTriState;
    
    const StringImpl* string = jsString->tryGetValueImpl();
    if (!string)
        return MixedTriState;
    
    return triState(string->at(0) == character);
}

static TriState equalToStringImpl(JSValue value, StringImpl* stringImpl)
{
    if (!value.isString())
        return FalseTriState;
    
    JSString* jsString = asString(value);
    const StringImpl* string = jsString->tryGetValueImpl();
    if (!string)
        return MixedTriState;
    
    return triState(WTF::equal(stringImpl, string));
}

const StringImpl* LazyJSValue::tryGetStringImpl(VM& vm) const
{
    switch (m_kind) {
    case KnownStringImpl:
    case NewStringImpl:
        return u.stringImpl;

    case KnownValue:
        if (JSString* string = value()->dynamicCast<JSString*>(vm))
            return string->tryGetValueImpl();
        return nullptr;

    default:
        return nullptr;
    }
}

String LazyJSValue::tryGetString(Graph& graph) const
{
    switch (m_kind) {
    case NewStringImpl:
        return u.stringImpl;

    case SingleCharacterString:
        return String(&u.character, 1);

    default:
        if (const StringImpl* string = tryGetStringImpl(graph.m_vm)) {
            unsigned ginormousStringLength = 10000;
            if (string->length() > ginormousStringLength)
                return String();
            
            auto result = graph.m_copiedStrings.add(string, String());
            if (result.isNewEntry)
                result.iterator->value = string->isolatedCopy();
            return result.iterator->value;
        }
        
        return String();
    }
}

TriState LazyJSValue::strictEqual(const LazyJSValue& other) const
{
    switch (m_kind) {
    case KnownValue:
        switch (other.m_kind) {
        case KnownValue:
            return JSValue::pureStrictEqual(value()->value(), other.value()->value());
        case SingleCharacterString:
            return equalToSingleCharacter(value()->value(), other.character());
        case KnownStringImpl:
        case NewStringImpl:
            return equalToStringImpl(value()->value(), other.stringImpl());
        }
        break;
    case SingleCharacterString:
        switch (other.m_kind) {
        case SingleCharacterString:
            return triState(character() == other.character());
        case KnownStringImpl:
        case NewStringImpl:
            if (other.stringImpl()->length() != 1)
                return FalseTriState;
            return triState(other.stringImpl()->at(0) == character());
        default:
            return other.strictEqual(*this);
        }
        break;
    case KnownStringImpl:
    case NewStringImpl:
        switch (other.m_kind) {
        case KnownStringImpl:
        case NewStringImpl:
            return triState(WTF::equal(stringImpl(), other.stringImpl()));
        default:
            return other.strictEqual(*this);
        }
        break;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return FalseTriState;
}

uintptr_t LazyJSValue::switchLookupValue(SwitchKind kind) const
{
    // NB. Not every kind of JSValue will be able to give you a switch lookup
    // value, and this method will assert, or do bad things, if you use it
    // for a kind of value that can't.
    switch (m_kind) {
    case KnownValue:
        switch (kind) {
        case SwitchImm:
            return value()->value().asInt32();
        case SwitchCell:
            return bitwise_cast<uintptr_t>(value()->value().asCell());
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return 0;
        }
    case SingleCharacterString:
        switch (kind) {
        case SwitchChar:
            return character();
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return 0;
        }
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
}

void LazyJSValue::emit(CCallHelpers& jit, JSValueRegs result) const
{
    if (m_kind == KnownValue) {
        jit.moveValue(value()->value(), result);
        return;
    }

    // It must be some kind of cell.
#if USE(JSVALUE32_64)
    jit.move(CCallHelpers::TrustedImm32(JSValue::CellTag), result.tagGPR());
#endif
    CCallHelpers::DataLabelPtr label = jit.moveWithPatch(
        CCallHelpers::TrustedImmPtr(static_cast<size_t>(0xd1e7beeflu)),
        result.payloadGPR());

    LazyJSValue thisValue = *this;

    // Once we do this, we're committed. Otherwise we leak memory. Note that we call ref/deref
    // manually to ensure that there is no concurrency shadiness. We are doing something here
    // that might be rather brutal: transfering ownership of this string.
    if (m_kind == NewStringImpl)
        thisValue.u.stringImpl->ref();

    CodeBlock* codeBlock = jit.codeBlock();
    
    jit.addLinkTask(
        [codeBlock, label, thisValue] (LinkBuffer& linkBuffer) {
            JSValue realValue = thisValue.getValue(linkBuffer.vm());
            RELEASE_ASSERT(realValue.isCell());

            codeBlock->addConstant(realValue);
            
            if (thisValue.m_kind == NewStringImpl)
                thisValue.u.stringImpl->deref();

            linkBuffer.patch(label, realValue.asCell());
        });
}

void LazyJSValue::dumpInContext(PrintStream& out, DumpContext* context) const
{
    switch (m_kind) {
    case KnownValue:
        value()->dumpInContext(out, context);
        return;
    case SingleCharacterString:
        out.print("Lazy:SingleCharacterString(");
        out.printf("%04X", static_cast<unsigned>(character()));
        out.print(" / ", StringImpl::utf8ForCharacters(&u.character, 1), ")");
        return;
    case KnownStringImpl:
        out.print("Lazy:KnownString(", stringImpl(), ")");
        return;
    case NewStringImpl:
        out.print("Lazy:NewString(", stringImpl(), ")");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void LazyJSValue::dump(PrintStream& out) const
{
    dumpInContext(out, 0);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

