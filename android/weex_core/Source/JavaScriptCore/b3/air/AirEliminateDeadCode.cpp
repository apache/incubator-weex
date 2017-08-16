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
#include "AirEliminateDeadCode.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"
#include <wtf/IndexSet.h>

namespace JSC { namespace B3 { namespace Air {

bool eliminateDeadCode(Code& code)
{
    PhaseScope phaseScope(code, "eliminateDeadCode");

    HashSet<Tmp> liveTmps;
    IndexSet<StackSlot> liveStackSlots;
    bool changed;

    auto isArgLive = [&] (const Arg& arg) -> bool {
        switch (arg.kind()) {
        case Arg::Tmp:
            if (arg.isReg())
                return true;
            return liveTmps.contains(arg.tmp());
        case Arg::Stack:
            if (arg.stackSlot()->isLocked())
                return true;
            return liveStackSlots.contains(arg.stackSlot());
        default:
            return true;
        }
    };

    auto addLiveArg = [&] (const Arg& arg) -> bool {
        switch (arg.kind()) {
        case Arg::Tmp:
            if (arg.isReg())
                return false;
            return liveTmps.add(arg.tmp()).isNewEntry;
        case Arg::Stack:
            if (arg.stackSlot()->isLocked())
                return false;
            return liveStackSlots.add(arg.stackSlot());
        default:
            return false;
        }
    };

    auto isInstLive = [&] (Inst& inst) -> bool {
        if (inst.hasNonArgEffects())
            return true;

        // This instruction should be presumed dead, if its Args are all dead.
        bool storesToLive = false;
        inst.forEachArg(
            [&] (Arg& arg, Arg::Role role, Bank, Width) {
                if (!Arg::isAnyDef(role))
                    return;
                if (role == Arg::Scratch)
                    return;
                storesToLive |= isArgLive(arg);
            });
        return storesToLive;
    };

    auto handleInst = [&] (Inst& inst) {
        if (!isInstLive(inst))
            return;

        // We get here if the Inst is live. For simplicity we say that a live instruction forces
        // liveness upon everything it mentions.
        for (Arg& arg : inst.args) {
            changed |= addLiveArg(arg);
            arg.forEachTmpFast(
                [&] (Tmp& tmp) {
                    changed |= addLiveArg(tmp);
                });
        }
    };

    auto runForward = [&] () -> bool {
        changed = false;
        for (BasicBlock* block : code) {
            for (Inst& inst : *block)
                handleInst(inst);
        }
        return changed;
    };

    auto runBackward = [&] () -> bool {
        changed = false;
        for (unsigned blockIndex = code.size(); blockIndex--;) {
            BasicBlock* block = code[blockIndex];
            if (!block)
                continue;
            for (unsigned instIndex = block->size(); instIndex--;)
                handleInst(block->at(instIndex));
        }
        return changed;
    };

    for (;;) {
        // Propagating backward is most likely to be profitable.
        if (!runBackward())
            break;
        if (!runBackward())
            break;

        // Occasionally propagating forward greatly reduces the likelihood of pathologies.
        if (!runForward())
            break;
    }

    unsigned removedInstCount = 0;
    for (BasicBlock* block : code) {
        removedInstCount += block->insts().removeAllMatching(
            [&] (Inst& inst) -> bool {
                return !isInstLive(inst);
            });
    }

    return !!removedInstCount;
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

