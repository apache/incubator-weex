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

#include "CodeBlock.h"
#include "CodeBlockSet.h"

namespace JSC {

inline void CodeBlockSet::mark(const AbstractLocker& locker, void* candidateCodeBlock)
{
    ASSERT(m_lock.isLocked());
    // We have to check for 0 and -1 because those are used by the HashMap as markers.
    uintptr_t value = reinterpret_cast<uintptr_t>(candidateCodeBlock);
    
    // This checks for both of those nasty cases in one go.
    // 0 + 1 = 1
    // -1 + 1 = 0
    if (value + 1 <= 1)
        return;

    CodeBlock* codeBlock = static_cast<CodeBlock*>(candidateCodeBlock); 
    if (!m_oldCodeBlocks.contains(codeBlock) && !m_newCodeBlocks.contains(codeBlock))
        return;

    mark(locker, codeBlock);
}

inline void CodeBlockSet::mark(const AbstractLocker&, CodeBlock* codeBlock)
{
    if (!codeBlock)
        return;

    m_currentlyExecuting.add(codeBlock);
}

template<typename Functor>
void CodeBlockSet::iterate(const Functor& functor)
{
    auto locker = holdLock(m_lock);
    iterate(locker, functor);
}

template<typename Functor>
void CodeBlockSet::iterate(const AbstractLocker&, const Functor& functor)
{
    for (auto& codeBlock : m_oldCodeBlocks) {
        bool done = functor(codeBlock);
        if (done)
            return;
    }
    
    for (auto& codeBlock : m_newCodeBlocks) {
        bool done = functor(codeBlock);
        if (done)
            return;
    }
}

template<typename Functor>
void CodeBlockSet::iterateCurrentlyExecuting(const Functor& functor)
{
    LockHolder locker(&m_lock);
    for (CodeBlock* codeBlock : m_currentlyExecuting)
        functor(codeBlock);
}

} // namespace JSC

