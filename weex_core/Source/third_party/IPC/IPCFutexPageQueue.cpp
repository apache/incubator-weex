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
/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#include "IPCFutexPageQueue.h"
#include "IPCCheck.h"
#include "IPCException.h"
#include "IPCLog.h"
#include "IPCType.h"
#include "futex.h"
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

IPCFutexPageQueue::IPCFutexPageQueue(void* sharedMemory, size_t s, size_t id)
    : m_currentWrite(id)
    , m_currentRead(id ^ 1)
    , m_pageSize(s / m_pagesCount)
    , m_sharedMemory(sharedMemory)
    , m_tid(gettid())
{
    IPC_DCHECK(s == ipc_size);
    IPC_LOGD("id: %zu", id);
    for (int i = m_currentWrite; i < m_pagesCount; i += 2) {
        uint32_t* data = static_cast<uint32_t*>(getPage(i));
        data[1] |= m_finishTag;
    }
    lock(m_currentWrite, true);
}

IPCFutexPageQueue::~IPCFutexPageQueue()
{
    // build a terminate msg.
    uint32_t* data = static_cast<uint32_t*>(getPage(m_currentWrite));
    data[1] = sizeof(uint32_t) * 2;
    data[2] = MSG_TERMINATE;
    data[3] = static_cast<uint32_t>(IPCType::END);
    try {
        unlock(m_currentWrite);
    } catch (IPCException& e) {
        IPC_LOGE("%s", e.msg());
    }
    munmap(m_sharedMemory, m_pageSize << 2);
}

void IPCFutexPageQueue::stepWrite()
{
    IPC_LOGD("stepWrite");
    clearFinishedTag();
    size_t current = m_currentWrite;
    m_currentWrite = step(m_currentWrite);
    lock(m_currentWrite, true);
    unlock(current);
}

void IPCFutexPageQueue::unlock(size_t id)
{
    IPC_LOGD("unlock: %zu", id);
    volatile uint32_t* pageStart = static_cast<volatile uint32_t*>(getPage(id));

    uint32_t l = m_tid;
    if (__atomic_compare_exchange_n(pageStart, &l, 0,
            false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        return;
    }
    if ((l & FUTEX_WAITERS) != 0) {
        int futexReturn = __futex(pageStart, FUTEX_UNLOCK_PI, 0, nullptr);
        if (futexReturn == -1) {
            throw IPCException("failed to futex unlock %s", strerror(errno));
        }
        return;
    } else if ((l & FUTEX_TID_MASK) != m_tid) {
        throw IPCException("l is not equal to tid: %d %d", l, m_tid);
    }
    throw IPCException("expected lock value");
}

void IPCFutexPageQueue::lock(size_t id, bool checkFinish)
{
    volatile uint32_t* pageStart = static_cast<volatile uint32_t*>(getPage(id));
    uint32_t l = m_tid;
    uint32_t expected = 0;
    IPC_LOGD("lock: %zu", id);
    // wait for the finished tag;
    if (checkFinish) {
        while (true) {
            uint32_t finishedTag = pageStart[1];
            if (finishedTag == m_finishTag) {
                break;
            }
            struct timespec waitTime = { m_timeoutSec, 0 };
            int futexReturn = __futex(pageStart + 1, FUTEX_WAIT, 0, &waitTime);
            if (futexReturn == -1) {
                int myerrno = errno;
                if (myerrno == EINTR)
                    continue;
                else if (myerrno == ETIMEDOUT) {
                    throw IPCException("IPCFutexPageQueue::lock timeout when waiting for finished");
                } else if (myerrno == EAGAIN) {
                    // become m_finishTag when enter the linux kernel.
                    break;
                } else {
                    throw IPCException("IPCException::lock waiting for finished tag %u: %s", pageStart[1], strerror(myerrno));
                }
            }
            break;
        }
    }
    if (__atomic_compare_exchange_n(pageStart, &expected, l,
            false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        return;
    }
    errno = 0;
    while (true) {
        int futexReturn = __futex(pageStart, FUTEX_LOCK_PI, 1, nullptr);
        if (futexReturn == -1 && errno != EINTR) {

            volatile uint32_t* pageStart0 = static_cast<volatile uint32_t*>(getPage(0));
            volatile uint32_t* pageStart1 = static_cast<volatile uint32_t*>(getPage(1));
            volatile uint32_t* pageStart2 = static_cast<volatile uint32_t*>(getPage(2));
            volatile uint32_t* pageStart3 = static_cast<volatile uint32_t*>(getPage(3));
            throw IPCException("futex lock pi failed: %s, %x %d (%x %x %x %x)", strerror(errno), *pageStart, m_tid, *pageStart0, *pageStart1, *pageStart2, *pageStart3);
        }
        if (futexReturn == -1)
            continue;
        break;
    }
    l = *pageStart;
    if ((l & FUTEX_OWNER_DIED)) {
        unlock(id);
        throw IPCException("original owner has die");
    }
}

void* IPCFutexPageQueue::getPage(size_t id)
{
    size_t offset = id * m_pageSize;
    return static_cast<char*>(m_sharedMemory) + offset;
}

void IPCFutexPageQueue::lockReadPage()
{
    IPC_LOGD("lockReadPage");
    uint32_t* pageStart = static_cast<uint32_t*>(getPage(m_currentRead));
    if (!*pageStart) {
        // this page should be locked.
    }
    lock(m_currentRead, false);
}

void IPCFutexPageQueue::unlockReadPageAndStep()
{
    IPC_LOGD("unlockReadPageAndStep");
    setFinishedTag();
    unlock(m_currentRead);
    m_currentRead = step(m_currentRead);
}

void IPCFutexPageQueue::spinWaitPeer()
{
    volatile uint32_t* pageStart = static_cast<volatile uint32_t*>(getPage(m_currentRead));
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (!*pageStart) {
        sched_yield();
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if ((now.tv_sec - start.tv_sec) > m_timeoutSec)
            throw IPCException("spinWaitPeer timeout");
    }
}

void IPCFutexPageQueue::setFinishedTag()
{
    uint32_t* pageStart = static_cast<uint32_t*>(getPage(m_currentRead));
    uint32_t expected = 0;
    if (__atomic_compare_exchange_n(pageStart + 1, &expected, m_finishTag,
            false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        IPC_LOGD("setFinishedTag:waking writer");
        __futex(pageStart + 1, FUTEX_WAKE, 1, nullptr);
        return;
    } else {
        IPC_LOGD("setFinishedTag unexpected value: %u", expected);
    }
}

void IPCFutexPageQueue::clearFinishedTag()
{
    uint32_t* pageRead = static_cast<uint32_t*>(getPage(m_currentWrite));
    pageRead[1] = 0;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}
