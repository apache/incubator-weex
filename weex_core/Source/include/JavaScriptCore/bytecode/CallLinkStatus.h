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
