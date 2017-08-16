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

#pragma once

#include "MarkingConstraint.h"
#include <wtf/BitVector.h>
#include <wtf/Vector.h>

namespace JSC {

class MarkingConstraintSet {
public:
    MarkingConstraintSet();
    ~MarkingConstraintSet();
    
    void didStartMarking();
    
    void add(
        CString abbreviatedName,
        CString name,
        ::Function<void(SlotVisitor&, const VisitingTimeout&)>,
        ConstraintVolatility);
    
    void add(
        CString abbreviatedName,
        CString name,
        ::Function<void(SlotVisitor&, const VisitingTimeout&)>,
        ::Function<double(SlotVisitor&)>,
        ConstraintVolatility);
    
    void add(std::unique_ptr<MarkingConstraint>);
    
    // Assuming that the mark stacks are all empty, this will give you a guess as to whether or
    // not the wavefront is advancing.
    bool isWavefrontAdvancing(SlotVisitor&);
    bool isWavefrontRetreating(SlotVisitor& visitor) { return !isWavefrontAdvancing(visitor); }
    
    // Returns true if this executed all constraints and none of them produced new work. This
    // assumes that you've alraedy visited roots and drained from there.
    bool executeConvergence(
        SlotVisitor&,
        MonotonicTime timeout = MonotonicTime::infinity());
    
    // Simply runs all constraints without any shenanigans.
    void executeAll(SlotVisitor&);
    
private:
    class ExecutionContext;
    friend class ExecutionContext;
    
    bool executeConvergenceImpl(SlotVisitor&, MonotonicTime timeout);
    
    bool drain(SlotVisitor&, MonotonicTime, BitVector& unexecuted, BitVector& executed, bool& didVisitSomething);
    
    BitVector m_unexecutedRoots;
    BitVector m_unexecutedOutgrowths;
    Vector<std::unique_ptr<MarkingConstraint>> m_set;
    Vector<MarkingConstraint*> m_ordered;
    Vector<MarkingConstraint*> m_outgrowths;
    unsigned m_iteration { 1 };
};

} // namespace JSC

