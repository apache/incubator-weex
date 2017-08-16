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
#include "ICStats.h"

namespace JSC {

bool ICEvent::operator<(const ICEvent& other) const
{
    if (m_classInfo != other.m_classInfo) {
        if (!m_classInfo)
            return true;
        if (!other.m_classInfo)
            return false;
        return strcmp(m_classInfo->className, other.m_classInfo->className) < 0;
    }
    
    if (m_propertyName != other.m_propertyName)
        return codePointCompare(m_propertyName.string(), other.m_propertyName.string()) < 0;
    
    return m_kind < other.m_kind;
}

void ICEvent::dump(PrintStream& out) const
{
    out.print(m_kind, "(", m_classInfo ? m_classInfo->className : "<null>", ", ", m_propertyName, ")");
}

void ICEvent::log() const
{
    ICStats::instance().add(*this);
}

Atomic<ICStats*> ICStats::s_instance;

ICStats::ICStats()
{
    m_thread = createThread(
        "JSC ICStats",
        [this] () {
            LockHolder locker(m_lock);
            for (;;) {
                m_condition.waitFor(
                    m_lock, Seconds(1), [this] () -> bool { return m_shouldStop; });
                if (m_shouldStop)
                    break;
                
                dataLog("ICStats:\n");
                auto list = m_spectrum.buildList();
                for (unsigned i = list.size(); i--;)
                    dataLog("    ", list[i].key, ": ", list[i].count, "\n");
            }
        });
}

ICStats::~ICStats()
{
    {
        LockHolder locker(m_lock);
        m_shouldStop = true;
        m_condition.notifyAll();
    }
    
    waitForThreadCompletion(m_thread);
}

void ICStats::add(const ICEvent& event)
{
    m_spectrum.add(event);
}

ICStats& ICStats::instance()
{
    for (;;) {
        ICStats* result = s_instance.load();
        if (result)
            return *result;
        
        ICStats* newStats = new ICStats();
        if (s_instance.compareExchangeWeak(nullptr, newStats))
            return *newStats;
        
        delete newStats;
    }
}

} // namespace JSC

namespace WTF {

using namespace JSC;

void printInternal(PrintStream& out, ICEvent::Kind kind)
{
    switch (kind) {
#define ICEVENT_KIND_DUMP(name) case ICEvent::name: out.print(#name); return;
        FOR_EACH_ICEVENT_KIND(ICEVENT_KIND_DUMP);
#undef ICEVENT_KIND_DUMP
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF


