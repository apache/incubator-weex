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
#include "AirTmpWidth.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInstInlines.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

TmpWidth::TmpWidth()
{
}

TmpWidth::TmpWidth(Code& code)
{
    recompute(code);
}

TmpWidth::~TmpWidth()
{
}

void TmpWidth::recompute(Code& code)
{
    // Set this to true to cause this analysis to always return pessimistic results.
    const bool beCareful = false;

    const bool verbose = false;

    if (verbose) {
        dataLog("Code before TmpWidth:\n");
        dataLog(code);
    }
    
    m_width.clear();
    
    auto assumeTheWorst = [&] (Tmp tmp) {
        Widths& widths = m_width.add(tmp, Widths()).iterator->value;
        Bank bank = Arg(tmp).bank();
        widths.use = conservativeWidth(bank);
        widths.def = conservativeWidth(bank);
    };
    
    // Assume the worst for registers.
    RegisterSet::allRegisters().forEach(
        [&] (Reg reg) {
            assumeTheWorst(Tmp(reg));
        });

    if (beCareful) {
        code.forAllTmps(assumeTheWorst);
        
        // We fall through because the fixpoint that follows can only make things even more
        // conservative. This mode isn't meant to be fast, just safe.
    }

    // Now really analyze everything but Move's over Tmp's, but set aside those Move's so we can find
    // them quickly during the fixpoint below. Note that we can make this analysis stronger by
    // recognizing more kinds of Move's or anything that has Move-like behavior, though it's probably not
    // worth it.
    Vector<Inst*> moves;
    for (BasicBlock* block : code) {
        for (Inst& inst : *block) {
            if (inst.kind.opcode == Move && inst.args[1].isTmp()) {
                if (inst.args[0].isTmp()) {
                    // Make sure that both sides of the Move have a width already initialized. The
                    // fixpoint below assumes that it never has to add things to the HashMap.
                    m_width.add(inst.args[0].tmp(), Widths(GP));
                    m_width.add(inst.args[1].tmp(), Widths(GP));
                    
                    moves.append(&inst);
                    continue;
                }
                if (inst.args[0].isImm()
                    && inst.args[0].value() >= 0) {
                    Tmp tmp = inst.args[1].tmp();
                    Widths& widths = m_width.add(tmp, Widths(GP)).iterator->value;
                    
                    if (inst.args[0].value() <= std::numeric_limits<int8_t>::max())
                        widths.def = std::max(widths.def, Width8);
                    else if (inst.args[0].value() <= std::numeric_limits<int16_t>::max())
                        widths.def = std::max(widths.def, Width16);
                    else if (inst.args[0].value() <= std::numeric_limits<int32_t>::max())
                        widths.def = std::max(widths.def, Width32);
                    else
                        widths.def = std::max(widths.def, Width64);

                    continue;
                }
            }
            inst.forEachTmp(
                [&] (Tmp& tmp, Arg::Role role, Bank bank, Width width) {
                    Widths& widths = m_width.add(tmp, Widths(bank)).iterator->value;
                    
                    if (Arg::isAnyUse(role))
                        widths.use = std::max(widths.use, width);

                    if (Arg::isZDef(role))
                        widths.def = std::max(widths.def, width);
                    else if (Arg::isAnyDef(role))
                        widths.def = conservativeWidth(bank);
                });
        }
    }

    // Finally, fixpoint over the Move's.
    bool changed = true;
    while (changed) {
        changed = false;
        for (Inst* move : moves) {
            ASSERT(move->kind.opcode == Move);
            ASSERT(move->args[0].isTmp());
            ASSERT(move->args[1].isTmp());

            // We already ensure that both tmps are added to the width map. That's important
            // because you cannot add both tmps here while simultaneously getting a reference to
            // their values, since the second add would invalidate the reference returned by the
            // first one.
            Widths& srcWidths = m_width.find(move->args[0].tmp())->value;
            Widths& dstWidths = m_width.find(move->args[1].tmp())->value;

            // Legend:
            //
            //     Move %src, %dst

            // defWidth(%dst) is a promise about how many high bits are zero. The smaller the width, the
            // stronger the promise. This Move may weaken that promise if we know that %src is making a
            // weaker promise. Such forward flow is the only thing that determines defWidth().
            if (dstWidths.def < srcWidths.def) {
                dstWidths.def = srcWidths.def;
                changed = true;
            }

            // srcWidth(%src) is a promise about how many high bits are ignored. The smaller the width,
            // the stronger the promise. This Move may weaken that promise if we know that %dst is making
            // a weaker promise. Such backward flow is the only thing that determines srcWidth().
            if (srcWidths.use < dstWidths.use) {
                srcWidths.use = dstWidths.use;
                changed = true;
            }
        }
    }

    if (verbose)
        dataLog("width: ", mapDump(m_width), "\n");
}

void TmpWidth::Widths::dump(PrintStream& out) const
{
    out.print("{use = ", use, ", def = ", def, "}");
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

