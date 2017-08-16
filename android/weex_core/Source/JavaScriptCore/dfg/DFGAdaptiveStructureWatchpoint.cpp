/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "DFGAdaptiveStructureWatchpoint.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

AdaptiveStructureWatchpoint::AdaptiveStructureWatchpoint(
    const ObjectPropertyCondition& key,
    CodeBlock* codeBlock)
    : m_key(key)
    , m_codeBlock(codeBlock)
{
    RELEASE_ASSERT(key.watchingRequiresStructureTransitionWatchpoint());
    RELEASE_ASSERT(!key.watchingRequiresReplacementWatchpoint());
}

void AdaptiveStructureWatchpoint::install()
{
    RELEASE_ASSERT(m_key.isWatchable());
    
    m_key.object()->structure()->addTransitionWatchpoint(this);
}

void AdaptiveStructureWatchpoint::fireInternal(const FireDetail& detail)
{
    if (m_key.isWatchable(PropertyCondition::EnsureWatchability)) {
        install();
        return;
    }
    
    if (DFG::shouldDumpDisassembly()) {
        dataLog(
            "Firing watchpoint ", RawPointer(this), " (", m_key, ") on ", *m_codeBlock, "\n");
    }
    
    StringPrintStream out;
    out.print("Adaptation of ", m_key, " failed: ", detail);
    
    StringFireDetail stringDetail(out.toCString().data());
    
    m_codeBlock->jettison(
        Profiler::JettisonDueToUnprofiledWatchpoint, CountReoptimization, &stringDetail);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

