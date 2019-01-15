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
#ifndef Bag_h
#define Bag_h

namespace WTF {

template<typename T>
class Bag {
    WTF_MAKE_NONCOPYABLE(Bag);
    WTF_MAKE_FAST_ALLOCATED;
private:
    class Node {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        template<typename... Args>
        Node(Args&&... args)
            : m_item(std::forward<Args>(args)...)
        {
        }
        
        T m_item;
        Node* m_next;
    };
    
public:
    Bag()
    {
    }

    Bag(Bag<T>&& other)
    {
        ASSERT(!m_head);
        m_head = other.m_head;
        other.m_head = nullptr;
    }

    Bag& operator=(Bag<T>&& other)
    {
        m_head = other.m_head;
        other.m_head = nullptr;
        return *this;
    }
    
    ~Bag()
    {
        clear();
    }
    
    void clear()
    {
        while (m_head) {
            Node* current = m_head;
            m_head = current->m_next;
            delete current;
        }
        m_head = nullptr;
    }
    
    template<typename... Args>
    T* add(Args&&... args)
    {
        Node* newNode = new Node(std::forward<Args>(args)...);
        newNode->m_next = m_head;
        m_head = newNode;
        return &newNode->m_item;
    }
    
    class iterator {
    public:
        iterator()
            : m_node(0)
        {
        }
        
        // This is sort of cheating; it's equivalent to iter == end().
        bool operator!() const { return !m_node; }
        
        T* operator*() const { return &m_node->m_item; }
        
        iterator& operator++()
        {
            m_node = m_node->m_next;
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
        template<typename U> friend class WTF::Bag;
        Node* m_node;
    };
    
    iterator begin()
    {
        iterator result;
        result.m_node = m_head;
        return result;
    }
    
    iterator end() { return iterator(); }
    
    bool isEmpty() const { return !m_head; }
    
private:
    Node* m_head { nullptr };
};

} // namespace WTF

using WTF::Bag;

#endif // Bag_h

