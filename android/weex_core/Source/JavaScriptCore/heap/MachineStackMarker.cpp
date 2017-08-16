/*
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 *  Copyright (C) 2009 Acision BV. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "MachineStackMarker.h"

#include "ConservativeRoots.h"
#include "GPRInfo.h"
#include "Heap.h"
#include "JSArray.h"
#include "JSCInlines.h"
#include "LLIntPCRanges.h"
#include "MacroAssembler.h"
#include "VM.h"
#include <setjmp.h>
#include <stdlib.h>
#include <wtf/MainThread.h>
#include <wtf/StdLibExtras.h>

#if OS(DARWIN)

#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/vm_map.h>

#elif OS(WINDOWS)

#include <windows.h>
#include <malloc.h>

#elif OS(UNIX)

#include <sys/mman.h>
#include <unistd.h>

#if OS(SOLARIS)
#include <thread.h>
#else
#include <pthread.h>
#endif

#if HAVE(PTHREAD_NP_H)
#include <pthread_np.h>
#endif

#if USE(PTHREADS) && !OS(WINDOWS) && !OS(DARWIN)
#include <signal.h>

// We use SIGUSR2 to suspend and resume machine threads in JavaScriptCore.
static const int SigThreadSuspendResume = SIGUSR2;
static StaticLock globalSignalLock;
thread_local static std::atomic<JSC::MachineThreads::Thread*> threadLocalCurrentThread;

static void pthreadSignalHandlerSuspendResume(int, siginfo_t*, void* ucontext)
{
    // Touching thread local atomic types from signal handlers is allowed.
    JSC::MachineThreads::Thread* thread = threadLocalCurrentThread.load();

    if (thread->suspended.load(std::memory_order_acquire)) {
        // This is signal handler invocation that is intended to be used to resume sigsuspend.
        // So this handler invocation itself should not process.
        //
        // When signal comes, first, the system calls signal handler. And later, sigsuspend will be resumed. Signal handler invocation always precedes.
        // So, the problem never happens that suspended.store(true, ...) will be executed before the handler is called.
        // http://pubs.opengroup.org/onlinepubs/009695399/functions/sigsuspend.html
        return;
    }

    ucontext_t* userContext = static_cast<ucontext_t*>(ucontext);
#if CPU(PPC)
    thread->suspendedMachineContext = *userContext->uc_mcontext.uc_regs;
#else
    thread->suspendedMachineContext = userContext->uc_mcontext;
#endif

    // Allow suspend caller to see that this thread is suspended.
    // sem_post is async-signal-safe function. It means that we can call this from a signal handler.
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_04.html#tag_02_04_03
    //
    // And sem_post emits memory barrier that ensures that suspendedMachineContext is correctly saved.
    // http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_11
    sem_post(&thread->semaphoreForSuspendResume);

    // Reaching here, SigThreadSuspendResume is blocked in this handler (this is configured by sigaction's sa_mask).
    // So before calling sigsuspend, SigThreadSuspendResume to this thread is deferred. This ensures that the handler is not executed recursively.
    sigset_t blockedSignalSet;
    sigfillset(&blockedSignalSet);
    sigdelset(&blockedSignalSet, SigThreadSuspendResume);
    sigsuspend(&blockedSignalSet);

    // Allow resume caller to see that this thread is resumed.
    sem_post(&thread->semaphoreForSuspendResume);
}
#endif // USE(PTHREADS) && !OS(WINDOWS) && !OS(DARWIN)

#endif

using namespace WTF;

namespace JSC {

using Thread = MachineThreads::Thread;

class ActiveMachineThreadsManager;
static ActiveMachineThreadsManager& activeMachineThreadsManager();

class ActiveMachineThreadsManager {
    WTF_MAKE_NONCOPYABLE(ActiveMachineThreadsManager);
public:

    class Locker {
    public:
        Locker(ActiveMachineThreadsManager& manager)
            : m_locker(manager.m_lock)
        {
        }

    private:
        LockHolder m_locker;
    };

    void add(MachineThreads* machineThreads)
    {
        LockHolder managerLock(m_lock);
        m_set.add(machineThreads);
    }

    void THREAD_SPECIFIC_CALL remove(MachineThreads* machineThreads)
    {
        LockHolder managerLock(m_lock);
        auto recordedMachineThreads = m_set.take(machineThreads);
        RELEASE_ASSERT(recordedMachineThreads == machineThreads);
    }

    bool contains(MachineThreads* machineThreads)
    {
        return m_set.contains(machineThreads);
    }

private:
    typedef HashSet<MachineThreads*> MachineThreadsSet;

    ActiveMachineThreadsManager() { }
    
    Lock m_lock;
    MachineThreadsSet m_set;

    friend ActiveMachineThreadsManager& activeMachineThreadsManager();
};

static ActiveMachineThreadsManager& activeMachineThreadsManager()
{
    static std::once_flag initializeManagerOnceFlag;
    static ActiveMachineThreadsManager* manager = nullptr;

    std::call_once(initializeManagerOnceFlag, [] {
        manager = new ActiveMachineThreadsManager();
    });
    return *manager;
}
    
MachineThreads::MachineThreads()
    : m_registeredThreads(0)
    , m_threadSpecificForMachineThreads(0)
{
    threadSpecificKeyCreate(&m_threadSpecificForMachineThreads, removeThread);
    activeMachineThreadsManager().add(this);
}

MachineThreads::~MachineThreads()
{
    activeMachineThreadsManager().remove(this);
    threadSpecificKeyDelete(m_threadSpecificForMachineThreads);

    LockHolder registeredThreadsLock(m_registeredThreadsMutex);
    for (Thread* t = m_registeredThreads; t;) {
        Thread* next = t->next;
        delete t;
        t = next;
    }
}

Thread* MachineThreads::Thread::createForCurrentThread()
{
    auto stackBounds = wtfThreadData().stack();
    return new Thread(currentPlatformThread(), stackBounds.origin(), stackBounds.end());
}

bool MachineThreads::Thread::operator==(const PlatformThread& other) const
{
#if OS(DARWIN) || OS(WINDOWS)
    return platformThread == other;
#elif USE(PTHREADS)
    return !!pthread_equal(platformThread, other);
#else
#error Need a way to compare threads on this platform
#endif
}

void MachineThreads::addCurrentThread()
{
    if (threadSpecificGet(m_threadSpecificForMachineThreads)) {
#ifndef NDEBUG
        LockHolder lock(m_registeredThreadsMutex);
        ASSERT(threadSpecificGet(m_threadSpecificForMachineThreads) == this);
#endif
        return;
    }

    Thread* thread = Thread::createForCurrentThread();
    threadSpecificSet(m_threadSpecificForMachineThreads, this);

    LockHolder lock(m_registeredThreadsMutex);

    thread->next = m_registeredThreads;
    m_registeredThreads = thread;
}

Thread* MachineThreads::machineThreadForCurrentThread()
{
    LockHolder lock(m_registeredThreadsMutex);
    PlatformThread platformThread = currentPlatformThread();
    for (Thread* thread = m_registeredThreads; thread; thread = thread->next) {
        if (*thread == platformThread)
            return thread;
    }

    RELEASE_ASSERT_NOT_REACHED();
    return nullptr;
}

void THREAD_SPECIFIC_CALL MachineThreads::removeThread(void* p)
{
    auto& manager = activeMachineThreadsManager();
    ActiveMachineThreadsManager::Locker lock(manager);
    auto machineThreads = static_cast<MachineThreads*>(p);
    if (manager.contains(machineThreads)) {
        // There's a chance that the MachineThreads registry that this thread
        // was registered with was already destructed, and another one happened
        // to be instantiated at the same address. Hence, this thread may or
        // may not be found in this MachineThreads registry. We only need to
        // do a removal if this thread is found in it.

#if PLATFORM(WIN)
        // On Windows the thread specific destructor is also called when the
        // main thread is exiting. This may lead to the main thread waiting
        // forever for the machine thread lock when exiting, if the sampling
        // profiler thread was terminated by the system while holding the
        // machine thread lock.
        if (WTF::isMainThread())
            return;
#endif

        machineThreads->removeThreadIfFound(currentPlatformThread());
    }
}

template<typename PlatformThread>
void MachineThreads::removeThreadIfFound(PlatformThread platformThread)
{
    LockHolder lock(m_registeredThreadsMutex);
    Thread* t = m_registeredThreads;
    if (*t == platformThread) {
        m_registeredThreads = m_registeredThreads->next;
        delete t;
    } else {
        Thread* last = m_registeredThreads;
        for (t = m_registeredThreads->next; t; t = t->next) {
            if (*t == platformThread) {
                last->next = t->next;
                break;
            }
            last = t;
        }
        delete t;
    }
}

SUPPRESS_ASAN
void MachineThreads::gatherFromCurrentThread(ConservativeRoots& conservativeRoots, JITStubRoutineSet& jitStubRoutines, CodeBlockSet& codeBlocks, CurrentThreadState& currentThreadState)
{
    if (currentThreadState.registerState) {
        void* registersBegin = currentThreadState.registerState;
        void* registersEnd = reinterpret_cast<void*>(roundUpToMultipleOf<sizeof(void*)>(reinterpret_cast<uintptr_t>(currentThreadState.registerState + 1)));
        conservativeRoots.add(registersBegin, registersEnd, jitStubRoutines, codeBlocks);
    }

    conservativeRoots.add(currentThreadState.stackTop, currentThreadState.stackOrigin, jitStubRoutines, codeBlocks);
}

MachineThreads::Thread::Thread(const PlatformThread& platThread, void* base, void* end)
    : platformThread(platThread)
    , stackBase(base)
    , stackEnd(end)
{
#if OS(WINDOWS)
    ASSERT(platformThread == GetCurrentThreadId());
    bool isSuccessful =
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
            &platformThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
    RELEASE_ASSERT(isSuccessful);
#elif USE(PTHREADS) && !OS(DARWIN)
    threadLocalCurrentThread.store(this);

    // Signal handlers are process global configuration.
    static std::once_flag initializeSignalHandler;
    std::call_once(initializeSignalHandler, [] {
        // Intentionally block SigThreadSuspendResume in the handler.
        // SigThreadSuspendResume will be allowed in the handler by sigsuspend.
        struct sigaction action;
        sigemptyset(&action.sa_mask);
        sigaddset(&action.sa_mask, SigThreadSuspendResume);

        action.sa_sigaction = pthreadSignalHandlerSuspendResume;
        action.sa_flags = SA_RESTART | SA_SIGINFO;
        sigaction(SigThreadSuspendResume, &action, 0);
    });

    sigset_t old_set;
    sigemptyset(&old_set);
    sigprocmask(SIG_BLOCK, NULL, &old_set);
    if (sigismember(&old_set, SigThreadSuspendResume) == 1) {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SigThreadSuspendResume);
        pthread_sigmask(SIG_UNBLOCK, &mask, 0);
    }

    sem_init(&semaphoreForSuspendResume, /* Only available in this process. */ 0, /* Initial value for the semaphore. */ 0);
