/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
#include "AirLowerAfterRegAlloc.h"

#if ENABLE(B3_JIT)

#include "AirArgInlines.h"
#include "AirCCallingConvention.h"
#include "AirCode.h"
#include "AirEmitShuffle.h"
#include "AirInsertionSet.h"
#include "AirInstInlines.h"
#include "AirLiveness.h"
#include "AirPhaseScope.h"
#include "B3CCallValue.h"
#include "B3ValueInlines.h"
#include "RegisterSet.h"
#include <wtf/HashMap.h>

namespace JSC { namespace B3 { namespace Air {

namespace {

bool verbose = false;
    
} // anonymous namespace

void lowerAfterRegAlloc(Code& code)
{
    PhaseScope phaseScope(code, "lowerAfterRegAlloc");

    if (verbose)
        dataLog("Code before lowerAfterRegAlloc:\n", code);

    HashMap<Inst*, RegisterSet> usedRegisters;

    RegLiveness liveness(code);
    for (BasicBlock* block : code) {
        RegLiveness::LocalCalc localCalc(liveness, block);

        for (unsigned instIndex = block->size(); instIndex--;) {
            Inst& inst = block->at(instIndex);
            
            RegisterSet set;

            bool isRelevant = inst.kind.opcode == Shuffle || inst.kind.opcode == ColdCCall;
            
            if (isRelevant) {
                for (Reg reg : localCalc.live())
                    set.set(reg);
            }
            
            localCalc.execute(instIndex);

            if (isRelevant)
                usedRegisters.add(&inst, set);
        }
    }

    auto getScratches = [&] (RegisterSet set, Bank bank) -> std::array<Arg, 2> {
        std::array<Arg, 2> result;
        for (unsigned i = 0; i < 2; ++i) {
            bool found = false;
            for (Reg reg : code.regsInPriorityOrder(bank)) {
                if (!set.get(reg)) {
                    result[i] = Tmp(reg);
                    set.set(reg);
                    found = true;
                    break;
                }
            }
            if (!found) {
                result[i] = Arg::stack(
                    code.addStackSlot(
                        bytes(conservativeWidth(bank)),
                        StackSlotKind::Spill));
            }
        }
        return result;
    };

    // Now transform the code.
    InsertionSet insertionSet(code);
    for (BasicBlock* block : code) {
        for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
            Inst& inst = block->at(instIndex);

            switch (inst.kind.opcode) {
            case Shuffle: {
                RegisterSet set = usedRegisters.get(&inst);
                Vector<ShufflePair> pairs;
                for (unsigned i = 0; i < inst.args.size(); i += 3) {
                    Arg src = inst.args[i + 0];
                    Arg dst = inst.args[i + 1];
                    Width width = inst.args[i + 2].width();

                    // The used register set contains things live after the shuffle. But
                    // emitShuffle() wants a scratch register that is not just dead but also does not
                    // interfere with either sources or destinations.
                    auto excludeRegisters = [&] (Tmp tmp) {
                        if (tmp.isReg())
                            set.set(tmp.reg());
                    };
                    src.forEachTmpFast(excludeRegisters);
                    dst.forEachTmpFast(excludeRegisters);
                    
                    pairs.append(ShufflePair(src, dst, width));
                }
                std::array<Arg, 2> gpScratch = getScratches(set, GP);
                std::array<Arg, 2> fpScratch = getScratches(set, FP);
                insertionSet.insertInsts(
                    instIndex, emitShuffle(code, pairs, gpScratch, fpScratch, inst.origin));
                inst = Inst();
                break;
            }

            case ColdCCall: {
                CCallValue* value = inst.origin->as<CCallValue>();
                Kind oldKind = inst.kind;

                RegisterSet liveRegs = usedRegisters.get(&inst);
                RegisterSet regsToSave = liveRegs;
                regsToSave.exclude(RegisterSet::calleeSaveRegisters());
                regsToSave.exclude(RegisterSet::stackRegisters());
                regsToSave.exclude(RegisterSet::reservedHardwareRegisters());

                RegisterSet preUsed = regsToSave;
                Vector<Arg> destinations = computeCCallingConvention(code, value);
                Tmp result = cCallResult(value->type());
                Arg originalResult = result ? inst.args[1] : Arg();
                
                Vector<ShufflePair> pairs;
                for (unsigned i = 0; i < destinations.size(); ++i) {
                    Value* child = value->child(i);
                    Arg src = inst.args[result ? (i >= 1 ? i + 1 : i) : i ];
                    Arg dst = destinations[i];
                    Width width = widthForType(child->type());
                    pairs.append(ShufflePair(src, dst, width));

                    auto excludeRegisters = [&] (Tmp tmp) {
                        if (tmp.isReg())
                            preUsed.set(tmp.reg());
                    };
                    src.forEachTmpFast(excludeRegisters);
                    dst.forEachTmpFast(excludeRegisters);
                }

                std::array<Arg, 2> gpScratch = getScratches(preUsed, GP);
                std::array<Arg, 2> fpScratch = getScratches(preUsed, FP);
                
                // Also need to save all live registers. Don't need to worry about the result
                // register.
                if (originalResult.isReg())
                    regsToSave.clear(originalResult.reg());
                Vector<StackSlot*> stackSlots;
                regsToSave.forEach(
                    [&] (Reg reg) {
                        Tmp tmp(reg);
                        Arg arg(tmp);
                        Width width = conservativeWidth(arg.bank());
                        StackSlot* stackSlot =
                            code.addStackSlot(bytes(width), StackSlotKind::Spill);
                        pairs.append(ShufflePair(arg, Arg::stack(stackSlot), width));
                        stackSlots.append(stackSlot);
                    });

                if (verbose)
                    dataLog("Pre-call pairs for ", inst, ": ", listDump(pairs), "\n");
                
                insertionSet.insertInsts(
                    instIndex, emitShuffle(code, pairs, gpScratch, fpScratch, inst.origin));

                inst = buildCCall(code, inst.origin, destinations);
                if (oldKind.traps)
                    inst.kind.traps = true;

                // Now we need to emit code to restore registers.
                pairs.resize(0);
                unsigned stackSlotIndex = 0;
                regsToSave.forEach(
                    [&] (Reg reg) {
                        Tmp tmp(reg);
                        Arg arg(tmp);
                        Width width = conservativeWidth(arg.bank());
                        StackSlot* stackSlot = stackSlots[stackSlotIndex++];
                        pairs.append(ShufflePair(Arg::stack(stackSlot), arg, width));
                    });
                if (result) {
                    ShufflePair pair(result, originalResult, widthForType(value->type()));
                    pairs.append(pair);
                }

                // For finding scratch registers, we need to account for the possibility that
                // the result is dead.
                if (originalResult.isReg())
                    liveRegs.set(originalResult.reg());

                gpScratch = getScratches(liveRegs, GP);
                fpScratch = getScratches(liveRegs, FP);
                
                insertionSet.insertInsts(
                    instIndex + 1, emitShuffle(code, pairs, gpScratch, fpScratch, inst.origin));
                break;
            }

            default:
                break;
            }
        }

        insertionSet.execute(block);

        block->insts().removeAllMatching(
            [&] (Inst& inst) -> bool {
                return !inst;
            });
    }

    if (verbose)
        dataLog("Code after lowerAfterRegAlloc:\n", code);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

