/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
