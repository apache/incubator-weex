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

