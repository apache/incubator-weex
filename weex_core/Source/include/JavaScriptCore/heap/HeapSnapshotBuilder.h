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

#include <functional>
#include <wtf/Lock.h>
#include <wtf/Vector.h>
#include <wtf/text/UniquedStringImpl.h>
#include <wtf/text/WTFString.h>

namespace JSC {

class HeapProfiler;
class HeapSnapshot;
class JSCell;

struct HeapSnapshotNode {
    HeapSnapshotNode(JSCell* cell, unsigned identifier)
        : cell(cell)
        , identifier(identifier)
    { }

    JSCell* cell;
    unsigned identifier;
};

enum class EdgeType : uint8_t {
    Internal,     // Normal strong reference. No name.
    Property,     // Named property. In `object.property` the name is "property"
    Index,        // Indexed property. In `array[0]` name is index "0".
    Variable,     // Variable held by a scope. In `let x, f=() => x++` name is "x" in f's captured scope.
    // FIXME: <https://webkit.org/b/154934> Heap Snapshot should include "Weak" edges
};

struct HeapSnapshotEdge {
    HeapSnapshotEdge(JSCell* fromCell, JSCell* toCell)
        : type(EdgeType::Internal)
    {
        from.cell = fromCell;
        to.cell = toCell;
    }

    HeapSnapshotEdge(JSCell* fromCell, JSCell* toCell, EdgeType type, UniquedStringImpl* name)
        : type(type)
    {
        ASSERT(type == EdgeType::Property || type == EdgeType::Variable);
        from.cell = fromCell;
        to.cell = toCell;
        u.name = name;
    }

    HeapSnapshotEdge(JSCell* fromCell, JSCell* toCell, uint32_t index)
        : type(EdgeType::Index)
    {
        from.cell = fromCell;
        to.cell = toCell;
        u.index = index;
    }

    union {
        JSCell *cell;
        unsigned identifier;
    } from;

    union {
        JSCell *cell;
        unsigned identifier;
    } to;

    union {
        UniquedStringImpl* name;
        uint32_t index;
    } u;

    EdgeType type;
};

class JS_EXPORT_PRIVATE HeapSnapshotBuilder {
    WTF_MAKE_FAST_ALLOCATED;
public:
    HeapSnapshotBuilder(HeapProfiler&);
    ~HeapSnapshotBuilder();

    static unsigned nextAvailableObjectIdentifier;
    static unsigned getNextObjectIdentifier();
    static void resetNextAvailableObjectIdentifier();

    // Performs a garbage collection that builds a snapshot of all live cells.
    void buildSnapshot();

    // A marked cell.
    void appendNode(JSCell*);

    // A reference from one cell to another.
    void appendEdge(JSCell* from, JSCell* to);
    void appendPropertyNameEdge(JSCell* from, JSCell* to, UniquedStringImpl* propertyName);
    void appendVariableNameEdge(JSCell* from, JSCell* to, UniquedStringImpl* variableName);
    void appendIndexEdge(JSCell* from, JSCell* to, uint32_t index);

    String json();
    String json(std::function<bool (const HeapSnapshotNode&)> allowNodeCallback);

private:
    // Finalized snapshots are not modified during building. So searching them
    // for an existing node can be done concurrently without a lock.
    bool hasExistingNodeForCell(JSCell*);

    HeapProfiler& m_profiler;

    // SlotVisitors run in parallel.
    Lock m_buildingNodeMutex;
    std::unique_ptr<HeapSnapshot> m_snapshot;
    Lock m_buildingEdgeMutex;
    Vector<HeapSnapshotEdge> m_edges;
};

} // namespace JSC
