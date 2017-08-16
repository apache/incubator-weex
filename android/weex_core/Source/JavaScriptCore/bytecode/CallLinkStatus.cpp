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

#include "config.h"
#include "CallLinkStatus.h"

#include "CallLinkInfo.h"
#include "CodeBlock.h"
#include "DFGJITCode.h"
#include "InlineCallFrame.h"
#include "Interpreter.h"
#include "LLIntCallLinkInfo.h"
#include "JSCInlines.h"
#include <wtf/CommaPrinter.h>
#include <wtf/ListDump.h>

namespace JSC {

static const bool verbose = false;

CallLinkStatus::CallLinkStatus(JSValue value)
    : m_couldTakeSlowPath(false)
    , m_isProved(false)
{
    if (!value || !value.isCell()) {
        m_couldTakeSlowPath = true;
        return;
    }
    
    m_variants.append(CallVariant(value.asCell()));
}

CallLinkStatus CallLinkStatus::computeFromLLInt(const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, unsigned bytecodeIndex)
{
    UNUSED_PARAM(profiledBlock);
    UNUSED_PARAM(bytecodeIndex);
#if ENABLE(DFG_JIT)
    if (profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadCell))) {
        // We could force this to be a closure call, but instead we'll just assume that it
        // takes slow path.
        return takesSlowPath();
    }
#else
    UNUSED_PARAM(locker);
#endif

    VM& vm = *profiledBlock->vm();
    
    Instruction* instruction = profiledBlock->instructions().begin() + bytecodeIndex;
    OpcodeID op = vm.interpreter->getOpcodeID(instruction[0].u.opcode);
    if (op != op_call && op != op_construct && op != op_tail_call)
        return CallLinkStatus();
    
    LLIntCallLinkInfo* callLinkInfo = instruction[5].u.callLinkInfo;
    
    return CallLinkStatus(callLinkInfo->lastSeenCallee.get());
}

CallLinkStatus CallLinkStatus::computeFor(
    CodeBlock* profiledBlock, unsigned bytecodeIndex, const CallLinkInfoMap& map)
{
    ConcurrentJSLocker locker(profiledBlock->m_lock);
    
    UNUSED_PARAM(profiledBlock);
    UNUSED_PARAM(bytecodeIndex);
    UNUSED_PARAM(map);
#if ENABLE(DFG_JIT)
    ExitSiteData exitSiteData = computeExitSiteData(locker, profiledBlock, bytecodeIndex);
    
    CallLinkInfo* callLinkInfo = map.get(CodeOrigin(bytecodeIndex));
    if (!callLinkInfo) {
        if (exitSiteData.takesSlowPath)
            return takesSlowPath();
        return computeFromLLInt(locker, profiledBlock, bytecodeIndex);
    }
    
    return computeFor(locker, profiledBlock, *callLinkInfo, exitSiteData);
#else
    return CallLinkStatus();
#endif
}

CallLinkStatus::ExitSiteData CallLinkStatus::computeExitSiteData(
    const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, unsigned bytecodeIndex)
{
    ExitSiteData exitSiteData;
    
#if ENABLE(DFG_JIT)
    exitSiteData.takesSlowPath =
        profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadType))
        || profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadExecutable));
    exitSiteData.badFunction =
        profiledBlock->hasExitSite(locker, DFG::FrequentExitSite(bytecodeIndex, BadCell));
#else
    UNUSED_PARAM(locker);
    UNUSED_PARAM(profiledBlock);
    UNUSED_PARAM(bytecodeIndex);
#endif
    
    return exitSiteData;
}

#if ENABLE(JIT)
CallLinkStatus CallLinkStatus::computeFor(
    const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, CallLinkInfo& callLinkInfo)
{
    // We don't really need this, but anytime we have to debug this code, it becomes indispensable.
    UNUSED_PARAM(profiledBlock);
    
    CallLinkStatus result = computeFromCallLinkInfo(locker, callLinkInfo);
    result.m_maxNumArguments = callLinkInfo.maxNumArguments();
    return result;
}

