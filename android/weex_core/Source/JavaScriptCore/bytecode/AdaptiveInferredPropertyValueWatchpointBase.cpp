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
#include "AdaptiveInferredPropertyValueWatchpointBase.h"

#include "JSCInlines.h"

namespace JSC {

AdaptiveInferredPropertyValueWatchpointBase::AdaptiveInferredPropertyValueWatchpointBase(const ObjectPropertyCondition& key)
    : m_key(key)
{
    RELEASE_ASSERT(key.kind() == PropertyCondition::Equivalence);
}

void AdaptiveInferredPropertyValueWatchpointBase::install()
{
    RELEASE_ASSERT(m_key.isWatchable());

    m_key.object()->structure()->addTransitionWatchpoint(&m_structureWatchpoint);

    PropertyOffset offset = m_key.object()->structure()->getConcurrently(m_key.uid());
    WatchpointSet* set = m_key.object()->structure()->propertyReplacementWatchpointSet(offset);
    set->add(&m_propertyWatchpoint);
}

void AdaptiveInferredPropertyValueWatchpointBase::fire(const FireDetail& detail)
{
    // One of the watchpoints fired, but the other one didn't. Make sure that neither of them are
    // in any set anymore. This simplifies things by allowing us to reinstall the watchpoints
    // wherever from scratch.
    if (m_structureWatchpoint.isOnList())
        m_structureWatchpoint.remove();
    if (m_propertyWatchpoint.isOnList())
        m_propertyWatchpoint.remove();

    if (m_key.isWatchable(PropertyCondition::EnsureWatchability)) {
        install();
        return;
    }

    handleFire(detail);
}

void AdaptiveInferredPropertyValueWatchpointBase::StructureWatchpoint::fireInternal(const FireDetail& detail)
{
    ptrdiff_t myOffset = OBJECT_OFFSETOF(AdaptiveInferredPropertyValueWatchpointBase, m_structureWatchpoint);

    AdaptiveInferredPropertyValueWatchpointBase* parent = bitwise_cast<AdaptiveInferredPropertyValueWatchpointBase*>(bitwise_cast<char*>(this) - myOffset);

    parent->fire(detail);
}

void AdaptiveInferredPropertyValueWatchpointBase::PropertyWatchpoint::fireInternal(const FireDetail& detail)
{
    ptrdiff_t myOffset = OBJECT_OFFSETOF(AdaptiveInferredPropertyValueWatchpointBase, m_propertyWatchpoint);

    AdaptiveInferredPropertyValueWatchpointBase* parent = bitwise_cast<AdaptiveInferredPropertyValueWatchpointBase*>(bitwise_cast<char*>(this) - myOffset);
    
    parent->fire(detail);
}
    
} // namespace JSC
