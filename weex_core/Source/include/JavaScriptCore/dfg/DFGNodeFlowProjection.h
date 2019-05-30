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

#if ENABLE(DFG_JIT)

#include "DFGNode.h"
#include <wtf/HashTable.h>

namespace JSC { namespace DFG {

class NodeFlowProjection {
public:
    enum Kind {
        Primary,
        Shadow
    };
    
    NodeFlowProjection() { }
    
    NodeFlowProjection(Node* node)
        : m_word(bitwise_cast<uintptr_t>(node))
    {
        ASSERT(kind() == Primary);
    }
    
    NodeFlowProjection(Node* node, Kind kind)
        : m_word(bitwise_cast<uintptr_t>(node) | (kind == Shadow ? shadowBit : 0))
    {
        ASSERT(this->kind() == kind);
    }
    
    NodeFlowProjection(WTF::HashTableDeletedValueType)
        : m_word(shadowBit)
    {
    }
    
    explicit operator bool() const { return !!m_word; }
    
    Kind kind() const { return (m_word & shadowBit) ? Shadow : Primary; }
    
    Node* node() const { return bitwise_cast<Node*>(m_word & ~shadowBit); }
    
    Node& operator*() const { return *node(); }
    Node* operator->() const { return node(); }
    
    unsigned hash() const
    {
        return m_word;
    }
    
    bool operator==(NodeFlowProjection other) const
    {
        return m_word == other.m_word;
    }
    
    bool operator!=(NodeFlowProjection other) const
    {
        return !(*this == other);
    }
    
    bool operator<(NodeFlowProjection other) const
    {
        if (kind() != other.kind())
            return kind() < other.kind();
        return node() < other.node();
    }
    
    bool operator>(NodeFlowProjection other) const
    {
        return other < *this;
    }
    
    bool operator<=(NodeFlowProjection other) const
    {
        return !(*this > other);
    }
    
    bool operator>=(NodeFlowProjection other) const
    {
        return !(*this < other);
    }
    
    bool isHashTableDeletedValue() const
    {
        return *this == NodeFlowProjection(WTF::HashTableDeletedValue);
    }
    
    // Phi shadow projections can become invalid because the Phi might be folded to something else.
    bool isStillValid() const
    {
        return *this && (kind() == Primary || node()->op() == Phi);
    }
    
    void dump(PrintStream&) const;
    
    template<typename Func>
    static void forEach(Node* node, const Func& func)
    {
        func(NodeFlowProjection(node));
        if (node->op() == Phi)
            func(NodeFlowProjection(node, Shadow));
    }
    
public:
    static const uintptr_t shadowBit = 1;
    
    uintptr_t m_word { 0 };
};

struct NodeFlowProjectionHash {
    static unsigned hash(NodeFlowProjection key) { return key.hash(); }
    static bool equal(NodeFlowProjection a, NodeFlowProjection b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::NodeFlowProjection> {
    typedef JSC::DFG::NodeFlowProjectionHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::NodeFlowProjection> : SimpleClassHashTraits<JSC::DFG::NodeFlowProjection> { };

} // namespace WTF

#endif // ENABLE(DFG_JIT)