CallLinkStatus CallLinkStatus::computeFromCallLinkInfo(
    const ConcurrentJSLocker&, CallLinkInfo& callLinkInfo)
{
    if (callLinkInfo.clearedByGC())
        return takesSlowPath();
    
    // Note that despite requiring that the locker is held, this code is racy with respect
    // to the CallLinkInfo: it may get cleared while this code runs! This is because
    // CallLinkInfo::unlink() may be called from a different CodeBlock than the one that owns
    // the CallLinkInfo and currently we save space by not having CallLinkInfos know who owns
    // them. So, there is no way for either the caller of CallLinkInfo::unlock() or unlock()
    // itself to figure out which lock to lock.
    //
    // Fortunately, that doesn't matter. The only things we ask of CallLinkInfo - the slow
    // path count, the stub, and the target - can all be asked racily. Stubs and targets can
    // only be deleted at next GC, so if we load a non-null one, then it must contain data
    // that is still marginally valid (i.e. the pointers ain't stale). This kind of raciness
    // is probably OK for now.
    
    // PolymorphicCallStubRoutine is a GCAwareJITStubRoutine, so if non-null, it will stay alive
    // until next GC even if the CallLinkInfo is concurrently cleared. Also, the variants list is
    // never mutated after the PolymorphicCallStubRoutine is instantiated. We have some conservative
    // fencing in place to make sure that we see the variants list after construction.
    if (PolymorphicCallStubRoutine* stub = callLinkInfo.stub()) {
        WTF::loadLoadFence();
        
        CallEdgeList edges = stub->edges();
        
        // Now that we've loaded the edges list, there are no further concurrency concerns. We will
        // just manipulate and prune this list to our liking - mostly removing entries that are too
        // infrequent and ensuring that it's sorted in descending order of frequency.
        
        RELEASE_ASSERT(edges.size());
        
        std::sort(
            edges.begin(), edges.end(),
            [] (CallEdge a, CallEdge b) {
                return a.count() > b.count();
            });
        RELEASE_ASSERT(edges.first().count() >= edges.last().count());
        
        double totalCallsToKnown = 0;
        double totalCallsToUnknown = callLinkInfo.slowPathCount();
        CallVariantList variants;
        for (size_t i = 0; i < edges.size(); ++i) {
            CallEdge edge = edges[i];
            // If the call is at the tail of the distribution, then we don't optimize it and we
            // treat it as if it was a call to something unknown. We define the tail as being either
            // a call that doesn't belong to the N most frequent callees (N =
            // maxPolymorphicCallVariantsForInlining) or that has a total call count that is too
            // small.
            if (i >= Options::maxPolymorphicCallVariantsForInlining()
                || edge.count() < Options::frequentCallThreshold())
                totalCallsToUnknown += edge.count();
            else {
                totalCallsToKnown += edge.count();
                variants.append(edge.callee());
            }
        }
        
        // Bail if we didn't find any calls that qualified.
        RELEASE_ASSERT(!!totalCallsToKnown == !!variants.size());
        if (variants.isEmpty())
            return takesSlowPath();
        
        // We require that the distribution of callees is skewed towards a handful of common ones.
        if (totalCallsToKnown / totalCallsToUnknown < Options::minimumCallToKnownRate())
            return takesSlowPath();
        
        RELEASE_ASSERT(totalCallsToKnown);
        RELEASE_ASSERT(variants.size());
        
        CallLinkStatus result;
        result.m_variants = variants;
        result.m_couldTakeSlowPath = !!totalCallsToUnknown;
        result.m_isBasedOnStub = true;
        return result;
    }
    
    CallLinkStatus result;
    
    if (JSFunction* target = callLinkInfo.lastSeenCallee()) {
        CallVariant variant(target);
        if (callLinkInfo.hasSeenClosure())
            variant = variant.despecifiedClosure();
        result.m_variants.append(variant);
    }
    
    result.m_couldTakeSlowPath = !!callLinkInfo.slowPathCount();

    return result;
}

CallLinkStatus CallLinkStatus::computeFor(
    const ConcurrentJSLocker& locker, CodeBlock* profiledBlock, CallLinkInfo& callLinkInfo,
    ExitSiteData exitSiteData)
{
    CallLinkStatus result = computeFor(locker, profiledBlock, callLinkInfo);
    if (exitSiteData.badFunction) {
        if (result.isBasedOnStub()) {
            // If we have a polymorphic stub, then having an exit site is not quite so useful. In
            // most cases, the information in the stub has higher fidelity.
            result.makeClosureCall();
        } else {
            // We might not have a polymorphic stub for any number of reasons. When this happens, we
            // are in less certain territory, so exit sites mean a lot.
            result.m_couldTakeSlowPath = true;
        }
    }
    if (exitSiteData.takesSlowPath)
        result.m_couldTakeSlowPath = true;
    
    return result;
}
#endif

