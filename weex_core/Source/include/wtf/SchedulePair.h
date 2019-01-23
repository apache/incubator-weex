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
#ifndef SchedulePair_h
#define SchedulePair_h

#include <wtf/HashSet.h>
#include <wtf/RetainPtr.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

#if PLATFORM(COCOA) && !USE(CFURLCONNECTION)
OBJC_CLASS NSRunLoop;
#endif

namespace WTF {

class SchedulePair : public ThreadSafeRefCounted<SchedulePair> {
public:
    static Ref<SchedulePair> create(CFRunLoopRef runLoop, CFStringRef mode) { return adoptRef(*new SchedulePair(runLoop, mode)); }

#if PLATFORM(COCOA) && !USE(CFURLCONNECTION)
    static Ref<SchedulePair> create(NSRunLoop* runLoop, CFStringRef mode) { return adoptRef(*new SchedulePair(runLoop, mode)); }
    NSRunLoop* nsRunLoop() const { return m_nsRunLoop.get(); }
#endif

    CFRunLoopRef runLoop() const { return m_runLoop.get(); }
    CFStringRef mode() const { return m_mode.get(); }

    WTF_EXPORT_PRIVATE bool operator==(const SchedulePair& other) const;

private:
    SchedulePair(CFRunLoopRef runLoop, CFStringRef mode)
        : m_runLoop(runLoop)
    {
        if (mode)
            m_mode = adoptCF(CFStringCreateCopy(0, mode));
    }

#if PLATFORM(COCOA) && !USE(CFURLCONNECTION)
    WTF_EXPORT_PRIVATE SchedulePair(NSRunLoop*, CFStringRef);
    RetainPtr<NSRunLoop*> m_nsRunLoop;
#endif

    RetainPtr<CFRunLoopRef> m_runLoop;
    RetainPtr<CFStringRef> m_mode;
};

struct SchedulePairHash {
    static unsigned hash(const RefPtr<SchedulePair>& pair)
    {
        uintptr_t hashCodes[2] = { reinterpret_cast<uintptr_t>(pair->runLoop()), pair->mode() ? CFHash(pair->mode()) : 0 };
        return StringHasher::hashMemory<sizeof(hashCodes)>(hashCodes);
    }

    static bool equal(const RefPtr<SchedulePair>& a, const RefPtr<SchedulePair>& b) { return a == b; }

    static const bool safeToCompareToEmptyOrDeleted = true;
};

typedef HashSet<RefPtr<SchedulePair>, SchedulePairHash> SchedulePairHashSet;

} // namespace WTF

using WTF::SchedulePair;
using WTF::SchedulePairHashSet;

#endif
