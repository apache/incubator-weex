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
#include "ExitingJITType.h"
#include "PutByIdVariant.h"
#include <wtf/text/StringImpl.h>

namespace JSC {

class CodeBlock;
class VM;
class JSGlobalObject;
class Structure;
class StructureChain;
class StructureStubInfo;

typedef HashMap<CodeOrigin, StructureStubInfo*, CodeOriginApproximateHash> StubInfoMap;

class PutByIdStatus {
public:
    enum State {
        // It's uncached so we have no information.
        NoInformation,
        // It's cached as a simple store of some kind.
        Simple,
        // It's known to often take slow path.
        TakesSlowPath,
        // It's known to take paths that make calls.
        MakesCalls
    };
    
    PutByIdStatus()
        : m_state(NoInformation)
    {
    }
    
    explicit PutByIdStatus(State state)
        : m_state(state)
    {
        ASSERT(m_state == NoInformation || m_state == TakesSlowPath || m_state == MakesCalls);
    }
    
    PutByIdStatus(const PutByIdVariant& variant)
        : m_state(Simple)
    {
        m_variants.append(variant);
    }
    
    static PutByIdStatus computeFor(CodeBlock*, StubInfoMap&, unsigned bytecodeIndex, UniquedStringImpl* uid);
    static PutByIdStatus computeFor(JSGlobalObject*, const StructureSet&, UniquedStringImpl* uid, bool isDirect);
    
    static PutByIdStatus computeFor(CodeBlock* baselineBlock, CodeBlock* dfgBlock, StubInfoMap& baselineMap, StubInfoMap& dfgMap, CodeOrigin, UniquedStringImpl* uid);

#if ENABLE(JIT)
    static PutByIdStatus computeForStubInfo(const ConcurrentJSLocker&, CodeBlock* baselineBlock, StructureStubInfo*, CodeOrigin, UniquedStringImpl* uid);
#endif
    
    State state() const { return m_state; }
    
    bool isSet() const { return m_state != NoInformation; }
    bool operator!() const { return m_state == NoInformation; }
    bool isSimple() const { return m_state == Simple; }
    bool takesSlowPath() const { return m_state == TakesSlowPath || m_state == MakesCalls; }
    bool makesCalls() const;
    
    size_t numVariants() const { return m_variants.size(); }
    const Vector<PutByIdVariant, 1>& variants() const { return m_variants; }
    const PutByIdVariant& at(size_t index) const { return m_variants[index]; }
    const PutByIdVariant& operator[](size_t index) const { return at(index); }
    
    void dump(PrintStream&) const;
    
private:
#if ENABLE(DFG_JIT)
    static bool hasExitSite(const ConcurrentJSLocker&, CodeBlock*, unsigned bytecodeIndex);
#endif
#if ENABLE(JIT)
    static PutByIdStatus computeForStubInfo(
        const ConcurrentJSLocker&, CodeBlock*, StructureStubInfo*, UniquedStringImpl* uid,
        CallLinkStatus::ExitSiteData);
#endif
    static PutByIdStatus computeFromLLInt(CodeBlock*, unsigned bytecodeIndex, UniquedStringImpl* uid);
    
    bool appendVariant(const PutByIdVariant&);
    
    State m_state;
    Vector<PutByIdVariant, 1> m_variants;
};

} // namespace JSC
