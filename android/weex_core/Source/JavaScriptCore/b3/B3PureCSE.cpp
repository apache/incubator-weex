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
#include "B3PureCSE.h"

#if ENABLE(B3_JIT)

#include "B3Dominators.h"
#include "B3PhaseScope.h"
#include "B3Value.h"

namespace JSC { namespace B3 {

PureCSE::PureCSE()
{
}

PureCSE::~PureCSE()
{
}

void PureCSE::clear()
{
    m_map.clear();
}

Value* PureCSE::findMatch(const ValueKey& key, BasicBlock* block, Dominators& dominators)
{
    if (!key)
        return nullptr;

    auto iter = m_map.find(key);
    if (iter == m_map.end())
        return nullptr;

    for (Value* match : iter->value) {
        if (!match->owner)
            continue;
        if (dominators.dominates(match->owner, block))
            return match;
    }

    return nullptr;
}

bool PureCSE::process(Value* value, Dominators& dominators)
{
    if (value->opcode() == Identity)
        return false;

    ValueKey key = value->key();
    if (!key)
        return false;

    Matches& matches = m_map.add(key, Matches()).iterator->value;

    for (Value* match : matches) {
        if (!match->owner)
            continue;
        if (dominators.dominates(match->owner, value->owner)) {
            value->replaceWithIdentity(match);
            return true;
        }
    }

    matches.append(value);
    return false;
}

bool pureCSE(Procedure& proc)
{
    PhaseScope phaseScope(proc, "pureCSE");
    
    Dominators& dominators = proc.dominators();
    PureCSE pureCSE;
    bool result = false;
    for (BasicBlock* block : proc.blocksInPreOrder()) {
        for (Value* value : *block) {
            result |= value->performSubstitution();
            result |= pureCSE.process(value, dominators);
        }
    }
    
    return result;
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

