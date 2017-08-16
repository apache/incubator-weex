/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "HeapProfiler.h"

#include "HeapSnapshot.h"
#include "VM.h"

namespace JSC {

HeapProfiler::HeapProfiler(VM& vm)
    : m_vm(vm)
{
}

HeapProfiler::~HeapProfiler()
{
}

HeapSnapshot* HeapProfiler::mostRecentSnapshot()
{
    if (m_snapshots.isEmpty())
        return nullptr;
    return m_snapshots.last().get();
}

void HeapProfiler::appendSnapshot(std::unique_ptr<HeapSnapshot> snapshot)
{
    m_snapshots.append(WTFMove(snapshot));
}

void HeapProfiler::clearSnapshots()
{
    m_snapshots.clear();
}

void HeapProfiler::setActiveSnapshotBuilder(HeapSnapshotBuilder* builder)
{
    ASSERT(!!m_activeBuilder != !!builder);
    m_activeBuilder = builder;
}

} // namespace JSC
