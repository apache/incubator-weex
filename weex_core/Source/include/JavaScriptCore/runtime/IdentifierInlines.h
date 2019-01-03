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

#include "CallFrame.h"
#include "Identifier.h"
#include "Symbol.h"

namespace JSC  {

inline Identifier::Identifier(ExecState* exec, AtomicStringImpl* string)
    : m_string(string)
{
#ifndef NDEBUG
    checkCurrentAtomicStringTable(exec);
    if (string)
        ASSERT_WITH_MESSAGE(!string->length() || string->isSymbol() || AtomicStringImpl::isInAtomicStringTable(string), "The atomic string comes from an other thread!");
#else
    UNUSED_PARAM(exec);
#endif
}

inline Identifier::Identifier(ExecState* exec, const AtomicString& string)
    : m_string(string.string())
{
#ifndef NDEBUG
    checkCurrentAtomicStringTable(exec);
    if (!string.isNull())
        ASSERT_WITH_MESSAGE(!string.length() || string.impl()->isSymbol() || AtomicStringImpl::isInAtomicStringTable(string.impl()), "The atomic string comes from an other thread!");
#else
    UNUSED_PARAM(exec);
#endif
}

inline Ref<StringImpl> Identifier::add(ExecState* exec, StringImpl* r)
{
#ifndef NDEBUG
    checkCurrentAtomicStringTable(exec);
#endif
    return *AtomicStringImpl::addWithStringTableProvider(*exec, r);
}
inline Ref<StringImpl> Identifier::add(VM* vm, StringImpl* r)
{
#ifndef NDEBUG
    checkCurrentAtomicStringTable(vm);
#endif
    return *AtomicStringImpl::addWithStringTableProvider(*vm, r);
}

inline Identifier Identifier::fromUid(VM* vm, UniquedStringImpl* uid)
{
    if (!uid || !uid->isSymbol())
        return Identifier(vm, uid);
    return static_cast<SymbolImpl&>(*uid);
}

inline Identifier Identifier::fromUid(ExecState* exec, UniquedStringImpl* uid)
{
    return fromUid(&exec->vm(), uid);
}

inline Identifier Identifier::fromUid(const PrivateName& name)
{
    return name.uid();
}

template<unsigned charactersCount>
inline Identifier Identifier::fromString(VM* vm, const char (&characters)[charactersCount])
{
    return Identifier(vm, characters);
}

template<unsigned charactersCount>
inline Identifier Identifier::fromString(ExecState* exec, const char (&characters)[charactersCount])
{
    return Identifier(&exec->vm(), characters);
}

inline Identifier Identifier::fromString(VM* vm, const LChar* s, int length)
{
    return Identifier(vm, s, length);
}

inline Identifier Identifier::fromString(VM* vm, const UChar* s, int length)
{
    return Identifier(vm, s, length);
}

inline Identifier Identifier::fromString(VM* vm, const String& string)
{
    return Identifier(vm, string.impl());
}

inline Identifier Identifier::fromString(ExecState* exec, const String& string)
{
    return Identifier(&exec->vm(), string.impl());
}

inline Identifier Identifier::fromString(ExecState* exec, AtomicStringImpl* atomicString)
{
    return Identifier(exec, atomicString);
}

inline Identifier Identifier::fromString(ExecState* exec, const AtomicString& atomicString)
{
    return Identifier(exec, atomicString);
}

inline Identifier Identifier::fromString(ExecState* exec, const char* s)
{
    return Identifier(exec, AtomicString(s));
}

inline JSValue identifierToJSValue(VM& vm, const Identifier& identifier)
{
    if (identifier.isSymbol())
        return Symbol::create(vm, static_cast<SymbolImpl&>(*identifier.impl()));
    return jsString(&vm, identifier.impl());
}

inline JSValue identifierToSafePublicJSValue(VM& vm, const Identifier& identifier) 
{
    if (identifier.isSymbol() && !vm.propertyNames->isPrivateName(identifier))
        return Symbol::create(vm, static_cast<SymbolImpl&>(*identifier.impl()));
    return jsString(&vm, identifier.impl());
}

} // namespace JSC
