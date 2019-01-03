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
#ifndef WTF_OrderMaker_h
#define WTF_OrderMaker_h

#include <wtf/Bag.h>
#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>
#include <wtf/SentinelLinkedList.h>

namespace WTF {

// This is a collection that is meant to be used for building up lists in a certain order. It's
// not an efficient data structure for storing lists, but if you need to build a list by doing
// operations like insertBefore(existingValue, newValue), then this class is a good intermediate
// helper. Note that the type it operates on must be usable as a HashMap key.
template<typename T>
class OrderMaker {
    WTF_MAKE_NONCOPYABLE(OrderMaker);
    
    struct Node : BasicRawSentinelNode<Node> {
        Node(SentinelTag)
        {
        }

        Node()
        {
        }

        T payload;
    };
    
public:
    OrderMaker()
    {
    }

    void prepend(T value)
    {
        m_list.push(newNode(value));
    }

    void append(T value)
    {
        m_list.append(newNode(value));
    }

    void insertBefore(T existingValue, T newValue)
    {
        Node* node = m_map.get(existingValue);
        ASSERT(node);
        node->prepend(newNode(newValue));
    }
    
    void insertAfter(T existingValue, T newValue)
    {
        Node* node = m_map.get(existingValue);
        ASSERT(node);
        node->append(newNode(newValue));
    }

    class iterator {
    public:
        iterator()
        {
        }

        iterator(Node* node)
            : m_node(node)
        {
        }

        const T& operator*()
        {
            return m_node->payload;
        }

        iterator& operator++()
        {
            m_node = m_node->next();
            return *this;
        }

        bool operator==(const iterator& other) const
        {
            return m_node == other.m_node;
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }
        
    private:
        Node* m_node { nullptr };
    };

    iterator begin() const { return iterator(const_cast<SentinelLinkedList<Node>&>(m_list).begin()); }
    iterator end() const { return iterator(const_cast<SentinelLinkedList<Node>&>(m_list).end()); }
    
private:
    Node* newNode(T value)
    {
        Node* result = m_nodes.add();
        result->payload = value;
        m_map.set(value, result);
        return result;
    }
    
    HashMap<T, Node*> m_map;
    Bag<Node> m_nodes; // FIXME: We could just manually free the contents of the linked list.
    SentinelLinkedList<Node> m_list;
};

} // namespace WTF

using WTF::OrderMaker;

#endif // WTF_OrderMaker_h

