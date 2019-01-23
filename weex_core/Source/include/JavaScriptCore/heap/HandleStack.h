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

#include <wtf/Assertions.h>
#include <wtf/BlockStack.h>
#include "Handle.h"

namespace JSC {

class LocalScope;
class SlotVisitor;

class HandleStack {
public:
    class Frame {
    public:
        HandleSlot m_next;
        HandleSlot m_end;
    };

    HandleStack();
    
    void enterScope(Frame&);
    void leaveScope(Frame&);

    HandleSlot push();

    void visit(SlotVisitor&);

private:
    JS_EXPORT_PRIVATE void grow();
    void zapTo(Frame&);
    HandleSlot findFirstAfter(HandleSlot);

#ifndef NDEBUG
    size_t m_scopeDepth;
#endif
    BlockStack<JSValue> m_blockStack;
    Frame m_frame;
};

inline void HandleStack::enterScope(Frame& lastFrame)
{
#ifndef NDEBUG
    ++m_scopeDepth;
#endif

    lastFrame = m_frame;
}



inline void HandleStack::zapTo(Frame& lastFrame)
{
#ifdef NDEBUG
    UNUSED_PARAM(lastFrame);
#else
    const Vector<HandleSlot>& blocks = m_blockStack.blocks();
    
    if (lastFrame.m_end != m_frame.m_end) { // Zapping to a frame in a different block.
        int i = blocks.size() - 1;
        for ( ; blocks[i] + m_blockStack.blockLength != lastFrame.m_end; --i) {
            for (int j = m_blockStack.blockLength - 1; j >= 0; --j)
                blocks[i][j] = JSValue();
        }
        
        for (HandleSlot it = blocks[i] + m_blockStack.blockLength - 1; it != lastFrame.m_next - 1; --it)
            *it = JSValue();
        
        return;
    }
    
    for (HandleSlot it = m_frame.m_next - 1; it != lastFrame.m_next - 1; --it)
        *it = JSValue();
#endif
}

inline void HandleStack::leaveScope(Frame& lastFrame)
{
#ifndef NDEBUG
    --m_scopeDepth;
#endif

    zapTo(lastFrame);

    if (lastFrame.m_end != m_frame.m_end) // Popping to a frame in a different block.
        m_blockStack.shrink(lastFrame.m_end);

    m_frame = lastFrame;
}

inline HandleSlot HandleStack::push()
{
    ASSERT(m_scopeDepth); // Creating a Local outside of a LocalScope is a memory leak.
    if (m_frame.m_next == m_frame.m_end)
        grow();
    return m_frame.m_next++;
}

} // namespace JSC
