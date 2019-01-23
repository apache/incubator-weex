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

