/*
 * Copyright (C) 2012, 2013, 2014 Apple Inc. All rights reserved.
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
