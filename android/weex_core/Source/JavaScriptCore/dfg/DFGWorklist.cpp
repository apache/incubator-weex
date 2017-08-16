/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "DFGWorklist.h"

#include "CodeBlock.h"
#include "DFGLongLivedState.h"
#include "DFGSafepoint.h"
#include "DeferGC.h"
#include "JSCInlines.h"
#include "ReleaseHeapAccessScope.h"
#include <mutex>

namespace JSC { namespace DFG {

#if ENABLE(DFG_JIT)

class Worklist::ThreadBody : public AutomaticThread {
public:
    ThreadBody(const AbstractLocker& locker, Worklist& worklist, ThreadData& data, Box<Lock> lock, RefPtr<AutomaticThreadCondition> condition, int relativePriority)
        : AutomaticThread(locker, lock, condition)
        , m_worklist(worklist)
        , m_data(data)
        , m_relativePriority(relativePriority)
    {
    }
    
protected:
    PollResult poll(const AbstractLocker& locker) override
    {
        if (m_worklist.m_queue.isEmpty())
            return PollResult::Wait;
        
        m_plan = m_worklist.m_queue.takeFirst();
        if (!m_plan) {
            if (Options::verboseCompilationQueue()) {
                m_worklist.dump(locker, WTF::dataFile());
                dataLog(": Thread shutting down\n");
            }
            return PollResult::Stop;
        }
        RELEASE_ASSERT(m_plan->stage == Plan::Preparing);
        m_worklist.m_numberOfActiveThreads++;
        return PollResult::Work;
    }
    
    class WorkScope;
    friend class WorkScope;
    class WorkScope {
    public:
        WorkScope(ThreadBody& thread)
            : m_thread(thread)
        {
            RELEASE_ASSERT(m_thread.m_plan);
            RELEASE_ASSERT(m_thread.m_worklist.m_numberOfActiveThreads);
        }
        
        ~WorkScope()
        {
            LockHolder locker(*m_thread.m_worklist.m_lock);
            m_thread.m_plan = nullptr;
            m_thread.m_worklist.m_numberOfActiveThreads--;
        }
        
    private:
        ThreadBody& m_thread;
    };
    
    WorkResult work() override
    {
        WorkScope workScope(*this);
        
        LockHolder locker(m_data.m_rightToRun);
        {
            LockHolder locker(*m_worklist.m_lock);
            if (m_plan->stage == Plan::Cancelled)
                return WorkResult::Continue;
            m_plan->notifyCompiling();
        }
        
        if (Options::verboseCompilationQueue())
            dataLog(m_worklist, ": Compiling ", m_plan->key(), " asynchronously\n");
        
        // There's no way for the GC to be safepointing since we own rightToRun.
        if (m_plan->vm->heap.collectorBelievesThatTheWorldIsStopped()) {
            dataLog("Heap is stoped but here we are! (1)\n");
            RELEASE_ASSERT_NOT_REACHED();
        }
        m_plan->compileInThread(*m_longLivedState, &m_data);
        if (m_plan->stage != Plan::Cancelled) {
            if (m_plan->vm->heap.collectorBelievesThatTheWorldIsStopped()) {
                dataLog("Heap is stopped but here we are! (2)\n");
                RELEASE_ASSERT_NOT_REACHED();
            }
        }
        
        {
            LockHolder locker(*m_worklist.m_lock);
            if (m_plan->stage == Plan::Cancelled)
                return WorkResult::Continue;
            
            m_plan->notifyReady();
            
            if (Options::verboseCompilationQueue()) {
                m_worklist.dump(locker, WTF::dataFile());
                dataLog(": Compiled ", m_plan->key(), " asynchronously\n");
            }
            
            m_worklist.m_readyPlans.append(m_plan);
            
            m_worklist.m_planCompiled.notifyAll();
        }
        RELEASE_ASSERT(!m_plan->vm->heap.collectorBelievesThatTheWorldIsStopped());
        
        return WorkResult::Continue;
    }
    