#endif
}

MachineThreads::Thread::~Thread()
{
#if OS(WINDOWS)
    CloseHandle(platformThreadHandle);
#elif USE(PTHREADS) && !OS(DARWIN)
    sem_destroy(&semaphoreForSuspendResume);
#endif
}

bool MachineThreads::Thread::suspend()
{
#if OS(DARWIN)
    kern_return_t result = thread_suspend(platformThread);
    return result == KERN_SUCCESS;
#elif OS(WINDOWS)
    bool threadIsSuspended = (SuspendThread(platformThreadHandle) != (DWORD)-1);
    ASSERT(threadIsSuspended);
    return threadIsSuspended;
#elif USE(PTHREADS)
    ASSERT_WITH_MESSAGE(currentPlatformThread() != platformThread, "Currently we don't support suspend the current thread itself.");
    {
        // During suspend, suspend or resume should not be executed from the other threads.
        // We use global lock instead of per thread lock.
        // Consider the following case, there are threads A and B.
        // And A attempt to suspend B and B attempt to suspend A.
        // A and B send signals. And later, signals are delivered to A and B.
        // In that case, both will be suspended.
        LockHolder lock(globalSignalLock);
        if (!suspendCount) {
            // Ideally, we would like to use pthread_sigqueue. It allows us to pass the argument to the signal handler.
            // But it can be used in a few platforms, like Linux.
            // Instead, we use Thread* stored in the thread local storage to pass it to the signal handler.
            if (pthread_kill(platformThread, SigThreadSuspendResume) == ESRCH)
                return false;
            sem_wait(&semaphoreForSuspendResume);
            // Release barrier ensures that this operation is always executed after all the above processing is done.
            suspended.store(true, std::memory_order_release);
        }
        ++suspendCount;
    }
    return true;
#else
#error Need a way to suspend threads on this platform
#endif
}

