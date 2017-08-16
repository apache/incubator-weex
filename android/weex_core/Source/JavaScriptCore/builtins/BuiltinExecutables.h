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

#include "JSCBuiltins.h"
#include "ParserModes.h"
#include "SourceCode.h"
#include "Weak.h"
#include "WeakHandleOwner.h"

namespace JSC {

class UnlinkedFunctionExecutable;
class Identifier;
class VM;

class BuiltinExecutables final: private WeakHandleOwner {
    WTF_MAKE_FAST_ALLOCATED;
public:
    explicit BuiltinExecutables(VM&);

#define EXPOSE_BUILTIN_EXECUTABLES(name, functionName, length) \
UnlinkedFunctionExecutable* name##Executable(); \
const SourceCode& name##Source() { return m_##name##Source; }
    
    JSC_FOREACH_BUILTIN_CODE(EXPOSE_BUILTIN_EXECUTABLES)
#undef EXPOSE_BUILTIN_SOURCES

    UnlinkedFunctionExecutable* createDefaultConstructor(ConstructorKind, const Identifier& name);

    static UnlinkedFunctionExecutable* createExecutable(VM&, const SourceCode&, const Identifier&, ConstructorKind, ConstructAbility);
private:
    void finalize(Handle<Unknown>, void* context) override;

    VM& m_vm;

    UnlinkedFunctionExecutable* createBuiltinExecutable(const SourceCode&, const Identifier&, ConstructAbility);

#define DECLARE_BUILTIN_SOURCE_MEMBERS(name, functionName, length)\
    SourceCode m_##name##Source; \
    Weak<UnlinkedFunctionExecutable> m_##name##Executable;
    JSC_FOREACH_BUILTIN_CODE(DECLARE_BUILTIN_SOURCE_MEMBERS)
#undef DECLARE_BUILTIN_SOURCE_MEMBERS
};

}
