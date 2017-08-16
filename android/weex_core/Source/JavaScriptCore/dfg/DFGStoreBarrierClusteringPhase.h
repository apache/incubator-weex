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

#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

class Graph;

// Picks up groups of barriers that could be executed in any order with respect to each other and
// places then at the earliest point in the program where the cluster would be correct. This phase
// makes only the first of the cluster be a FencedStoreBarrier while the rest are normal
// StoreBarriers. This phase also removes redundant barriers - for example, the cluster may end up
// with two or more barriers on the same object, in which case it is totally safe for us to drop
// one of them. The reason why this is sound hinges on the "earliest point where the cluster would
// be correct" property. For example, take this input:
//
//     a: Call()
//     b: PutByOffset(@o, @o, @x)
//     c: FencedStoreBarrier(@o)
//     d: PutByOffset(@o, @o, @y)
//     e: FencedStoreBarrier(@o)
//     f: PutByOffset(@p, @p, @z)
//     g: FencedStoreBarrier(@p)
//     h: GetByOffset(@q)
//     i: Call()
//
// The cluster of barriers is @c, @e, and @g. All of the barriers are between two doesGC effects:
// the calls at @a and @i. Because there are no doesGC effects between @a and @i and there is no
// possible control flow entry into this sequence between @ and @i, we could could just execute all
// of the barriers just before @i in any order. The earliest point where the cluster would be
// correct is just after @f, since that's the last operation that needs a barrier. We use the
// earliest to reduce register pressure. When the barriers are clustered just after @f, we get:
//
//     a: Call()
//     b: PutByOffset(@o, @o, @x)
//     d: PutByOffset(@o, @o, @y)
//     f: PutByOffset(@p, @p, @z)
//     c: FencedStoreBarrier(@o)
//     e: FencedStoreBarrier(@o)
//     g: FencedStoreBarrier(@p)
//     h: GetByOffset(@q)
//     i: Call()
//
// This phase does more. It takes advantage of the clustering to remove fences and remove redundant
// barriers. So this phase will output this:
//
//     a: Call()
//     b: PutByOffset(@o, @o, @x)
//     d: PutByOffset(@o, @o, @y)
//     f: PutByOffset(@p, @p, @z)
//     c: FencedStoreBarrier(@o)
//     g: StoreBarrier(@p)
//     h: GetByOffset(@q)
//     i: Call()
//
// This optimization improves both overall throughput and the throughput while the concurrent GC is
// running. In the former, we are simplifying instruction selection for all but the first fence. In
// the latter, we are reducing the cost of all but the first barrier. The first barrier will awlays
// take slow path when there is concurrent GC activity, since the slow path contains the fence. But
// all of the other barriers will only take slow path if they really need to remember the object.
bool performStoreBarrierClustering(Graph&);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

