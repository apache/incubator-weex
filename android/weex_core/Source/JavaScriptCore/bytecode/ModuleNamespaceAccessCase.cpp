/*
 * Copyright (C) 2017 Yusuke Suzuki <utatane.tea@gmail.com>.
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
#include "ModuleNamespaceAccessCase.h"

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "HeapInlines.h"
#include "JSModuleEnvironment.h"
#include "JSModuleNamespaceObject.h"
#include "PolymorphicAccess.h"
#include "StructureStubInfo.h"

namespace JSC {

ModuleNamespaceAccessCase::ModuleNamespaceAccessCase(VM& vm, JSCell* owner, JSModuleNamespaceObject* moduleNamespaceObject, JSModuleEnvironment* moduleEnvironment, ScopeOffset scopeOffset)
    : Base(vm, owner, ModuleNamespaceLoad, invalidOffset, nullptr, ObjectPropertyConditionSet())
    , m_scopeOffset(scopeOffset)
{
    m_moduleNamespaceObject.set(vm, owner, moduleNamespaceObject);
    m_moduleEnvironment.set(vm, owner, moduleEnvironment);
}

std::unique_ptr<AccessCase> ModuleNamespaceAccessCase::create(VM& vm, JSCell* owner, JSModuleNamespaceObject* moduleNamespaceObject, JSModuleEnvironment* moduleEnvironment, ScopeOffset scopeOffset)
{
    return std::unique_ptr<AccessCase>(new ModuleNamespaceAccessCase(vm, owner, moduleNamespaceObject, moduleEnvironment, scopeOffset));
}

ModuleNamespaceAccessCase::~ModuleNamespaceAccessCase()
{
}

std::unique_ptr<AccessCase> ModuleNamespaceAccessCase::clone() const
{
    std::unique_ptr<ModuleNamespaceAccessCase> result(new ModuleNamespaceAccessCase(*this));
    result->resetState();
    return WTFMove(result);
}

void ModuleNamespaceAccessCase::emit(AccessGenerationState& state, MacroAssembler::JumpList& fallThrough)
{
    CCallHelpers& jit = *state.jit;
    JSValueRegs valueRegs = state.valueRegs;
    GPRReg baseGPR = state.baseGPR;

    fallThrough.append(
        jit.branchPtr(
            CCallHelpers::NotEqual,
            baseGPR,
            CCallHelpers::TrustedImmPtr(m_moduleNamespaceObject.get())));

    jit.loadValue(&m_moduleEnvironment->variableAt(m_scopeOffset), valueRegs);
    state.failAndIgnore.append(jit.branchIfEmpty(valueRegs));
    state.succeed();
}


} // namespace JSC

#endif // ENABLE(JIT)
