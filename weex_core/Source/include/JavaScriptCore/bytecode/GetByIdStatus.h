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

#include "CallLinkStatus.h"
#include "CodeOrigin.h"
#include "ConcurrentJSLock.h"
#include "ExitingJITType.h"
#include "GetByIdVariant.h"
#include "ScopeOffset.h"

namespace JSC {

class AccessCase;
class CodeBlock;
class JSModuleEnvironment;
class JSModuleNamespaceObject;
class ModuleNamespaceAccessCase;
class StructureStubInfo;

typedef HashMap<CodeOrigin, StructureStubInfo*, CodeOriginApproximateHash> StubInfoMap;

class GetByIdStatus {
public:
    enum State {
        // It's uncached so we have no information.
        NoInformation,
        // It's cached for a simple access to a known object property with
        // a possible structure chain and a possible specific value.
        Simple,
        // It's cached for a custom accessor with a possible structure chain.
        Custom,
        // It's cached for an access to a module namespace object's binding.
        ModuleNamespace,
        // It's known to often take slow path.
        TakesSlowPath,
        // It's known to take paths that make calls.
        MakesCalls,
    };

    GetByIdStatus()
        : m_state(NoInformation)
    {
    }
    
    explicit GetByIdStatus(State state)
        : m_state(state)
    {
        ASSERT(state == NoInformation || state == TakesSlowPath || state == MakesCalls);
    }

    
    GetByIdStatus(
        State state, bool wasSeenInJIT, const GetByIdVariant& variant = GetByIdVariant())
        : m_state(state)
        , m_wasSeenInJIT(wasSeenInJIT)
    {
        ASSERT((state == Simple || state == Custom) == variant.isSet());
        m_variants.append(variant);
    }
    
    static GetByIdStatus computeFor(CodeBlock*, StubInfoMap&, unsigned bytecodeIndex, UniquedStringImpl* uid);
    static GetByIdStatus computeFor(const StructureSet&, UniquedStringImpl* uid);
    
    static GetByIdStatus computeFor(CodeBlock* baselineBlock, CodeBlock* dfgBlock, StubInfoMap& baselineMap, StubInfoMap& dfgMap, CodeOrigin, UniquedStringImpl* uid);

#if ENABLE(DFG_JIT)
    static GetByIdStatus computeForStubInfo(const ConcurrentJSLocker&, CodeBlock* baselineBlock, StructureStubInfo*, CodeOrigin, UniquedStringImpl* uid);
#endif

    State state() const { return m_state; }
    
    bool isSet() const { return m_state != NoInformation; }
    bool operator!() const { return !isSet(); }
    bool isSimple() const { return m_state == Simple; }
    bool isCustom() const { return m_state == Custom; }
    bool isModuleNamespace() const { return m_state == ModuleNamespace; }

    size_t numVariants() const { return m_variants.size(); }
    const Vector<GetByIdVariant, 1>& variants() const { return m_variants; }
    const GetByIdVariant& at(size_t index) const { return m_variants[index]; }
    const GetByIdVariant& operator[](size_t index) const { return at(index); }

    bool takesSlowPath() const { return m_state == TakesSlowPath || m_state == MakesCalls || m_state == Custom || m_state == ModuleNamespace; }
    bool makesCalls() const;
    
    bool wasSeenInJIT() const { return m_wasSeenInJIT; }
    
    // Attempts to reduce the set of variants to fit the given structure set. This may be approximate.
    void filter(const StructureSet&);

    JSModuleNamespaceObject* moduleNamespaceObject() const { return m_moduleNamespaceObject; }
    JSModuleEnvironment* moduleEnvironment() const { return m_moduleEnvironment; }
    ScopeOffset scopeOffset() const { return m_scopeOffset; }
    
    void dump(PrintStream&) const;
    
private:
#if ENABLE(DFG_JIT)
    static bool hasExitSite(const ConcurrentJSLocker&, CodeBlock*, unsigned bytecodeIndex);
#endif
#if ENABLE(JIT)
    GetByIdStatus(const ModuleNamespaceAccessCase&);
    static GetByIdStatus computeForStubInfoWithoutExitSiteFeedback(
        const ConcurrentJSLocker&, CodeBlock* profiledBlock, StructureStubInfo*,
        UniquedStringImpl* uid, CallLinkStatus::ExitSiteData);
#endif
    static GetByIdStatus computeFromLLInt(CodeBlock*, unsigned bytecodeIndex, UniquedStringImpl* uid);
    
    bool appendVariant(const GetByIdVariant&);
    
    State m_state;
    Vector<GetByIdVariant, 1> m_variants;
    bool m_wasSeenInJIT;
    JSModuleNamespaceObject* m_moduleNamespaceObject { nullptr };
    JSModuleEnvironment* m_moduleEnvironment { nullptr };
    ScopeOffset m_scopeOffset { };
};

} // namespace JSC
