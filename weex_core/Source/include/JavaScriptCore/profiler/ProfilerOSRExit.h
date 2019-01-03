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

#include "ExitKind.h"
#include "JSCJSValue.h"
#include "ProfilerOriginStack.h"

namespace JSC { namespace Profiler {

class OSRExit {
public:
    OSRExit(unsigned id, const OriginStack&, ExitKind, bool isWatchpoint);
    ~OSRExit();
    
    unsigned id() const { return m_id; }
    const OriginStack& origin() const { return m_origin; }
    ExitKind exitKind() const { return m_exitKind; }
    bool isWatchpoint() const { return m_isWatchpoint; }
    
    uint64_t* counterAddress() { return &m_counter; }
    uint64_t count() const { return m_counter; }
    
    JSValue toJS(ExecState*) const;

private:
    unsigned m_id;
    OriginStack m_origin;
    ExitKind m_exitKind;
    bool m_isWatchpoint;
    uint64_t m_counter;
};

} } // namespace JSC::Profiler
