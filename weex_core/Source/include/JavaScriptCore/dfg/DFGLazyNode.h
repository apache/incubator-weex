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

#include "DFGCommon.h"
#include "DFGInsertionSet.h"
#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

class LazyNode {
public:
    static const size_t jsConstantTag = 0;
    static const size_t doubleConstantTag = 1;
    static const size_t int52ConstantTag = 2;

    static const uintptr_t tagMask = 0x3;
    static const uintptr_t pointerMask = ~tagMask;

    explicit LazyNode(Node* node = nullptr)
        : m_node(node)
        , m_value(reinterpret_cast<uintptr_t>(nullptr))
    {
        if (node && node->isConstant())
            setFrozenValue(node->constant(), node->op());
    }

    explicit LazyNode(FrozenValue* value, NodeType op = JSConstant)
        : m_node(nullptr)
        , m_value(reinterpret_cast<uintptr_t>(nullptr))
    {
        setFrozenValue(value, op);
    }

    LazyNode(std::nullptr_t)
        : m_node(nullptr)
        , m_value(reinterpret_cast<uintptr_t>(nullptr))
    {
    }

    LazyNode(WTF::HashTableDeletedValueType)
        : m_node(reinterpret_cast<Node*>(-1))
    {
    }

    void setNode(Node* node)
    {
        m_node = node;
        if (node && node->isConstant())
            setFrozenValue(node->constant(), node->op());
    }

    bool isHashTableDeletedValue() const { return m_node == reinterpret_cast<Node*>(-1); }

    bool isNode() const { return m_node; }

    NodeType op() const
    {
        if (m_node)
            return m_node->op();

        switch (m_value & tagMask) {
        case jsConstantTag:
            return JSConstant;
        case doubleConstantTag:
            return DoubleConstant;
        case int52ConstantTag:
            return Int52Constant;
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    Node* asNode() const
    {
        ASSERT(m_node || !asValue());
        return m_node;
    }

    FrozenValue* asValue() const
    {
        return reinterpret_cast<FrozenValue*>(m_value & pointerMask);
    }

    unsigned hash() const
    {
        void* toHash = m_node;
        if (FrozenValue* value = asValue())
            toHash = value;
        return WTF::PtrHash<void*>::hash(toHash);
    }

    bool operator==(const LazyNode& other) const
    {
        if (asValue() || other.asValue())
            return m_value == other.m_value;
        return m_node == other.m_node;
    }

    bool operator!=(const LazyNode& other) const
    {
        return !(*this == other);
    }

    Node* ensureIsNode(InsertionSet& insertionSet, BasicBlock* block, unsigned nodeIndex)
    {
        if (!m_node)
            m_node = insertionSet.insertConstant(nodeIndex, block->at(nodeIndex)->origin, asValue(), op());

        return asNode();
    }

    Node* operator->() const { return asNode(); }

    Node& operator*() const { return *asNode(); }

    bool operator!() const { return !asValue() && !asNode(); }

    explicit operator bool() const { return !!*this; }

    void dump(PrintStream& out) const;

private:
    void setFrozenValue(FrozenValue* value, NodeType op)
    {
        ASSERT(value);
        m_value = reinterpret_cast<uintptr_t>(value);
        ASSERT(m_value == (m_value & pointerMask));
        switch (op) {
        case JSConstant:
            m_value |= jsConstantTag;
            break;
        case DoubleConstant:
            m_value |= doubleConstantTag;
            break;
        case Int52Constant:
            m_value |= int52ConstantTag;
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
    }

    Node* m_node;
    uintptr_t m_value;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::LazyNode> : SimpleClassHashTraits<JSC::DFG::LazyNode> {
    static const bool emptyValueIsZero = true;
};

} // namespace WTF

#endif // ENABLE(DFG_JIT)
