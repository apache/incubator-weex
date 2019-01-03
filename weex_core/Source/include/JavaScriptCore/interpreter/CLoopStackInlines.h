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

#if !ENABLE(JIT)

#include "CLoopStack.h"
#include "CallFrame.h"
#include "CodeBlock.h"
#include "VM.h"

namespace JSC {

inline bool CLoopStack::ensureCapacityFor(Register* newTopOfStack)
{
    Register* newEnd = newTopOfStack - 1;
    if (newEnd >= m_end)
        return true;
    return grow(newTopOfStack);
}

inline Register* CLoopStack::topOfFrameFor(CallFrame* frame)
{
    if (UNLIKELY(!frame))
        return baseOfStack();
    return frame->topOfFrame() - 1;
}

inline Register* CLoopStack::topOfStack()
{
    return topOfFrameFor(m_topCallFrame);
}

inline void CLoopStack::shrink(Register* newTopOfStack)
{
    Register* newEnd = newTopOfStack - 1;
    if (newEnd >= m_end)
        return;
    setCLoopStackLimit(newTopOfStack);
    // Note: Clang complains of an unresolved linkage to maxExcessCapacity if
    // invoke std::max() with it as an argument. To work around this, we first
    // assign the constant to a local variable, and use the local instead.
    ptrdiff_t maxExcessCapacity = CLoopStack::maxExcessCapacity;
    ptrdiff_t maxExcessInRegisters = std::max(maxExcessCapacity, m_softReservedZoneSizeInRegisters);
    if (m_end == baseOfStack() && (highAddress() - m_commitTop) >= maxExcessInRegisters)
        releaseExcessCapacity();
}

inline void CLoopStack::setCLoopStackLimit(Register* newTopOfStack)
{
    Register* newEnd = newTopOfStack - 1;
    m_end = newEnd;
    m_vm.setCLoopStackLimit(newTopOfStack);
}

} // namespace JSC

#endif // !ENABLE(JIT)
