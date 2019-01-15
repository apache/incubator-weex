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

#include "DFGPlan.h"
#include "DFGThreadData.h"
#include <wtf/AutomaticThread.h>
#include <wtf/Condition.h>
#include <wtf/Deque.h>
#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>

namespace JSC {

class SlotVisitor;

namespace DFG {

#if ENABLE(DFG_JIT)

class Worklist : public RefCounted<Worklist> {
public:
    enum State { NotKnown, Compiling, Compiled };

    ~Worklist();
    
    static Ref<Worklist> create(CString worklistName, unsigned numberOfThreads, int relativePriority = 0);
    
    void enqueue(Ref<Plan>&&);
    
    // This is equivalent to:
    // worklist->waitUntilAllPlansForVMAreReady(vm);
    // worklist->completeAllReadyPlansForVM(vm);
    void completeAllPlansForVM(VM&);

    template<typename Func>
    void iterateCodeBlocksForGC(VM&, const Func&);

    void waitUntilAllPlansForVMAreReady(VM&);
    State completeAllReadyPlansForVM(VM&, CompilationKey = CompilationKey());
    void removeAllReadyPlansForVM(VM&);
    
    State compilationState(CompilationKey);
    
    size_t queueLength();
    
    void suspendAllThreads();
    void resumeAllThreads();
    
    bool isActiveForVM(VM&) const;
    
    // Only called on the main thread after suspending all threads.
    void visitWeakReferences(SlotVisitor&);
    void removeDeadPlans(VM&);
    
    void removeNonCompilingPlansForVM(VM&);
    
    void dump(PrintStream&) const;
    
private:
    Worklist(CString worklistName);
    void finishCreation(unsigned numberOfThreads, int);
    
    class ThreadBody;
    friend class ThreadBody;
    
    void runThread(ThreadData*);
    static void threadFunction(void* argument);
    
    void removeAllReadyPlansForVM(VM&, Vector<RefPtr<Plan>, 8>&);

    void dump(const AbstractLocker&, PrintStream&) const;
    
    CString m_threadName;
    
    // Used to inform the thread about what work there is left to do.
    Deque<RefPtr<Plan>> m_queue;
    
    // Used to answer questions about the current state of a code block. This
    // is particularly great for the cti_optimize OSR slow path, which wants
    // to know: did I get here because a better version of me just got
    // compiled?
    typedef HashMap<CompilationKey, RefPtr<Plan>> PlanMap;
    PlanMap m_plans;
    
    // Used to quickly find which plans have been compiled and are ready to
    // be completed.
    Vector<RefPtr<Plan>, 16> m_readyPlans;

    Lock m_suspensionLock;
    
    Box<Lock> m_lock;
    RefPtr<AutomaticThreadCondition> m_planEnqueued;
    Condition m_planCompiled;
    
    Vector<std::unique_ptr<ThreadData>> m_threads;
    unsigned m_numberOfActiveThreads;
};

// For DFGMode compilations.
Worklist& ensureGlobalDFGWorklist();
Worklist* existingGlobalDFGWorklistOrNull();

// For FTLMode and FTLForOSREntryMode compilations.
Worklist& ensureGlobalFTLWorklist();
Worklist* existingGlobalFTLWorklistOrNull();

Worklist& ensureGlobalWorklistFor(CompilationMode);

// Simplify doing things for all worklists.
unsigned numberOfWorklists();
Worklist& ensureWorklistForIndex(unsigned index);
Worklist* existingWorklistForIndexOrNull(unsigned index);
Worklist& existingWorklistForIndex(unsigned index);

#endif // ENABLE(DFG_JIT)

void completeAllPlansForVM(VM&);
void markCodeBlocks(VM&, SlotVisitor&);

template<typename Func>
void iterateCodeBlocksForGC(VM&, const Func&);

} } // namespace JSC::DFG

