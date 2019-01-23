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

#include "JSCJSValue.h"
#include "ProfilerOriginStack.h"
#include <wtf/text/CString.h>

namespace JSC { namespace Profiler {

class CompiledBytecode {
public:
    // It's valid to have an empty OriginStack, which indicates that this is some
    // sort of non-bytecode-related machine code.
    CompiledBytecode(const OriginStack&, const CString& description);
    ~CompiledBytecode();
    
    const OriginStack& originStack() const { return m_origin; }
    const CString& description() const { return m_description; }
    
    JSValue toJS(ExecState*) const;
    
private:
    OriginStack m_origin;
    CString m_description;
};

} } // namespace JSC::Profiler
