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
#include "ProfilerCompilationKind.h"
#include "ProfilerCompiledBytecode.h"
#include "ProfilerExecutionCounter.h"
#include "ProfilerJettisonReason.h"
#include "ProfilerOSRExit.h"
#include "ProfilerOSRExitSite.h"
#include "ProfilerOriginStack.h"
#include "ProfilerProfiledBytecodes.h"
#include "ProfilerUID.h"
#include <wtf/RefCounted.h>
#include <wtf/SegmentedVector.h>

namespace JSC {

class FireDetail;

namespace Profiler {

class Bytecodes;
class Database;

// Represents the act of executing some bytecodes in some engine, and does
// all of the counting for those executions.

class Compilation : public RefCounted<Compilation> {
public:
    Compilation(Bytecodes*, CompilationKind);
    ~Compilation();
    
    void addProfiledBytecodes(Database&, CodeBlock*);
    unsigned profiledBytecodesSize() const { return m_profiledBytecodes.size(); }
    const ProfiledBytecodes& profiledBytecodesAt(unsigned i) const { return m_profiledBytecodes[i]; }
    
    void noticeInlinedGetById() { m_numInlinedGetByIds++; }
    void noticeInlinedPutById() { m_numInlinedPutByIds++; }
    void noticeInlinedCall() { m_numInlinedCalls++; }
    
    Bytecodes* bytecodes() const { return m_bytecodes; }
    CompilationKind kind() const { return m_kind; }
    
    void addDescription(const CompiledBytecode&);
    void addDescription(const OriginStack&, const CString& description);
    ExecutionCounter* executionCounterFor(const OriginStack&);
    void addOSRExitSite(const Vector<const void*>& codeAddresses);
    OSRExit* addOSRExit(unsigned id, const OriginStack&, ExitKind, bool isWatchpoint);
    
    void setJettisonReason(JettisonReason, const FireDetail*);
    
    UID uid() const { return m_uid; }
    
    void dump(PrintStream&) const;
    JSValue toJS(ExecState*) const;
    
private:
    Bytecodes* m_bytecodes;
    CompilationKind m_kind;
    JettisonReason m_jettisonReason;
    CString m_additionalJettisonReason;
    Vector<ProfiledBytecodes> m_profiledBytecodes;
    Vector<CompiledBytecode> m_descriptions;
    HashMap<OriginStack, std::unique_ptr<ExecutionCounter>> m_counters;
    Vector<OSRExitSite> m_osrExitSites;
    SegmentedVector<OSRExit> m_osrExits;
    unsigned m_numInlinedGetByIds;
    unsigned m_numInlinedPutByIds;
    unsigned m_numInlinedCalls;
    UID m_uid;
};

} } // namespace JSC::Profiler
