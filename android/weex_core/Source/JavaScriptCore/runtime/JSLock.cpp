/*
 * Copyright (C) 2005-2017 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the NU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA 
 *
 */

#include "config.h"
#include "JSLock.h"

#include "Heap.h"
#include "CallFrame.h"
#include "JSGlobalObject.h"
#include "JSObject.h"
#include "JSCInlines.h"
#include "MachineStackMarker.h"
#include "SamplingProfiler.h"
#include <thread>

namespace JSC {

StaticLock GlobalJSLock::s_sharedInstanceMutex;

GlobalJSLock::GlobalJSLock()
{
    s_sharedInstanceMutex.lock();
}

GlobalJSLock::~GlobalJSLock()
{
    s_sharedInstanceMutex.unlock();
}

JSLockHolder::JSLockHolder(ExecState* exec)
    : m_vm(&exec->vm())
{
    init();
}

JSLockHolder::JSLockHolder(VM* vm)
    : m_vm(vm)
{
    init();
}

JSLockHolder::JSLockHolder(VM& vm)
    : m_vm(&vm)
{
    init();
}

void JSLockHolder::init()
{
    m_vm->apiLock().lock();
}

JSLockHolder::~JSLockHolder()
{
    RefPtr<JSLock> apiLock(&m_vm->apiLock());
    m_vm = nullptr;
    apiLock->unlock();
}

JSLock::JSLock(VM* vm)
    : m_lockCount(0)
    , m_lockDropDepth(0)
    , m_vm(vm)
    , m_entryAtomicStringTable(nullptr)
{
}

JSLock::~JSLock()
{
}

void JSLock::willDestroyVM(VM* vm)
{
    ASSERT_UNUSED(vm, m_vm == vm);
    m_vm = nullptr;
}

void JSLock::lock()
{
    lock(1);
}

void JSLock::lock(intptr_t lockCount)
{
    ASSERT(lockCount > 0);
    bool success = m_lock.tryLock();
    if (UNLIKELY(!success)) {
        if (currentThreadIsHoldingLock()) {
            m_lockCount += lockCount;
            return;
        }
        m_lock.lock();
    }

    m_ownerThread = currentPlatformThread();
    WTF::storeStoreFence();
    m_hasOwnerThread = true;
    ASSERT(!m_lockCount);
    m_lockCount = lockCount;

    didAcquireLock();
}

void JSLock::didAcquireLock()
{
    // FIXME: What should happen to the per-thread identifier table if we don't have a VM?
    if (!m_vm)
        return;
    
    WTFThreadData& threadData = wtfThreadData();
    ASSERT(!m_entryAtomicStringTable);
    m_entryAtomicStringTable = threadData.setCurrentAtomicStringTable(m_vm->atomicStringTable());
    ASSERT(m_entryAtomicStringTable);

    if (m_vm->heap.hasAccess())
        m_shouldReleaseHeapAccess = false;
    else {
        m_vm->heap.acquireAccess();
        m_shouldReleaseHeapAccess = true;
    }

    RELEASE_ASSERT(!m_vm->stackPointerAtVMEntry());
    void* p = &p; // A proxy for the current stack pointer.
    m_vm->setStackPointerAtVMEntry(p);

    m_vm->setLastStackTop(threadData.savedLastStackTop());

    m_vm->heap.machineThreads().addCurrentThread();

    m_vm->traps().notifyGrabAllLocks();

#if ENABLE(SAMPLING_PROFILER)
    // Note: this must come after addCurrentThread().
    if (SamplingProfiler* samplingProfiler = m_vm->samplingProfiler())
        samplingProfiler->noticeJSLockAcquisition();
#endif
}

void JSLock::unlock()
{
    unlock(1);
}

void JSLock::unlock(intptr_t unlockCount)
{
    RELEASE_ASSERT(currentThreadIsHoldingLock());
    ASSERT(m_lockCount >= unlockCount);

    // Maintain m_lockCount while calling willReleaseLock() so that its callees know that
    // they still have the lock.
    if (unlockCount == m_lockCount)
        willReleaseLock();

    m_lockCount -= unlockCount;

    if (!m_lockCount) {
        m_hasOwnerThread = false;
        m_lock.unlock();
    }
}

void JSLock::willReleaseLock()
{
    RefPtr<VM> vm = m_vm;
    if (vm) {
        vm->drainMicrotasks();

        vm->heap.releaseDelayedReleasedObjects();
        vm->setStackPointerAtVMEntry(nullptr);
        
        if (m_shouldReleaseHeapAccess)
            vm->heap.releaseAccess();
    }

    if (m_entryAtomicStringTable) {
        wtfThreadData().setCurrentAtomicStringTable(m_entryAtomicStringTable);
        m_entryAtomicStringTable = nullptr;
    }
}

void JSLock::lock(ExecState* exec)
{
    exec->vm().apiLock().lock();
}

void JSLock::unlock(ExecState* exec)
{
    exec->vm().apiLock().unlock();
}

// This function returns the number of locks that were dropped.
unsigned JSLock::dropAllLocks(DropAllLocks* dropper)
{
    if (!currentThreadIsHoldingLock())
        return 0;

    ++m_lockDropDepth;

    dropper->setDropDepth(m_lockDropDepth);

    WTFThreadData& threadData = wtfThreadData();
    threadData.setSavedStackPointerAtVMEntry(m_vm->stackPointerAtVMEntry());
    threadData.setSavedLastStackTop(m_vm->lastStackTop());

    unsigned droppedLockCount = m_lockCount;
    unlock(droppedLockCount);

    return droppedLockCount;
}

void JSLock::grabAllLocks(DropAllLocks* dropper, unsigned droppedLockCount)
{
    // If no locks were dropped, nothing to do!
    if (!droppedLockCount)
        return;

    ASSERT(!currentThreadIsHoldingLock());
    lock(droppedLockCount);

    while (dropper->dropDepth() != m_lockDropDepth) {
        unlock(droppedLockCount);
        std::this_thread::yield();
        lock(droppedLockCount);
    }

    --m_lockDropDepth;

    WTFThreadData& threadData = wtfThreadData();
    m_vm->setStackPointerAtVMEntry(threadData.savedStackPointerAtVMEntry());
    m_vm->setLastStackTop(threadData.savedLastStackTop());
}

JSLock::DropAllLocks::DropAllLocks(VM* vm)
    : m_droppedLockCount(0)
    // If the VM is in the middle of being destroyed then we don't want to resurrect it
    // by allowing DropAllLocks to ref it. By this point the JSLock has already been 
    // released anyways, so it doesn't matter that DropAllLocks is a no-op.
    , m_vm(vm->refCount() ? vm : nullptr)
{
    if (!m_vm)
        return;
    RELEASE_ASSERT(!m_vm->apiLock().currentThreadIsHoldingLock() || !m_vm->isCollectorBusyOnCurrentThread());
    m_droppedLockCount = m_vm->apiLock().dropAllLocks(this);
}

JSLock::DropAllLocks::DropAllLocks(ExecState* exec)
    : DropAllLocks(exec ? &exec->vm() : nullptr)
{
}

JSLock::DropAllLocks::DropAllLocks(VM& vm)
    : DropAllLocks(&vm)
{
}

JSLock::DropAllLocks::~DropAllLocks()
{
    if (!m_vm)
        return;
    m_vm->apiLock().grabAllLocks(this, m_droppedLockCount);
}

} // namespace JSC
