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

#include "ProfilerDatabase.h"
#include "VM.h"
#include "Watchdog.h"

namespace JSC {
    
bool VM::ensureStackCapacityFor(Register* newTopOfStack)
{
#if ENABLE(JIT)
    ASSERT(wtfThreadData().stack().isGrowingDownward());
    return newTopOfStack >= m_softStackLimit;
#else
    return ensureStackCapacityForCLoop(newTopOfStack);
#endif
    
}

bool VM::isSafeToRecurseSoft() const
{
    bool safe = isSafeToRecurse(m_softStackLimit);
#if !ENABLE(JIT)
    safe = safe && isSafeToRecurseSoftCLoop();
#endif
    return safe;
}

template<typename Func>
void VM::logEvent(CodeBlock* codeBlock, const char* summary, const Func& func)
{
    if (LIKELY(!m_perBytecodeProfiler))
        return;
    
    m_perBytecodeProfiler->logEvent(codeBlock, summary, func());
}

} // namespace JSC
