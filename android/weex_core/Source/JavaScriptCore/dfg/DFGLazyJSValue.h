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

#pragma once

#if ENABLE(DFG_JIT)

#include "DFGCommon.h"
#include "DFGFrozenValue.h"
#include <wtf/text/StringImpl.h>

namespace JSC {

class CCallHelpers;

namespace DFG {

class Graph;

// Represents either a JSValue, or for JSValues that require allocation in the heap,
// it tells you everything you'd need to know in order to allocate it.

class LazyJSValue {
public:
    enum LazinessKind {
        KnownValue,
        SingleCharacterString,
        KnownStringImpl,
        NewStringImpl
    };

    LazyJSValue(FrozenValue* value = FrozenValue::emptySingleton())
        : m_kind(KnownValue)
    {
        u.value = value;
    }
    
    static LazyJSValue singleCharacterString(UChar character)
    {
        LazyJSValue result;
        result.m_kind = SingleCharacterString;
        result.u.character = character;
        return result;
    }
    
    static LazyJSValue knownStringImpl(StringImpl* string)
    {
        LazyJSValue result;
        result.m_kind = KnownStringImpl;
        result.u.stringImpl = string;
        return result;
    }

    static LazyJSValue newString(Graph&, const String&);

    LazinessKind kind() const { return m_kind; }
    
    FrozenValue* tryGetValue(Graph&) const
    {
        if (m_kind == KnownValue)
            return value();
        return nullptr;
    }
    
    JSValue getValue(VM&) const;
    
    FrozenValue* value() const
    {
        ASSERT(m_kind == KnownValue);
        return u.value;
    }
    
    UChar character() const
    {
        ASSERT(m_kind == SingleCharacterString);
        return u.character;
    }

    const StringImpl* tryGetStringImpl(VM&) const;
    
    String tryGetString(Graph&) const;
    
    StringImpl* stringImpl() const
    {
        ASSERT(m_kind == KnownStringImpl || m_kind == NewStringImpl);
        return u.stringImpl;
    }

    TriState strictEqual(const LazyJSValue& other) const;
    
    uintptr_t switchLookupValue(SwitchKind) const;

    void emit(CCallHelpers&, JSValueRegs) const;
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;
    
private:
    union {
        FrozenValue* value;
        UChar character;
        StringImpl* stringImpl;
    } u;
    LazinessKind m_kind;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
