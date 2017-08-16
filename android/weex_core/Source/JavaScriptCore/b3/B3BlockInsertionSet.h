/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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

#if ENABLE(B3_JIT)

#include "B3GenericBlockInsertionSet.h"
#include "B3Procedure.h"
#include <wtf/Insertion.h>
#include <wtf/Vector.h>

namespace JSC { namespace B3 {

class InsertionSet;

typedef GenericBlockInsertionSet<BasicBlock>::BlockInsertion BlockInsertion;

class BlockInsertionSet : public GenericBlockInsertionSet<BasicBlock> {
public:
    BlockInsertionSet(Procedure&);
    ~BlockInsertionSet();
    
    // A helper to split a block when forward iterating over it. It creates a new block to hold
    // everything before the instruction at valueIndex. The current block is left with
    // everything at and after valueIndex. If the optional InsertionSet is provided, it will get
    // executed on the newly created block - this makes sense if you had previously inserted
    // things into the original block, since the newly created block will be indexed identically
    // to how this block was indexed for all values prior to valueIndex. After this runs, it sets
    // valueIndex to zero. This allows you to use this method for things like:
    //
    // for (unsigned valueIndex = 0; valueIndex < block->size(); ++valueIndex) {
    //     Value* value = block->at(valueIndex);
    //     if (value->opcode() == Foo) {
    //         BasicBlock* predecessor =
    //             m_blockInsertionSet.splitForward(block, valueIndex, &m_insertionSet);
    //         ... // Now you can append to predecessor, insert new blocks before 'block', and
    //         ... // you can use m_insertionSet to insert more thing before 'value'.
    //         predecessor->updatePredecessorsAfter();
    //     }
    // }
    //
    // Note how usually this idiom ends in a all to updatePredecessorsAftter(), which ensures
    // that the predecessors involved in any of the new control flow that you've created are up
    // to date.
    BasicBlock* splitForward(
        BasicBlock*, unsigned& valueIndex, InsertionSet* = nullptr,
        double frequency = PNaN);

private:
    Procedure& m_proc;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
