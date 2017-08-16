/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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
#include "DFGCommonData.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "DFGNode.h"
#include "DFGPlan.h"
#include "InlineCallFrame.h"
#include "JSCInlines.h"
#include "TrackedReferences.h"
#include "VM.h"

namespace JSC { namespace DFG {

void CommonData::notifyCompilingStructureTransition(Plan& plan, CodeBlock* codeBlock, Node* node)
{
    plan.transitions.addLazily(
        codeBlock,
        node->origin.semantic.codeOriginOwner(),
        node->transition()->previous.get(),
        node->transition()->next.get());
}

CallSiteIndex CommonData::addCodeOrigin(CodeOrigin codeOrigin)
{
    if (codeOrigins.isEmpty()
        || codeOrigins.last() != codeOrigin)
        codeOrigins.append(codeOrigin);
    unsigned index = codeOrigins.size() - 1;
    ASSERT(codeOrigins[index] == codeOrigin);
    return CallSiteIndex(index);
}

CallSiteIndex CommonData::addUniqueCallSiteIndex(CodeOrigin codeOrigin)
{
    if (callSiteIndexFreeList.size())
        return CallSiteIndex(callSiteIndexFreeList.takeAny());

    codeOrigins.append(codeOrigin);
    unsigned index = codeOrigins.size() - 1;
    ASSERT(codeOrigins[index] == codeOrigin);
    return CallSiteIndex(index);
}

CallSiteIndex CommonData::lastCallSite() const
{
    RELEASE_ASSERT(codeOrigins.size());
    return CallSiteIndex(codeOrigins.size() - 1);
}

void CommonData::removeCallSiteIndex(CallSiteIndex callSite)
{
    RELEASE_ASSERT(callSite.bits() < codeOrigins.size());
    callSiteIndexFreeList.add(callSite.bits());
}

void CommonData::shrinkToFit()
{
    codeOrigins.shrinkToFit();
    weakReferences.shrinkToFit();
    transitions.shrinkToFit();
}

bool CommonData::invalidate()
{
    if (!isStillValid)
        return false;
    for (unsigned i = jumpReplacements.size(); i--;)
        jumpReplacements[i].fire();
    isStillValid = false;
    return true;
}

void CommonData::installVMTrapBreakpoints()
{
    if (!isStillValid || hasVMTrapsBreakpointsInstalled)
        return;
    hasVMTrapsBreakpointsInstalled = true;
    for (unsigned i = jumpReplacements.size(); i--;)
        jumpReplacements[i].installVMTrapBreakpoint();
}

bool CommonData::isVMTrapBreakpoint(void* address)
{
    if (!isStillValid)
        return false;
    for (unsigned i = jumpReplacements.size(); i--;) {
        if (address == jumpReplacements[i].dataLocation())
            return true;
    }
    return false;
}

void CommonData::validateReferences(const TrackedReferences& trackedReferences)
{
    if (InlineCallFrameSet* set = inlineCallFrames.get()) {
        for (InlineCallFrame* inlineCallFrame : *set) {
            for (ValueRecovery& recovery : inlineCallFrame->arguments) {
                if (recovery.isConstant())
                    trackedReferences.check(recovery.constant());
            }
            
            if (CodeBlock* baselineCodeBlock = inlineCallFrame->baselineCodeBlock.get())
                trackedReferences.check(baselineCodeBlock);
            
            if (inlineCallFrame->calleeRecovery.isConstant())
                trackedReferences.check(inlineCallFrame->calleeRecovery.constant());
        }
    }
    
    for (AdaptiveStructureWatchpoint* watchpoint : adaptiveStructureWatchpoints)
        watchpoint->key().validateReferences(trackedReferences);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

