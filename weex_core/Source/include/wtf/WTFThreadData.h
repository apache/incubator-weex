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
#ifndef WTFThreadData_h
#define WTFThreadData_h

#include <wtf/FastTLS.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/Noncopyable.h>
#include <wtf/StackBounds.h>
#include <wtf/StackStats.h>
#include <wtf/ThreadSpecific.h>
#include <wtf/Threading.h>
#include <wtf/text/StringHash.h>

namespace WTF {

class AtomicStringTable;

typedef void (*AtomicStringTableDestructor)(AtomicStringTable*);

class WTFThreadData {
    WTF_MAKE_NONCOPYABLE(WTFThreadData);
public:
    WTF_EXPORT_PRIVATE WTFThreadData();
    WTF_EXPORT_PRIVATE ~WTFThreadData();

    AtomicStringTable* atomicStringTable()
    {
        return m_currentAtomicStringTable;
    }

    AtomicStringTable* setCurrentAtomicStringTable(AtomicStringTable* atomicStringTable)
    {
        AtomicStringTable* oldAtomicStringTable = m_currentAtomicStringTable;
        m_currentAtomicStringTable = atomicStringTable;
        return oldAtomicStringTable;
    }

    const StackBounds& stack()
    {
        // We need to always get a fresh StackBounds from the OS due to how fibers work.
        // See https://bugs.webkit.org/show_bug.cgi?id=102411
#if OS(WINDOWS)
        m_stackBounds = StackBounds::currentThreadStackBounds();
#endif
        return m_stackBounds;
    }

#if ENABLE(STACK_STATS)
    StackStats::PerThreadStats& stackStats()
    {
        return m_stackStats;
    }
#endif

    void* savedStackPointerAtVMEntry()
    {
        return m_savedStackPointerAtVMEntry;
    }

    void setSavedStackPointerAtVMEntry(void* stackPointerAtVMEntry)
    {
        m_savedStackPointerAtVMEntry = stackPointerAtVMEntry;
    }

    void* savedLastStackTop()
    {
        return m_savedLastStackTop;
    }

    void setSavedLastStackTop(void* lastStackTop)
    {
        m_savedLastStackTop = lastStackTop;
    }

    void* m_apiData;

private:
    AtomicStringTable* m_currentAtomicStringTable;
    AtomicStringTable* m_defaultAtomicStringTable;
    AtomicStringTableDestructor m_atomicStringTableDestructor;

    StackBounds m_stackBounds;
#if ENABLE(STACK_STATS)
    StackStats::PerThreadStats m_stackStats;
#endif
    void* m_savedStackPointerAtVMEntry;
    void* m_savedLastStackTop;

#if HAVE(FAST_TLS)
    WTF_EXPORT_PRIVATE static WTFThreadData& createAndRegisterForGetspecificDirect();
#else
    static WTF_EXPORTDATA ThreadSpecific<WTFThreadData>* staticData;
#endif

    friend WTFThreadData& wtfThreadData();
    friend class AtomicStringTable;
};

inline WTFThreadData& wtfThreadData()
{
    // WRT WebCore:
    //    WTFThreadData is used on main thread before it could possibly be used
    //    on secondary ones, so there is no need for synchronization here.
    // WRT JavaScriptCore:
    //    wtfThreadData() is initially called from initializeThreading(), ensuring
    //    this is initially called in a pthread_once locked context.
#if !HAVE(FAST_TLS)
    if (!WTFThreadData::staticData)
        WTFThreadData::staticData = new ThreadSpecific<WTFThreadData>;
    return **WTFThreadData::staticData;
#else
    if (WTFThreadData* data = static_cast<WTFThreadData*>(_pthread_getspecific_direct(WTF_THREAD_DATA_KEY)))
        return *data;
    return WTFThreadData::createAndRegisterForGetspecificDirect();
#endif
}

} // namespace WTF

using WTF::WTFThreadData;
using WTF::wtfThreadData;
using WTF::AtomicStringTable;

#endif // WTFThreadData_h
