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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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

