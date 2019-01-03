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

#include "MachineContext.h"
#include "PlatformThread.h"
#include "RegisterState.h"
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/ScopedLambda.h>
#include <wtf/ThreadSpecific.h>

#if USE(PTHREADS) && !OS(WINDOWS) && !OS(DARWIN)
#include <semaphore.h>
#include <signal.h>
// Using signal.h didn't make mcontext_t and ucontext_t available on FreeBSD.
// This bug has been fixed in FreeBSD 11.0-CURRENT, so this workaround can be
// removed after FreeBSD 10.x goes EOL.
// https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=207079
#if OS(FREEBSD)
#include <ucontext.h>
#endif
#endif

namespace JSC {

class CodeBlockSet;
class ConservativeRoots;
class Heap;
class JITStubRoutineSet;

struct CurrentThreadState {
    void* stackOrigin { nullptr };
    void* stackTop { nullptr };
    RegisterState* registerState { nullptr };
};
    
class MachineThreads {
    WTF_MAKE_NONCOPYABLE(MachineThreads);
public:
    MachineThreads();
    ~MachineThreads();

    void gatherConservativeRoots(ConservativeRoots&, JITStubRoutineSet&, CodeBlockSet&, CurrentThreadState*);

    JS_EXPORT_PRIVATE void addCurrentThread(); // Only needs to be called by clients that can use the same heap from multiple threads.

    class Thread {
        WTF_MAKE_FAST_ALLOCATED;
        Thread(const PlatformThread& platThread, void* base, void* end);

    public:
        ~Thread();

        static Thread* createForCurrentThread();

        struct Registers {
            void* stackPointer() const;
#if ENABLE(SAMPLING_PROFILER)
            void* framePointer() const;
            void* instructionPointer() const;
            void* llintPC() const;
#endif // ENABLE(SAMPLING_PROFILER)
            
#if OS(DARWIN) || OS(WINDOWS)
            using PlatformRegisters = MachineContext::PlatformRegisters;
#elif (OS(FREEBSD) || defined(__GLIBC__)) && ENABLE(JIT)
            using PlatformRegisters = mcontext_t;
#elif USE(PTHREADS)
            struct PlatformRegisters {
                pthread_attr_t attribute;
            };
#else
#error Need a thread register struct for this platform
#endif
            
            PlatformRegisters regs;
        };
        
        bool operator==(const PlatformThread& other) const;
        bool operator!=(const PlatformThread& other) const { return !(*this == other); }

        bool suspend();
        void resume();
        size_t getRegisters(Registers&);
        void freeRegisters(Registers&);
        std::pair<void*, size_t> captureStack(void* stackTop);

        Thread* next;
        PlatformThread platformThread;
        void* stackBase;
        void* stackEnd;
#if OS(WINDOWS)
        HANDLE platformThreadHandle;
#elif USE(PTHREADS) && !OS(DARWIN)
        sem_t semaphoreForSuspendResume;
        mcontext_t suspendedMachineContext;
        int suspendCount { 0 };
        std::atomic<bool> suspended { false };
#endif
    };

    Lock& getLock() { return m_registeredThreadsMutex; }
    Thread* threadsListHead(const AbstractLocker&) const { ASSERT(m_registeredThreadsMutex.isLocked()); return m_registeredThreads; }
    Thread* machineThreadForCurrentThread();

private:
    void gatherFromCurrentThread(ConservativeRoots&, JITStubRoutineSet&, CodeBlockSet&, CurrentThreadState&);

    void tryCopyOtherThreadStack(Thread*, void*, size_t capacity, size_t*);
    bool tryCopyOtherThreadStacks(const AbstractLocker&, void*, size_t capacity, size_t*);

    static void THREAD_SPECIFIC_CALL removeThread(void*);

    template<typename PlatformThread>
    void removeThreadIfFound(PlatformThread);

    Lock m_registeredThreadsMutex;
    Thread* m_registeredThreads;
    WTF::ThreadSpecificKey m_threadSpecificForMachineThreads;
};

#define DECLARE_AND_COMPUTE_CURRENT_THREAD_STATE(stateName) \
    CurrentThreadState stateName; \
    stateName.stackTop = &stateName; \
    stateName.stackOrigin = wtfThreadData().stack().origin(); \
    ALLOCATE_AND_GET_REGISTER_STATE(stateName ## _registerState); \
    stateName.registerState = &stateName ## _registerState

// The return value is meaningless. We just use it to suppress tail call optimization.
int callWithCurrentThreadState(const ScopedLambda<void(CurrentThreadState&)>&);

} // namespace JSC