void MachineThreads::Thread::resume()
{
#if OS(DARWIN)
    thread_resume(platformThread);
#elif OS(WINDOWS)
    ResumeThread(platformThreadHandle);
#elif USE(PTHREADS)
    {
        // During resume, suspend or resume should not be executed from the other threads.
        LockHolder lock(globalSignalLock);
        if (suspendCount == 1) {
            // When allowing SigThreadSuspendResume interrupt in the signal handler by sigsuspend and SigThreadSuspendResume is actually issued,
            // the signal handler itself will be called once again.
            // There are several ways to distinguish the handler invocation for suspend and resume.
            // 1. Use different signal numbers. And check the signal number in the handler.
            // 2. Use some arguments to distinguish suspend and resume in the handler. If pthread_sigqueue can be used, we can take this.
            // 3. Use thread local storage with atomic variables in the signal handler.
            // In this implementaiton, we take (3). suspended flag is used to distinguish it.
            if (pthread_kill(platformThread, SigThreadSuspendResume) == ESRCH)
                return;
            sem_wait(&semaphoreForSuspendResume);
            // Release barrier ensures that this operation is always executed after all the above processing is done.
            suspended.store(false, std::memory_order_release);
        }
        --suspendCount;
    }
#else
#error Need a way to resume threads on this platform
#endif
}

