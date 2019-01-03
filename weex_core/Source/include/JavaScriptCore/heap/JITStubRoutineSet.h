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

#include "JITStubRoutine.h"
#include <wtf/FastMalloc.h>
#include <wtf/HashMap.h>
#include <wtf/Vector.h>

namespace JSC {

class GCAwareJITStubRoutine;
class SlotVisitor;

#if ENABLE(JIT)

class JITStubRoutineSet {
    WTF_MAKE_NONCOPYABLE(JITStubRoutineSet);
    WTF_MAKE_FAST_ALLOCATED;
    
public:
    JITStubRoutineSet();
    ~JITStubRoutineSet();
    
    void add(GCAwareJITStubRoutine*);

    void clearMarks();
    
    void mark(void* candidateAddress)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(candidateAddress);
        if (!JITStubRoutine::passesFilter(address))
            return;
        
        markSlow(address);
    }
    
    void deleteUnmarkedJettisonedStubRoutines();
    
    void traceMarkedStubRoutines(SlotVisitor&);
    
    unsigned size() const { return m_listOfRoutines.size(); }
    GCAwareJITStubRoutine* at(unsigned i) const { return m_listOfRoutines[i]; }
    
private:
    void markSlow(uintptr_t address);
    
    HashMap<uintptr_t, GCAwareJITStubRoutine*> m_addressToRoutineMap;
    Vector<GCAwareJITStubRoutine*> m_listOfRoutines;
};

#else // !ENABLE(JIT)

class JITStubRoutineSet {
    WTF_MAKE_NONCOPYABLE(JITStubRoutineSet);
    WTF_MAKE_FAST_ALLOCATED;
    
public:
    JITStubRoutineSet() { }
    ~JITStubRoutineSet() { }

    void add(GCAwareJITStubRoutine*) { }
    void clearMarks() { }
    void mark(void*) { }
    void deleteUnmarkedJettisonedStubRoutines() { }
    void traceMarkedStubRoutines(SlotVisitor&) { }
};

#endif // !ENABLE(JIT)

} // namespace JSC
