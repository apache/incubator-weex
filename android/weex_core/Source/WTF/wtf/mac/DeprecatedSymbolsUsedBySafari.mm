/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "HashMap.h"
#include "HashSet.h"
#include "Lock.h"
#include "MainThread.h"
#include "NeverDestroyed.h"
#include "StdLibExtras.h"
#include <stdint.h>

// This file contains deprecated symbols that the last released version of Safari uses.
// Once Safari stops using them, we should remove them.

namespace WTF {

#if (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED < 101200) || (PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED < 100000)
WTF_EXPORT_PRIVATE void callOnMainThread(void (*function)(void*), void* context);
WTF_EXPORT_PRIVATE void cancelCallOnMainThread(void (*function)(void*), void* context);

class MainThreadFunctionTracker {
public:
    static MainThreadFunctionTracker& singleton()
    {
        static std::once_flag onceFlag;

        static LazyNeverDestroyed<MainThreadFunctionTracker> tracker;

        std::call_once(onceFlag, [&] {
            tracker.construct();
        });

        return tracker;
    }

    void callOnMainThread(void (*function)(void*), void* context)
    {
        uint64_t identifier = addFunction(function, context);

        WTF::callOnMainThread([this, function, context, identifier] {
            if (!removeIdentifier(function, context, identifier))
                return;

            function(context);
        });
    }

    void cancelCallOnMainThread(void (*function)(void*), void* context)
    {
        removeFunctions(function, context);
    }

private:
    uint64_t addFunction(void (*function)(void*), void* context)
    {
        LockHolder lockHolder(m_lock);
        uint64_t identifier = ++m_currentIdentifier;

        auto& set = m_functions.add({ function, context }, HashSet<uint64_t> { }).iterator->value;
        set.add(identifier);

        return identifier;
    }

    bool removeIdentifier(void (*function)(void*), void* context, uint64_t identifier)
    {
        LockHolder lockHolder(m_lock);

        auto it = m_functions.find({ function, context });
        if (it == m_functions.end())
            return false;

        auto& set = it->value;
        if (!set.remove(identifier))
            return false;

        if (set.isEmpty())
            m_functions.remove(it);

        return true;
    }

    void removeFunctions(void (*function)(void*), void* context)
    {
        LockHolder lockHolder(m_lock);

        m_functions.remove({ function, context });
    }

    Lock m_lock;
    uint64_t m_currentIdentifier;
    HashMap<std::pair<void (*)(void*), void*>, HashSet<uint64_t>> m_functions;
};

void callOnMainThread(void (*function)(void*), void* context)
{
    MainThreadFunctionTracker::singleton().callOnMainThread(function, context);
}

void cancelCallOnMainThread(void (*function)(void*), void* context)
{
    ASSERT(function);

    MainThreadFunctionTracker::singleton().cancelCallOnMainThread(function, context);
}
#endif

} // namespace WTF
