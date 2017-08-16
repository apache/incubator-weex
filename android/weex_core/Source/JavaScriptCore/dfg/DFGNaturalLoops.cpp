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

#include "config.h"
#include "DFGNaturalLoops.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "JSCInlines.h"
#include <wtf/CommaPrinter.h>

namespace JSC { namespace DFG {

void NaturalLoop::dump(PrintStream& out) const
{
    out.print("[Header: ", *header(), ", Body:");
    for (unsigned i = 0; i < m_body.size(); ++i)
        out.print(" ", *m_body[i]);
    out.print("]");
}

NaturalLoops::NaturalLoops(Graph& graph)
{
    ASSERT(graph.m_dominators);

    // Implement the classic dominator-based natural loop finder. The first
    // step is to find all control flow edges A -> B where B dominates A.
    // Then B is a loop header and A is a backward branching block. We will
    // then accumulate, for each loop header, multiple backward branching
    // blocks. Then we backwards graph search from the backward branching
    // blocks to their loop headers, which gives us all of the blocks in the
    // loop body.
    
    static const bool verbose = false;
    
    if (verbose) {
        dataLog("Dominators:\n");
        graph.m_dominators->dump(WTF::dataFile());
    }
    
    m_loops.resize(0);
    
    for (BlockIndex blockIndex = graph.numBlocks(); blockIndex--;) {
        BasicBlock* block = graph.block(blockIndex);
        if (!block)
            continue;
        
        for (unsigned i = block->numSuccessors(); i--;) {
            BasicBlock* successor = block->successor(i);
            if (!graph.m_dominators->dominates(successor, block))
                continue;
            bool found = false;
            for (unsigned j = m_loops.size(); j--;) {
                if (m_loops[j].header() == successor) {
                    m_loops[j].addBlock(block);
                    found = true;
                    break;
                }
            }
            if (found)
                continue;
            NaturalLoop loop(successor, m_loops.size());
            loop.addBlock(block);
            m_loops.append(loop);
        }
    }
    
    if (verbose)
        dataLog("After bootstrap: ", *this, "\n");
    
    FastBitVector seenBlocks;
    Vector<BasicBlock*, 4> blockWorklist;
    seenBlocks.resize(graph.numBlocks());
    
    for (unsigned i = m_loops.size(); i--;) {
        NaturalLoop& loop = m_loops[i];
        
        seenBlocks.clearAll();
        ASSERT(blockWorklist.isEmpty());
        
        if (verbose)
            dataLog("Dealing with loop ", loop, "\n");
        
        for (unsigned j = loop.size(); j--;) {
            seenBlocks[loop[j]->index] = true;
            blockWorklist.append(loop[j]);
        }
        
        while (!blockWorklist.isEmpty()) {
            BasicBlock* block = blockWorklist.takeLast();
            
            if (verbose)
                dataLog("    Dealing with ", *block, "\n");
            
            if (block == loop.header())
                continue;
            
            for (unsigned j = block->predecessors.size(); j--;) {
                BasicBlock* predecessor = block->predecessors[j];
                if (seenBlocks[predecessor->index])
                    continue;
                
                loop.addBlock(predecessor);
                blockWorklist.append(predecessor);
                seenBlocks[predecessor->index] = true;
            }
        }
    }

    // Figure out reverse mapping from blocks to loops.
    for (BlockIndex blockIndex = graph.numBlocks(); blockIndex--;) {
        BasicBlock* block = graph.block(blockIndex);
        if (!block)
            continue;
        for (unsigned i = BasicBlock::numberOfInnerMostLoopIndices; i--;)
            block->innerMostLoopIndices[i] = UINT_MAX;
    }
    for (unsigned loopIndex = m_loops.size(); loopIndex--;) {
        NaturalLoop& loop = m_loops[loopIndex];
        
        for (unsigned blockIndexInLoop = loop.size(); blockIndexInLoop--;) {
            BasicBlock* block = loop[blockIndexInLoop];
            
            for (unsigned i = 0; i < BasicBlock::numberOfInnerMostLoopIndices; ++i) {
                unsigned thisIndex = block->innerMostLoopIndices[i];
                if (thisIndex == UINT_MAX || loop.size() < m_loops[thisIndex].size()) {
                    insertIntoBoundedVector(
                        block->innerMostLoopIndices, BasicBlock::numberOfInnerMostLoopIndices,
                        loopIndex, i);
                    break;
                }
            }
        }
    }
    
    // Now each block knows its inner-most loop and its next-to-inner-most loop. Use
    // this to figure out loop parenting.
    for (unsigned i = m_loops.size(); i--;) {
        NaturalLoop& loop = m_loops[i];
        RELEASE_ASSERT(loop.header()->innerMostLoopIndices[0] == i);
        
        loop.m_outerLoopIndex = loop.header()->innerMostLoopIndices[1];
    }
    
    if (validationEnabled()) {
        // Do some self-verification that we've done some of this correctly.
        
        for (BlockIndex blockIndex = graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = graph.block(blockIndex);
            if (!block)
                continue;
            
            Vector<const NaturalLoop*> simpleLoopsOf;
            
            for (unsigned i = m_loops.size(); i--;) {
                if (m_loops[i].contains(block))
                    simpleLoopsOf.append(&m_loops[i]);
            }
            
            Vector<const NaturalLoop*> fancyLoopsOf = loopsOf(block);
            
            std::sort(simpleLoopsOf.begin(), simpleLoopsOf.end());
            std::sort(fancyLoopsOf.begin(), fancyLoopsOf.end());
            
            RELEASE_ASSERT(simpleLoopsOf == fancyLoopsOf);
        }
    }
    
    if (verbose)
        dataLog("Results: ", *this, "\n");
}

NaturalLoops::~NaturalLoops() { }

Vector<const NaturalLoop*> NaturalLoops::loopsOf(BasicBlock* block) const
{
    Vector<const NaturalLoop*> result;
    for (const NaturalLoop* loop = innerMostLoopOf(block); loop; loop = innerMostOuterLoop(*loop))
        result.append(loop);
    return result;
}

void NaturalLoops::dump(PrintStream& out) const
{
    out.print("NaturalLoops:{");
    CommaPrinter comma;
    for (unsigned i = 0; i < m_loops.size(); ++i)
        out.print(comma, m_loops[i]);
    out.print("}");
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

