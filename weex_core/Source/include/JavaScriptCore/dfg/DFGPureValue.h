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
