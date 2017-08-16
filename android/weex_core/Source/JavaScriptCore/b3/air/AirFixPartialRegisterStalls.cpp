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
#include "AirFixPartialRegisterStalls.h"

#if ENABLE(B3_JIT)

#include "AirBasicBlock.h"
#include "AirCode.h"
#include "AirInsertionSet.h"
#include "AirInst.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"
#include "MacroAssembler.h"
#include <wtf/IndexMap.h>
#include <wtf/IndexSet.h>
#include <wtf/Vector.h>

namespace JSC { namespace B3 { namespace Air {

namespace {

bool hasPartialXmmRegUpdate(const Inst& inst)
{
    switch (inst.kind.opcode) {
    case ConvertDoubleToFloat:
    case ConvertFloatToDouble:
    case ConvertInt32ToDouble:
    case ConvertInt64ToDouble:
    case ConvertInt32ToFloat:
    case ConvertInt64ToFloat:
    case SqrtDouble:
    case SqrtFloat:
    case CeilDouble:
    case CeilFloat:
    case FloorDouble:
    case FloorFloat:
        return true;
    default:
        break;
    }
    return false;
}

bool isDependencyBreaking(const Inst& inst)
{
    // "xorps reg, reg" is used by the frontend to remove the dependency on its argument.
    return inst.kind.opcode == MoveZeroToDouble;
}

// FIXME: find a good distance per architecture experimentally.
// LLVM uses a distance of 16 but that comes from Nehalem.
unsigned char minimumSafeDistance = 16;

struct FPDefDistance {
    FPDefDistance()
    {
        for (unsigned i = 0; i < MacroAssembler::numberOfFPRegisters(); ++i)
            distance[i] = 255;
    }

    void reset(FPRReg reg)
    {
        unsigned index = MacroAssembler::fpRegisterIndex(reg);
        distance[index] = 255;
    }

    void add(FPRReg reg, unsigned registerDistance)
    {
        unsigned index = MacroAssembler::fpRegisterIndex(reg);
        if (registerDistance < distance[index])
            distance[index] = static_cast<unsigned char>(registerDistance);
    }

    bool updateFromPrecessor(FPDefDistance& precessorDistance, unsigned constantOffset = 0)
    {
        bool changed = false;
        for (unsigned i = 0; i < MacroAssembler::numberOfFPRegisters(); ++i) {
            unsigned regDistance = precessorDistance.distance[i] + constantOffset;
            if (regDistance < minimumSafeDistance && regDistance < distance[i]) {
                distance[i] = regDistance;
                changed = true;
            }
        }
        return changed;
    }

    unsigned char distance[MacroAssembler::numberOfFPRegisters()];
};

void updateDistances(Inst& inst, FPDefDistance& localDistance, unsigned& distanceToBlockEnd)
{
    --distanceToBlockEnd;

    if (isDependencyBreaking(inst)) {
        localDistance.reset(inst.args[0].tmp().fpr());
        return;
    }

    inst.forEachTmp([&] (Tmp& tmp, Arg::Role role, Bank, Width) {
        ASSERT_WITH_MESSAGE(tmp.isReg(), "This phase must be run after register allocation.");

        if (tmp.isFPR() && Arg::isAnyDef(role))
            localDistance.add(tmp.fpr(), distanceToBlockEnd);
    });
}

}

void fixPartialRegisterStalls(Code& code)
{
    if (!isX86())
        return;

    PhaseScope phaseScope(code, "fixPartialRegisterStalls");

    Vector<BasicBlock*> candidates;

    for (BasicBlock* block : code) {
        for (const Inst& inst : *block) {
            if (hasPartialXmmRegUpdate(inst)) {
                candidates.append(block);
                break;
            }
        }
    }

    // Fortunately, Partial Stalls are rarely used. Return early if no block
    // cares about them.
    if (candidates.isEmpty())
        return;

    // For each block, this provides the distance to the last instruction setting each register
    // on block *entry*.
    IndexMap<BasicBlock, FPDefDistance> lastDefDistance(code.size());

    // Blocks with dirty distance at head.
    IndexSet<BasicBlock> dirty;

    // First, we compute the local distance for each block and push it to the successors.
    for (BasicBlock* block : code) {
        FPDefDistance localDistance;

        unsigned distanceToBlockEnd = block->size();
        for (Inst& inst : *block)
            updateDistances(inst, localDistance, distanceToBlockEnd);

        for (BasicBlock* successor : block->successorBlocks()) {
            if (lastDefDistance[successor].updateFromPrecessor(localDistance))
                dirty.add(successor);
        }
    }

    // Now we propagate the minimums accross blocks.
    bool changed;
    do {
        changed = false;

        for (BasicBlock* block : code) {
            if (!dirty.remove(block))
                continue;

            // Little shortcut: if the block is big enough, propagating it won't add any information.
            if (block->size() >= minimumSafeDistance)
                continue;

            unsigned blockSize = block->size();
            FPDefDistance& blockDistance = lastDefDistance[block];
            for (BasicBlock* successor : block->successorBlocks()) {
                if (lastDefDistance[successor].updateFromPrecessor(blockDistance, blockSize)) {
                    dirty.add(successor);
                    changed = true;
                }
            }
        }
    } while (changed);

    // Finally, update each block as needed.
    InsertionSet insertionSet(code);
    for (BasicBlock* block : candidates) {
        unsigned distanceToBlockEnd = block->size();
        FPDefDistance& localDistance = lastDefDistance[block];

        for (unsigned i = 0; i < block->size(); ++i) {
            Inst& inst = block->at(i);

            if (hasPartialXmmRegUpdate(inst)) {
                RegisterSet defs;
                RegisterSet uses;
                inst.forEachTmp([&] (Tmp& tmp, Arg::Role role, Bank, Width) {
                    if (tmp.isFPR()) {
                        if (Arg::isAnyDef(role))
                            defs.set(tmp.fpr());
                        if (Arg::isAnyUse(role))
                            uses.set(tmp.fpr());
                    }
                });
                // We only care about values we define but not use. Otherwise we have to wait
                // for the value to be resolved anyway.
                defs.exclude(uses);

                defs.forEach([&] (Reg reg) {
                    if (localDistance.distance[MacroAssembler::fpRegisterIndex(reg.fpr())] < minimumSafeDistance)
                        insertionSet.insert(i, MoveZeroToDouble, inst.origin, Tmp(reg));
                });
            }

            updateDistances(inst, localDistance, distanceToBlockEnd);
        }
        insertionSet.execute(block);
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
