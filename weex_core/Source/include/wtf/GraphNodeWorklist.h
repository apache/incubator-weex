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
#ifndef GraphNodeWorklist_h
#define GraphNodeWorklist_h

#include <wtf/HashSet.h>

namespace WTF {

template<typename Node, typename Set = HashSet<Node>>
class GraphNodeWorklist {
public:
    GraphNodeWorklist() { }
    ~GraphNodeWorklist() { }

    // Returns true if we didn't know about the node before.
    bool push(Node node)
    {
        if (!m_seen.add(node))
            return false;
        m_stack.append(node);
        return true;
    }

    template<typename Iterable>
    void pushAll(const Iterable& iterable)
    {
        for (Node node : iterable)
            push(node);
    }

    bool isEmpty() const { return m_stack.isEmpty(); }
    bool notEmpty() const { return !m_stack.isEmpty(); }
    
    Node pop()
    {
        if (m_stack.isEmpty())
            return Node();
        return m_stack.takeLast();
    }

    bool saw(Node node) { return m_seen.contains(node); }
    
    const Set& seen() const { return m_seen; }

private:
    Set m_seen;
    Vector<Node, 16> m_stack;
};

template<typename Node, typename T>
struct GraphNodeWith {
    GraphNodeWith()
        : node()
        , data()
    {
    }

    GraphNodeWith(Node node, const T& data)
        : node(node)
        , data(data)
    {
    }

    explicit operator bool() const { return !!node; }
    
    Node node;
    T data;
};

template<typename Node, typename T, typename Set = HashSet<Node>>
class ExtendedGraphNodeWorklist {
public:
    ExtendedGraphNodeWorklist() { }

    void forcePush(const GraphNodeWith<Node, T>& entry)
    {
        m_stack.append(entry);
    }
    
    void forcePush(Node node, const T& data)
    {
        forcePush(GraphNodeWith<Node, T>(node, data));
    }
    
    bool push(const GraphNodeWith<Node, T>& entry)
    {
        if (!m_seen.add(entry.node))
            return false;
        
        forcePush(entry);
        return true;
    }
    
    bool push(Node node, const T& data)
    {
        return push(GraphNodeWith<Node, T>(node, data));
    }
    
    bool notEmpty() const { return !m_stack.isEmpty(); }
    
    GraphNodeWith<Node, T> pop()
    {
        if (m_stack.isEmpty())
            return GraphNodeWith<Node, T>();
        
        return m_stack.takeLast();
    }

private:
    Set m_seen;
    Vector<GraphNodeWith<Node, T>> m_stack;
};

enum class GraphVisitOrder : uint8_t {
    Pre,
    Post
};

template<typename Node>
struct GraphNodeWithOrder {
    GraphNodeWithOrder()
        : node()
        , order(GraphVisitOrder::Pre)
    {
    }
    
    GraphNodeWithOrder(Node node, GraphVisitOrder order)
        : node(node)
        , order(order)
    {
    }
    
    explicit operator bool() const { return node; }

    Node node;
    GraphVisitOrder order;
};

template<typename Node, typename Set = HashSet<Node>>
class PostOrderGraphNodeWorklist {
public:
    PostOrderGraphNodeWorklist()
    {
    }
    
    ~PostOrderGraphNodeWorklist()
    {
    }
    
    bool pushPre(Node node)
    {
        return m_worklist.push(node, GraphVisitOrder::Pre);
    }
    
    void pushPost(Node node)
    {
        m_worklist.forcePush(node, GraphVisitOrder::Post);
    }
    
    bool push(Node node, GraphVisitOrder order = GraphVisitOrder::Pre)
    {
        switch (order) {
        case GraphVisitOrder::Pre:
            return pushPre(node);
        case GraphVisitOrder::Post:
            pushPost(node);
            return true;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
    bool push(const GraphNodeWithOrder<Node>& data)
    {
        return push(data.node, data.order);
    }
    
    bool notEmpty() const { return m_worklist.notEmpty(); }
    
    GraphNodeWithOrder<Node> pop()
    {
        GraphNodeWith<Node, GraphVisitOrder> result = m_worklist.pop();
        return GraphNodeWithOrder<Node>(result.node, result.data);
    }

private:
    ExtendedGraphNodeWorklist<Node, GraphVisitOrder, Set> m_worklist;
};

} // namespace WTF

using WTF::GraphNodeWorklist;
using WTF::GraphNodeWith;
using WTF::ExtendedGraphNodeWorklist;
using WTF::GraphVisitOrder;
using WTF::GraphNodeWithOrder;
using WTF::PostOrderGraphNodeWorklist;

#endif // GraphNodeWorklist_h

