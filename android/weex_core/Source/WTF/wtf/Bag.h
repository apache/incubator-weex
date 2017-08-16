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

