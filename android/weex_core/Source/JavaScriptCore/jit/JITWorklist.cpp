/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "JITWorklist.h"

#if ENABLE(JIT)

#include "JIT.h"
#include "JSCInlines.h"
#include "VMInlines.h"

namespace JSC {

class JITWorklist::Plan : public ThreadSafeRefCounted<JITWorklist::Plan> {
public:
    Plan(CodeBlock* codeBlock, unsigned loopOSREntryBytecodeOffset)
        : m_codeBlock(codeBlock)
        , m_jit(codeBlock->vm(), codeBlock, loopOSREntryBytecodeOffset)
    {
        m_jit.doMainThreadPreparationBeforeCompile();
    }
    
    void compileInThread()
    {
        m_jit.compileWithoutLinking(JITCompilationCanFail);
        
        LockHolder locker(m_lock);
        m_isFinishedCompiling = true;
    }
    
    void finalize()
    {
        CompilationResult result = m_jit.link();
        switch (result) {
        case CompilationFailed:
            CODEBLOCK_LOG_EVENT(m_codeBlock, "delayJITCompile", ("compilation failed"));
            if (Options::verboseOSR())
                dataLogF("    JIT compilation failed.\n");
            m_codeBlock->dontJITAnytimeSoon();
            m_codeBlock->m_didFailJITCompilation = true;
            return;
        case CompilationSuccessful:
            if (Options::verboseOSR())
                dataLogF("    JIT compilation successful.\n");
            m_codeBlock->ownerScriptExecutable()->installCode(m_codeBlock);
            m_codeBlock->jitSoon();
            return;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return;
        }
    }
    
    CodeBlock* codeBlock() { return m_codeBlock; }
    VM* vm() { return m_codeBlock->vm(); }
    
    bool isFinishedCompiling()
    {
        LockHolder locker(m_lock);
        return m_isFinishedCompiling;
    }
    
    static void compileNow(CodeBlock* codeBlock, unsigned loopOSREntryBytecodeOffset)
    {
        Plan plan(codeBlock, loopOSREntryBytecodeOffset);
        plan.compileInThread();
        plan.finalize();
    }
    
private:
    CodeBlock* m_codeBlock;
    JIT m_jit;
    Lock m_lock;
    bool m_isFinishedCompiling { false };
};

class JITWorklist::Thread : public AutomaticThread {
public:
    Thread(const AbstractLocker& locker, JITWorklist& worklist)
        : AutomaticThread(locker, worklist.m_lock, worklist.m_condition)
        , m_worklist(worklist)
    {
        m_worklist.m_numAvailableThreads++;
    }
    
protected:
    PollResult poll(const AbstractLocker&) override
    {
        RELEASE_ASSERT(m_worklist.m_numAvailableThreads);
        
        if (m_worklist.m_queue.isEmpty())
            return PollResult::Wait;
        
        m_myPlans = WTFMove(m_worklist.m_queue);
        m_worklist.m_numAvailableThreads--;
        return PollResult::Work;
    }
    
