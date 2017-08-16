/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "AirSimplifyCFG.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"

namespace JSC { namespace B3 { namespace Air {

bool simplifyCFG(Code& code)
{
    const bool verbose = false;
    
    PhaseScope phaseScope(code, "simplifyCFG");
    
    // We have three easy simplification rules:
    //
    // 1) If a successor is a block that just jumps to another block, then jump directly to
    //    that block.
    //
    // 2) If all successors are the same and the operation has no effects, then use a jump
    //    instead.
    //
    // 3) If you jump to a block that is not you and has one predecessor, then merge.
    //
    // Note that because of the first rule, this phase may introduce critical edges. That's fine.
    // If you need broken critical edges, then you have to break them yourself.

    bool result = false;
    for (;;) {
        if (verbose) {
            dataLog("Air before an iteration of simplifyCFG:\n");
            dataLog(code);
        }
        
        bool changed = false;
        for (BasicBlock* block : code) {
            // We rely on predecessors being conservatively correct. Verify this here.
            if (shouldValidateIRAtEachPhase()) {
                for (BasicBlock* block : code) {
                    for (BasicBlock* successor : block->successorBlocks())
                        RELEASE_ASSERT(successor->containsPredecessor(block));
                }
            }

            // We don't care about blocks that don't have successors.
            if (!block->numSuccessors())
                continue;

            // First check if any of the successors of this block can be forwarded over.
            for (BasicBlock*& successor : block->successorBlocks()) {
                if (successor != block
                    && successor->size() == 1
                    && successor->last().kind.opcode == Jump) {
                    BasicBlock* newSuccessor = successor->successorBlock(0);
                    if (newSuccessor != successor) {
                        if (verbose) {
                            dataLog(
                                "Replacing ", pointerDump(block), "->", pointerDump(successor),
                                " with ", pointerDump(block), "->", pointerDump(newSuccessor), "\n");
                        }
                        // Note that we do not do replacePredecessor() because the block we're
                        // skipping will still have newSuccessor as its successor.
                        newSuccessor->addPredecessor(block);
                        successor = newSuccessor;
                        changed = true;
                    }
                }
            }

            // Now check if the block's terminal can be replaced with a jump. The terminal must not
            // have weird effects.
            if (block->numSuccessors() > 1 
                && !block->last().hasNonControlEffects()) {
                // All of the successors must be the same.
                bool allSame = true;
                BasicBlock* firstSuccessor = block->successorBlock(0);
                for (unsigned i = 1; i < block->numSuccessors(); ++i) {
                    if (block->successorBlock(i) != firstSuccessor) {
                        allSame = false;
                        break;
                    }
                }
                if (allSame) {
                    if (verbose)
                        dataLog("Changing ", pointerDump(block), "'s terminal to a Jump.\n");
                    block->last() = Inst(Jump, block->last().origin);
                    block->successors().resize(1);
                    block->successors()[0].frequency() = FrequencyClass::Normal;
                    changed = true;
                }
            }

            // Finally handle jumps to a block with one predecessor.
            if (block->numSuccessors() == 1
                && !block->last().hasNonControlEffects()) {
                BasicBlock* successor = block->successorBlock(0);
                if (successor != block && successor->numPredecessors() == 1) {
                    RELEASE_ASSERT(successor->predecessor(0) == block);

                    // We can merge the two blocks because the predecessor only jumps to the successor
                    // and the successor is only reachable from the predecessor.

                    // Remove the terminal.
                    Value* origin = block->insts().takeLast().origin;

                    // Append the full contents of the successor to the predecessor.
                    block->insts().reserveCapacity(block->size() + successor->size());
                    for (Inst& inst : *successor)
                        block->appendInst(WTFMove(inst));

                    // Make sure that our successors are the successor's successors.
                    block->successors() = WTFMove(successor->successors());

                    // Make sure that the successor has nothing left in it except an oops.
                    successor->resize(1);
                    successor->last() = Inst(Oops, origin);
                    successor->successors().clear();

                    // Ensure that the predecessors of block's new successors know what's up.
                    for (BasicBlock* newSuccessor : block->successorBlocks())
                        newSuccessor->replacePredecessor(successor, block);

                    if (verbose)
                        dataLog("Merged ", pointerDump(block), "->", pointerDump(successor), "\n");
                    changed = true;
                }
            }
        }

        if (!changed)
            break;
        result = true;
        code.resetReachability();
    }

    return result;
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)