size_t MachineThreads::Thread::getRegisters(Thread::Registers& registers)
{
    Thread::Registers::PlatformRegisters& regs = registers.regs;
#if OS(DARWIN)
#if CPU(X86)
    unsigned user_count = sizeof(regs)/sizeof(int);
    thread_state_flavor_t flavor = i386_THREAD_STATE;
#elif CPU(X86_64)
    unsigned user_count = x86_THREAD_STATE64_COUNT;
    thread_state_flavor_t flavor = x86_THREAD_STATE64;
#elif CPU(PPC) 
    unsigned user_count = PPC_THREAD_STATE_COUNT;
    thread_state_flavor_t flavor = PPC_THREAD_STATE;
#elif CPU(PPC64)
    unsigned user_count = PPC_THREAD_STATE64_COUNT;
    thread_state_flavor_t flavor = PPC_THREAD_STATE64;
#elif CPU(ARM)
    unsigned user_count = ARM_THREAD_STATE_COUNT;
    thread_state_flavor_t flavor = ARM_THREAD_STATE;
#elif CPU(ARM64)
    unsigned user_count = ARM_THREAD_STATE64_COUNT;
    thread_state_flavor_t flavor = ARM_THREAD_STATE64;
#else
#error Unknown Architecture
#endif

    kern_return_t result = thread_get_state(platformThread, flavor, (thread_state_t)&regs, &user_count);
    if (result != KERN_SUCCESS) {
        WTFReportFatalError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, 
                            "JavaScript garbage collection failed because thread_get_state returned an error (%d). This is probably the result of running inside Rosetta, which is not supported.", result);
        CRASH();
    }
    return user_count * sizeof(uintptr_t);
