/*
 * Copyright (C) 2012, 2015-2016 Apple Inc. All rights reserved.
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
#include "StructureStubClearingWatchpoint.h"

#if ENABLE(JIT)

#include "CodeBlock.h"
#include "JSCInlines.h"
#include "StructureStubInfo.h"

namespace JSC {

StructureStubClearingWatchpoint::~StructureStubClearingWatchpoint()
{
    for (auto current = WTFMove(m_next); current; current = WTFMove(current->m_next)) { }
}

StructureStubClearingWatchpoint* StructureStubClearingWatchpoint::push(
    const ObjectPropertyCondition& key,
    WatchpointsOnStructureStubInfo& holder,
    std::unique_ptr<StructureStubClearingWatchpoint>& head)
{
    head = std::make_unique<StructureStubClearingWatchpoint>(key, holder, WTFMove(head));
    return head.get();
}

void StructureStubClearingWatchpoint::fireInternal(const FireDetail&)
{
    if (!m_key || !m_key.isWatchable(PropertyCondition::EnsureWatchability)) {
        // This will implicitly cause my own demise: stub reset removes all watchpoints.
        // That works, because deleting a watchpoint removes it from the set's list, and
        // the set's list traversal for firing is robust against the set changing.
        ConcurrentJSLocker locker(m_holder.codeBlock()->m_lock);
        m_holder.stubInfo()->reset(m_holder.codeBlock());
        return;
    }

    if (m_key.kind() == PropertyCondition::Presence) {
        // If this was a presence condition, let's watch the property for replacements. This is profitable
        // for the DFG, which will want the replacement set to be valid in order to do constant folding.
        VM& vm = *Heap::heap(m_key.object())->vm();
        m_key.object()->structure()->startWatchingPropertyForReplacements(vm, m_key.offset());
    }

    m_key.object()->structure()->addTransitionWatchpoint(this);
}

WatchpointsOnStructureStubInfo::~WatchpointsOnStructureStubInfo()
{
}

StructureStubClearingWatchpoint* WatchpointsOnStructureStubInfo::addWatchpoint(const ObjectPropertyCondition& key)
{
    return StructureStubClearingWatchpoint::push(key, *this, m_head);
}

StructureStubClearingWatchpoint* WatchpointsOnStructureStubInfo::ensureReferenceAndAddWatchpoint(
    std::unique_ptr<WatchpointsOnStructureStubInfo>& holderRef, CodeBlock* codeBlock,
    StructureStubInfo* stubInfo, const ObjectPropertyCondition& key)
{
    if (!holderRef)
        holderRef = std::make_unique<WatchpointsOnStructureStubInfo>(codeBlock, stubInfo);
    else {
        ASSERT(holderRef->m_codeBlock == codeBlock);
        ASSERT(holderRef->m_stubInfo == stubInfo);
    }
    
    return holderRef->addWatchpoint(key);
}

} // namespace JSC

#endif // ENABLE(JIT)

