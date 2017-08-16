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
