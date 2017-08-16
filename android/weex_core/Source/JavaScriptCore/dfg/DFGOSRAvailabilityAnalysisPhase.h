/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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
#include "DFGCommon.h"

namespace JSC { namespace DFG {

class Graph;

// Computes BasicBlock::ssa->availabiltiyAtHead/Tail. This is a forward flow type inference
// over MovHints and SetLocals. This analysis is run directly by the Plan for preparing for
// lowering to B3 IR, but it can also be used as a utility. Note that if you run it before
// stack layout, all of the flush availability will omit the virtual register - but it will
// tell you the format.

bool performOSRAvailabilityAnalysis(Graph&);

// Local calculator for figuring out the availability at any node in a basic block. Requires
// having run the availability analysis.
class LocalOSRAvailabilityCalculator {
public:
    LocalOSRAvailabilityCalculator(Graph&);
    ~LocalOSRAvailabilityCalculator();
    
    void beginBlock(BasicBlock*);
    void endBlock(BasicBlock*); // Useful if you want to get data for the end of the block. You don't need to call this if you did beginBlock() and then executeNode() for every node.
    void executeNode(Node*);
    
    AvailabilityMap m_availability;
    Graph& m_graph;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
