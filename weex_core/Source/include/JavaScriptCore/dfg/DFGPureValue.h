/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

namespace JSC { namespace DFG {

class PureValue {
public:
    PureValue()
        : m_op(LastNodeType)
        , m_info(0)
    {
    }
    
    PureValue(NodeType op, const AdjacencyList& children, uintptr_t info)
        : m_op(op)
        , m_children(children.sanitized())
        , m_info(info)
    {
        ASSERT(!(defaultFlags(op) & NodeHasVarArgs));
    }
    
    PureValue(NodeType op, const AdjacencyList& children, const void* ptr)
        : PureValue(op, children, bitwise_cast<uintptr_t>(ptr))
    {
    }
    
    PureValue(NodeType op, const AdjacencyList& children)
        : PureValue(op, children, static_cast<uintptr_t>(0))
    {
    }
    
    PureValue(Node* node, uintptr_t info)
        : PureValue(node->op(), node->children, info)
    {
    }
    
    PureValue(Node* node, const void* ptr)
        : PureValue(node->op(), node->children, ptr)
    {
    }
    
    PureValue(Node* node)
        : PureValue(node->op(), node->children)
    {
    }
    
    PureValue(WTF::HashTableDeletedValueType)
        : m_op(LastNodeType)
        , m_info(1)
    {
    }
    
    bool operator!() const { return m_op == LastNodeType && !m_info; }
    
    NodeType op() const { return m_op; }
    const AdjacencyList& children() const { return m_children; }
    uintptr_t info() const { return m_info; }

    unsigned hash() const
    {
        return WTF::IntHash<int>::hash(static_cast<int>(m_op)) + m_children.hash() + m_info;
    }
    
    bool operator==(const PureValue& other) const
    {
        return m_op == other.m_op
            && m_children == other.m_children
            && m_info == other.m_info;
    }
    
    bool isHashTableDeletedValue() const
    {
        return m_op == LastNodeType && m_info;
    }
    
    void dump(PrintStream& out) const;
    
private:
    NodeType m_op;
    AdjacencyList m_children;
    uintptr_t m_info;
};

struct PureValueHash {
    static unsigned hash(const PureValue& key) { return key.hash(); }
    static bool equal(const PureValue& a, const PureValue& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::PureValue> {
    typedef JSC::DFG::PureValueHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::PureValue> : SimpleClassHashTraits<JSC::DFG::PureValue> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

namespace JSC { namespace DFG {

typedef HashMap<PureValue, Node*> PureMap;
typedef HashMap<PureValue, Vector<Node*>> PureMultiMap;

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
