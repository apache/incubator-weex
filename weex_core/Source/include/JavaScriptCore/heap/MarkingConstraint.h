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

#include "ConstraintVolatility.h"
#include "VisitingTimeout.h"
#include <wtf/FastMalloc.h>
#include <wtf/Function.h>
#include <wtf/MonotonicTime.h>
#include <wtf/Noncopyable.h>
#include <wtf/text/CString.h>

namespace JSC {

class MarkingConstraintSet;
class SlotVisitor;

class MarkingConstraint {
    WTF_MAKE_NONCOPYABLE(MarkingConstraint);
    WTF_MAKE_FAST_ALLOCATED;
public:
    JS_EXPORT_PRIVATE MarkingConstraint(
        CString abbreviatedName, CString name,
        ::Function<void(SlotVisitor&, const VisitingTimeout&)>,
        ConstraintVolatility);
    
    JS_EXPORT_PRIVATE MarkingConstraint(
        CString abbreviatedName, CString name,
        ::Function<void(SlotVisitor&, const VisitingTimeout&)>,
        ::Function<double(SlotVisitor&)>,
        ConstraintVolatility);
    
    JS_EXPORT_PRIVATE ~MarkingConstraint();
    
    unsigned index() const { return m_index; }
    
    const char* abbreviatedName() const { return m_abbreviatedName.data(); }
    const char* name() const { return m_name.data(); }
    
    void resetStats();
    
    size_t lastVisitCount() const { return m_lastVisitCount; }
    
    void execute(SlotVisitor&, bool& didVisitSomething, MonotonicTime timeout);
    
    double quickWorkEstimate(SlotVisitor& visitor)
    {
        if (!m_quickWorkEstimateFunction)
            return 0;
        return m_quickWorkEstimateFunction(visitor);
    }
    
    double workEstimate(SlotVisitor& visitor)
    {
        return lastVisitCount() + quickWorkEstimate(visitor);
    }
    
    ConstraintVolatility volatility() const { return m_volatility; }
    
private:
    friend class MarkingConstraintSet; // So it can set m_index.
    
    unsigned m_index { UINT_MAX };
    CString m_abbreviatedName;
    CString m_name;
    ::Function<void(SlotVisitor&, const VisitingTimeout& timeout)> m_executeFunction;
    ::Function<double(SlotVisitor&)> m_quickWorkEstimateFunction;
    ConstraintVolatility m_volatility;
    size_t m_lastVisitCount { 0 };
};

} // namespace JSC

