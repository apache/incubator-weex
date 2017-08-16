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

#pragma once

#if ENABLE(B3_JIT)

#include "B3HeapRange.h"
#include "B3Value.h"

namespace JSC { namespace B3 {

class JS_EXPORT_PRIVATE FenceValue : public Value {
public:
    static bool accepts(Kind kind) { return kind == Fence; }
    
    ~FenceValue();
    
    // The read/write heaps are reflected in the effects() of this value. The compiler may change
    // the lowering of a Fence based on the heaps. For example, if a fence does not write anything
    // then it is understood to be a store-store fence. On x86, this may lead us to not emit any
    // code, while on ARM we may emit a cheaper fence (dmb ishst instead of dmb ish). We will do
    // the same optimization for load-load fences, which are expressed as a Fence that writes but
    // does not read.
    //
    // This abstraction allows us to cover all of the fences on x86 and all of the standalone fences
    // on ARM. X86 really just has one fence: mfence. This fence should be used to protect stores
    // from being sunk below loads. WTF calls it the storeLoadFence. A classic example is the Steele
    // barrier:
    //
    //     o.f = v  =>  o.f = v
    //                  if (color(o) == black)
    //                      log(o)
    //
    // We are trying to ensure that if the store to o.f occurs after the collector has started
    // visiting o, then we will log o. Under sequential consistency, this would work. The collector
    // would set color(o) to black just before it started visiting. But x86's illusion of sequential
    // consistency is broken in exactly just this store->load ordering case. The store to o.f may
    // get buffered, and it may occur some time after we have loaded and checked color(o). As well,
    // the collector's store to set color(o) to black may get buffered and it may occur some time
    // after the collector has finished visiting o. Therefore, we need mfences. In B3 we model this
    // as a Fence that reads and writes some heaps. Setting writes to the empty set will cause B3 to
    // not emit any barrier on x86.
    //
    // On ARM there are many more fences. The Fence instruction is meant to model just two of them:
    // dmb ish and dmb ishst. You can emit a dmb ishst by using a Fence with an empty write heap.
    // Otherwise, you will get a dmb ish.
    // FIXME: Add fenced memory accesses. https://bugs.webkit.org/show_bug.cgi?id=162349
    // FIXME: Add a Depend operation. https://bugs.webkit.org/show_bug.cgi?id=162350
    HeapRange read { HeapRange::top() };
    HeapRange write { HeapRange::top() };

protected:
    Value* cloneImpl() const override;

private:
    friend class Procedure;
    
    FenceValue(Origin origin, HeapRange read, HeapRange write);
    
    FenceValue(Origin origin);
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

