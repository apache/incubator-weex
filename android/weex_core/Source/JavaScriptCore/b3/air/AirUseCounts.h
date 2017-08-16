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

#pragma once

#if ENABLE(B3_JIT)

#include "AirArgInlines.h"
#include "AirBlockWorklist.h"
#include "AirCode.h"
#include "AirInstInlines.h"
#include <wtf/HashMap.h>
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

class Code;

// Computes the number of uses of a variable based on frequency of execution. The frequency of blocks
// that are only reachable by rare edges is scaled by Options::rareBlockPenalty().

// Thing can be either Tmp or Arg.
template<typename Thing>
class UseCounts {
public:
    struct Counts {
        void dump(PrintStream& out) const
        {
            out.print(
                "{numWarmUses = ", numWarmUses, ", numColdUses = ", numColdUses, ", numDefs = ",
                numDefs, "}");
        }
        
        double numWarmUses { 0 };
        double numColdUses { 0 };
        double numDefs { 0 };
        double numConstDefs { 0 };
    };
    
    UseCounts(Code& code)
    {
        // Find non-rare blocks.
        BlockWorklist fastWorklist;
        fastWorklist.push(code[0]);
        while (BasicBlock* block = fastWorklist.pop()) {
            for (FrequentedBlock& successor : block->successors()) {
                if (!successor.isRare())
                    fastWorklist.push(successor.block());
            }
        }
        
        for (BasicBlock* block : code) {
            double frequency = block->frequency();
            if (!fastWorklist.saw(block))
                frequency *= Options::rareBlockPenalty();
            for (Inst& inst : *block) {
                inst.forEach<Thing>(
                    [&] (Thing& arg, Arg::Role role, Bank, Width) {
                        Counts& counts = m_counts.add(arg, Counts()).iterator->value;

                        if (Arg::isWarmUse(role))
                            counts.numWarmUses += frequency;
                        if (Arg::isColdUse(role))
                            counts.numColdUses += frequency;
                        if (Arg::isAnyDef(role))
                            counts.numDefs += frequency;
                    });

                if ((inst.kind.opcode == Move || inst.kind.opcode == Move32)
                    && inst.args[0].isSomeImm()
                    && inst.args[1].is<Thing>())
                    m_counts.add(inst.args[1].as<Thing>(), Counts()).iterator->value.numConstDefs++;
            }
        }
    }

    const Counts* operator[](const Thing& arg) const
    {
        auto iter = m_counts.find(arg);
        if (iter == m_counts.end())
            return nullptr;
        return &iter->value;
    }

    void dump(PrintStream& out) const
    {
        out.print(mapDump(m_counts));
    }

private:
    HashMap<Thing, Counts> m_counts;
};

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
