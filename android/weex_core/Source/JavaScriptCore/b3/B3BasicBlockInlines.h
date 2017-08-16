/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#include "B3BasicBlock.h"
#include "B3ProcedureInlines.h"
#include "B3Value.h"

namespace JSC { namespace B3 {

template<typename ValueType, typename... Arguments>
ValueType* BasicBlock::appendNew(Procedure& procedure, Arguments... arguments)
{
    ValueType* result = procedure.add<ValueType>(arguments...);
    append(result);
    return result;
}

template<typename ValueType, typename... Arguments>
ValueType* BasicBlock::appendNewNonTerminal(Procedure& procedure, Arguments... arguments)
{
    ValueType* result = procedure.add<ValueType>(arguments...);
    appendNonTerminal(result);
    return result;
}

template<typename ValueType, typename... Arguments>
ValueType* BasicBlock::replaceLastWithNew(Procedure& procedure, Arguments... arguments)
{
    ValueType* result = procedure.add<ValueType>(arguments...);
    replaceLast(procedure, result);
    return result;
}

inline const FrequentedBlock& BasicBlock::taken() const
{
    ASSERT(last()->opcode() == Jump || last()->opcode() == Branch);
    return m_successors[0];
}

inline FrequentedBlock& BasicBlock::taken()
{
    ASSERT(last()->opcode() == Jump || last()->opcode() == Branch);
    return m_successors[0];
}

inline const FrequentedBlock& BasicBlock::notTaken() const
{
    ASSERT(last()->opcode() == Branch);
    return m_successors[1];
}

inline FrequentedBlock& BasicBlock::notTaken()
{
    ASSERT(last()->opcode() == Branch);
    return m_successors[1];
}

inline const FrequentedBlock& BasicBlock::fallThrough() const
{
    ASSERT(last()->opcode() == Branch || last()->opcode() == Switch);
    return m_successors.last();
}

inline FrequentedBlock& BasicBlock::fallThrough()
{
    ASSERT(last()->opcode() == Branch || last()->opcode() == Switch);
    return m_successors.last();
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
