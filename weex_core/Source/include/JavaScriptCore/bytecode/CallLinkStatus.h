/*
 * Copyright (C) 2012-2016 Apple Inc. All rights reserved.
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

#include "CallLinkInfo.h"
#include "CallVariant.h"
#include "CodeOrigin.h"
#include "CodeSpecializationKind.h"
#include "ConcurrentJSLock.h"
#include "ExitingJITType.h"
#include "Intrinsic.h"
#include "JSCJSValue.h"

namespace JSC {

class CodeBlock;
class InternalFunction;
class JSFunction;
class Structure;
class CallLinkInfo;

class CallLinkStatus {
    WTF_MAKE_FAST_ALLOCATED;
public:
    CallLinkStatus()
    {
    }
    
    static CallLinkStatus takesSlowPath()
    {
        CallLinkStatus result;
        result.m_couldTakeSlowPath = true;
        return result;
    }
    
    explicit CallLinkStatus(JSValue);
    
    CallLinkStatus(CallVariant variant)
        : m_variants(1, variant)
    {
    }
    
    static CallLinkStatus computeFor(
        CodeBlock*, unsigned bytecodeIndex, const CallLinkInfoMap&);

    struct ExitSiteData {
        bool takesSlowPath { false };
        bool badFunction { false };
    };
    static ExitSiteData computeExitSiteData(const ConcurrentJSLocker&, CodeBlock*, unsigned bytecodeIndex);
    
#if ENABLE(JIT)
    // Computes the status assuming that we never took slow path and never previously
    // exited.
    static CallLinkStatus computeFor(const ConcurrentJSLocker&, CodeBlock*, CallLinkInfo&);
    static CallLinkStatus computeFor(
        const ConcurrentJSLocker&, CodeBlock*, CallLinkInfo&, ExitSiteData);
#endif
    
    typedef HashMap<CodeOrigin, CallLinkStatus, CodeOriginApproximateHash> ContextMap;
    
    // Computes all of the statuses of the DFG code block. Doesn't include statuses that had
    // no information. Currently we use this when compiling FTL code, to enable polyvariant
    // inlining.
    static void computeDFGStatuses(CodeBlock* dfgCodeBlock, ContextMap&);
    
    // Helper that first consults the ContextMap and then does computeFor().
    static CallLinkStatus computeFor(
        CodeBlock*, CodeOrigin, const CallLinkInfoMap&, const ContextMap&);
    
    void setProvenConstantCallee(CallVariant);
    
    bool isSet() const { return !m_variants.isEmpty() || m_couldTakeSlowPath; }
    
    bool operator!() const { return !isSet(); }
    
    bool couldTakeSlowPath() const { return m_couldTakeSlowPath; }
    
    void setCouldTakeSlowPath(bool value) { m_couldTakeSlowPath = value; }
    
    CallVariantList variants() const { return m_variants; }
    unsigned size() const { return m_variants.size(); }
    CallVariant at(unsigned i) const { return m_variants[i]; }
    CallVariant operator[](unsigned i) const { return at(i); }
    bool isProved() const { return m_isProved; }
    bool isBasedOnStub() const { return m_isBasedOnStub; }
    bool canOptimize() const { return !m_variants.isEmpty(); }

    bool isClosureCall() const; // Returns true if any callee is a closure call.
    
    unsigned maxNumArguments() const { return m_maxNumArguments; }
    
    void dump(PrintStream&) const;
    
private:
    void makeClosureCall();
    
    static CallLinkStatus computeFromLLInt(const ConcurrentJSLocker&, CodeBlock*, unsigned bytecodeIndex);
#if ENABLE(JIT)
    static CallLinkStatus computeFromCallLinkInfo(
        const ConcurrentJSLocker&, CallLinkInfo&);
#endif
    
    CallVariantList m_variants;
    bool m_couldTakeSlowPath { false };
    bool m_isProved { false };
    bool m_isBasedOnStub { false };
    unsigned m_maxNumArguments { 0 };
};

} // namespace JSC
