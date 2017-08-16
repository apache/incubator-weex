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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "MarkingConstraintSet.h"

#include "Options.h"
#include <wtf/TimeWithDynamicClockType.h>

namespace JSC {

class MarkingConstraintSet::ExecutionContext {
public:
    ExecutionContext(MarkingConstraintSet& set, SlotVisitor& visitor, MonotonicTime timeout)
        : m_set(set)
        , m_visitor(visitor)
        , m_timeout(timeout)
    {
    }
    
    bool didVisitSomething() const
    {
        return m_didVisitSomething;
    }
    
    bool shouldTimeOut() const
    {
        return didVisitSomething() && hasElapsed(m_timeout);
    }
    
    // Returns false if it times out.
    bool drain(BitVector& unexecuted)
    {
        for (size_t index : unexecuted) {
            execute(index);
            unexecuted.clear(index);
            if (shouldTimeOut())
                return false;
        }
        return true;
    }
    
    bool didExecute(size_t index) const { return m_executed.get(index); }

    void execute(size_t index)
    {
        m_set.m_set[index]->execute(m_visitor, m_didVisitSomething, m_timeout);
        m_executed.set(index);
    }
    
private:
    MarkingConstraintSet& m_set;
    SlotVisitor& m_visitor;
    MonotonicTime m_timeout;
    BitVector m_executed;
    bool m_didVisitSomething { false };
};

MarkingConstraintSet::MarkingConstraintSet()
{
}

MarkingConstraintSet::~MarkingConstraintSet()
{
}

void MarkingConstraintSet::didStartMarking()
{
    m_unexecutedRoots.clearAll();
    m_unexecutedOutgrowths.clearAll();
    for (auto& constraint : m_set) {
        constraint->resetStats();
        switch (constraint->volatility()) {
        case ConstraintVolatility::GreyedByExecution:
            m_unexecutedRoots.set(constraint->index());
            break;
        case ConstraintVolatility::GreyedByMarking:
            m_unexecutedOutgrowths.set(constraint->index());
            break;
        case ConstraintVolatility::SeldomGreyed:
            break;
        }
    }
    m_iteration = 1;
}

void MarkingConstraintSet::add(CString abbreviatedName, CString name, Function<void(SlotVisitor&, const VisitingTimeout&)> function, ConstraintVolatility volatility)
{
    add(std::make_unique<MarkingConstraint>(WTFMove(abbreviatedName), WTFMove(name), WTFMove(function), volatility));
}

void MarkingConstraintSet::add(
    CString abbreviatedName, CString name,
    Function<void(SlotVisitor&, const VisitingTimeout&)> executeFunction,
    Function<double(SlotVisitor&)> quickWorkEstimateFunction,
    ConstraintVolatility volatility)
{
    add(std::make_unique<MarkingConstraint>(WTFMove(abbreviatedName), WTFMove(name), WTFMove(executeFunction), WTFMove(quickWorkEstimateFunction), volatility));
}

void MarkingConstraintSet::add(
    std::unique_ptr<MarkingConstraint> constraint)
{
    constraint->m_index = m_set.size();
    m_ordered.append(constraint.get());
    if (constraint->volatility() == ConstraintVolatility::GreyedByMarking)
        m_outgrowths.append(constraint.get());
    m_set.append(WTFMove(constraint));
}

bool MarkingConstraintSet::executeConvergence(SlotVisitor& visitor, MonotonicTime timeout)
{
    bool result = executeConvergenceImpl(visitor, timeout);
    if (Options::logGC())
        dataLog(" ");
    return result;
}

bool MarkingConstraintSet::isWavefrontAdvancing(SlotVisitor& visitor)
{
    for (MarkingConstraint* outgrowth : m_outgrowths) {
        if (outgrowth->workEstimate(visitor))
            return true;
    }
    return false;
}

bool MarkingConstraintSet::executeConvergenceImpl(SlotVisitor& visitor, MonotonicTime timeout)
{
    ExecutionContext executionContext(*this, visitor, timeout);
    
    unsigned iteration = m_iteration++;
    
    if (Options::logGC())
        dataLog("i#", iteration, ":");

    // If there are any constraints that we have not executed at all during this cycle, then
    // we should execute those now.
    if (!executionContext.drain(m_unexecutedRoots))
        return false;
    
    // First iteration is before any visitor draining, so it's unlikely to trigger any constraints other
    // than roots.
    if (iteration == 1)
        return false;
    
    if (!executionContext.drain(m_unexecutedOutgrowths))
        return false;
    
    // We want to keep preferring the outgrowth constraints - the ones that need to be fixpointed
    // even in a stop-the-world GC - until they stop producing. They have a tendency to go totally
    // silent at some point during GC, at which point it makes sense not to run them again until
    // the end. Outgrowths producing new information corresponds almost exactly to the wavefront
    // advancing: it usually means that we are marking objects that should be marked based on
    // other objects that we would have marked anyway. Once the wavefront is no longer advancing,
    // we want to run mostly the root constraints (based on their predictions of how much work
    // they will have) because at this point we are just trying to outpace the retreating
    // wavefront.
    //
    // Note that this function (executeConvergenceImpl) only returns true if it runs all
    // constraints. So, all we are controlling are the heuristics that say which constraints to
    // run first. Choosing the constraints that are the most likely to produce means running fewer
    // constraints before returning.
    bool isWavefrontAdvancing = this->isWavefrontAdvancing(visitor);
    
    std::sort(
        m_ordered.begin(), m_ordered.end(),
        [&] (MarkingConstraint* a, MarkingConstraint* b) -> bool {
            // Remember: return true if a should come before b.
            
            auto volatilityScore = [] (MarkingConstraint* constraint) -> unsigned {
                return constraint->volatility() == ConstraintVolatility::GreyedByMarking ? 1 : 0;
            };
            
            unsigned aVolatilityScore = volatilityScore(a);
            unsigned bVolatilityScore = volatilityScore(b);
            
            if (aVolatilityScore != bVolatilityScore) {
                if (isWavefrontAdvancing)
                    return aVolatilityScore > bVolatilityScore;
                else
                    return aVolatilityScore < bVolatilityScore;
            }
            
            double aWorkEstimate = a->workEstimate(visitor);
            double bWorkEstimate = b->workEstimate(visitor);
            
            if (aWorkEstimate != bWorkEstimate)
                return aWorkEstimate > bWorkEstimate;
            
            // This causes us to use SeldomGreyed vs GreyedByExecution as a final tie-breaker.
            return a->volatility() > b->volatility();
        });
    
    for (MarkingConstraint* constraint : m_ordered) {
        size_t i = constraint->index();
        
        if (executionContext.didExecute(i))
            continue;
        executionContext.execute(i);
        
        // Once we're in convergence, it makes the most sense to let some marking happen anytime
        // we find work.
        // FIXME: Maybe this should execute all constraints until timeout? Not clear if that's
        // better or worse. Maybe even better is this:
        // - If the visitor is empty, keep running.
        // - If the visitor is has at least N things, return.
        // - Else run until timeout.
        // https://bugs.webkit.org/show_bug.cgi?id=166832
        if (executionContext.didVisitSomething())
            return false;
    }
    
    return true;
}

void MarkingConstraintSet::executeAll(SlotVisitor& visitor)
{
    bool didVisitSomething = false;
    for (auto& constraint : m_set)
        constraint->execute(visitor, didVisitSomething, MonotonicTime::infinity());
    if (Options::logGC())
        dataLog(" ");
}

} // namespace JSC