    WorkResult work() override
    {
        RELEASE_ASSERT(!m_myPlans.isEmpty());
        
        for (RefPtr<Plan>& plan : m_myPlans) {
            plan->compileInThread();
            plan = nullptr;
            
            // Make sure that the main thread realizes that we just compiled something. Notifying
            // a condition is basically free if nobody is waiting.
            LockHolder locker(*m_worklist.m_lock);
            m_worklist.m_condition->notifyAll(locker);
        }
        
        m_myPlans.clear();
        
        LockHolder locker(*m_worklist.m_lock);
        m_worklist.m_numAvailableThreads++;
        return WorkResult::Continue;
    }
    
private:
    JITWorklist& m_worklist;
    Plans m_myPlans;
};

JITWorklist::JITWorklist()
    : m_lock(Box<Lock>::create())
    , m_condition(AutomaticThreadCondition::create())
{
    LockHolder locker(*m_lock);
    m_thread = new Thread(locker, *this);
}

JITWorklist::~JITWorklist()
{
    UNREACHABLE_FOR_PLATFORM();
}

bool JITWorklist::completeAllForVM(VM& vm)
{
    bool result = false;
    DeferGC deferGC(vm.heap);
    for (;;) {
        Vector<RefPtr<Plan>, 32> myPlans;
        {
            LockHolder locker(*m_lock);
            for (;;) {
                bool didFindUnfinishedPlan = false;
                m_plans.removeAllMatching(
                    [&] (RefPtr<Plan>& plan) {
                        if (plan->vm() != &vm)
                            return false;
                        if (!plan->isFinishedCompiling()) {
                            didFindUnfinishedPlan = true;
                            return false;
                        }
                        myPlans.append(WTFMove(plan));
                        return true;
                    });
                
                // If we found plans then we should finalize them now.
                if (!myPlans.isEmpty())
                    break;
                
                // If we don't find plans, then we're either done or we need to wait, depending on
                // whether we found some unfinished plans.
                if (!didFindUnfinishedPlan)
                    return result;
                
                m_condition->wait(*m_lock);
            }
        }
        
        RELEASE_ASSERT(!myPlans.isEmpty());
        result = true;
        finalizePlans(myPlans);
    }
}

void JITWorklist::poll(VM& vm)
{
    DeferGC deferGC(vm.heap);
    Plans myPlans;
    {
        LockHolder locker(*m_lock);
        m_plans.removeAllMatching(
            [&] (RefPtr<Plan>& plan) {
                if (plan->vm() != &vm)
                    return false;
                if (!plan->isFinishedCompiling())
                    return false;
                myPlans.append(WTFMove(plan));
                return true;
            });
    }
    
    finalizePlans(myPlans);
}

void JITWorklist::compileLater(CodeBlock* codeBlock, unsigned loopOSREntryBytecodeOffset)
{
    DeferGC deferGC(codeBlock->vm()->heap);
    RELEASE_ASSERT(codeBlock->jitType() == JITCode::InterpreterThunk);
    
    if (codeBlock->m_didFailJITCompilation) {
        codeBlock->dontJITAnytimeSoon();
        return;
    }
    
    if (!Options::useConcurrentJIT()) {
        Plan::compileNow(codeBlock, loopOSREntryBytecodeOffset);
        return;
    }
    
    codeBlock->jitSoon();

    {
        LockHolder locker(*m_lock);
        
        if (m_planned.contains(codeBlock))
            return;
        
        if (m_numAvailableThreads) {
            m_planned.add(codeBlock);
            RefPtr<Plan> plan = adoptRef(new Plan(codeBlock, loopOSREntryBytecodeOffset));
            m_plans.append(plan);
            m_queue.append(plan);
            m_condition->notifyAll(locker);
            return;
        }
    }

    // Compiling on the main thread if the helper thread isn't available is a defense against this
    // pathology:
    //
    // 1) Do something that is allowed to take a while, like load a giant piece of initialization
    //    code. This plans the compile of the init code, but doesn't finish it. It will take a
    //    while.
    //
    // 2) Do something that is supposed to be quick. Now all baseline compiles, and so all DFG and
    //    FTL compiles, of everything is blocked on the long-running baseline compile of that
    //    initialization code.
    //
    // The single-threaded concurrent JIT has this tendency to convoy everything while at the same
    // time postponing when it happens, which means that the convoy delays are less predictable.
    // This works around the issue. If the concurrent JIT thread is convoyed, we revert to main
    // thread compiles. This is probably not as good as if we had multiple JIT threads. Maybe we
    // can do that someday.
    Plan::compileNow(codeBlock, loopOSREntryBytecodeOffset);
}

void JITWorklist::compileNow(CodeBlock* codeBlock, unsigned loopOSREntryBytecodeOffset)
{
    DeferGC deferGC(codeBlock->vm()->heap);
    if (codeBlock->jitType() != JITCode::InterpreterThunk)
        return;
    
    bool isPlanned;
    {
        LockHolder locker(*m_lock);
        isPlanned = m_planned.contains(codeBlock);
    }
    
    if (isPlanned) {
        RELEASE_ASSERT(Options::useConcurrentJIT());
        // This is expensive, but probably good enough.
        completeAllForVM(*codeBlock->vm());
    }
    
    // Now it might be compiled!
    if (codeBlock->jitType() != JITCode::InterpreterThunk)
        return;
    
    // We do this in case we had previously attempted, and then failed, to compile with the
    // baseline JIT.
    codeBlock->resetJITData();
    
    // OK, just compile it.
    JIT::compile(codeBlock->vm(), codeBlock, JITCompilationMustSucceed, loopOSREntryBytecodeOffset);
    codeBlock->ownerScriptExecutable()->installCode(codeBlock);
}

void JITWorklist::finalizePlans(Plans& myPlans)
{
    for (RefPtr<Plan>& plan : myPlans) {
        plan->finalize();
        
        LockHolder locker(*m_lock);
        m_planned.remove(plan->codeBlock());
    }
}

JITWorklist* JITWorklist::instance()
{
    static JITWorklist* worklist;
    static std::once_flag once;
    std::call_once(
        once,
        [] {
            worklist = new JITWorklist();
        });
    return worklist;
}

} // namespace JSC

#endif // ENABLE(JIT)