// end OS(DARWIN)

#elif OS(WINDOWS)
    regs.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
    GetThreadContext(platformThreadHandle, &regs);
    return sizeof(CONTEXT);
#elif ((OS(FREEBSD) || defined(__GLIBC__)) && ENABLE(JIT))
    regs = suspendedMachineContext;
    return sizeof(Registers::PlatformRegisters);
#elif USE(PTHREADS)
    pthread_attr_init(&regs.attribute);
#if HAVE(PTHREAD_NP_H) || OS(NETBSD)
#if !OS(OPENBSD)
    // e.g. on FreeBSD 5.4, neundorf@kde.org
    pthread_attr_get_np(platformThread, &regs.attribute);
#endif
#else
    // FIXME: this function is non-portable; other POSIX systems may have different np alternatives
    pthread_getattr_np(platformThread, &regs.attribute);
#endif
    return 0;
#else
#error Need a way to get thread registers on this platform
#endif
}

void* MachineThreads::Thread::Registers::stackPointer() const
{
#if OS(DARWIN) || OS(WINDOWS) || ((OS(FREEBSD) || defined(__GLIBC__)) && ENABLE(JIT))
    return MachineContext::stackPointer(regs);
#elif USE(PTHREADS)
    void* stackBase = 0;
    size_t stackSize = 0;
#if OS(OPENBSD)
    stack_t ss;
    int rc = pthread_stackseg_np(pthread_self(), &ss);
    stackBase = (void*)((size_t) ss.ss_sp - ss.ss_size);
    stackSize = ss.ss_size;
#else
    int rc = pthread_attr_getstack(&regs.attribute, &stackBase, &stackSize);
#endif
    (void)rc; // FIXME: Deal with error code somehow? Seems fatal.
    ASSERT(stackBase);
    return static_cast<char*>(stackBase) + stackSize;
#else
#error Need a way to get the stack pointer for another thread on this platform
#endif
}

#if ENABLE(SAMPLING_PROFILER)
void* MachineThreads::Thread::Registers::framePointer() const
{
#if OS(DARWIN) || OS(WINDOWS) || (OS(FREEBSD) || defined(__GLIBC__))
    return MachineContext::framePointer(regs);
#else
#error Need a way to get the frame pointer for another thread on this platform
#endif
}

void* MachineThreads::Thread::Registers::instructionPointer() const
{
#if OS(DARWIN) || OS(WINDOWS) || (OS(FREEBSD) || defined(__GLIBC__))
    return MachineContext::instructionPointer(regs);
#else
#error Need a way to get the instruction pointer for another thread on this platform
#endif
}

void* MachineThreads::Thread::Registers::llintPC() const
{
    // LLInt uses regT4 as PC.
#if OS(DARWIN) || OS(WINDOWS) || (OS(FREEBSD) || defined(__GLIBC__))
    return MachineContext::llintInstructionPointer(regs);
#else
#error Need a way to get the LLIntPC for another thread on this platform
#endif
}
#endif // ENABLE(SAMPLING_PROFILER)

void MachineThreads::Thread::freeRegisters(Thread::Registers& registers)
{
    Thread::Registers::PlatformRegisters& regs = registers.regs;
#if USE(PTHREADS) && !OS(WINDOWS) && !OS(DARWIN) && !((OS(FREEBSD) || defined(__GLIBC__)) && ENABLE(JIT))
    pthread_attr_destroy(&regs.attribute);
#else
    UNUSED_PARAM(regs);
#endif
}

