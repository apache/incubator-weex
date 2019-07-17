/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#pragma once

#include "DFGPlanInlines.h"
#include "DFGWorklist.h"

namespace JSC { namespace DFG {

#if ENABLE(DFG_JIT)

template<typename Func>
void iterateCodeBlocksForGC(VM& vm, const Func& func)
{
    for (unsigned i = DFG::numberOfWorklists(); i--;) {
        if (DFG::Worklist* worklist = DFG::existingWorklistForIndexOrNull(i))
            worklist->iterateCodeBlocksForGC(vm, func);
    }
}

template<typename Func>
void Worklist::iterateCodeBlocksForGC(VM& vm, const Func& func)
{
    LockHolder locker(*m_lock);
    for (PlanMap::iterator iter = m_plans.begin(); iter != m_plans.end(); ++iter) {
        Plan* plan = iter->value.get();
        if (plan->vm != &vm)
            continue;
        plan->iterateCodeBlocksForGC(func);
    }
}

#else // ENABLE(DFG_JIT)

template<typename Func>
void iterateCodeBlocksForGC(VM&, const Func&)
{
}

#endif // ENABLE(DFG_JIT)

} } // namespace JSC::DFG

