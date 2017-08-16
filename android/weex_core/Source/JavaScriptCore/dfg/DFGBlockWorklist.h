/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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

#include "DFGBasicBlock.h"
#include "DFGBlockSet.h"
#include <wtf/GraphNodeWorklist.h>

namespace JSC { namespace DFG {

struct BasicBlock;

typedef GraphNodeWorklist<BasicBlock*, BlockSet> BlockWorklist;

// When you say BlockWith<int> you should read it as "block with an int".
template<typename T> using BlockWith = GraphNodeWith<BasicBlock*, T>;

// Extended block worklist is useful for enqueueing some meta-data along with the block. It also
// permits forcibly enqueueing things even if the block has already been seen. It's useful for
// things like building a spanning tree, in which case T (the auxiliary payload) would be the
// successor index.
template<typename T> using ExtendedBlockWorklist = ExtendedGraphNodeWorklist<BasicBlock*, T, BlockSet>;

typedef GraphVisitOrder VisitOrder;

typedef GraphNodeWithOrder<BasicBlock*> BlockWithOrder;

typedef PostOrderGraphNodeWorklist<BasicBlock*, BlockSet> PostOrderBlockWorklist;

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
