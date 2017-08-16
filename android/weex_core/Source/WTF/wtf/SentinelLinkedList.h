/*
 * Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
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

//    A SentinelLinkedList is a linked list with dummy head and tail sentinels,
//    which allow for branch-less insertion and removal, and removal without a
//    pointer to the list.
//    
//    Requires: Node is a concrete class with:
//        Node(SentinelTag);
//        void setPrev(Node*);
//        Node* prev();
//        void setNext(Node*);
//        Node* next();

#ifndef SentinelLinkedList_h
#define SentinelLinkedList_h

namespace WTF {

enum SentinelTag { Sentinel };

template<typename T>
class BasicRawSentinelNode {
    WTF_MAKE_FAST_ALLOCATED;
public:
    BasicRawSentinelNode(SentinelTag)
        : m_next(0)
        , m_prev(0)
    {
    }
    
    BasicRawSentinelNode()
        : m_next(0)
        , m_prev(0)
    {
    }
    
    void setPrev(BasicRawSentinelNode* prev) { m_prev = prev; }
    void setNext(BasicRawSentinelNode* next) { m_next = next; }
    
    T* prev() { return static_cast<T*>(m_prev); }
    T* next() { return static_cast<T*>(m_next); }
    
    bool isOnList() const
    {
        ASSERT(!!m_prev == !!m_next);
        return !!m_prev;
    }
    
    void remove();

    void prepend(BasicRawSentinelNode*);
    void append(BasicRawSentinelNode*);
    
private:
    BasicRawSentinelNode* m_next;
    BasicRawSentinelNode* m_prev;
};

template <typename T, typename RawNode = T> class SentinelLinkedList {
public:
    typedef T* iterator;

    SentinelLinkedList();

    // Pushes to the front of the list. It's totally backwards from what you'd expect.
    void push(T*);

    // Appends to the end of the list.
    void append(T*);
    
    static void remove(T*);
    static void prepend(T* existingNode, T* newNode);
    static void append(T* existingNode, T* newNode);
    
    bool isOnList(T*);

    iterator begin();
    iterator end();
    
    bool isEmpty() { return begin() == end(); }
    
    template<typename Func>
    void forEach(const Func& func)
    {
        for (iterator iter = begin(); iter != end();) {
            iterator next = iter->next();
            func(iter);
            iter = next;
        }
    }
    
    void takeFrom(SentinelLinkedList<T, RawNode>&);
    
private:
    RawNode m_headSentinel;
    RawNode m_tailSentinel;
};

template <typename T> void BasicRawSentinelNode<T>::remove()
{
    SentinelLinkedList<T, BasicRawSentinelNode<T>>::remove(static_cast<T*>(this));
}

template <typename T> void BasicRawSentinelNode<T>::prepend(BasicRawSentinelNode* node)
{
    SentinelLinkedList<T, BasicRawSentinelNode<T>>::prepend(
        static_cast<T*>(this), static_cast<T*>(node));
}

template <typename T> void BasicRawSentinelNode<T>::append(BasicRawSentinelNode* node)
{
    SentinelLinkedList<T, BasicRawSentinelNode<T>>::append(
        static_cast<T*>(this), static_cast<T*>(node));
}

template <typename T, typename RawNode> inline SentinelLinkedList<T, RawNode>::SentinelLinkedList()
    : m_headSentinel(Sentinel)
    , m_tailSentinel(Sentinel)
{
    m_headSentinel.setNext(&m_tailSentinel);
    m_headSentinel.setPrev(0);

    m_tailSentinel.setPrev(&m_headSentinel);
    m_tailSentinel.setNext(0);
}

template <typename T, typename RawNode> inline typename SentinelLinkedList<T, RawNode>::iterator SentinelLinkedList<T, RawNode>::begin()
{
    return static_cast<T*>(m_headSentinel.next());
}

template <typename T, typename RawNode> inline typename SentinelLinkedList<T, RawNode>::iterator SentinelLinkedList<T, RawNode>::end()
{
    return static_cast<T*>(&m_tailSentinel);
}

template <typename T, typename RawNode> inline void SentinelLinkedList<T, RawNode>::push(T* node)
{
    ASSERT(node);
    ASSERT(!node->prev());
    ASSERT(!node->next());
    
    RawNode* prev = &m_headSentinel;
    RawNode* next = m_headSentinel.next();

    node->setPrev(prev);
    node->setNext(next);

    prev->setNext(node);
    next->setPrev(node);
}

template <typename T, typename RawNode> inline void SentinelLinkedList<T, RawNode>::append(T* node)
{
    ASSERT(node);
    ASSERT(!node->prev());
    ASSERT(!node->next());
    
    RawNode* prev = m_tailSentinel.prev();
    RawNode* next = &m_tailSentinel;

    node->setPrev(prev);
    node->setNext(next);

    prev->setNext(node);
    next->setPrev(node);
}

template <typename T, typename RawNode> inline void SentinelLinkedList<T, RawNode>::remove(T* node)
{
    ASSERT(node);
    ASSERT(!!node->prev());
    ASSERT(!!node->next());
    
    RawNode* prev = node->prev();
    RawNode* next = node->next();

    prev->setNext(next);
    next->setPrev(prev);
    
    node->setPrev(0);
    node->setNext(0);
}

template <typename T, typename RawNode>
inline void SentinelLinkedList<T, RawNode>::prepend(T* existingNode, T* newNode)
{
    ASSERT(existingNode);
    ASSERT(!!existingNode->prev());
    ASSERT(!!existingNode->next());
    ASSERT(newNode);
    ASSERT(!newNode->prev());
    ASSERT(!newNode->next());

    RawNode* prev = existingNode->prev();

    newNode->setNext(existingNode);
    newNode->setPrev(prev);
    
    prev->setNext(newNode);
    existingNode->setPrev(newNode);
}

template <typename T, typename RawNode>
inline void SentinelLinkedList<T, RawNode>::append(T* existingNode, T* newNode)
{
    ASSERT(existingNode);
    ASSERT(!!existingNode->prev());
    ASSERT(!!existingNode->next());
    ASSERT(newNode);
    ASSERT(!newNode->prev());
    ASSERT(!newNode->next());

    RawNode* next = existingNode->next();

    newNode->setNext(next);
    newNode->setPrev(existingNode);
    
    next->setPrev(newNode);
    existingNode->setNext(newNode);
}

template <typename T, typename RawNode> inline bool SentinelLinkedList<T, RawNode>::isOnList(T* node)
{
    if (!node->isOnList())
        return false;
    
    for (T* iter = begin(); iter != end(); iter = iter->next()) {
        if (iter == node)
            return true;
    }
    
    return false;
}

template <typename T, typename RawNode>
inline void SentinelLinkedList<T, RawNode>::takeFrom(SentinelLinkedList<T, RawNode>& other)
{
    if (other.isEmpty())
        return;
    
    m_tailSentinel.prev()->setNext(other.m_headSentinel.next());
    other.m_headSentinel.next()->setPrev(m_tailSentinel.prev());
    
    m_tailSentinel.setPrev(other.m_tailSentinel.prev());
    m_tailSentinel.prev()->setNext(&m_tailSentinel);

    other.m_headSentinel.setNext(&other.m_tailSentinel);
    other.m_tailSentinel.setPrev(&other.m_headSentinel);
}

}

using WTF::BasicRawSentinelNode;
using WTF::SentinelLinkedList;

#endif