static inline int osRedZoneAdjustment()
{
    int redZoneAdjustment = 0;
#if !OS(WINDOWS)
#if CPU(X86_64)
    // See http://people.freebsd.org/~obrien/amd64-elf-abi.pdf Section 3.2.2.
    redZoneAdjustment = -128;
#elif CPU(ARM64)
    // See https://developer.apple.com/library/ios/documentation/Xcode/Conceptual/iPhoneOSABIReference/Articles/ARM64FunctionCallingConventions.html#//apple_ref/doc/uid/TP40013702-SW7
    redZoneAdjustment = -128;
#endif
#endif // !OS(WINDOWS)
    return redZoneAdjustment;
}

std::pair<void*, size_t> MachineThreads::Thread::captureStack(void* stackTop)
{
    char* begin = reinterpret_cast_ptr<char*>(stackBase);
    char* end = bitwise_cast<char*>(WTF::roundUpToMultipleOf<sizeof(void*)>(reinterpret_cast<uintptr_t>(stackTop)));
    ASSERT(begin >= end);

    char* endWithRedZone = end + osRedZoneAdjustment();
    ASSERT(WTF::roundUpToMultipleOf<sizeof(void*)>(reinterpret_cast<uintptr_t>(endWithRedZone)) == reinterpret_cast<uintptr_t>(endWithRedZone));

    if (endWithRedZone < stackEnd)
        endWithRedZone = reinterpret_cast_ptr<char*>(stackEnd);

    std::swap(begin, endWithRedZone);
    return std::make_pair(begin, endWithRedZone - begin);
}

SUPPRESS_ASAN
static void copyMemory(void* dst, const void* src, size_t size)
{
    size_t dstAsSize = reinterpret_cast<size_t>(dst);
    size_t srcAsSize = reinterpret_cast<size_t>(src);
    RELEASE_ASSERT(dstAsSize == WTF::roundUpToMultipleOf<sizeof(intptr_t)>(dstAsSize));
    RELEASE_ASSERT(srcAsSize == WTF::roundUpToMultipleOf<sizeof(intptr_t)>(srcAsSize));
    RELEASE_ASSERT(size == WTF::roundUpToMultipleOf<sizeof(intptr_t)>(size));

    intptr_t* dstPtr = reinterpret_cast<intptr_t*>(dst);
    const intptr_t* srcPtr = reinterpret_cast<const intptr_t*>(src);
    size /= sizeof(intptr_t);
    while (size--)
        *dstPtr++ = *srcPtr++;
}
    


// This function must not call malloc(), free(), or any other function that might
// acquire a lock. Since 'thread' is suspended, trying to acquire a lock
// will deadlock if 'thread' holds that lock.
// This function, specifically the memory copying, was causing problems with Address Sanitizer in
// apps. Since we cannot blacklist the system memcpy we must use our own naive implementation,
// copyMemory, for ASan to work on either instrumented or non-instrumented builds. This is not a
// significant performance loss as tryCopyOtherThreadStack is only called as part of an O(heapsize)
// operation. As the heap is generally much larger than the stack the performance hit is minimal.
// See: https://bugs.webkit.org/show_bug.cgi?id=146297
void MachineThreads::tryCopyOtherThreadStack(Thread* thread, void* buffer, size_t capacity, size_t* size)
{
    Thread::Registers registers;
    size_t registersSize = thread->getRegisters(registers);
    std::pair<void*, size_t> stack = thread->captureStack(registers.stackPointer());

    bool canCopy = *size + registersSize + stack.second <= capacity;

    if (canCopy)
        copyMemory(static_cast<char*>(buffer) + *size, &registers, registersSize);
    *size += registersSize;

    if (canCopy)
        copyMemory(static_cast<char*>(buffer) + *size, stack.first, stack.second);
    *size += stack.second;

    thread->freeRegisters(registers);
}

