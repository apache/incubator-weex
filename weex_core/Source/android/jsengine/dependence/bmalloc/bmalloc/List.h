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
#ifndef List_h
#define List_h

namespace bmalloc {

template<typename T>
struct ListNode {
    ListNode()
        : prev(this)
        , next(this)
    {
    }

    ListNode<T>* prev;
    ListNode<T>* next;
};

template<typename T>
class List {
    static_assert(std::is_trivially_destructible<T>::value, "List must have a trivial destructor.");
public:
    bool isEmpty() { return m_root.next == &m_root; }

    T* head() { return static_cast<T*>(m_root.next); }
    T* tail() { return static_cast<T*>(m_root.prev); }

    void push(T* node)
    {
        ListNode<T>* it = tail();
        insertAfter(it, node);
    }

    T* pop()
    {
        ListNode<T>* result = tail();
        remove(result);
        return static_cast<T*>(result);
    }

    T* popFront()
    {
        ListNode<T>* result = head();
        remove(result);
        return static_cast<T*>(result);
    }

    void insertAfter(ListNode<T>* it, ListNode<T>* node)
    {
        ListNode<T>* prev = it;
        ListNode<T>* next = it->next;

        node->next = next;
        next->prev = node;

        node->prev = prev;
        prev->next = node;
    }

    void remove(ListNode<T>* node)
    {
        ListNode<T>* next = node->next;
        ListNode<T>* prev = node->prev;

        next->prev = prev;
        prev->next = next;
        
        node->prev = node;
        node->next = node;
    }

private:
    ListNode<T> m_root;
};

} // namespace bmalloc

#endif // List_h
