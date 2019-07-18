/*
 * Copyright (C) 2012-2016 Apple Inc. All rights reserved.
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

#include "ExceptionHelpers.h"
#include "JSCJSValue.h"

namespace JSC {

class JSStringJoiner {
public:
    JSStringJoiner(ExecState&, LChar separator, unsigned stringCount);
    JSStringJoiner(ExecState&, StringView separator, unsigned stringCount);
    ~JSStringJoiner();

    void append(ExecState&, JSValue);
    bool appendWithoutSideEffects(ExecState&, JSValue);
    void appendEmptyString();

    JSValue join(ExecState&);

private:
    void append(StringViewWithUnderlyingString&&);
    void append8Bit(const String&);
    void appendLiteral(const Identifier&);
    unsigned joinedLength(ExecState&) const;

    LChar m_singleCharacterSeparator;
    StringView m_separator;
    Vector<StringViewWithUnderlyingString> m_strings;
    Checked<unsigned, RecordOverflow> m_accumulatedStringsLength;
    bool m_isAll8Bit { true };
};

inline JSStringJoiner::JSStringJoiner(ExecState& state, StringView separator, unsigned stringCount)
    : m_separator(separator)
    , m_isAll8Bit(m_separator.is8Bit())
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (UNLIKELY(!m_strings.tryReserveCapacity(stringCount)))
        throwOutOfMemoryError(&state, scope);
}

inline JSStringJoiner::JSStringJoiner(ExecState& state, LChar separator, unsigned stringCount)
    : m_singleCharacterSeparator(separator)
    , m_separator { &m_singleCharacterSeparator, 1 }
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (UNLIKELY(!m_strings.tryReserveCapacity(stringCount)))
        throwOutOfMemoryError(&state, scope);
}

ALWAYS_INLINE void JSStringJoiner::append(StringViewWithUnderlyingString&& string)
{
    m_accumulatedStringsLength += string.view.length();
    m_isAll8Bit = m_isAll8Bit && string.view.is8Bit();
    m_strings.uncheckedAppend(WTFMove(string));
}

ALWAYS_INLINE void JSStringJoiner::append8Bit(const String& string)
{
    ASSERT(string.is8Bit());
    m_accumulatedStringsLength += string.length();
    m_strings.uncheckedAppend({ string, string });
}

ALWAYS_INLINE void JSStringJoiner::appendLiteral(const Identifier& literal)
{
    m_accumulatedStringsLength += literal.length();
    ASSERT(literal.string().is8Bit());
    m_strings.uncheckedAppend({ literal.string(), { } });
}

ALWAYS_INLINE void JSStringJoiner::appendEmptyString()
{
    m_strings.uncheckedAppend({ { }, { } });
}

ALWAYS_INLINE bool JSStringJoiner::appendWithoutSideEffects(ExecState& state, JSValue value)
{
    // The following code differs from using the result of JSValue::toString in the following ways:
    // 1) It's inlined more than JSValue::toString is.
    // 2) It includes conversion to WTF::String in a way that avoids allocating copies of substrings.
    // 3) It doesn't create a JSString for numbers, true, or false.
    // 4) It turns undefined and null into the empty string instead of "undefined" and "null".
    // 5) It uses optimized code paths for all the cases known to be 8-bit and for the empty string.
    // If we might make an effectful calls, return false. Otherwise return true.

    if (value.isCell()) {
        JSString* jsString;
        if (!value.asCell()->isString())
            return false;
        jsString = asString(value);
        append(jsString->viewWithUnderlyingString(state));
        return true;
    }

    if (value.isInt32()) {
        append8Bit(state.vm().numericStrings.add(value.asInt32()));
        return true;
    }
    if (value.isDouble()) {
        append8Bit(state.vm().numericStrings.add(value.asDouble()));
        return true;
    }
    if (value.isTrue()) {
        append8Bit(state.vm().propertyNames->trueKeyword.string());
        return true;
    }
    if (value.isFalse()) {
        append8Bit(state.vm().propertyNames->falseKeyword.string());
        return true;
    }
    ASSERT(value.isUndefinedOrNull());
    appendEmptyString();
    return true;
}

ALWAYS_INLINE void JSStringJoiner::append(ExecState& state, JSValue value)
{
    if (!appendWithoutSideEffects(state, value)) {
        JSString* jsString = value.toString(&state);
        append(jsString->viewWithUnderlyingString(state));
    }
}

} // namespace JSC
