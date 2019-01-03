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

#include <wtf/DoublyLinkedList.h>

namespace JSC {

class HandleSet;
class HandleNode;

class HandleBlock : public DoublyLinkedListNode<HandleBlock> {
    friend class WTF::DoublyLinkedListNode<HandleBlock>;
public:
    static HandleBlock* create(HandleSet*);
    static void destroy(HandleBlock*);
    static HandleBlock* blockFor(HandleNode*);

    static const size_t blockSize = 4 * KB;

    HandleSet* handleSet();

    HandleNode* nodes();
    HandleNode* nodeAtIndex(unsigned);
    unsigned nodeCapacity();

private:
    HandleBlock(HandleSet*);

    char* payload();
    char* payloadEnd();

    static const size_t s_blockMask = ~(blockSize - 1);

    HandleBlock* m_prev;
    HandleBlock* m_next;
    HandleSet* m_handleSet;
};

inline HandleBlock* HandleBlock::blockFor(HandleNode* node)
{
    return reinterpret_cast<HandleBlock*>(reinterpret_cast<size_t>(node) & s_blockMask);
}

inline HandleSet* HandleBlock::handleSet()
{
    return m_handleSet;
}

} // namespace JSC