    void threadDidStart() override
    {
        if (Options::verboseCompilationQueue())
            dataLog(m_worklist, ": Thread started\n");
        
        if (m_relativePriority)
            changeThreadPriority(currentThread(), m_relativePriority);
        
        m_compilationScope = std::make_unique<CompilationScope>();
        m_longLivedState = std::make_unique<LongLivedState>();
    }
    
    void threadIsStopping(const AbstractLocker&) override
    {
        // We're holding the Worklist::m_lock, so we should be careful not to deadlock.
        
        if (Options::verboseCompilationQueue())
            dataLog(m_worklist, ": Thread will stop\n");
        
        ASSERT(!m_plan);
        
        m_compilationScope = nullptr;
        m_longLivedState = nullptr;
        m_plan = nullptr;
    }
    
private:
    Worklist& m_worklist;
    ThreadData& m_data;
    int m_relativePriority;
    std::unique_ptr<CompilationScope> m_compilationScope;
    std::unique_ptr<LongLivedState> m_longLivedState;
    RefPtr<Plan> m_plan;
};

Worklist::Worklist(CString worklistName)
    : m_threadName(toCString(worklistName, " Worker Thread"))
    , m_lock(Box<Lock>::create())
    , m_planEnqueued(AutomaticThreadCondition::create())
    , m_numberOfActiveThreads(0)
{
}

Worklist::~Worklist()
{
    {
        LockHolder locker(*m_lock);
        for (unsigned i = m_threads.size(); i--;)
            m_queue.append(nullptr); // Use null plan to indicate that we want the thread to terminate.
        m_planEnqueued->notifyAll(locker);
    }
    for (unsigned i = m_threads.size(); i--;)
        m_threads[i]->m_thread->join();
    ASSERT(!m_numberOfActiveThreads);
}

void Worklist::finishCreation(unsigned numberOfThreads, int relativePriority)
{
    RELEASE_ASSERT(numberOfThreads);
    LockHolder locker(*m_lock);
    for (unsigned i = numberOfThreads; i--;) {
        std::unique_ptr<ThreadData> data = std::make_unique<ThreadData>(this);
        data->m_thread = adoptRef(new ThreadBody(locker, *this, *data, m_lock, m_planEnqueued, relativePriority));
        m_threads.append(WTFMove(data));
    }
}

Ref<Worklist> Worklist::create(CString worklistName, unsigned numberOfThreads, int relativePriority)
{
    Ref<Worklist> result = adoptRef(*new Worklist(worklistName));
    result->finishCreation(numberOfThreads, relativePriority);
    return result;
}

bool Worklist::isActiveForVM(VM& vm) const
{
    LockHolder locker(*m_lock);
    PlanMap::const_iterator end = m_plans.end();
    for (PlanMap::const_iterator iter = m_plans.begin(); iter != end; ++iter) {
        if (iter->value->vm == &vm)
            return true;
    }
    return false;
}

void Worklist::enqueue(Ref<Plan>&& plan)
{
    LockHolder locker(*m_lock);
    if (Options::verboseCompilationQueue()) {
        dump(locker, WTF::dataFile());
        dataLog(": Enqueueing plan to optimize ", plan->key(), "\n");
    }
    ASSERT(m_plans.find(plan->key()) == m_plans.end());
    m_plans.add(plan->key(), plan.copyRef());
    m_queue.append(WTFMove(plan));
    m_planEnqueued->notifyOne(locker);
}

Worklist::State Worklist::compilationState(CompilationKey key)
{
    LockHolder locker(*m_lock);
    PlanMap::iterator iter = m_plans.find(key);
    if (iter == m_plans.end())
        return NotKnown;
    return iter->value->stage == Plan::Ready ? Compiled : Compiling;
}

void Worklist::waitUntilAllPlansForVMAreReady(VM& vm)
{
    DeferGC deferGC(vm.heap);
    
    // While we are waiting for the compiler to finish, the collector might have already suspended
    // the compiler and then it will be waiting for us to stop. That's a deadlock. We avoid that
    // deadlock by relinquishing our heap access, so that the collector pretends that we are stopped
    // even if we aren't.
    ReleaseHeapAccessScope releaseHeapAccessScope(vm.heap);
    
    // Wait for all of the plans for the given VM to complete. The idea here
    // is that we want all of the caller VM's plans to be done. We don't care
    // about any other VM's plans, and we won't attempt to wait on those.
    // After we release this lock, we know that although other VMs may still
    // be adding plans, our VM will not be.
    
    LockHolder locker(*m_lock);
    
    if (Options::verboseCompilationQueue()) {
        dump(locker, WTF::dataFile());
        dataLog(": Waiting for all in VM to complete.\n");
    }
    
    for (;;) {
        bool allAreCompiled = true;
        PlanMap::iterator end = m_plans.end();
        for (PlanMap::iterator iter = m_plans.begin(); iter != end; ++iter) {
            if (iter->value->vm != &vm)
                continue;
            if (iter->value->stage != Plan::Ready) {
                allAreCompiled = false;
                break;
            }
        }
        
        if (allAreCompiled)
            break;
        
        m_planCompiled.wait(*m_lock);
    }
}

void Worklist::removeAllReadyPlansForVM(VM& vm, Vector<RefPtr<Plan>, 8>& myReadyPlans)
{
    DeferGC deferGC(vm.heap);
    LockHolder locker(*m_lock);
    for (size_t i = 0; i < m_readyPlans.size(); ++i) {
        RefPtr<Plan> plan = m_readyPlans[i];
        if (plan->vm != &vm)
            continue;
        if (plan->stage != Plan::Ready)
            continue;
        myReadyPlans.append(plan);
        m_readyPlans[i--] = m_readyPlans.last();
        m_readyPlans.removeLast();
        m_plans.remove(plan->key());
    }
}

void Worklist::removeAllReadyPlansForVM(VM& vm)
{
    Vector<RefPtr<Plan>, 8> myReadyPlans;
    removeAllReadyPlansForVM(vm, myReadyPlans);
}

Worklist::State Worklist::completeAllReadyPlansForVM(VM& vm, CompilationKey requestedKey)
{
    DeferGC deferGC(vm.heap);
    Vector<RefPtr<Plan>, 8> myReadyPlans;
    
    removeAllReadyPlansForVM(vm, myReadyPlans);
    
    State resultingState = NotKnown;

    while (!myReadyPlans.isEmpty()) {
        RefPtr<Plan> plan = myReadyPlans.takeLast();
        CompilationKey currentKey = plan->key();
        
        if (Options::verboseCompilationQueue())
            dataLog(*this, ": Completing ", currentKey, "\n");
        
        RELEASE_ASSERT(plan->stage == Plan::Ready);
        
        plan->finalizeAndNotifyCallback();
        
        if (currentKey == requestedKey)
            resultingState = Compiled;
    }
    
    if (!!requestedKey && resultingState == NotKnown) {
        LockHolder locker(*m_lock);
        if (m_plans.contains(requestedKey))
            resultingState = Compiling;
    }
    
    return resultingState;
}

void Worklist::completeAllPlansForVM(VM& vm)
{
    DeferGC deferGC(vm.heap);
    waitUntilAllPlansForVMAreReady(vm);
    completeAllReadyPlansForVM(vm);
}

void Worklist::suspendAllThreads()
{
    m_suspensionLock.lock();
    for (unsigned i = m_threads.size(); i--;)
        m_threads[i]->m_rightToRun.lock();
}

void Worklist::resumeAllThreads()
{
    for (unsigned i = m_threads.size(); i--;)
        m_threads[i]->m_rightToRun.unlock();
    m_suspensionLock.unlock();
}

void Worklist::visitWeakReferences(SlotVisitor& visitor)
{
    VM* vm = visitor.heap()->vm();
    {
        LockHolder locker(*m_lock);
        for (PlanMap::iterator iter = m_plans.begin(); iter != m_plans.end(); ++iter) {
            Plan* plan = iter->value.get();
            if (plan->vm != vm)
                continue;
            plan->checkLivenessAndVisitChildren(visitor);
        }
    }
    // This loop doesn't need locking because:
    // (1) no new threads can be added to m_threads. Hence, it is immutable and needs no locks.
    // (2) ThreadData::m_safepoint is protected by that thread's m_rightToRun which we must be
    //     holding here because of a prior call to suspendAllThreads().
    for (unsigned i = m_threads.size(); i--;) {
        ThreadData* data = m_threads[i].get();
        Safepoint* safepoint = data->m_safepoint;
        if (safepoint && safepoint->vm() == vm)
            safepoint->checkLivenessAndVisitChildren(visitor);
    }
}

void Worklist::removeDeadPlans(VM& vm)
{
    {
        LockHolder locker(*m_lock);
        HashSet<CompilationKey> deadPlanKeys;
        for (PlanMap::iterator iter = m_plans.begin(); iter != m_plans.end(); ++iter) {
            Plan* plan = iter->value.get();
            if (plan->vm != &vm)
                continue;
            if (plan->isKnownToBeLiveDuringGC())
                continue;
            RELEASE_ASSERT(plan->stage != Plan::Cancelled); // Should not be cancelled, yet.
            ASSERT(!deadPlanKeys.contains(plan->key()));
            deadPlanKeys.add(plan->key());
        }
        if (!deadPlanKeys.isEmpty()) {
            for (HashSet<CompilationKey>::iterator iter = deadPlanKeys.begin(); iter != deadPlanKeys.end(); ++iter)
                m_plans.take(*iter)->cancel();
            Deque<RefPtr<Plan>> newQueue;
            while (!m_queue.isEmpty()) {
                RefPtr<Plan> plan = m_queue.takeFirst();
                if (plan->stage != Plan::Cancelled)
                    newQueue.append(plan);
            }
            m_queue.swap(newQueue);
            for (unsigned i = 0; i < m_readyPlans.size(); ++i) {
                if (m_readyPlans[i]->stage != Plan::Cancelled)
                    continue;
                m_readyPlans[i--] = m_readyPlans.last();
                m_readyPlans.removeLast();
            }
        }
    }
    
    // No locking needed for this part, see comment in visitWeakReferences().
    for (unsigned i = m_threads.size(); i--;) {
        ThreadData* data = m_threads[i].get();
        Safepoint* safepoint = data->m_safepoint;
        if (!safepoint)
            continue;
        if (safepoint->vm() != &vm)
            continue;
        if (safepoint->isKnownToBeLiveDuringGC())
            continue;
        safepoint->cancel();
    }
}

void Worklist::removeNonCompilingPlansForVM(VM& vm)
{
    LockHolder locker(*m_lock);
    HashSet<CompilationKey> deadPlanKeys;
    Vector<RefPtr<Plan>> deadPlans;
    for (auto& entry : m_plans) {
        Plan* plan = entry.value.get();
        if (plan->vm != &vm)
            continue;
        if (plan->stage == Plan::Compiling)
            continue;
        deadPlanKeys.add(plan->key());
        deadPlans.append(plan);
    }
    for (CompilationKey key : deadPlanKeys)
        m_plans.remove(key);
    Deque<RefPtr<Plan>> newQueue;
    while (!m_queue.isEmpty()) {
        RefPtr<Plan> plan = m_queue.takeFirst();
        if (!deadPlanKeys.contains(plan->key()))
            newQueue.append(WTFMove(plan));
    }
    m_queue = WTFMove(newQueue);
    m_readyPlans.removeAllMatching(
        [&] (RefPtr<Plan>& plan) -> bool {
            return deadPlanKeys.contains(plan->key());
        });
    for (auto& plan : deadPlans)
        plan->cancel();
}

size_t Worklist::queueLength()
{
    LockHolder locker(*m_lock);
    return m_queue.size();
}

void Worklist::dump(PrintStream& out) const
{
    LockHolder locker(*m_lock);
    dump(locker, out);
}

void Worklist::dump(const AbstractLocker&, PrintStream& out) const
{
    out.print(
        "Worklist(", RawPointer(this), ")[Queue Length = ", m_queue.size(),
        ", Map Size = ", m_plans.size(), ", Num Ready = ", m_readyPlans.size(),
        ", Num Active Threads = ", m_numberOfActiveThreads, "/", m_threads.size(), "]");
}

static Worklist* theGlobalDFGWorklist;

Worklist& ensureGlobalDFGWorklist()
{
    static std::once_flag initializeGlobalWorklistOnceFlag;
    std::call_once(initializeGlobalWorklistOnceFlag, [] {
        theGlobalDFGWorklist = &Worklist::create("DFG Worklist", Options::numberOfDFGCompilerThreads(), Options::priorityDeltaOfDFGCompilerThreads()).leakRef();
    });
    return *theGlobalDFGWorklist;
}

Worklist* existingGlobalDFGWorklistOrNull()
{
    return theGlobalDFGWorklist;
}

static Worklist* theGlobalFTLWorklist;

Worklist& ensureGlobalFTLWorklist()
{
    static std::once_flag initializeGlobalWorklistOnceFlag;
    std::call_once(initializeGlobalWorklistOnceFlag, [] {
        theGlobalFTLWorklist = &Worklist::create("FTL Worklist", Options::numberOfFTLCompilerThreads(), Options::priorityDeltaOfFTLCompilerThreads()).leakRef();
    });
    return *theGlobalFTLWorklist;
}

Worklist* existingGlobalFTLWorklistOrNull()
{
    return theGlobalFTLWorklist;
}

Worklist& ensureGlobalWorklistFor(CompilationMode mode)
{
    switch (mode) {
    case InvalidCompilationMode:
        RELEASE_ASSERT_NOT_REACHED();
        return ensureGlobalDFGWorklist();
    case DFGMode:
        return ensureGlobalDFGWorklist();
    case FTLMode:
    case FTLForOSREntryMode:
        return ensureGlobalFTLWorklist();
    }
    RELEASE_ASSERT_NOT_REACHED();
    return ensureGlobalDFGWorklist();
}

unsigned numberOfWorklists() { return 2; }

Worklist& ensureWorklistForIndex(unsigned index)
{
    switch (index) {
    case 0:
        return ensureGlobalDFGWorklist();
    case 1:
        return ensureGlobalFTLWorklist();
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return ensureGlobalDFGWorklist();
    }
}

Worklist* existingWorklistForIndexOrNull(unsigned index)
{
    switch (index) {
    case 0:
        return existingGlobalDFGWorklistOrNull();
    case 1:
        return existingGlobalFTLWorklistOrNull();
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
}

Worklist& existingWorklistForIndex(unsigned index)
{
    Worklist* result = existingWorklistForIndexOrNull(index);
    RELEASE_ASSERT(result);
    return *result;
}

void completeAllPlansForVM(VM& vm)
{
    for (unsigned i = DFG::numberOfWorklists(); i--;) {
        if (DFG::Worklist* worklist = DFG::existingWorklistForIndexOrNull(i))
            worklist->completeAllPlansForVM(vm);
    }
}

#else // ENABLE(DFG_JIT)

void completeAllPlansForVM(VM&)
{
}

void markCodeBlocks(VM&, SlotVisitor&)
{
}

#endif // ENABLE(DFG_JIT)

} } // namespace JSC::DFG


