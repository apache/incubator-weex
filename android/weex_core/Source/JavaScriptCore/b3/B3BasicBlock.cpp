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
#include "B3BasicBlock.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlockInlines.h"
#include "B3BasicBlockUtils.h"
#include "B3Procedure.h"
#include "B3ValueInlines.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 {

const char* const BasicBlock::dumpPrefix = "#";

BasicBlock::BasicBlock(unsigned index, double frequency)
    : m_index(index)
    , m_frequency(frequency)
{
}

BasicBlock::~BasicBlock()
{
}

void BasicBlock::append(Value* value)
{
    m_values.append(value);
    value->owner = this;
}

void BasicBlock::appendNonTerminal(Value* value)
{
    m_values.append(m_values.last());
    m_values[m_values.size() - 1] = value;
    value->owner = this;
}

void BasicBlock::removeLast(Procedure& proc)
{
    ASSERT(!m_values.isEmpty());
    proc.deleteValue(m_values.takeLast());
}

void BasicBlock::replaceLast(Procedure& proc, Value* value)
{
    removeLast(proc);
    append(value);
}

Value* BasicBlock::appendIntConstant(Procedure& proc, Origin origin, Type type, int64_t value)
{
    Value* result = proc.addIntConstant(origin, type, value);
    append(result);
    return result;
}

Value* BasicBlock::appendIntConstant(Procedure& proc, Value* likeValue, int64_t value)
{
    return appendIntConstant(proc, likeValue->origin(), likeValue->type(), value);
}

Value* BasicBlock::appendBoolConstant(Procedure& proc, Origin origin, bool value)
{
    return appendIntConstant(proc, origin, Int32, value ? 1 : 0);
}

void BasicBlock::clearSuccessors()
{
    m_successors.clear();
}

void BasicBlock::appendSuccessor(FrequentedBlock target)
{
    m_successors.append(target);
}

void BasicBlock::setSuccessors(FrequentedBlock target)
{
    m_successors.resize(1);
    m_successors[0] = target;
}

void BasicBlock::setSuccessors(FrequentedBlock taken, FrequentedBlock notTaken)
{
    m_successors.resize(2);
    m_successors[0] = taken;
    m_successors[1] = notTaken;
}

bool BasicBlock::replaceSuccessor(BasicBlock* from, BasicBlock* to)
{
    bool result = false;
    for (BasicBlock*& successor : successorBlocks()) {
        if (successor == from) {
            successor = to;
            result = true;
            
            // Keep looping because a successor may be mentioned multiple times, like in a Switch.
        }
    }
    return result;
}

bool BasicBlock::addPredecessor(BasicBlock* block)
{
    return B3::addPredecessor(this, block);
}

bool BasicBlock::removePredecessor(BasicBlock* block)
{
    return B3::removePredecessor(this, block);
}

bool BasicBlock::replacePredecessor(BasicBlock* from, BasicBlock* to)
{
    return B3::replacePredecessor(this, from, to);
}

void BasicBlock::updatePredecessorsAfter()
{
    B3::updatePredecessorsAfter(this);
}

void BasicBlock::dump(PrintStream& out) const
{
    out.print(dumpPrefix, m_index);
}

void BasicBlock::deepDump(const Procedure& proc, PrintStream& out) const
{
    out.print("BB", *this, ": ; frequency = ", m_frequency, "\n");
    if (predecessors().size())
        out.print("  Predecessors: ", pointerListDump(predecessors()), "\n");
    for (Value* value : *this)
        out.print("    ", B3::deepDump(proc, value), "\n");
    if (!successors().isEmpty()) {
        out.print("  Successors: ");
        if (size())
            last()->dumpSuccessors(this, out);
        else
            out.print(listDump(successors()));
        out.print("\n");
    }
}

Value* BasicBlock::appendNewControlValue(Procedure& proc, Opcode opcode, Origin origin)
{
    RELEASE_ASSERT(opcode == Oops || opcode == Return);
    clearSuccessors();
    return appendNew<Value>(proc, opcode, origin);
}

Value* BasicBlock::appendNewControlValue(Procedure& proc, Opcode opcode, Origin origin, Value* value)
{
    RELEASE_ASSERT(opcode == Return);
    clearSuccessors();
    return appendNew<Value>(proc, opcode, origin, value);
}

Value* BasicBlock::appendNewControlValue(Procedure& proc, Opcode opcode, Origin origin, const FrequentedBlock& target)
{
    RELEASE_ASSERT(opcode == Jump);
    setSuccessors(target);
    return appendNew<Value>(proc, opcode, origin);
}

Value* BasicBlock::appendNewControlValue(Procedure& proc, Opcode opcode, Origin origin, Value* predicate, const FrequentedBlock& taken, const FrequentedBlock& notTaken)
{
    RELEASE_ASSERT(opcode == Branch);
    setSuccessors(taken, notTaken);
    return appendNew<Value>(proc, opcode, origin, predicate);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
