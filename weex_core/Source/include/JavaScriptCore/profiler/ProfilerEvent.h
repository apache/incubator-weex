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
#include <wtf/PrintStream.h>
#include <wtf/text/CString.h>

namespace JSC { namespace Profiler {

class Bytecodes;
class Compilation;

class Event {
public:
    Event()
    {
    }
    
    Event(double time, Bytecodes* bytecodes, Compilation* compilation, const char* summary, const CString& detail)
        : m_time(time)
        , m_bytecodes(bytecodes)
        , m_compilation(compilation)
        , m_summary(summary)
        , m_detail(detail)
    {
    }
    
    explicit operator bool() const
    {
        return m_bytecodes;
    }
    
    double time() const { return m_time; }
    Bytecodes* bytecodes() const { return m_bytecodes; }
    Compilation* compilation() const { return m_compilation; }
    const char* summary() const { return m_summary; }
    const CString& detail() const { return m_detail; }
    
    void dump(PrintStream&) const;
    JSValue toJS(ExecState*) const;
    
private:
    double m_time { 0 };
    Bytecodes* m_bytecodes { nullptr };
    Compilation* m_compilation { nullptr };
    const char* m_summary { nullptr };
    CString m_detail;
};

} } // namespace JSC::Profiler
