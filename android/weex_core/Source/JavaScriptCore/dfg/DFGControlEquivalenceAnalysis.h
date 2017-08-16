/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
