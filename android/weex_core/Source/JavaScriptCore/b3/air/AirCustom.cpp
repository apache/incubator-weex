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
#include "AirCustom.h"

#if ENABLE(B3_JIT)

#include "AirInstInlines.h"
#include "B3CCallValue.h"
#include "B3ValueInlines.h"

namespace JSC { namespace B3 { namespace Air {

bool PatchCustom::isValidForm(Inst& inst)
{
    if (inst.args.size() < 1)
        return false;
    if (!inst.args[0].isSpecial())
        return false;
    if (!inst.args[0].special()->isValid(inst))
        return false;
    RegisterSet clobberedEarly = inst.extraEarlyClobberedRegs();
    RegisterSet clobberedLate = inst.extraClobberedRegs();
    bool ok = true;
    inst.forEachTmp(
        [&] (Tmp& tmp, Arg::Role role, Bank, Width) {
            if (!tmp.isReg())
                return;
            if (Arg::isLateDef(role) || Arg::isLateUse(role))
                ok &= !clobberedLate.get(tmp.reg());
            else
                ok &= !clobberedEarly.get(tmp.reg());
        });
    return ok;
}

bool CCallCustom::isValidForm(Inst& inst)
{
    CCallValue* value = inst.origin->as<CCallValue>();
    if (!value)
        return false;

    if (inst.args.size() != (value->type() == Void ? 0 : 1) + value->numChildren())
        return false;

    // The arguments can only refer to the stack, tmps, or immediates.
    for (Arg& arg : inst.args) {
        if (!arg.isTmp() && !arg.isStackMemory() && !arg.isSomeImm())
            return false;
    }

    unsigned offset = 0;

    if (!inst.args[0].isGP())
        return false;

    // If there is a result then it cannot be an immediate.
    if (value->type() != Void) {
        if (inst.args[1].isSomeImm())
            return false;
        if (!inst.args[1].canRepresent(value))
            return false;
        offset++;
    }

    for (unsigned i = value->numChildren(); i-- > 1;) {
        Value* child = value->child(i);
        Arg arg = inst.args[offset + i];
        if (!arg.canRepresent(child))
            return false;
    }

    return true;
}

CCallHelpers::Jump CCallCustom::generate(Inst& inst, CCallHelpers&, GenerationContext&)
{
    dataLog("FATAL: Unlowered C call: ", inst, "\n");
    UNREACHABLE_FOR_PLATFORM();
    return CCallHelpers::Jump();
}

bool ShuffleCustom::isValidForm(Inst& inst)
{
    if (inst.args.size() % 3)
        return false;

    // A destination may only appear once. This requirement allows us to avoid the undefined behavior
    // of having a destination that is supposed to get multiple inputs simultaneously. It also
    // imposes some interesting constraints on the "shape" of the shuffle. If we treat a shuffle pair
    // as an edge and the Args as nodes, then the single-destination requirement means that the
    // shuffle graph consists of two kinds of subgraphs:
    //
    // - Spanning trees. We call these shifts. They can be executed as a sequence of Move
    //   instructions and don't usually require scratch registers.
    //
    // - Closed loops. These loops consist of nodes that have one successor and one predecessor, so
    //   there is no way to "get into" the loop from outside of it. These can be executed using swaps
    //   or by saving one of the Args to a scratch register and executing it as a shift.
    HashSet<Arg> dsts;

    for (unsigned i = 0; i < inst.args.size(); ++i) {
        Arg arg = inst.args[i];
        unsigned mode = i % 3;

        if (mode == 2) {
            // It's the width.
            if (!arg.isWidthArg())
                return false;
            continue;
        }

        // The source can be an immediate.
        if (!mode) {
            if (arg.isSomeImm())
                continue;

            if (!arg.isCompatibleBank(inst.args[i + 1]))
                return false;
        } else {
            ASSERT(mode == 1);
            if (!dsts.add(arg).isNewEntry)
                return false;
        }

        if (arg.isTmp() || arg.isMemory())
            continue;

        return false;
    }

    // No destination register may appear in any address expressions. The lowering can't handle it
    // and it's not useful for the way we end up using Shuffles. Normally, Shuffles only used for
    // stack addresses and non-stack registers.
    for (Arg& arg : inst.args) {
        if (!arg.isMemory())
            continue;
        bool ok = true;
        arg.forEachTmpFast(
            [&] (Tmp tmp) {
                if (dsts.contains(tmp))
                    ok = false;
            });
        if (!ok)
            return false;
    }

    return true;
}

CCallHelpers::Jump ShuffleCustom::generate(Inst& inst, CCallHelpers&, GenerationContext&)
{
    dataLog("FATAL: Unlowered shuffle: ", inst, "\n");
    UNREACHABLE_FOR_PLATFORM();
    return CCallHelpers::Jump();
}

bool WasmBoundsCheckCustom::isValidForm(Inst& inst)
{
    if (inst.args.size() != 2)
        return false;
    if (!inst.args[0].isTmp() && !inst.args[0].isSomeImm())
        return false;

    return inst.args[1].isReg();
}


} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

