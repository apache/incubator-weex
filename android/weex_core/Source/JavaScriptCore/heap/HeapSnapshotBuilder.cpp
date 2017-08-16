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
#include "HeapSnapshotBuilder.h"

#include "DeferGC.h"
#include "Heap.h"
#include "HeapProfiler.h"
#include "HeapSnapshot.h"
#include "JSCInlines.h"
#include "JSCell.h"
#include "PreventCollectionScope.h"
#include "VM.h"
#include <wtf/text/StringBuilder.h>

namespace JSC {
    
unsigned HeapSnapshotBuilder::nextAvailableObjectIdentifier = 1;
unsigned HeapSnapshotBuilder::getNextObjectIdentifier() { return nextAvailableObjectIdentifier++; }
void HeapSnapshotBuilder::resetNextAvailableObjectIdentifier() { HeapSnapshotBuilder::nextAvailableObjectIdentifier = 1; }

HeapSnapshotBuilder::HeapSnapshotBuilder(HeapProfiler& profiler)
    : m_profiler(profiler)
{
}

HeapSnapshotBuilder::~HeapSnapshotBuilder()
{
}

void HeapSnapshotBuilder::buildSnapshot()
{
    PreventCollectionScope preventCollectionScope(m_profiler.vm().heap);
    
    m_snapshot = std::make_unique<HeapSnapshot>(m_profiler.mostRecentSnapshot());
    {
        m_profiler.setActiveSnapshotBuilder(this);
        m_profiler.vm().heap.collectAllGarbage();
        m_profiler.setActiveSnapshotBuilder(nullptr);
    }
    m_snapshot->finalize();

    m_profiler.appendSnapshot(WTFMove(m_snapshot));
}

void HeapSnapshotBuilder::appendNode(JSCell* cell)
{
    ASSERT(m_profiler.activeSnapshotBuilder() == this);
    ASSERT(Heap::isMarkedConcurrently(cell));

    if (hasExistingNodeForCell(cell))
        return;

    std::lock_guard<Lock> lock(m_buildingNodeMutex);

    m_snapshot->appendNode(HeapSnapshotNode(cell, getNextObjectIdentifier()));
}

void HeapSnapshotBuilder::appendEdge(JSCell* from, JSCell* to)
{
    ASSERT(m_profiler.activeSnapshotBuilder() == this);
    ASSERT(to);

    // Avoid trivial edges.
    if (from == to)
        return;

    std::lock_guard<Lock> lock(m_buildingEdgeMutex);

    m_edges.append(HeapSnapshotEdge(from, to));
}

void HeapSnapshotBuilder::appendPropertyNameEdge(JSCell* from, JSCell* to, UniquedStringImpl* propertyName)
{
    ASSERT(m_profiler.activeSnapshotBuilder() == this);
    ASSERT(to);

    std::lock_guard<Lock> lock(m_buildingEdgeMutex);

    m_edges.append(HeapSnapshotEdge(from, to, EdgeType::Property, propertyName));
}

void HeapSnapshotBuilder::appendVariableNameEdge(JSCell* from, JSCell* to, UniquedStringImpl* variableName)
{
    ASSERT(m_profiler.activeSnapshotBuilder() == this);
    ASSERT(to);

    std::lock_guard<Lock> lock(m_buildingEdgeMutex);

    m_edges.append(HeapSnapshotEdge(from, to, EdgeType::Variable, variableName));
}

void HeapSnapshotBuilder::appendIndexEdge(JSCell* from, JSCell* to, uint32_t index)
{
    ASSERT(m_profiler.activeSnapshotBuilder() == this);
    ASSERT(to);

    std::lock_guard<Lock> lock(m_buildingEdgeMutex);

    m_edges.append(HeapSnapshotEdge(from, to, index));
}

bool HeapSnapshotBuilder::hasExistingNodeForCell(JSCell* cell)
{
    if (!m_snapshot->previous())
        return false;

    return !!m_snapshot->previous()->nodeForCell(cell);
}


// Heap Snapshot JSON Format:
//
//   {
//      "version": 1.0,
//      "nodes": [
//          <nodeId>, <sizeInBytes>, <nodeClassNameIndex>, <internal>,
//          <nodeId>, <sizeInBytes>, <nodeClassNameIndex>, <internal>,
//          ...
//      ],
//      "nodeClassNames": [
//          "string", "Structure", "Object", ...
//      ],
//      "edges": [
//          <fromNodeId>, <toNodeId>, <edgeTypeIndex>, <edgeExtraData>,
//          <fromNodeId>, <toNodeId>, <edgeTypeIndex>, <edgeExtraData>,
//          ...
//      ],
//      "edgeTypes": [
//          "Internal", "Property", "Index", "Variable"
//      ],
//      "edgeNames": [
//          "propertyName", "variableName", ...
//      ]
//   }
//
// Notes:
//
//     <nodeClassNameIndex>
//       - index into the "nodeClassNames" list.
//
//     <internal>
//       - 0 = false, 1 = true.
//
//     <edgeTypeIndex>
//       - index into the "edgeTypes" list.
//
//     <edgeExtraData>
//       - for Internal edges this should be ignored (0).
//       - for Index edges this is the index value.
//       - for Property or Variable edges this is an index into the "edgeNames" list.

static uint8_t edgeTypeToNumber(EdgeType type)
{
    return static_cast<uint8_t>(type);
}

static const char* edgeTypeToString(EdgeType type)
{
    switch (type) {
    case EdgeType::Internal:
        return "Internal";
    case EdgeType::Property:
        return "Property";
    case EdgeType::Index:
        return "Index";
    case EdgeType::Variable:
        return "Variable";
    }
    ASSERT_NOT_REACHED();
    return "Internal";
}

String HeapSnapshotBuilder::json()
{
    return json([] (const HeapSnapshotNode&) { return true; });
}

String HeapSnapshotBuilder::json(std::function<bool (const HeapSnapshotNode&)> allowNodeCallback)
{
    VM& vm = m_profiler.vm();
    DeferGCForAWhile deferGC(vm.heap);

    // Build a node to identifier map of allowed nodes to use when serializing edges.
    HashMap<JSCell*, unsigned> allowedNodeIdentifiers;

    // Build a list of used class names.
    HashMap<const char*, unsigned> classNameIndexes;
    classNameIndexes.set("<root>", 0);
    unsigned nextClassNameIndex = 1;

    // Build a list of used edge names.
    HashMap<UniquedStringImpl*, unsigned> edgeNameIndexes;
    unsigned nextEdgeNameIndex = 0;

    StringBuilder json;

    auto appendNodeJSON = [&] (const HeapSnapshotNode& node) {
        // Let the client decide if they want to allow or disallow certain nodes.
        if (!allowNodeCallback(node))
            return;

        allowedNodeIdentifiers.set(node.cell, node.identifier);

        auto result = classNameIndexes.add(node.cell->classInfo(vm)->className, nextClassNameIndex);
        if (result.isNewEntry)
            nextClassNameIndex++;
        unsigned classNameIndex = result.iterator->value;

        bool isInternal = false;
        if (!node.cell->isString()) {
            Structure* structure = node.cell->structure(vm);
            isInternal = !structure || !structure->globalObject();
        }

        // <nodeId>, <sizeInBytes>, <className>, <optionalInternalBoolean>
        json.append(',');
        json.appendNumber(node.identifier);
        json.append(',');
        json.appendNumber(node.cell->estimatedSizeInBytes());
        json.append(',');
        json.appendNumber(classNameIndex);
        json.append(',');
        json.append(isInternal ? '1' : '0');
    };

    bool firstEdge = true;
    auto appendEdgeJSON = [&] (const HeapSnapshotEdge& edge) {
        if (!firstEdge)
            json.append(',');
        firstEdge = false;

        // <fromNodeId>, <toNodeId>, <edgeTypeIndex>, <edgeExtraData>
        json.appendNumber(edge.from.identifier);
        json.append(',');
        json.appendNumber(edge.to.identifier);
        json.append(',');
        json.appendNumber(edgeTypeToNumber(edge.type));
        json.append(',');
        switch (edge.type) {
        case EdgeType::Property:
        case EdgeType::Variable: {
            auto result = edgeNameIndexes.add(edge.u.name, nextEdgeNameIndex);
            if (result.isNewEntry)
                nextEdgeNameIndex++;
            unsigned edgeNameIndex = result.iterator->value;
            json.appendNumber(edgeNameIndex);
            break;
        }
        case EdgeType::Index:
            json.appendNumber(edge.u.index);
            break;
        default:
            // No data for this edge type.
            json.append('0');
            break;
        }
    };

    json.append('{');

    // version
    json.appendLiteral("\"version\":1");

    // nodes
    json.append(',');
    json.appendLiteral("\"nodes\":");
    json.append('[');
    json.appendLiteral("0,0,0,0"); // <root>
    for (HeapSnapshot* snapshot = m_profiler.mostRecentSnapshot(); snapshot; snapshot = snapshot->previous()) {
        for (auto& node : snapshot->m_nodes)
            appendNodeJSON(node);
    }
    json.append(']');

    // node class names
    json.append(',');
    json.appendLiteral("\"nodeClassNames\":");
    json.append('[');
    Vector<const char *> orderedClassNames(classNameIndexes.size());
    for (auto& entry : classNameIndexes)
        orderedClassNames[entry.value] = entry.key;
    classNameIndexes.clear();
    bool firstClassName = true;
    for (auto& className : orderedClassNames) {
        if (!firstClassName)
            json.append(',');
        firstClassName = false;
        json.appendQuotedJSONString(className);
    }
    orderedClassNames.clear();
    json.append(']');

    // Process edges.
    // Replace pointers with identifiers.
    // Remove any edges that we won't need.
    m_edges.removeAllMatching([&] (HeapSnapshotEdge& edge) {
        // If the from cell is null, this means a <root> edge.
        if (!edge.from.cell)
            edge.from.identifier = 0;
        else {
            auto fromLookup = allowedNodeIdentifiers.find(edge.from.cell);
            if (fromLookup == allowedNodeIdentifiers.end())
                return true;
            edge.from.identifier = fromLookup->value;
        }

        if (!edge.to.cell)
            edge.to.identifier = 0;
        else {
            auto toLookup = allowedNodeIdentifiers.find(edge.to.cell);
            if (toLookup == allowedNodeIdentifiers.end())
                return true;
            edge.to.identifier = toLookup->value;
        }

        return false;
    });
    allowedNodeIdentifiers.clear();
    m_edges.shrinkToFit();

    // Sort edges based on from identifier.
    std::sort(m_edges.begin(), m_edges.end(), [&] (const HeapSnapshotEdge& a, const HeapSnapshotEdge& b) {
        return a.from.identifier < b.from.identifier;
    });

    // edges
    json.append(',');
    json.appendLiteral("\"edges\":");
    json.append('[');
    for (auto& edge : m_edges)
        appendEdgeJSON(edge);
    json.append(']');

    // edge types
    json.append(',');
    json.appendLiteral("\"edgeTypes\":");
    json.append('[');
    json.appendQuotedJSONString(edgeTypeToString(EdgeType::Internal));
    json.append(',');
    json.appendQuotedJSONString(edgeTypeToString(EdgeType::Property));
    json.append(',');
    json.appendQuotedJSONString(edgeTypeToString(EdgeType::Index));
    json.append(',');
    json.appendQuotedJSONString(edgeTypeToString(EdgeType::Variable));
    json.append(']');

    // edge names
    json.append(',');
    json.appendLiteral("\"edgeNames\":");
    json.append('[');
    Vector<UniquedStringImpl*> orderedEdgeNames(edgeNameIndexes.size());
    for (auto& entry : edgeNameIndexes)
        orderedEdgeNames[entry.value] = entry.key;
    edgeNameIndexes.clear();
    bool firstEdgeName = true;
    for (auto& edgeName : orderedEdgeNames) {
        if (!firstEdgeName)
            json.append(',');
        firstEdgeName = false;
        json.appendQuotedJSONString(edgeName);
    }
    orderedEdgeNames.clear();
    json.append(']');

    json.append('}');
    return json.toString();
}

} // namespace JSC