bool MachineThreads::tryCopyOtherThreadStacks(const AbstractLocker&, void* buffer, size_t capacity, size_t* size)
{
    // Prevent two VMs from suspending each other's threads at the same time,
    // which can cause deadlock: <rdar://problem/20300842>.
    static StaticLock mutex;
    std::lock_guard<StaticLock> lock(mutex);

    *size = 0;

    PlatformThread platformThread = currentPlatformThread();
    int numberOfThreads = 0; // Using 0 to denote that we haven't counted the number of threads yet.
    int index = 1;
    Thread* threadsToBeDeleted = nullptr;

    Thread* previousThread = nullptr;
    for (Thread* thread = m_registeredThreads; thread; index++) {
        if (*thread != platformThread) {
            bool success = thread->suspend();
#if OS(DARWIN)
            if (!success) {
                if (!numberOfThreads) {
                    for (Thread* countedThread = m_registeredThreads; countedThread; countedThread = countedThread->next)
                        numberOfThreads++;
                }
                
                // Re-do the suspension to get the actual failure result for logging.
                kern_return_t error = thread_suspend(thread->platformThread);
                ASSERT(error != KERN_SUCCESS);

                WTFReportError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION,
                    "JavaScript garbage collection encountered an invalid thread (err 0x%x): Thread [%d/%d: %p] platformThread %p.",
                    error, index, numberOfThreads, thread, reinterpret_cast<void*>(thread->platformThread));

                // Put the invalid thread on the threadsToBeDeleted list.
                // We can't just delete it here because we have suspended other
                // threads, and they may still be holding the C heap lock which
                // we need for deleting the invalid thread. Hence, we need to
                // defer the deletion till after we have resumed all threads.
                Thread* nextThread = thread->next;
                thread->next = threadsToBeDeleted;
                threadsToBeDeleted = thread;

                if (previousThread)
                    previousThread->next = nextThread;
                else
                    m_registeredThreads = nextThread;
                thread = nextThread;
                continue;
            }
#else
            UNUSED_PARAM(numberOfThreads);
            UNUSED_PARAM(previousThread);
            ASSERT_UNUSED(success, success);
#endif
        }
        previousThread = thread;
        thread = thread->next;
    }

    for (Thread* thread = m_registeredThreads; thread; thread = thread->next) {
        if (*thread != platformThread)
            tryCopyOtherThreadStack(thread, buffer, capacity, size);
    }

    for (Thread* thread = m_registeredThreads; thread; thread = thread->next) {
        if (*thread != platformThread)
            thread->resume();
    }

    for (Thread* thread = threadsToBeDeleted; thread; ) {
        Thread* nextThread = thread->next;
        delete thread;
        thread = nextThread;
    }
    
    return *size <= capacity;
}

static void growBuffer(size_t size, void** buffer, size_t* capacity)
{
    if (*buffer)
        fastFree(*buffer);

    *capacity = WTF::roundUpToMultipleOf(WTF::pageSize(), size * 2);
    *buffer = fastMalloc(*capacity);
}

void MachineThreads::gatherConservativeRoots(ConservativeRoots& conservativeRoots, JITStubRoutineSet& jitStubRoutines, CodeBlockSet& codeBlocks, CurrentThreadState* currentThreadState)
{
    if (currentThreadState)
        gatherFromCurrentThread(conservativeRoots, jitStubRoutines, codeBlocks, *currentThreadState);

    size_t size;
    size_t capacity = 0;
    void* buffer = nullptr;
    LockHolder lock(m_registeredThreadsMutex);
    while (!tryCopyOtherThreadStacks(lock, buffer, capacity, &size))
        growBuffer(size, &buffer, &capacity);

    if (!buffer)
        return;

    conservativeRoots.add(buffer, static_cast<char*>(buffer) + size, jitStubRoutines, codeBlocks);
    fastFree(buffer);
}

NEVER_INLINE int callWithCurrentThreadState(const ScopedLambda<void(CurrentThreadState&)>& lambda)
{
    DECLARE_AND_COMPUTE_CURRENT_THREAD_STATE(state);
    lambda(state);
    return 42; // Suppress tail call optimization.
}

} // namespace JSC
