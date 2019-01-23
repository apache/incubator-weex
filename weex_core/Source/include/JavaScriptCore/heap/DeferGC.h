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

#include "Heap.h"
#include <wtf/Noncopyable.h>
#include <wtf/ThreadSpecific.h>

namespace JSC {

class DeferGC {
    WTF_MAKE_NONCOPYABLE(DeferGC);
public:
    DeferGC(Heap& heap)
        : m_heap(heap)
    {
        m_heap.incrementDeferralDepth();
    }
    
    ~DeferGC()
    {
        m_heap.decrementDeferralDepthAndGCIfNeeded();
    }

private:
    Heap& m_heap;
};

class DeferGCForAWhile {
    WTF_MAKE_NONCOPYABLE(DeferGCForAWhile);
public:
    DeferGCForAWhile(Heap& heap)
        : m_heap(heap)
    {
        m_heap.incrementDeferralDepth();
    }
    
    ~DeferGCForAWhile()
    {
        m_heap.decrementDeferralDepth();
    }

private:
    Heap& m_heap;
};

class DisallowGC {
    WTF_MAKE_NONCOPYABLE(DisallowGC);
public:
    DisallowGC()
    {
#ifndef NDEBUG
        WTF::threadSpecificSet(s_isGCDisallowedOnCurrentThread, reinterpret_cast<void*>(true));
#endif
    }

    ~DisallowGC()
    {
#ifndef NDEBUG
        WTF::threadSpecificSet(s_isGCDisallowedOnCurrentThread, reinterpret_cast<void*>(false));
#endif
    }

    static bool isGCDisallowedOnCurrentThread()
    {
#ifndef NDEBUG
        return !!WTF::threadSpecificGet(s_isGCDisallowedOnCurrentThread);
#else
        return false;
#endif
    }
    static void initialize()
    {
#ifndef NDEBUG
        WTF::threadSpecificKeyCreate(&s_isGCDisallowedOnCurrentThread, 0);
#endif
    }

#ifndef NDEBUG
    JS_EXPORT_PRIVATE static WTF::ThreadSpecificKey s_isGCDisallowedOnCurrentThread;
#endif
};

} // namespace JSC
