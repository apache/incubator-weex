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

#include "HandleBlock.h"
#include <wtf/FastMalloc.h>

namespace JSC {

inline HandleBlock* HandleBlock::create(HandleSet* handleSet)
{
    return new (NotNull, fastAlignedMalloc(blockSize, blockSize)) HandleBlock(handleSet);
}

inline void HandleBlock::destroy(HandleBlock* block)
{
    block->~HandleBlock();
    fastAlignedFree(block);
}

inline HandleBlock::HandleBlock(HandleSet* handleSet)
    : DoublyLinkedListNode<HandleBlock>()
    , m_handleSet(handleSet)
{
}

inline char* HandleBlock::payloadEnd()
{
    return reinterpret_cast<char*>(this) + blockSize;
}

inline char* HandleBlock::payload()
{
    return reinterpret_cast<char*>(this) + WTF::roundUpToMultipleOf<sizeof(double)>(sizeof(HandleBlock));
}

inline HandleNode* HandleBlock::nodes()
{
    return reinterpret_cast_ptr<HandleNode*>(payload());
}

inline HandleNode* HandleBlock::nodeAtIndex(unsigned i)
{
    ASSERT(i < nodeCapacity());
    return &nodes()[i];
}

inline unsigned HandleBlock::nodeCapacity()
{
    return (payloadEnd() - payload()) / sizeof(HandleNode);
}

} // namespace JSC
