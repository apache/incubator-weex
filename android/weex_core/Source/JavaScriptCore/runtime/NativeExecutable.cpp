/*
 * Copyright (C) 2009, 2010, 2013, 2015-2016 Apple Inc. All rights reserved.
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

#include "BatchedTransitionOptimizer.h"
#include "CodeBlock.h"
#include "Debugger.h"
#include "JIT.h"
#include "JSCInlines.h"
#include "LLIntEntrypoint.h"
#include "Parser.h"
#include "TypeProfiler.h"
#include "VMInlines.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

const ClassInfo NativeExecutable::s_info = { "NativeExecutable", &ExecutableBase::s_info, 0, CREATE_METHOD_TABLE(NativeExecutable) };

NativeExecutable* NativeExecutable::create(VM& vm, Ref<JITCode>&& callThunk, NativeFunction function, Ref<JITCode>&& constructThunk, NativeFunction constructor, Intrinsic intrinsic, const DOMJIT::Signature* signature, const String& name)
{
    NativeExecutable* executable;
    executable = new (NotNull, allocateCell<NativeExecutable>(vm.heap)) NativeExecutable(vm, function, constructor, intrinsic, signature);
    executable->finishCreation(vm, WTFMove(callThunk), WTFMove(constructThunk), name);
    return executable;
}

void NativeExecutable::destroy(JSCell* cell)
{
    static_cast<NativeExecutable*>(cell)->NativeExecutable::~NativeExecutable();
}

Structure* NativeExecutable::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
{
    return Structure::create(vm, globalObject, proto, TypeInfo(CellType, StructureFlags), info());
}

void NativeExecutable::finishCreation(VM& vm, Ref<JITCode>&& callThunk, Ref<JITCode>&& constructThunk, const String& name)
{
    Base::finishCreation(vm);
    m_jitCodeForCall = WTFMove(callThunk);
    m_jitCodeForConstruct = WTFMove(constructThunk);
    m_jitCodeForCallWithArityCheck = m_jitCodeForCall->addressForCall(MustCheckArity);
    m_jitCodeForConstructWithArityCheck = m_jitCodeForConstruct->addressForCall(MustCheckArity);
    m_name = name;
}

NativeExecutable::NativeExecutable(VM& vm, NativeFunction function, NativeFunction constructor, Intrinsic intrinsic, const DOMJIT::Signature* signature)
    : ExecutableBase(vm, vm.nativeExecutableStructure.get(), NUM_PARAMETERS_IS_HOST, intrinsic)
    , m_function(function)
    , m_constructor(constructor)
    , m_signature(signature)
{
}

CodeBlockHash NativeExecutable::hashFor(CodeSpecializationKind kind) const
{
    if (kind == CodeForCall)
        return CodeBlockHash(static_cast<unsigned>(bitwise_cast<size_t>(m_function)));
    
    RELEASE_ASSERT(kind == CodeForConstruct);
    return CodeBlockHash(static_cast<unsigned>(bitwise_cast<size_t>(m_constructor)));
}

} // namespace JSC
