/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "AirLowerEntrySwitch.h"

#if ENABLE(B3_JIT)

#include "AirBlockWorklist.h"
#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"
#include "B3Procedure.h"

namespace JSC { namespace B3 { namespace Air {

void lowerEntrySwitch(Code& code)
{
    PhaseScope phaseScope(code, "lowerEntrySwitch");
    
    // Figure out the set of blocks that should be duplicated.
    BlockWorklist worklist;
    for (BasicBlock* block : code) {
        if (block->last().kind.opcode == EntrySwitch)
            worklist.push(block);
    }
    
    // It's possible that we don't have any EntrySwitches. That's fine.
    if (worklist.seen().isEmpty()) {
        Vector<FrequentedBlock> entrypoints(code.proc().numEntrypoints(), FrequentedBlock(code[0]));
        code.setEntrypoints(WTFMove(entrypoints));
        return;
    }
    
    while (BasicBlock* block = worklist.pop())
        worklist.pushAll(block->predecessors());
    
    RELEASE_ASSERT(worklist.saw(code[0]));
    
    Vector<FrequencyClass> entrypointFrequencies(code.proc().numEntrypoints(), FrequencyClass::Rare);
    for (BasicBlock* block : code) {
        if (block->last().kind.opcode != EntrySwitch)
            continue;
        for (unsigned entrypointIndex = code.proc().numEntrypoints(); entrypointIndex--;) {
            entrypointFrequencies[entrypointIndex] = maxFrequency(
                entrypointFrequencies[entrypointIndex],
                block->successor(entrypointIndex).frequency());
        }
    }
    
    auto fixEntrySwitch = [&] (BasicBlock* block, unsigned entrypointIndex) {
        if (block->last().kind.opcode != EntrySwitch)
            return;
        FrequentedBlock target = block->successor(entrypointIndex);
        block->last().kind.opcode = Jump;
        block->successors().resize(1);
        block->successor(0) = target;
    };
    
    // Now duplicate them.
    Vector<FrequentedBlock> entrypoints;
    entrypoints.append(FrequentedBlock(code[0], entrypointFrequencies[0]));
    IndexMap<BasicBlock, BasicBlock*> map(code.size());
    for (unsigned entrypointIndex = 1; entrypointIndex < code.proc().numEntrypoints(); ++entrypointIndex) {
        map.clear();
        for (BasicBlock* block : worklist.seen().values(code))
            map[block] = code.addBlock(block->frequency());
        entrypoints.append(FrequentedBlock(map[code[0]], entrypointFrequencies[entrypointIndex]));
        for (BasicBlock* block : worklist.seen().values(code)) {
            BasicBlock* newBlock = map[block];
            for (const Inst& inst : *block)
                newBlock->appendInst(inst);
            newBlock->successors() = block->successors();
            for (BasicBlock*& successor : newBlock->successorBlocks()) {
                if (BasicBlock* replacement = map[successor])
                    successor = replacement;
            }
            fixEntrySwitch(newBlock, entrypointIndex);
        }
    }
    for (BasicBlock* block : worklist.seen().values(code))
        fixEntrySwitch(block, 0);
    
    code.setEntrypoints(WTFMove(entrypoints));
    code.resetReachability();
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)


