/*
 * Copyright (C) 2008, 2010 Apple Inc. All Rights Reserved.
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
 *
 */

#include "config.h"
#include "WTFThreadData.h"

#include <wtf/text/AtomicStringTable.h>

#if USE(WEB_THREAD)
#include <wtf/MainThread.h>
#endif

namespace WTF {

#if !HAVE(FAST_TLS)
ThreadSpecific<WTFThreadData>* WTFThreadData::staticData;
#endif

WTFThreadData::WTFThreadData()
    : m_apiData(0)
    , m_currentAtomicStringTable(0)
    , m_defaultAtomicStringTable(0)
    , m_atomicStringTableDestructor(0)
    , m_stackBounds(StackBounds::currentThreadStackBounds())
#if ENABLE(STACK_STATS)
    , m_stackStats()
#endif
    , m_savedStackPointerAtVMEntry(0)
    , m_savedLastStackTop(stack().origin())
{
    AtomicStringTable::create(*this);
    m_currentAtomicStringTable = m_defaultAtomicStringTable;
}

WTFThreadData::~WTFThreadData()
{
    if (m_atomicStringTableDestructor)
        m_atomicStringTableDestructor(m_defaultAtomicStringTable);
}

#if HAVE(FAST_TLS)
WTFThreadData& WTFThreadData::createAndRegisterForGetspecificDirect()
{
    WTFThreadData* data = new WTFThreadData;
    _pthread_setspecific_direct(WTF_THREAD_DATA_KEY, data);
    pthread_key_init_np(WTF_THREAD_DATA_KEY, [](void* data){
        delete static_cast<WTFThreadData*>(data);
    });
    return *data;
}
#endif

} // namespace WTF
