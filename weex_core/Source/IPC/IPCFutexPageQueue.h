/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#ifndef IPCFUTEXPAGEQUEUE_H
#define IPCFUTEXPAGEQUEUE_H

#include <stdint.h>

// shared memory page layout:
// futex uint32_t
// state word uint32_t
// package size uint32_t
// msg type uint32_t
// types[] uint32_t[]
//
// data  whatever types indicate
// A page queue is composed of m_pagesCount pages
// and will use repeatedly.
class IPCFutexPageQueue {
public:
    IPCFutexPageQueue(void* sharedMemory, size_t s, size_t id);
    ~IPCFutexPageQueue();
    void stepWrite();
    void lockReadPage();
    void unlockReadPageAndStep();
    void spinWaitPeer();
    inline void* getCurrentReadPage() { return sizeof(uint32_t) * 2 + static_cast<char*>(getPage(m_currentRead)); }
    inline void* getCurrentWritePage() { return sizeof(uint32_t) * 2 + static_cast<char*>(getPage(m_currentWrite)); }
    inline size_t getPageSize() const { return m_pageSize - sizeof(uint32_t) * 2; }

    static const size_t ipc_size = 4 * 1024 * 1024;

private:
    void unlock(size_t id);
    void lock(size_t id, bool checkFinish);
    void* getPage(size_t id);
    inline size_t step(size_t s) { return (s + 2) & (m_pagesCount - 1); }
    void setFinishedTag();
    void clearFinishedTag();

    size_t m_currentWrite;
    size_t m_currentRead;
    size_t m_pageSize;
    void* m_sharedMemory;
    int m_tid;
    static const uint32_t m_finishTag = static_cast<uint32_t>(1);
    static const size_t m_pagesCount = 16;
    static const int m_timeoutSec = 32;
};

#endif /* IPCFUTEXPAGEQUEUE_H */
