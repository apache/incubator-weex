/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "DFGBasicBlock.h"

#if ENABLE(DFG_JIT)

#include "JSCInlines.h"

namespace JSC { namespace DFG {

BasicBlock::BasicBlock(
    unsigned bytecodeBegin, unsigned numArguments, unsigned numLocals, float executionCount)
    : bytecodeBegin(bytecodeBegin)
    , index(NoBlock)
    , isOSRTarget(false)
    , cfaHasVisited(false)
    , cfaShouldRevisit(false)
    , cfaFoundConstants(false)
    , cfaDidFinish(true)
    , cfaStructureClobberStateAtHead(StructuresAreWatched)
    , cfaStructureClobberStateAtTail(StructuresAreWatched)
    , cfaBranchDirection(InvalidBranchDirection)
#if !ASSERT_DISABLED
    , isLinked(false)
#endif
    , isReachable(false)
    , variablesAtHead(numArguments, numLocals)
    , variablesAtTail(numArguments, numLocals)
    , valuesAtHead(numArguments, numLocals)
    , valuesAtTail(numArguments, numLocals)
    , intersectionOfPastValuesAtHead(numArguments, numLocals, AbstractValue::fullTop())
    , intersectionOfCFAHasVisited(true)
    , executionCount(executionCount)
{
}

BasicBlock::~BasicBlock()
{
}

void BasicBlock::ensureLocals(unsigned newNumLocals)
{
    variablesAtHead.ensureLocals(newNumLocals);
    variablesAtTail.ensureLocals(newNumLocals);
    valuesAtHead.ensureLocals(newNumLocals);
    valuesAtTail.ensureLocals(newNumLocals);
    intersectionOfPastValuesAtHead.ensureLocals(newNumLocals, AbstractValue::fullTop());
}

void BasicBlock::replaceTerminal(Node* node)
{
    NodeAndIndex result = findTerminal();
    if (!result)
        append(node);
    else {
        m_nodes.insert(result.index + 1, node);
        result.node->remove();
    }
    
    ASSERT(terminal());
}

bool BasicBlock::isInPhis(Node* node) const
{
    for (size_t i = 0; i < phis.size(); ++i) {
        if (phis[i] == node)
            return true;
    }
    return false;
}

bool BasicBlock::isInBlock(Node* myNode) const
{
    for (size_t i = 0; i < numNodes(); ++i) {
        if (node(i) == myNode)
            return true;
    }
    return false;
}

void BasicBlock::removePredecessor(BasicBlock* block)
{
    for (unsigned i = 0; i < predecessors.size(); ++i) {
        if (predecessors[i] != block)
            continue;
        predecessors[i] = predecessors.last();
        predecessors.removeLast();
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void BasicBlock::replacePredecessor(BasicBlock* from, BasicBlock* to)
{
    for (unsigned i = predecessors.size(); i--;) {
        if (predecessors[i] != from)
            continue;
        predecessors[i] = to;
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

void BasicBlock::dump(PrintStream& out) const
{
    out.print("#", index);
}

BasicBlock::SSAData::SSAData(BasicBlock* block)
{
    availabilityAtHead.m_locals = Operands<Availability>(OperandsLike, block->variablesAtHead);
    availabilityAtTail.m_locals = Operands<Availability>(OperandsLike, block->variablesAtHead);
}

BasicBlock::SSAData::~SSAData() { }

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

