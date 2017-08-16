/*
 * Copyright (C) 2012-2016 Apple Inc. All rights reserved.
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

#pragma once

#include <wtf/Atomics.h>
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/PrintStream.h>
#include <wtf/SentinelLinkedList.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace JSC {

class FireDetail {
    void* operator new(size_t) = delete;
    
public:
    FireDetail()
    {
    }
    
    virtual ~FireDetail()
    {
    }
    
    virtual void dump(PrintStream&) const = 0;
};

class StringFireDetail : public FireDetail {
public:
    StringFireDetail(const char* string)
        : m_string(string)
    {
    }
    
    void dump(PrintStream& out) const override;

private:
    const char* m_string;
};

class WatchpointSet;

class Watchpoint : public BasicRawSentinelNode<Watchpoint> {
    WTF_MAKE_NONCOPYABLE(Watchpoint);
    WTF_MAKE_FAST_ALLOCATED;
public:
    Watchpoint()
    {
    }
    
    virtual ~Watchpoint();

protected:
    virtual void fireInternal(const FireDetail&) = 0;

private:
    friend class WatchpointSet;
    void fire(const FireDetail&);
};

enum WatchpointState {
    ClearWatchpoint,
    IsWatched,
    IsInvalidated
};

class InlineWatchpointSet;
class VM;

class WatchpointSet : public ThreadSafeRefCounted<WatchpointSet> {
    friend class LLIntOffsetsExtractor;
public:
    JS_EXPORT_PRIVATE WatchpointSet(WatchpointState);
    
    // FIXME: In many cases, it would be amazing if this *did* fire the watchpoints. I suspect that
    // this might be hard to get right, but still, it might be awesome.
    JS_EXPORT_PRIVATE ~WatchpointSet(); // Note that this will not fire any of the watchpoints; if you need to know when a WatchpointSet dies then you need a separate mechanism for this.
    
    // Fast way of getting the state, which only works from the main thread.
    WatchpointState stateOnJSThread() const
    {
        return static_cast<WatchpointState>(m_state);
    }
    
    // It is safe to call this from another thread. It may return an old
    // state. Guarantees that if *first* read the state() of the thing being
    // watched and it returned IsWatched and *second* you actually read its
    // value then it's safe to assume that if the state being watched changes
    // then also the watchpoint state() will change to IsInvalidated.
    WatchpointState state() const
    {
        WTF::loadLoadFence();
        WatchpointState result = static_cast<WatchpointState>(m_state);
        WTF::loadLoadFence();
        return result;
    }
    
    // It is safe to call this from another thread.  It may return true
    // even if the set actually had been invalidated, but that ought to happen
    // only in the case of races, and should be rare. Guarantees that if you
    // call this after observing something that must imply that the set is
    // invalidated, then you will see this return false. This is ensured by
    // issuing a load-load fence prior to querying the state.
    bool isStillValid() const
    {
        return state() != IsInvalidated;
    }
    // Like isStillValid(), may be called from another thread.
    bool hasBeenInvalidated() const { return !isStillValid(); }
    
    // As a convenience, this will ignore 0. That's because code paths in the DFG
    // that create speculation watchpoints may choose to bail out if speculation
    // had already been terminated.
    void add(Watchpoint*);
    
    // Force the watchpoint set to behave as if it was being watched even if no
    // watchpoints have been installed. This will result in invalidation if the
    // watchpoint would have fired. That's a pretty good indication that you
    // probably don't want to set watchpoints, since we typically don't want to
    // set watchpoints that we believe will actually be fired.
    void startWatching()
    {
        ASSERT(m_state != IsInvalidated);
        if (m_state == IsWatched)
            return;
        WTF::storeStoreFence();
        m_state = IsWatched;
        WTF::storeStoreFence();
    }
    
    void fireAll(VM& vm, const FireDetail& detail)
    {
        if (LIKELY(m_state != IsWatched))
            return;
        fireAllSlow(vm, detail);
    }
    
    void fireAll(VM& vm, const char* reason)
    {
        if (LIKELY(m_state != IsWatched))
            return;
        fireAllSlow(vm, reason);
    }
    
    void touch(VM& vm, const FireDetail& detail)
    {
        if (state() == ClearWatchpoint)
            startWatching();
        else
            fireAll(vm, detail);
    }
    
    void touch(VM& vm, const char* reason)
    {
        touch(vm, StringFireDetail(reason));
    }
    
    void invalidate(VM& vm, const FireDetail& detail)
    {
        if (state() == IsWatched)
            fireAll(vm, detail);
        m_state = IsInvalidated;
    }
    
    void invalidate(VM& vm, const char* reason)
    {
        invalidate(vm, StringFireDetail(reason));
    }
    
    bool isBeingWatched() const
    {
        return m_setIsNotEmpty;
    }
    
    int8_t* addressOfState() { return &m_state; }
    static ptrdiff_t offsetOfState() { return OBJECT_OFFSETOF(WatchpointSet, m_state); }
    int8_t* addressOfSetIsNotEmpty() { return &m_setIsNotEmpty; }
    
    JS_EXPORT_PRIVATE void fireAllSlow(VM&, const FireDetail&); // Call only if you've checked isWatched.
    JS_EXPORT_PRIVATE void fireAllSlow(VM&, const char* reason); // Ditto.
    
private:
    void fireAllWatchpoints(VM&, const FireDetail&);
    
    friend class InlineWatchpointSet;

    int8_t m_state;
    int8_t m_setIsNotEmpty;

    SentinelLinkedList<Watchpoint, BasicRawSentinelNode<Watchpoint>> m_set;
};

// InlineWatchpointSet is a low-overhead, non-copyable watchpoint set in which
// it is not possible to quickly query whether it is being watched in a single
// branch. There is a fairly simple tradeoff between WatchpointSet and
// InlineWatchpointSet:
//
// Do you have to emit JIT code that rapidly tests whether the watchpoint set
// is being watched?  If so, use WatchpointSet.
//
// Do you need multiple parties to have pointers to the same WatchpointSet?
// If so, use WatchpointSet.
//
// Do you have to allocate a lot of watchpoint sets?  If so, use
// InlineWatchpointSet unless you answered "yes" to the previous questions.
//
// InlineWatchpointSet will use just one pointer-width word of memory unless
// you actually add watchpoints to it, in which case it internally inflates
// to a pointer to a WatchpointSet, and transfers its state to the
// WatchpointSet.

class InlineWatchpointSet {
    WTF_MAKE_NONCOPYABLE(InlineWatchpointSet);
public:
    InlineWatchpointSet(WatchpointState state)
        : m_data(encodeState(state))
    {
    }
    
    ~InlineWatchpointSet()
    {
        if (isThin())
            return;
        freeFat();
    }
    
    // Fast way of getting the state, which only works from the main thread.
    WatchpointState stateOnJSThread() const
    {
        uintptr_t data = m_data;
        if (isFat(data))
            return fat(data)->stateOnJSThread();
        return decodeState(data);
    }

    // It is safe to call this from another thread. It may return a prior state,
    // but that should be fine since you should only perform actions based on the
    // state if you also add a watchpoint.
    WatchpointState state() const
    {
        WTF::loadLoadFence();
        uintptr_t data = m_data;
        WTF::loadLoadFence();
        if (isFat(data))
            return fat(data)->state();
        return decodeState(data);
    }
    
    // It is safe to call this from another thread.  It may return false
    // even if the set actually had been invalidated, but that ought to happen
    // only in the case of races, and should be rare.
    bool hasBeenInvalidated() const
    {
        return state() == IsInvalidated;
    }
    
    // Like hasBeenInvalidated(), may be called from another thread.
    bool isStillValid() const
    {
        return !hasBeenInvalidated();
    }
    
    void add(Watchpoint*);
    
    void startWatching()
    {
        if (isFat()) {
            fat()->startWatching();
            return;
        }
        ASSERT(decodeState(m_data) != IsInvalidated);
        m_data = encodeState(IsWatched);
    }
    
    void fireAll(VM& vm, const FireDetail& detail)
    {
        if (isFat()) {
            fat()->fireAll(vm, detail);
            return;
        }
        if (decodeState(m_data) == ClearWatchpoint)
            return;
        m_data = encodeState(IsInvalidated);
        WTF::storeStoreFence();
    }
    
    void invalidate(VM& vm, const FireDetail& detail)
    {
        if (isFat())
            fat()->invalidate(vm, detail);
        else
            m_data = encodeState(IsInvalidated);
    }
    
    JS_EXPORT_PRIVATE void fireAll(VM&, const char* reason);
    
    void touch(VM& vm, const FireDetail& detail)
    {
        if (isFat()) {
            fat()->touch(vm, detail);
            return;
        }
        uintptr_t data = m_data;
        if (decodeState(data) == IsInvalidated)
            return;
        WTF::storeStoreFence();
        if (decodeState(data) == ClearWatchpoint)
            m_data = encodeState(IsWatched);
        else
            m_data = encodeState(IsInvalidated);
        WTF::storeStoreFence();
    }
    
    void touch(VM& vm, const char* reason)
    {
        touch(vm, StringFireDetail(reason));
    }

    // Note that for any watchpoint that is visible from the DFG, it would be incorrect to write code like:
    //
    // if (w.isBeingWatched())
    //     w.fireAll()
    //
    // Concurrently to this, the DFG could do:
    //
    // if (w.isStillValid())
    //     perform optimizations;
    // if (!w.isStillValid())
    //     retry compilation;
    //
    // Note that the DFG algorithm is widespread, and sound, because fireAll() and invalidate() will leave
    // the watchpoint in a !isStillValid() state. Hence, if fireAll() or invalidate() interleaved between
    // the first isStillValid() check and the second one, then it would simply cause the DFG to retry
    // compilation later.
    //
    // But, if you change some piece of state that the DFG might optimize for, but invalidate the
    // watchpoint by doing:
    //
    // if (w.isBeingWatched())
    //     w.fireAll()
    //
    // then the DFG would never know that you invalidated state between the two checks.
    //
    // There are two ways to work around this:
    //
    // - Call fireAll() without a isBeingWatched() check. Then, the DFG will know that the watchpoint has
    //   been invalidated when it does its second check.
    //
    // - Do not expose the watchpoint set to the DFG directly, and have your own way of validating whether
    //   the assumptions that the DFG thread used are still valid when the DFG code is installed.
    bool isBeingWatched() const
    {
        if (isFat())
            return fat()->isBeingWatched();
        return false;
    }
    
private:
    static const uintptr_t IsThinFlag        = 1;
    static const uintptr_t StateMask         = 6;
    static const uintptr_t StateShift        = 1;
    
    static bool isThin(uintptr_t data) { return data & IsThinFlag; }
    static bool isFat(uintptr_t data) { return !isThin(data); }
    
    static WatchpointState decodeState(uintptr_t data)
    {
        ASSERT(isThin(data));
        return static_cast<WatchpointState>((data & StateMask) >> StateShift);
    }
    
    static uintptr_t encodeState(WatchpointState state)
    {
        return (static_cast<uintptr_t>(state) << StateShift) | IsThinFlag;
    }
    
    bool isThin() const { return isThin(m_data); }
    bool isFat() const { return isFat(m_data); };
    
    static WatchpointSet* fat(uintptr_t data)
    {
        return bitwise_cast<WatchpointSet*>(data);
    }
    
    WatchpointSet* fat()
    {
        ASSERT(isFat());
        return fat(m_data);
    }
    
    const WatchpointSet* fat() const
    {
        ASSERT(isFat());
        return fat(m_data);
    }
    
    WatchpointSet* inflate()
    {
        if (LIKELY(isFat()))
            return fat();
        return inflateSlow();
    }
    
    JS_EXPORT_PRIVATE WatchpointSet* inflateSlow();
    JS_EXPORT_PRIVATE void freeFat();
    
    uintptr_t m_data;
};

} // namespace JSC
