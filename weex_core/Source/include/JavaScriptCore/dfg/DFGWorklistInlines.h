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

