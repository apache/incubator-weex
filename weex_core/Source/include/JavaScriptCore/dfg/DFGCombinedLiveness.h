/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
