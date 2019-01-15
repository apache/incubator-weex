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

#include "DFGBlockMap.h"
#include "DFGGraph.h"

namespace JSC { namespace DFG {

// Returns the set of nodes live at tail, both due to due DFG and due to bytecode (i.e. OSR exit).
NodeSet liveNodesAtHead(Graph&, BasicBlock*);

// WARNING: This currently does not reason about the liveness of shadow values. The execution
// semantics of DFG SSA are that an Upsilon stores to the shadow value of a Phi, and the Phi loads
// from that shadow value. Hence, the shadow values are like variables, and have liveness. The normal
// liveness analysis will tell you about the liveness of shadow values. It's OK to ignore shadow
// values if you treat Upsilon as an opaque escape, and all of the clients of CombinedLiveness do so.
struct CombinedLiveness {
    CombinedLiveness() { }
    
    CombinedLiveness(Graph&);
    
    BlockMap<NodeSet> liveAtHead;
    BlockMap<NodeSet> liveAtTail;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
