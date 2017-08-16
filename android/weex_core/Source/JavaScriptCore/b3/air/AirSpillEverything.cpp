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
#include "AirSpillEverything.h"

#if ENABLE(B3_JIT)

#include "AirArgInlines.h"
#include "AirCode.h"
#include "AirInsertionSet.h"
#include "AirInstInlines.h"
#include "AirLiveness.h"
#include "AirPadInterference.h"
#include "AirPhaseScope.h"
#include <wtf/IndexMap.h>

namespace JSC { namespace B3 { namespace Air {

void spillEverything(Code& code)
{
    PhaseScope phaseScope(code, "spillEverything");
    
    padInterference(code);

    // We want to know the set of registers used at every point in every basic block.
    IndexMap<BasicBlock, Vector<RegisterSet>> usedRegisters(code.size());
    GPLiveness gpLiveness(code);
    FPLiveness fpLiveness(code);
    for (BasicBlock* block : code) {
        GPLiveness::LocalCalc gpLocalCalc(gpLiveness, block);
        FPLiveness::LocalCalc fpLocalCalc(fpLiveness, block);

        usedRegisters[block].resize(block->size() + 1);

        auto setUsedRegisters = [&] (unsigned index) {
            RegisterSet& registerSet = usedRegisters[block][index];
            for (Tmp tmp : gpLocalCalc.live()) {
                if (tmp.isReg())
                    registerSet.set(tmp.reg());
            }
            for (Tmp tmp : fpLocalCalc.live()) {
                if (tmp.isReg())
                    registerSet.set(tmp.reg());
            }

            // Gotta account for dead assignments to registers. These may happen because the input
            // code is suboptimal.
            Inst::forEachDefWithExtraClobberedRegs<Tmp>(
                block->get(index - 1), block->get(index),
                [&] (const Tmp& tmp, Arg::Role, Bank, Width) {
                    if (tmp.isReg())
                        registerSet.set(tmp.reg());
                });
        };

        for (unsigned instIndex = block->size(); instIndex--;) {
            setUsedRegisters(instIndex + 1);
            gpLocalCalc.execute(instIndex);
            fpLocalCalc.execute(instIndex);
        }
        setUsedRegisters(0);
    }

    // Allocate a stack slot for each tmp.
    Vector<StackSlot*> allStackSlots[numBanks];
    for (unsigned bankIndex = 0; bankIndex < numBanks; ++bankIndex) {
        Vector<StackSlot*>& stackSlots = allStackSlots[bankIndex];
        Bank bank = static_cast<Bank>(bankIndex);
        stackSlots.resize(code.numTmps(bank));
        for (unsigned tmpIndex = code.numTmps(bank); tmpIndex--;)
            stackSlots[tmpIndex] = code.addStackSlot(8, StackSlotKind::Spill);
    }

    InsertionSet insertionSet(code);
    for (BasicBlock* block : code) {
        for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
            RegisterSet& setBefore = usedRegisters[block][instIndex];
            RegisterSet& setAfter = usedRegisters[block][instIndex + 1];
            Inst& inst = block->at(instIndex);

            // First try to spill directly.
            for (unsigned i = 0; i < inst.args.size(); ++i) {
                Arg& arg = inst.args[i];

                if (arg.isTmp()) {
                    if (arg.isReg())
                        continue;

                    if (inst.admitsStack(i)) {
                        StackSlot* stackSlot = allStackSlots[arg.bank()][arg.tmpIndex()];
                        arg = Arg::stack(stackSlot);
                        continue;
                    }
                }
            }

            // Now fall back on spilling using separate Move's to load/store the tmp.
            inst.forEachTmp(
                [&] (Tmp& tmp, Arg::Role role, Bank bank, Width) {
                    if (tmp.isReg())
                        return;
                    
                    StackSlot* stackSlot = allStackSlots[bank][tmp.tmpIndex()];
                    Arg arg = Arg::stack(stackSlot);

                    // Need to figure out a register to use. How we do that depends on the role.
                    Reg chosenReg;
                    switch (role) {
                    case Arg::Use:
                    case Arg::ColdUse:
                        for (Reg reg : code.regsInPriorityOrder(bank)) {
                            if (!setBefore.get(reg)) {
                                setBefore.set(reg);
                                chosenReg = reg;
                                break;
                            }
                        }
                        break;
                    case Arg::Def:
                    case Arg::ZDef:
                        for (Reg reg : code.regsInPriorityOrder(bank)) {
                            if (!setAfter.get(reg)) {
                                setAfter.set(reg);
                                chosenReg = reg;
                                break;
                            }
                        }
                        break;
                    case Arg::UseDef:
                    case Arg::UseZDef:
                    case Arg::LateUse:
                    case Arg::LateColdUse:
                    case Arg::Scratch:
                    case Arg::EarlyDef:
                    case Arg::EarlyZDef:
                        for (Reg reg : code.regsInPriorityOrder(bank)) {
                            if (!setBefore.get(reg) && !setAfter.get(reg)) {
                                setAfter.set(reg);
                                setBefore.set(reg);
                                chosenReg = reg;
                                break;
                            }
                        }
                        break;
                    case Arg::UseAddr:
                        // We will never UseAddr a Tmp, that doesn't make sense.
                        RELEASE_ASSERT_NOT_REACHED();
                        break;
                    }
                    RELEASE_ASSERT(chosenReg);

                    tmp = Tmp(chosenReg);

                    Opcode move = bank == GP ? Move : MoveDouble;

                    if (Arg::isAnyUse(role) && role != Arg::Scratch)
                        insertionSet.insert(instIndex, move, inst.origin, arg, tmp);
                    if (Arg::isAnyDef(role))
                        insertionSet.insert(instIndex + 1, move, inst.origin, tmp, arg);
                });
        }
        insertionSet.execute(block);
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
