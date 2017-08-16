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
#include "B3SwitchValue.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlock.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 {

SwitchValue::~SwitchValue()
{
}

SwitchCase SwitchValue::removeCase(BasicBlock* block, unsigned index)
{
    FrequentedBlock resultBlock = block->successor(index);
    int64_t resultValue = m_values[index];
    block->successor(index) = block->successors().last();
    block->successors().removeLast();
    m_values[index] = m_values.last();
    m_values.removeLast();
    return SwitchCase(resultValue, resultBlock);
}

bool SwitchValue::hasFallThrough(const BasicBlock* block) const
{
    unsigned numSuccessors = block->numSuccessors();
    unsigned numValues = m_values.size();
    RELEASE_ASSERT(numValues == numSuccessors || numValues + 1 == numSuccessors);
    
    return numValues + 1 == numSuccessors;
}

bool SwitchValue::hasFallThrough() const
{
    return hasFallThrough(owner);
}

void SwitchValue::setFallThrough(BasicBlock* block, const FrequentedBlock& target)
{
    if (!hasFallThrough())
        block->successors().append(target);
    else
        block->successors().last() = target;
    ASSERT(hasFallThrough(block));
}

void SwitchValue::appendCase(BasicBlock* block, const SwitchCase& switchCase)
{
    if (!hasFallThrough())
        block->successors().append(switchCase.target());
    else {
        block->successors().append(block->successors().last());
        block->successor(block->numSuccessors() - 2) = switchCase.target();
    }
    m_values.append(switchCase.caseValue());
}

void SwitchValue::setFallThrough(const FrequentedBlock& target)
{
    setFallThrough(owner, target);
}

void SwitchValue::appendCase(const SwitchCase& switchCase)
{
    appendCase(owner, switchCase);
}

void SwitchValue::dumpSuccessors(const BasicBlock* block, PrintStream& out) const
{
    // We must not crash due to a number-of-successors mismatch! Someone debugging a
    // number-of-successors bug will want to dump IR!
    if (numCaseValues() + 1 != block->numSuccessors()) {
        Value::dumpSuccessors(block, out);
        return;
    }
    
    out.print(cases(block));
}

void SwitchValue::dumpMeta(CommaPrinter& comma, PrintStream& out) const
{
    out.print(comma, "cases = [", listDump(m_values), "]");
}

Value* SwitchValue::cloneImpl() const
{
    return new SwitchValue(*this);
}

SwitchValue::SwitchValue(Origin origin, Value* child)
    : Value(CheckedOpcode, Switch, Void, origin, child)
{
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