void CallLinkStatus::computeDFGStatuses(
    CodeBlock* dfgCodeBlock, CallLinkStatus::ContextMap& map)
{
#if ENABLE(DFG_JIT)
    RELEASE_ASSERT(dfgCodeBlock->jitType() == JITCode::DFGJIT);
    CodeBlock* baselineCodeBlock = dfgCodeBlock->alternative();
    for (auto iter = dfgCodeBlock->callLinkInfosBegin(); !!iter; ++iter) {
        CallLinkInfo& info = **iter;
        if (info.isDirect()) {
            // If the DFG was able to get a direct call then probably so will we. However, there is
            // a remote chance that it's bad news to lose information about what the DFG did. We'd
            // ideally like to just know that the DFG had emitted a DirectCall.
            continue;
        }
        CodeOrigin codeOrigin = info.codeOrigin();
        
        // Check if we had already previously made a terrible mistake in the FTL for this
        // code origin. Note that this is approximate because we could have a monovariant
        // inline in the FTL that ended up failing. We should fix that at some point by
        // having data structures to track the context of frequent exits. This is currently
        // challenging because it would require creating a CodeOrigin-based database in
        // baseline CodeBlocks, but those CodeBlocks don't really have a place to put the
        // InlineCallFrames.
        CodeBlock* currentBaseline =
            baselineCodeBlockForOriginAndBaselineCodeBlock(codeOrigin, baselineCodeBlock);
        ExitSiteData exitSiteData;
        {
            ConcurrentJSLocker locker(currentBaseline->m_lock);
            exitSiteData = computeExitSiteData(
                locker, currentBaseline, codeOrigin.bytecodeIndex);
        }
        
        {
            ConcurrentJSLocker locker(dfgCodeBlock->m_lock);
            map.add(info.codeOrigin(), computeFor(locker, dfgCodeBlock, info, exitSiteData));
        }
    }
#else
    UNUSED_PARAM(dfgCodeBlock);
#endif // ENABLE(DFG_JIT)
    
    if (verbose) {
        dataLog("Context map:\n");
        ContextMap::iterator iter = map.begin();
        ContextMap::iterator end = map.end();
        for (; iter != end; ++iter) {
            dataLog("    ", iter->key, ":\n");
            dataLog("        ", iter->value, "\n");
        }
    }
}

CallLinkStatus CallLinkStatus::computeFor(
    CodeBlock* profiledBlock, CodeOrigin codeOrigin,
    const CallLinkInfoMap& baselineMap, const CallLinkStatus::ContextMap& dfgMap)
{
    auto iter = dfgMap.find(codeOrigin);
    if (iter != dfgMap.end())
        return iter->value;
    
    return computeFor(profiledBlock, codeOrigin.bytecodeIndex, baselineMap);
}

void CallLinkStatus::setProvenConstantCallee(CallVariant variant)
{
    m_variants = CallVariantList{ variant };
    m_couldTakeSlowPath = false;
    m_isProved = true;
}

bool CallLinkStatus::isClosureCall() const
{
    for (unsigned i = m_variants.size(); i--;) {
        if (m_variants[i].isClosureCall())
            return true;
    }
    return false;
}

void CallLinkStatus::makeClosureCall()
{
    m_variants = despecifiedVariantList(m_variants);
}

void CallLinkStatus::dump(PrintStream& out) const
{
    if (!isSet()) {
        out.print("Not Set");
        return;
    }
    
    CommaPrinter comma;
    
    if (m_isProved)
        out.print(comma, "Statically Proved");
    
    if (m_couldTakeSlowPath)
        out.print(comma, "Could Take Slow Path");
    
    if (m_isBasedOnStub)
        out.print(comma, "Based On Stub");
    
    if (!m_variants.isEmpty())
        out.print(comma, listDump(m_variants));
    
    if (m_maxNumArguments)
        out.print(comma, "maxNumArguments = ", m_maxNumArguments);
}

} // namespace JSC

