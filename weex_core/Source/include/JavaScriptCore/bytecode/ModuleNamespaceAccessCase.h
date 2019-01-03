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
