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

#include "config.h"
#include "B3StackmapGenerationParams.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirGenerationContext.h"
#include "B3StackmapValue.h"

namespace JSC { namespace B3 {

using namespace Air;

const RegisterSet& StackmapGenerationParams::usedRegisters() const
{
    return m_value->m_usedRegisters;
}

RegisterSet StackmapGenerationParams::unavailableRegisters() const
{
    RegisterSet result = usedRegisters();
    
    RegisterSet unsavedCalleeSaves = RegisterSet::vmCalleeSaveRegisters();
    for (const RegisterAtOffset& regAtOffset : m_context.code->calleeSaveRegisters())
        unsavedCalleeSaves.clear(regAtOffset.reg());

    result.merge(unsavedCalleeSaves);

    for (GPRReg gpr : m_gpScratch)
        result.clear(gpr);
    for (FPRReg fpr : m_fpScratch)
        result.clear(fpr);
    
    return result;
}

Vector<Box<CCallHelpers::Label>> StackmapGenerationParams::successorLabels() const
{
    RELEASE_ASSERT(m_context.indexInBlock == m_context.currentBlock->size() - 1);
    RELEASE_ASSERT(m_value->effects().terminal);
    
    Vector<Box<CCallHelpers::Label>> result(m_context.currentBlock->numSuccessors());
    for (unsigned i = m_context.currentBlock->numSuccessors(); i--;)
        result[i] = m_context.blockLabels[m_context.currentBlock->successorBlock(i)];
    return result;
}

bool StackmapGenerationParams::fallsThroughToSuccessor(unsigned successorIndex) const
{
    RELEASE_ASSERT(m_context.indexInBlock == m_context.currentBlock->size() - 1);
    RELEASE_ASSERT(m_value->effects().terminal);
    
    Air::BasicBlock* successor = m_context.currentBlock->successorBlock(successorIndex);
    Air::BasicBlock* nextBlock = m_context.code->findNextBlock(m_context.currentBlock);
    return successor == nextBlock;
}

Procedure& StackmapGenerationParams::proc() const
{
    return m_context.code->proc();
}

StackmapGenerationParams::StackmapGenerationParams(
    StackmapValue* value, const Vector<ValueRep>& reps, Air::GenerationContext& context)
    : m_value(value)
    , m_reps(reps)
    , m_context(context)
{
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

