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

#include "config.h"
#include "AirOptimizeBlockOrder.h"

#if ENABLE(B3_JIT)

#include "AirBlockWorklist.h"
#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"
#include <wtf/BubbleSort.h>

namespace JSC { namespace B3 { namespace Air {

namespace {

class SortedSuccessors {
public:
    SortedSuccessors()
    {
    }

    void append(BasicBlock* block)
    {
        m_successors.append(block);
    }

    void process(BlockWorklist& worklist)
    {
        // We prefer a stable sort, and we don't want it to go off the rails if we see NaN. Also, the number
        // of successors is bounded. In fact, it currently cannot be more than 2. :-)
        bubbleSort(
            m_successors.begin(), m_successors.end(),
            [] (BasicBlock* left, BasicBlock* right) {
                return left->frequency() < right->frequency();
            });

        // Pushing the successors in ascending order of frequency ensures that the very next block we visit
        // is our highest-frequency successor (unless that successor has already been visited).
        for (unsigned i = 0; i < m_successors.size(); ++i)
            worklist.push(m_successors[i]);
        
        m_successors.resize(0);
    }

private:
    Vector<BasicBlock*, 2> m_successors;
};

} // anonymous namespace

Vector<BasicBlock*> blocksInOptimizedOrder(Code& code)
{
    Vector<BasicBlock*> blocksInOrder;

    BlockWorklist fastWorklist;
    SortedSuccessors sortedSuccessors;
    SortedSuccessors sortedSlowSuccessors;
    
    // We expect entrypoint lowering to have already happened.
    RELEASE_ASSERT(code.numEntrypoints());

    auto appendSuccessor = [&] (const FrequentedBlock& block) {
        if (block.isRare())
            sortedSlowSuccessors.append(block.block());
        else
            sortedSuccessors.append(block.block());
    };
    
    // For everything but the first entrypoint, we push them in order of frequency and frequency
    // class.
    for (unsigned i = 1; i < code.numEntrypoints(); ++i)
        appendSuccessor(code.entrypoint(i));
    
    // Always push the primary successor last so that it gets highest priority.
    fastWorklist.push(code.entrypoint(0).block());
    
    while (BasicBlock* block = fastWorklist.pop()) {
        blocksInOrder.append(block);
        for (FrequentedBlock& successor : block->successors())
            appendSuccessor(successor);
        sortedSuccessors.process(fastWorklist);
    }

    BlockWorklist slowWorklist;
    sortedSlowSuccessors.process(slowWorklist);

    while (BasicBlock* block = slowWorklist.pop()) {
        // We might have already processed this block.
        if (fastWorklist.saw(block))
            continue;
        
        blocksInOrder.append(block);
        for (BasicBlock* successor : block->successorBlocks())
            sortedSuccessors.append(successor);
        sortedSuccessors.process(slowWorklist);
    }

    ASSERT(fastWorklist.isEmpty());
    ASSERT(slowWorklist.isEmpty());

    return blocksInOrder;
}

void optimizeBlockOrder(Code& code)
{
    PhaseScope phaseScope(code, "optimizeBlockOrder");

    Vector<BasicBlock*> blocksInOrder = blocksInOptimizedOrder(code);
    
    // Place blocks into Code's block list according to the ordering in blocksInOrder. We do this by leaking
    // all of the blocks and then readopting them.
    for (auto& entry : code.blockList())
        entry.release();

    code.blockList().resize(0);

    for (unsigned i = 0; i < blocksInOrder.size(); ++i) {
        BasicBlock* block = blocksInOrder[i];
        block->setIndex(i);
        code.blockList().append(std::unique_ptr<BasicBlock>(block));
    }

    // Finally, flip any branches that we recognize. It's most optimal if the taken successor does not point
    // at the next block.
    for (BasicBlock* block : code) {
        Inst& branch = block->last();

        // It's somewhat tempting to just say that if the block has two successors and the first arg is
        // invertible, then we can do the optimization. But that's wagging the dog. The fact that an
        // instruction happens to have an argument that is invertible doesn't mean it's a branch, even though
        // it is true that currently only branches have invertible arguments. It's also tempting to say that
        // the /branch flag in AirOpcode.opcodes tells us that something is a branch - except that there,
        // /branch also means Jump. The approach taken here means that if you add new branch instructions and
        // forget about this phase, then at worst your new instructions won't opt into the inversion
        // optimization.  You'll probably realize that as soon as you look at the disassembly, and it
        // certainly won't cause any correctness issues.
        
        switch (branch.kind.opcode) {
        case Branch8:
        case Branch32:
        case Branch64:
        case BranchTest8:
        case BranchTest32:
        case BranchTest64:
        case BranchFloat:
        case BranchDouble:
        case BranchAdd32:
        case BranchAdd64:
        case BranchMul32:
        case BranchMul64:
        case BranchSub32:
        case BranchSub64:
        case BranchNeg32:
        case BranchNeg64:
        case BranchAtomicStrongCAS8:
        case BranchAtomicStrongCAS16:
        case BranchAtomicStrongCAS32:
        case BranchAtomicStrongCAS64:
            if (code.findNextBlock(block) == block->successorBlock(0) && branch.args[0].isInvertible()) {
                std::swap(block->successor(0), block->successor(1));
                branch.args[0] = branch.args[0].inverted();
            }
            break;
            
        default:
            break;
        }
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
