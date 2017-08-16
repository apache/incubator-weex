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
#include "AirAllocateStack.h"

#if ENABLE(B3_JIT)

#include "AirArgInlines.h"
#include "AirCode.h"
#include "AirInsertionSet.h"
#include "AirInstInlines.h"
#include "AirLiveness.h"
#include "AirPhaseScope.h"
#include "StackAlignment.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

namespace {

const bool verbose = false;

bool attemptAssignment(
    StackSlot* slot, intptr_t offsetFromFP, const Vector<StackSlot*>& otherSlots)
{
    if (verbose)
        dataLog("Attempting to assign ", pointerDump(slot), " to ", offsetFromFP, " with interference ", pointerListDump(otherSlots), "\n");

    // Need to align it to the slot's desired alignment.
    offsetFromFP = -WTF::roundUpToMultipleOf(slot->alignment(), -offsetFromFP);
    
    for (StackSlot* otherSlot : otherSlots) {
        if (!otherSlot->offsetFromFP())
            continue;
        bool overlap = WTF::rangesOverlap(
            offsetFromFP,
            offsetFromFP + static_cast<intptr_t>(slot->byteSize()),
            otherSlot->offsetFromFP(),
            otherSlot->offsetFromFP() + static_cast<intptr_t>(otherSlot->byteSize()));
        if (overlap)
            return false;
    }

    if (verbose)
        dataLog("Assigned ", pointerDump(slot), " to ", offsetFromFP, "\n");
    slot->setOffsetFromFP(offsetFromFP);
    return true;
}

void assign(StackSlot* slot, const Vector<StackSlot*>& otherSlots)
{
    if (verbose)
        dataLog("Attempting to assign ", pointerDump(slot), " with interference ", pointerListDump(otherSlots), "\n");
    
    if (attemptAssignment(slot, -static_cast<intptr_t>(slot->byteSize()), otherSlots))
        return;

    for (StackSlot* otherSlot : otherSlots) {
        if (!otherSlot->offsetFromFP())
            continue;
        bool didAssign = attemptAssignment(
            slot, otherSlot->offsetFromFP() - static_cast<intptr_t>(slot->byteSize()), otherSlots);
        if (didAssign)
            return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} // anonymous namespace

void allocateStack(Code& code)
{
    PhaseScope phaseScope(code, "allocateStack");

    // Allocate all of the escaped slots in order. This is kind of a crazy algorithm to allow for
    // the possibility of stack slots being assigned frame offsets before we even get here.
    ASSERT(!code.frameSize());
    Vector<StackSlot*> assignedEscapedStackSlots;
    Vector<StackSlot*> escapedStackSlotsWorklist;
    for (StackSlot* slot : code.stackSlots()) {
        if (slot->isLocked()) {
            if (slot->offsetFromFP())
                assignedEscapedStackSlots.append(slot);
            else
                escapedStackSlotsWorklist.append(slot);
        } else {
            // It would be super strange to have an unlocked stack slot that has an offset already.
            ASSERT(!slot->offsetFromFP());
        }
    }
    // This is a fairly expensive loop, but it's OK because we'll usually only have a handful of
    // escaped stack slots.
    while (!escapedStackSlotsWorklist.isEmpty()) {
        StackSlot* slot = escapedStackSlotsWorklist.takeLast();
        assign(slot, assignedEscapedStackSlots);
        assignedEscapedStackSlots.append(slot);
    }

    // Now we handle the spill slots.
    StackSlotLiveness liveness(code);
    IndexMap<StackSlot, HashSet<StackSlot*>> interference(code.stackSlots().size());
    Vector<StackSlot*> slots;

    for (BasicBlock* block : code) {
        StackSlotLiveness::LocalCalc localCalc(liveness, block);

        auto interfere = [&] (unsigned instIndex) {
            if (verbose)
                dataLog("Interfering: ", WTF::pointerListDump(localCalc.live()), "\n");

            Inst::forEachDef<Arg>(
                block->get(instIndex), block->get(instIndex + 1),
                [&] (Arg& arg, Arg::Role, Bank, Width) {
                    if (!arg.isStack())
                        return;
                    StackSlot* slot = arg.stackSlot();
                    if (slot->kind() != StackSlotKind::Spill)
                        return;

                    for (StackSlot* otherSlot : localCalc.live()) {
                        interference[slot].add(otherSlot);
                        interference[otherSlot].add(slot);
                    }
                });
        };

        for (unsigned instIndex = block->size(); instIndex--;) {
            if (verbose)
                dataLog("Analyzing: ", block->at(instIndex), "\n");

            // Kill dead stores. For simplicity we say that a store is killable if it has only late
            // defs and those late defs are to things that are dead right now. We only do that
            // because that's the only kind of dead stack store we will see here.
            Inst& inst = block->at(instIndex);
            if (!inst.hasNonArgEffects()) {
                bool ok = true;
                inst.forEachArg(
                    [&] (Arg& arg, Arg::Role role, Bank, Width) {
                        if (Arg::isEarlyDef(role)) {
                            ok = false;
                            return;
                        }
                        if (!Arg::isLateDef(role))
                            return;
                        if (!arg.isStack()) {
                            ok = false;
                            return;
                        }
                        StackSlot* slot = arg.stackSlot();
                        if (slot->kind() != StackSlotKind::Spill) {
                            ok = false;
                            return;
                        }

                        if (localCalc.isLive(slot)) {
                            ok = false;
                            return;
                        }
                    });
                if (ok)
                    inst = Inst();
            }
            
            interfere(instIndex);
            localCalc.execute(instIndex);
        }
        interfere(-1);
        
        block->insts().removeAllMatching(
            [&] (const Inst& inst) -> bool {
                return !inst;
            });
    }

    if (verbose) {
        for (StackSlot* slot : code.stackSlots())
            dataLog("Interference of ", pointerDump(slot), ": ", pointerListDump(interference[slot]), "\n");
    }

    // Now we assign stack locations. At its heart this algorithm is just first-fit. For each
    // StackSlot we just want to find the offsetFromFP that is closest to zero while ensuring no
    // overlap with other StackSlots that this overlaps with.
    Vector<StackSlot*> otherSlots = assignedEscapedStackSlots;
    for (StackSlot* slot : code.stackSlots()) {
        if (slot->offsetFromFP()) {
            // Already assigned an offset.
            continue;
        }

        HashSet<StackSlot*>& interferingSlots = interference[slot];
        otherSlots.resize(assignedEscapedStackSlots.size());
        otherSlots.resize(assignedEscapedStackSlots.size() + interferingSlots.size());
        unsigned nextIndex = assignedEscapedStackSlots.size();
        for (StackSlot* otherSlot : interferingSlots)
            otherSlots[nextIndex++] = otherSlot;

        assign(slot, otherSlots);
    }

    // Figure out how much stack we're using for stack slots.
    unsigned frameSizeForStackSlots = 0;
    for (StackSlot* slot : code.stackSlots()) {
        frameSizeForStackSlots = std::max(
            frameSizeForStackSlots,
            static_cast<unsigned>(-slot->offsetFromFP()));
    }

    frameSizeForStackSlots = WTF::roundUpToMultipleOf(stackAlignmentBytes(), frameSizeForStackSlots);

    // Now we need to deduce how much argument area we need.
    for (BasicBlock* block : code) {
        for (Inst& inst : *block) {
            for (Arg& arg : inst.args) {
                if (arg.isCallArg()) {
                    // For now, we assume that we use 8 bytes of the call arg. But that's not
                    // such an awesome assumption.
                    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=150454
                    ASSERT(arg.offset() >= 0);
                    code.requestCallArgAreaSizeInBytes(arg.offset() + 8);
                }
            }
        }
    }

    code.setFrameSize(frameSizeForStackSlots + code.callArgAreaSizeInBytes());

    // Finally, transform the code to use Addr's instead of StackSlot's. This is a lossless
    // transformation since we can search the StackSlots array to figure out which StackSlot any
    // offset-from-FP refers to.

    // FIXME: This may produce addresses that aren't valid if we end up with a ginormous stack frame.
    // We would have to scavenge for temporaries if this happened. Fortunately, this case will be
    // extremely rare so we can do crazy things when it arises.
    // https://bugs.webkit.org/show_bug.cgi?id=152530

    InsertionSet insertionSet(code);
    for (BasicBlock* block : code) {
        for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
            Inst& inst = block->at(instIndex);
            inst.forEachArg(
                [&] (Arg& arg, Arg::Role role, Bank, Width width) {
                    auto stackAddr = [&] (int32_t offset) -> Arg {
                        return Arg::stackAddr(offset, code.frameSize(), width);
                    };
                    
                    switch (arg.kind()) {
                    case Arg::Stack: {
                        StackSlot* slot = arg.stackSlot();
                        if (Arg::isZDef(role)
                            && slot->kind() == StackSlotKind::Spill
                            && slot->byteSize() > bytes(width)) {
                            // Currently we only handle this simple case because it's the only one
                            // that arises: ZDef's are only 32-bit right now. So, when we hit these
                            // assertions it means that we need to implement those other kinds of
                            // zero fills.
                            RELEASE_ASSERT(slot->byteSize() == 8);
                            RELEASE_ASSERT(width == Width32);

                            RELEASE_ASSERT(isValidForm(StoreZero32, Arg::Stack));
                            insertionSet.insert(
                                instIndex + 1, StoreZero32, inst.origin,
                                stackAddr(arg.offset() + 4 + slot->offsetFromFP()));
                        }
                        arg = stackAddr(arg.offset() + slot->offsetFromFP());
                        break;
                    }
                    case Arg::CallArg:
                        arg = stackAddr(arg.offset() - code.frameSize());
                        break;
                    default:
                        break;
                    }
                }
            );
        }
        insertionSet.execute(block);
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)


