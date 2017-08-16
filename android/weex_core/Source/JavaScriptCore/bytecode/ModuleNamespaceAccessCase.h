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

#pragma once

#if ENABLE(JIT)

#include "AccessCase.h"

namespace JSC {

class JSModuleEnvironment;
class JSModuleNamespaceObject;

class ModuleNamespaceAccessCase : public AccessCase {
public:
    using Base = AccessCase;
    friend class AccessCase;

    JSModuleNamespaceObject* moduleNamespaceObject() const { return m_moduleNamespaceObject.get(); }
    JSModuleEnvironment* moduleEnvironment() const { return m_moduleEnvironment.get(); }
    ScopeOffset scopeOffset() const { return m_scopeOffset; }

    static std::unique_ptr<AccessCase> create(VM&, JSCell* owner, JSModuleNamespaceObject*, JSModuleEnvironment*, ScopeOffset);

    std::unique_ptr<AccessCase> clone() const override;

    void emit(AccessGenerationState&, MacroAssembler::JumpList& fallThrough);

    ~ModuleNamespaceAccessCase();

private:
    ModuleNamespaceAccessCase(VM&, JSCell* owner, JSModuleNamespaceObject*, JSModuleEnvironment*, ScopeOffset);

    WriteBarrier<JSModuleNamespaceObject> m_moduleNamespaceObject;
    WriteBarrier<JSModuleEnvironment> m_moduleEnvironment;
    ScopeOffset m_scopeOffset;
};

} // namespace JSC

#endif // ENABLE(JIT)
