/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

namespace JSC { namespace DFG {

class BlockInsertionSet;
class Graph;
struct BasicBlock;

// Inserts dummy basic blocks before any loop headers that don't already have
// a single non-loop predecessor.

bool performLoopPreHeaderCreation(Graph&);

// Creates a new basic block (a pre-header) that jumps to the given block. All
// predecessors of the given block that aren't dominated by it are rerouted to
// the pre-header.
//
// This function is used internally and it's used in a surgical fashion by
// OSREntrypointCreationPhase.
//
// Note that executing this function requires having an intact dominators
// analysis. You should run that analysis before doing damage to the CFG, even
// if said damage is done before you call this.
BasicBlock* createPreHeader(Graph&, BlockInsertionSet&, BasicBlock*);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
