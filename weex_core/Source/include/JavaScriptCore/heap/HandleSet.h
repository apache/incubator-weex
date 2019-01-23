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

#include "Handle.h"
#include "HandleBlock.h"
#include "HeapCell.h"
#include <wtf/DoublyLinkedList.h>
#include <wtf/HashCountedSet.h>
#include <wtf/SentinelLinkedList.h>
#include <wtf/SinglyLinkedList.h>

namespace JSC {

class HandleSet;
class VM;
class JSValue;
class SlotVisitor;

class HandleNode {
public:
    HandleNode(WTF::SentinelTag);
    HandleNode();
    
    HandleSlot slot();
    HandleSet* handleSet();

    void setPrev(HandleNode*);
    HandleNode* prev();

    void setNext(HandleNode*);
    HandleNode* next();

private:
    JSValue m_value;
    HandleNode* m_prev;
    HandleNode* m_next;
};

class HandleSet {
    friend class HandleBlock;
public:
    static HandleSet* heapFor(HandleSlot);

    HandleSet(VM*);
    ~HandleSet();

    VM* vm();

    HandleSlot allocate();
    void deallocate(HandleSlot);

    void visitStrongHandles(SlotVisitor&);

    JS_EXPORT_PRIVATE void writeBarrier(HandleSlot, const JSValue&);

    unsigned protectedGlobalObjectCount();

    template<typename Functor> void forEachStrongHandle(const Functor&, const HashCountedSet<JSCell*>& skipSet);

private:
    typedef HandleNode Node;
    static HandleSlot toHandle(Node*);
    static Node* toNode(HandleSlot);

    JS_EXPORT_PRIVATE void grow();
    
#if ENABLE(GC_VALIDATION) || !ASSERT_DISABLED
    bool isLiveNode(Node*);
#endif

    VM* m_vm;
    DoublyLinkedList<HandleBlock> m_blockList;

    SentinelLinkedList<Node> m_strongList;
    SentinelLinkedList<Node> m_immediateList;
    SinglyLinkedList<Node> m_freeList;
};

inline HandleSet* HandleSet::heapFor(HandleSlot handle)
{
    return toNode(handle)->handleSet();
}

inline VM* HandleSet::vm()
{
    return m_vm;
}

inline HandleSlot HandleSet::toHandle(HandleSet::Node* node)
{
    return reinterpret_cast<HandleSlot>(node);
}

inline HandleSet::Node* HandleSet::toNode(HandleSlot handle)
{
    return reinterpret_cast<HandleSet::Node*>(handle);
}

inline HandleSlot HandleSet::allocate()
{
    if (m_freeList.isEmpty())
        grow();

    HandleSet::Node* node = m_freeList.pop();
    new (NotNull, node) HandleSet::Node();
    m_immediateList.push(node);
    return toHandle(node);
}

inline void HandleSet::deallocate(HandleSlot handle)
{
    HandleSet::Node* node = toNode(handle);
    SentinelLinkedList<HandleSet::Node>::remove(node);
    m_freeList.push(node);
}

inline HandleNode::HandleNode()
    : m_prev(0)
    , m_next(0)
{
}

inline HandleNode::HandleNode(WTF::SentinelTag)
    : m_prev(0)
    , m_next(0)
{
}

inline HandleSlot HandleNode::slot()
{
    return &m_value;
}

inline HandleSet* HandleNode::handleSet()
{
    return HandleBlock::blockFor(this)->handleSet();
}

inline void HandleNode::setPrev(HandleNode* prev)
{
    m_prev = prev;
}

inline HandleNode* HandleNode::prev()
{
    return m_prev;
}

inline void HandleNode::setNext(HandleNode* next)
{
    m_next = next;
}

inline HandleNode* HandleNode::next()
{
    return m_next;
}

template<typename Functor> void HandleSet::forEachStrongHandle(const Functor& functor, const HashCountedSet<JSCell*>& skipSet)
{
    HandleSet::Node* end = m_strongList.end();
    for (HandleSet::Node* node = m_strongList.begin(); node != end; node = node->next()) {
        JSValue value = *node->slot();
        if (!value || !value.isCell())
            continue;
        if (skipSet.contains(value.asCell()))
            continue;
        functor(value.asCell());
    }
}

} // namespace JSC
