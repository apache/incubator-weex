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
#include "AirBasicBlock.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlockUtils.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 { namespace Air {

const char* const BasicBlock::dumpPrefix = "#";

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

void BasicBlock::dump(PrintStream& out) const
{
    out.print(dumpPrefix, m_index);
}

void BasicBlock::deepDump(PrintStream& out) const
{
    dumpHeader(out);
    for (const Inst& inst : *this)
        out.print("    ", inst, "\n");
    dumpFooter(out);
}

void BasicBlock::dumpHeader(PrintStream& out) const
{
    out.print("BB", *this, ": ; frequency = ", m_frequency, "\n");
    if (predecessors().size())
        out.print("  Predecessors: ", pointerListDump(predecessors()), "\n");
}

void BasicBlock::dumpFooter(PrintStream& out) const
{
    if (successors().size())
        out.print("  Successors: ", listDump(successors()), "\n");
}

BasicBlock::BasicBlock(unsigned index, double frequency)
    : m_index(index)
    , m_frequency(frequency)
{
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
