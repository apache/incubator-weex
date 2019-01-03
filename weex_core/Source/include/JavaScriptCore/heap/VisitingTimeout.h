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

#include "SlotVisitor.h"
#include <wtf/MonotonicTime.h>

namespace JSC {

class VisitingTimeout {
public:
    VisitingTimeout()
    {
    }
    
    VisitingTimeout(SlotVisitor& visitor, bool didVisitSomething, MonotonicTime timeout)
        : m_didVisitSomething(didVisitSomething)
        , m_visitCountBefore(visitor.visitCount())
        , m_timeout(timeout)
    {
    }
    
    size_t visitCount(SlotVisitor& visitor) const
    {
        return visitor.visitCount() - m_visitCountBefore;
    }

    bool didVisitSomething(SlotVisitor& visitor) const
    {
        return m_didVisitSomething || visitCount(visitor);
    }
    
    bool shouldTimeOut(SlotVisitor& visitor) const
    {
        return didVisitSomething(visitor) && hasElapsed(m_timeout);
    }
    
private:
    bool m_didVisitSomething { false };
    size_t m_visitCountBefore { 0 };
    MonotonicTime m_timeout;
};

} // namespace JSC

