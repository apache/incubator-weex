/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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
#include "Watchpoint.h"

#include "HeapInlines.h"
#include "VM.h"
#include <wtf/CompilationThread.h>

namespace JSC {

void StringFireDetail::dump(PrintStream& out) const
{
    out.print(m_string);
}

Watchpoint::~Watchpoint()
{
    if (isOnList()) {
        // This will happen if we get destroyed before the set fires. That's totally a valid
        // possibility. For example:
        //
        // CodeBlock has a Watchpoint on transition from structure S1. The transition never
        // happens, but the CodeBlock gets destroyed because of GC.
        remove();
    }
}

void Watchpoint::fire(const FireDetail& detail)
{
    RELEASE_ASSERT(!isOnList());
    fireInternal(detail);
}

WatchpointSet::WatchpointSet(WatchpointState state)
    : m_state(state)
    , m_setIsNotEmpty(false)
{
}

WatchpointSet::~WatchpointSet()
{
    // Remove all watchpoints, so that they don't try to remove themselves. Note that we
    // don't fire watchpoints on deletion. We assume that any code that is interested in
    // watchpoints already also separately has a mechanism to make sure that the code is
    // either keeping the watchpoint set's owner alive, or does some weak reference thing.
    while (!m_set.isEmpty())
        m_set.begin()->remove();
}

void WatchpointSet::add(Watchpoint* watchpoint)
{
    ASSERT(!isCompilationThread());
    ASSERT(state() != IsInvalidated);
    if (!watchpoint)
        return;
    m_set.push(watchpoint);
    m_setIsNotEmpty = true;
    m_state = IsWatched;
}

void WatchpointSet::fireAllSlow(VM& vm, const FireDetail& detail)
{
    ASSERT(state() == IsWatched);
    
    WTF::storeStoreFence();
    m_state = IsInvalidated; // Do this first. Needed for adaptive watchpoints.
    fireAllWatchpoints(vm, detail);
    WTF::storeStoreFence();
}

void WatchpointSet::fireAllSlow(VM& vm, const char* reason)
{
    fireAllSlow(vm, StringFireDetail(reason));
}

void WatchpointSet::fireAllWatchpoints(VM& vm, const FireDetail& detail)
{
    // In case there are any adaptive watchpoints, we need to make sure that they see that this
    // watchpoint has been already invalidated.
    RELEASE_ASSERT(hasBeenInvalidated());

    // Firing a watchpoint may cause a GC to happen. This GC could destroy various
    // Watchpoints themselves while they're in the process of firing. It's not safe
    // for most Watchpoints to be destructed while they're in the middle of firing.
    // This GC could also destroy us, and we're not in a safe state to be destroyed.
    // The safest thing to do is to DeferGCForAWhile to prevent this GC from happening.
    DeferGCForAWhile deferGC(vm.heap);
    
    while (!m_set.isEmpty()) {
        Watchpoint* watchpoint = m_set.begin();
        ASSERT(watchpoint->isOnList());
        
        // Removing the Watchpoint before firing it makes it possible to implement watchpoints
        // that add themselves to a different set when they fire. This kind of "adaptive"
        // watchpoint can be used to track some semantic property that is more fine-graiend than
        // what the set can convey. For example, we might care if a singleton object ever has a
        // property called "foo". We can watch for this by checking if its Structure has "foo" and
        // then watching its transitions. But then the watchpoint fires if any property is added.
        // So, before the watchpoint decides to invalidate any code, it can check if it is
        // possible to add itself to the transition watchpoint set of the singleton object's new
        // Structure.
        watchpoint->remove();
        ASSERT(m_set.begin() != watchpoint);
        ASSERT(!watchpoint->isOnList());
        
        watchpoint->fire(detail);
        // After we fire the watchpoint, the watchpoint pointer may be a dangling pointer. That's
        // fine, because we have no use for the pointer anymore.
    }
}

void InlineWatchpointSet::add(Watchpoint* watchpoint)
{
    inflate()->add(watchpoint);
}

void InlineWatchpointSet::fireAll(VM& vm, const char* reason)
{
    fireAll(vm, StringFireDetail(reason));
}

WatchpointSet* InlineWatchpointSet::inflateSlow()
{
    ASSERT(isThin());
    ASSERT(!isCompilationThread());
    WatchpointSet* fat = adoptRef(new WatchpointSet(decodeState(m_data))).leakRef();
    WTF::storeStoreFence();
    m_data = bitwise_cast<uintptr_t>(fat);
    return fat;
}

void InlineWatchpointSet::freeFat()
{
    ASSERT(isFat());
    fat()->deref();
}

} // namespace JSC

