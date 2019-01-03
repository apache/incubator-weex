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
#include "DFGMinifiedID.h"
#include "DFGNodeType.h"

namespace JSC { namespace DFG {

struct Node;

inline bool belongsInMinifiedGraph(NodeType type)
{
    switch (type) {
    case JSConstant:
    case Int52Constant:
    case DoubleConstant:
    case PhantomDirectArguments:
    case PhantomClonedArguments:
        return true;
    default:
        return false;
    }
}

class MinifiedNode {
public:
    MinifiedNode() { }
    
    static MinifiedNode fromNode(Node*);
    
    MinifiedID id() const { return m_id; }
    NodeType op() const { return m_op; }
    
    bool hasConstant() const { return hasConstant(m_op); }
    
    JSValue constant() const
    {
        return JSValue::decode(bitwise_cast<EncodedJSValue>(m_info));
    }
    
    bool hasInlineCallFrame() const { return hasInlineCallFrame(m_op); }
    
    InlineCallFrame* inlineCallFrame() const
    {
        return bitwise_cast<InlineCallFrame*>(static_cast<uintptr_t>(m_info));
    }
    
    static MinifiedID getID(MinifiedNode* node) { return node->id(); }
    static bool compareByNodeIndex(const MinifiedNode& a, const MinifiedNode& b)
    {
        return a.m_id < b.m_id;
    }
    
private:
    static bool hasConstant(NodeType type)
    {
        return type == JSConstant || type == Int52Constant || type == DoubleConstant;
    }
    
    static bool hasInlineCallFrame(NodeType type)
    {
        return type == PhantomDirectArguments || type == PhantomClonedArguments;
    }
    
    MinifiedID m_id;
    uint64_t m_info;
    NodeType m_op;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
