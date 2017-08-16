/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "VMInspector.h"

#include "CodeBlock.h"
#include "CodeBlockSet.h"
#include "HeapInlines.h"
#include "MachineContext.h"
#include <mutex>
#include <wtf/Expected.h>

#if !OS(WINDOWS)
#include <unistd.h>
#endif

namespace JSC {

VMInspector& VMInspector::instance()
{
    static VMInspector* manager;
    static std::once_flag once;
    std::call_once(once, [] {
        manager = new VMInspector();
    });
    return *manager;
}

void VMInspector::add(VM* vm)
{
    auto locker = holdLock(m_lock);
    m_list.append(vm);
}

void VMInspector::remove(VM* vm)
{
    auto locker = holdLock(m_lock);
    m_list.remove(vm);
}

auto VMInspector::lock(Seconds timeout) -> Expected<Locker, Error>
{
    // This function may be called from a signal handler (e.g. via visit()). Hence,
    // it should only use APIs that are safe to call from signal handlers. This is
    // why we use unistd.h's sleep() instead of its alternatives.

    // We'll be doing sleep(1) between tries below. Hence, sleepPerRetry is 1.
    unsigned maxRetries = (timeout < Seconds::infinity()) ? timeout.value() : UINT_MAX;

    Expected<Locker, Error> locker = Locker::tryLock(m_lock);
    unsigned tryCount = 0;
    while (!locker && tryCount < maxRetries) {
        // We want the version of sleep from unistd.h. Cast to disambiguate.
#if !OS(WINDOWS)
        (static_cast<unsigned (*)(unsigned)>(sleep))(1);
#endif
        locker = Locker::tryLock(m_lock);
    }

    if (!locker)
        return makeUnexpected(Error::TimedOut);
    return locker;
}

#if ENABLE(JIT)
static bool ensureIsSafeToLock(Lock& lock)
{
    unsigned maxRetries = 2;
    unsigned tryCount = 0;
    while (tryCount <= maxRetries) {
        bool success = lock.tryLock();
        if (success) {
            lock.unlock();
            return true;
        }
        tryCount++;
    }
    return false;
};
#endif // ENABLE(JIT)

auto VMInspector::isValidExecutableMemory(const VMInspector::Locker&, void* machinePC) -> Expected<bool, Error>
{
#if ENABLE(JIT)
    bool found = false;
    bool hasTimeout = false;
    iterate([&] (VM& vm) -> FunctorStatus {
        auto allocator = vm.executableAllocator;
        auto& lock = allocator.getLock();

        bool isSafeToLock = ensureIsSafeToLock(lock);
        if (!isSafeToLock) {
            hasTimeout = true;
            return FunctorStatus::Continue; // Skip this VM.
        }

        LockHolder executableAllocatorLocker(lock);
        if (allocator.isValidExecutableMemory(executableAllocatorLocker, machinePC)) {
            found = true;
            return FunctorStatus::Done;
        }
        return FunctorStatus::Continue;
    });

    if (!found && hasTimeout)
        return makeUnexpected(Error::TimedOut);
    return found;
#else
    UNUSED_PARAM(machinePC);
    return false;
#endif
}

auto VMInspector::codeBlockForMachinePC(const VMInspector::Locker&, void* machinePC) -> Expected<CodeBlock*, Error>
{
#if ENABLE(JIT)
    CodeBlock* codeBlock = nullptr;
    bool hasTimeout = false;
    iterate([&] (VM& vm) {
        if (!vm.currentThreadIsHoldingAPILock())
            return FunctorStatus::Continue;

        // It is safe to call Heap::forEachCodeBlockIgnoringJITPlans here because:
        // 1. CodeBlocks are added to the CodeBlockSet from the main thread before
        //    they are handed to the JIT plans. Those codeBlocks will have a null jitCode,
        //    but we check for that in our lambda functor.
        // 2. We will acquire the CodeBlockSet lock before iterating.
        //    This ensures that a CodeBlock won't be GCed while we're iterating.
        // 3. We do a tryLock on the CodeBlockSet's lock first to ensure that it is
        //    safe for the current thread to lock it before calling
        //    Heap::forEachCodeBlockIgnoringJITPlans(). Hence, there's no risk of
        //    re-entering the lock and deadlocking on it.

        auto& codeBlockSetLock = vm.heap.codeBlockSet().getLock();
        bool isSafeToLock = ensureIsSafeToLock(codeBlockSetLock);
        if (!isSafeToLock) {
            hasTimeout = true;
            return FunctorStatus::Continue; // Skip this VM.
        }

        auto locker = holdLock(codeBlockSetLock);
        vm.heap.forEachCodeBlockIgnoringJITPlans(locker, [&] (CodeBlock* cb) {
            JITCode* jitCode = cb->jitCode().get();
            if (!jitCode) {
                // If the codeBlock is a replacement codeBlock which is in the process of being
                // compiled, its jitCode will be null, and we can disregard it as a match for
                // the machinePC we're searching for.
                return false;
            }

            if (!JITCode::isJIT(jitCode->jitType()))
                return false;

            if (jitCode->contains(machinePC)) {
                codeBlock = cb;
                return true;
            }
            return false;
        });
        if (codeBlock)
            return FunctorStatus::Done;
        return FunctorStatus::Continue;
    });

    if (!codeBlock && hasTimeout)
        return makeUnexpected(Error::TimedOut);
    return codeBlock;
#else
    UNUSED_PARAM(machinePC);
    return nullptr;
#endif
}

} // namespace JSC
