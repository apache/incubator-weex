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
