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

#include "HeapSnapshotBuilder.h"
#include "TinyBloomFilter.h"
#include <wtf/Optional.h>

namespace JSC {

class HeapSnapshot {
    WTF_MAKE_FAST_ALLOCATED;
public:
    HeapSnapshot(HeapSnapshot*);
    ~HeapSnapshot();

    HeapSnapshot* previous() const { return m_previous; }

    void appendNode(const HeapSnapshotNode&);
    void sweepCell(JSCell*);
    void shrinkToFit();
    void finalize();

    bool isEmpty() const { return m_nodes.isEmpty(); }
    std::optional<HeapSnapshotNode> nodeForCell(JSCell*);
    std::optional<HeapSnapshotNode> nodeForObjectIdentifier(unsigned objectIdentifier);

private:
    friend class HeapSnapshotBuilder;
    static const intptr_t CellToSweepTag = 1;

    Vector<HeapSnapshotNode> m_nodes;
    TinyBloomFilter m_filter;
    HeapSnapshot* m_previous { nullptr };
    unsigned m_firstObjectIdentifier { 0 };
    unsigned m_lastObjectIdentifier { 0 };
    bool m_finalized { false };
    bool m_hasCellsToSweep { false };
};

} // namespace JSC
