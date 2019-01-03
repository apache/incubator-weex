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

#if ENABLE(DFG_JIT)

#include "DFGFlushedAt.h"
#include "DFGVariableAccessData.h"

namespace JSC { namespace DFG {

struct Node;

class Availability {
public:
    Availability()
        : m_node(0)
        , m_flushedAt(DeadFlush)
    {
    }
    
    explicit Availability(Node* node)
        : m_node(node)
        , m_flushedAt(ConflictingFlush)
    {
    }
    
    explicit Availability(FlushedAt flushedAt)
        : m_node(unavailableMarker())
        , m_flushedAt(flushedAt)
    {
    }
    
    Availability(Node* node, FlushedAt flushedAt)
        : m_node(node)
        , m_flushedAt(flushedAt)
    {
    }
    
    static Availability unavailable()
    {
        return Availability(unavailableMarker(), FlushedAt(ConflictingFlush));
    }
    
    Availability withFlush(FlushedAt flush) const
    {
        return Availability(m_node, flush);
    }
    
    Availability withNode(Node* node) const
    {
        return Availability(node, m_flushedAt);
    }
    
    Availability withUnavailableNode() const
    {
        return withNode(unavailableMarker());
    }
    
    void setFlush(FlushedAt flushedAt)
    {
        m_flushedAt = flushedAt;
    }
    
    void setNode(Node* node)
    {
        m_node = node;
    }
    
    void setNodeUnavailable()
    {
        m_node = unavailableMarker();
    }
    
    bool nodeIsUndecided() const { return !m_node; }
    bool nodeIsUnavailable() const { return m_node == unavailableMarker(); }
    
    bool hasNode() const { return !nodeIsUndecided() && !nodeIsUnavailable(); }
    bool shouldUseNode() const { return !isFlushUseful() && hasNode(); }
    
    Node* node() const
    {
        ASSERT(!nodeIsUndecided());
        ASSERT(!nodeIsUnavailable());
        return m_node;
    }
    
    FlushedAt flushedAt() const { return m_flushedAt; }
    bool isFlushUseful() const
    {
        return flushedAt().format() != DeadFlush && flushedAt().format() != ConflictingFlush;
    }
    
    bool isDead() const { return !isFlushUseful() && !hasNode(); }
    
    bool operator!() const { return nodeIsUnavailable() && flushedAt().format() == ConflictingFlush; }

    bool operator==(const Availability& other) const
    {
        return m_node == other.m_node
            && m_flushedAt == other.m_flushedAt;
    }
    
    bool operator!=(const Availability& other) const
    {
        return !(*this == other);
    }
    
    Availability merge(const Availability& other) const
    {
        return Availability(
            mergeNodes(m_node, other.m_node),
            m_flushedAt.merge(other.m_flushedAt));
    }
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;

private:
    static Node* mergeNodes(Node* a, Node* b)
    {
        if (!a)
            return b;
        if (!b)
            return a;
        if (a == b)
            return a;
        return unavailableMarker();
    }
    
    static Node* unavailableMarker()
    {
        return bitwise_cast<Node*>(static_cast<intptr_t>(1));
    }
    
    Node* m_node;
    FlushedAt m_flushedAt;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
