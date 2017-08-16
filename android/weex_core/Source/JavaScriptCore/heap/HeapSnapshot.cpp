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
#include "HeapSnapshot.h"

#include "JSCInlines.h"

namespace JSC {

HeapSnapshot::HeapSnapshot(HeapSnapshot* previousSnapshot)
    : m_previous(previousSnapshot)
{
}

HeapSnapshot::~HeapSnapshot()
{
}

void HeapSnapshot::appendNode(const HeapSnapshotNode& node)
{
    ASSERT(!m_finalized);
    ASSERT(!m_previous || !m_previous->nodeForCell(node.cell));

    m_nodes.append(node);
    m_filter.add(bitwise_cast<uintptr_t>(node.cell));
}

void HeapSnapshot::sweepCell(JSCell* cell)
{
    ASSERT(cell);

    if (m_finalized && !m_filter.ruleOut(bitwise_cast<uintptr_t>(cell))) {
        ASSERT_WITH_MESSAGE(!isEmpty(), "Our filter should have ruled us out if we are empty.");
        unsigned start = 0;
        unsigned end = m_nodes.size();
        while (start != end) {
            unsigned middle = start + ((end - start) / 2);
            HeapSnapshotNode& node = m_nodes[middle];
            if (cell == node.cell) {
                // Cells should always have 0 as low bits.
                // Mark this cell for removal by setting the low bit.
                ASSERT(!(reinterpret_cast<intptr_t>(node.cell) & CellToSweepTag));
                node.cell = reinterpret_cast<JSCell*>(reinterpret_cast<intptr_t>(node.cell) | CellToSweepTag);
                m_hasCellsToSweep = true;
                return;
            }
            if (cell < node.cell)
                end = middle;
            else
                start = middle + 1;
        }
    }

    if (m_previous)
        m_previous->sweepCell(cell);
}

void HeapSnapshot::shrinkToFit()
{
    if (m_finalized && m_hasCellsToSweep) {
        m_filter.reset();
        m_nodes.removeAllMatching(
            [&] (const HeapSnapshotNode& node) -> bool {
                bool willRemoveCell = bitwise_cast<intptr_t>(node.cell) & CellToSweepTag;
                if (!willRemoveCell)
                    m_filter.add(bitwise_cast<uintptr_t>(node.cell));
                return willRemoveCell;
            });
        m_nodes.shrinkToFit();
        m_hasCellsToSweep = false;
    }

    if (m_previous)
        m_previous->shrinkToFit();
}

void HeapSnapshot::finalize()
{
    ASSERT(!m_finalized);
    m_finalized = true;

    // Nodes are appended to the snapshot in identifier order.
    // Now that we have the complete list of nodes we will sort
    // them in a different order. Remember the range of identifiers
    // in this snapshot.
    if (!isEmpty()) {
        m_firstObjectIdentifier = m_nodes.first().identifier;
        m_lastObjectIdentifier = m_nodes.last().identifier;
    }

    std::sort(m_nodes.begin(), m_nodes.end(), [] (const HeapSnapshotNode& a, const HeapSnapshotNode& b) {
        return a.cell < b.cell;
    });

#ifndef NDEBUG
    // Assert there are no duplicates or nullptr cells.
    JSCell* previousCell = nullptr;
    for (auto& node : m_nodes) {
        ASSERT(node.cell);
        ASSERT(!(reinterpret_cast<intptr_t>(node.cell) & CellToSweepTag));
        if (node.cell == previousCell) {
            dataLog("Seeing same cell twice: ", RawPointer(previousCell), "\n");
            ASSERT(node.cell != previousCell);
        }
        previousCell = node.cell;
    }
#endif
}

std::optional<HeapSnapshotNode> HeapSnapshot::nodeForCell(JSCell* cell)
{
    ASSERT(m_finalized);

    if (!m_filter.ruleOut(bitwise_cast<uintptr_t>(cell))) {
        ASSERT_WITH_MESSAGE(!isEmpty(), "Our filter should have ruled us out if we are empty.");
        unsigned start = 0;
        unsigned end = m_nodes.size();
        while (start != end) {
            unsigned middle = start + ((end - start) / 2);
            HeapSnapshotNode& node = m_nodes[middle];
            if (cell == node.cell)
                return std::optional<HeapSnapshotNode>(node);
            if (cell < node.cell)
                end = middle;
            else
                start = middle + 1;
        }
    }

    if (m_previous)
        return m_previous->nodeForCell(cell);

    return std::nullopt;
}

std::optional<HeapSnapshotNode> HeapSnapshot::nodeForObjectIdentifier(unsigned objectIdentifier)
{
    if (isEmpty()) {
        if (m_previous)
            return m_previous->nodeForObjectIdentifier(objectIdentifier);
        return std::nullopt;
    }

    if (objectIdentifier > m_lastObjectIdentifier)
        return std::nullopt;

    if (objectIdentifier < m_firstObjectIdentifier) {
        if (m_previous)
            return m_previous->nodeForObjectIdentifier(objectIdentifier);
        return std::nullopt;
    }

    for (auto& node : m_nodes) {
        if (node.identifier == objectIdentifier)
            return std::optional<HeapSnapshotNode>(node);
    }

    return std::nullopt;
}

} // namespace JSC
