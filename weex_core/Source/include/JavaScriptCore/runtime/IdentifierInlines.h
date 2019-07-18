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
