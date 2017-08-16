/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#include "DFGSSACalculator.h"

#if ENABLE(DFG_JIT)

#include "DFGBlockMapInlines.h"
#include <wtf/CommaPrinter.h>
#include <wtf/ListDump.h>

namespace JSC { namespace DFG {

void SSACalculator::Variable::dump(PrintStream& out) const
{
    out.print("var", m_index);
}

void SSACalculator::Variable::dumpVerbose(PrintStream& out) const
{
    dump(out);
    if (!m_blocksWithDefs.isEmpty()) {
        out.print("(defs: ");
        CommaPrinter comma;
        for (BasicBlock* block : m_blocksWithDefs)
            out.print(comma, *block);
        out.print(")");
    }
}

void SSACalculator::Def::dump(PrintStream& out) const
{
    out.print("def(", *m_variable, ", ", *m_block, ", ", m_value, ")");
}

SSACalculator::SSACalculator(Graph& graph)
    : m_data(graph)
    , m_graph(graph)
{
}

SSACalculator::~SSACalculator()
{
}

void SSACalculator::reset()
{
    m_variables.clear();
    m_defs.clear();
    m_phis.clear();
    for (BlockIndex blockIndex = m_data.size(); blockIndex--;) {
        m_data[blockIndex].m_defs.clear();
        m_data[blockIndex].m_phis.clear();
    }
}

SSACalculator::Variable* SSACalculator::newVariable()
{
    return &m_variables.alloc(Variable(m_variables.size()));
}

SSACalculator::Def* SSACalculator::newDef(Variable* variable, BasicBlock* block, Node* value)
{
    Def* def = m_defs.add(Def(variable, block, value));
    auto result = m_data[block].m_defs.add(variable, def);
    if (result.isNewEntry)
        variable->m_blocksWithDefs.append(block);
    else
        result.iterator->value = def;
    return def;
}

SSACalculator::Def* SSACalculator::nonLocalReachingDef(BasicBlock* block, Variable* variable)
{
    return reachingDefAtTail(m_graph.m_dominators->idom(block), variable);
}

SSACalculator::Def* SSACalculator::reachingDefAtTail(BasicBlock* block, Variable* variable)
{
    for (; block; block = m_graph.m_dominators->idom(block)) {
        if (Def* def = m_data[block].m_defs.get(variable))
            return def;
    }
    return nullptr;
}

void SSACalculator::dump(PrintStream& out) const
{
    out.print("<Variables: [");
    CommaPrinter comma;
    for (unsigned i = 0; i < m_variables.size(); ++i) {
        out.print(comma);
        m_variables[i].dumpVerbose(out);
    }
    out.print("], Defs: [");
    comma = CommaPrinter();
    for (Def* def : const_cast<SSACalculator*>(this)->m_defs)
        out.print(comma, *def);
    out.print("], Phis: [");
    comma = CommaPrinter();
    for (Def* def : const_cast<SSACalculator*>(this)->m_phis)
        out.print(comma, *def);
    out.print("], Block data: [");
    comma = CommaPrinter();
    for (BlockIndex blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
        BasicBlock* block = m_graph.block(blockIndex);
        if (!block)
            continue;
        
        out.print(comma, *block, "=>(");
        out.print("Defs: {");
        CommaPrinter innerComma;
        for (auto entry : m_data[block].m_defs)
            out.print(innerComma, *entry.key, "->", *entry.value);
        out.print("}, Phis: {");
        innerComma = CommaPrinter();
        for (Def* def : m_data[block].m_phis)
            out.print(innerComma, *def);
        out.print("})");
    }
    out.print("]>");
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

