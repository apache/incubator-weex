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

#if ENABLE(DFG_JIT)

#include "DFGBackwardsDominators.h"
#include "DFGDominators.h"

namespace JSC { namespace DFG {

class ControlEquivalenceAnalysis {
    WTF_MAKE_NONCOPYABLE(ControlEquivalenceAnalysis);
    WTF_MAKE_FAST_ALLOCATED;
public:
    ControlEquivalenceAnalysis(Graph& graph)
        : m_dominators(graph.ensureDominators())
        , m_backwardsDominators(graph.ensureBackwardsDominators())
    {
    }
    
    // This returns true iff:
    //
    // - If b executes then a must have executed before it (a dominates b).
    // - If a executes then b will execute after it (b backwards-dominates a).
    //
    // Note that like Dominators and BackwardsDominators, this analysis ignores OSR:
    //
    // - This may return true even if we OSR enter in beteen a and b. OSR entry would mean that b
    //   could execute even if a had not executed. This is impossible in DFG SSA but it's possible
    //   in DFG CPS.
    // - This may return true even if we OSR exit in between a and b. OSR exit would mean that a
    //   could execute even though b will not execute. This is possible in all forms of DFG IR.
    //
    // In DFG SSA you only have to worry about the definition being weaked by exits. This is usually
    // OK, since we use this analysis to determine the cost of moving exits from one block to
    // another. If we move an exit from b to a and a equivalently dominates b then at worst we have
    // made the exit happen sooner. If we move an exit from b to a and a dominates b but not
    // equivalently then we've done something much worse: the program may now exit even if it would
    // not have ever exited before.
    bool dominatesEquivalently(BasicBlock* a, BasicBlock* b)
    {
        return m_dominators.dominates(a, b)
            && m_backwardsDominators.dominates(b, a);
    }
    
    // This returns true iff the execution of a implies that b also executes and vice-versa.
    bool areEquivalent(BasicBlock* a, BasicBlock* b)
    {
        return dominatesEquivalently(a, b)
            || dominatesEquivalently(b, a);
    }

private:
    Dominators& m_dominators;
    BackwardsDominators& m_backwardsDominators;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
