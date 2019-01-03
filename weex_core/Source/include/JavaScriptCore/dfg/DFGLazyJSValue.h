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
