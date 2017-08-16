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
#include "DFGPrePostNumbering.h"

#if ENABLE(DFG_JIT)

#include "DFGBlockMapInlines.h"
#include "DFGBlockWorklist.h"
#include "DFGGraph.h"

namespace JSC { namespace DFG {

PrePostNumbering::PrePostNumbering(Graph& graph)
{
    m_map = BlockMap<Numbering>(graph);
    
    PostOrderBlockWorklist worklist;
    worklist.push(graph.block(0));
    unsigned nextPreNumber = 0;
    unsigned nextPostNumber = 0;
    while (BlockWithOrder item = worklist.pop()) {
        switch (item.order) {
        case VisitOrder::Pre:
            m_map[item.node].m_preNumber = nextPreNumber++;
            worklist.pushPost(item.node);
            for (BasicBlock* successor : item.node->successors())
                worklist.push(successor);
            break;
        case VisitOrder::Post:
            m_map[item.node].m_postNumber = nextPostNumber++;
            break;
        }
    }
}

PrePostNumbering::~PrePostNumbering() { }

} } // namespace JSC::DFG

namespace WTF {

using namespace JSC::DFG;

void printInternal(PrintStream& out, EdgeKind kind)
{
    switch (kind) {
    case ForwardEdge:
        out.print("ForwardEdge");
        return;
    case CrossEdge:
        out.print("CrossEdge");
        return;
    case BackEdge:
        out.print("BackEdge");
        return;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)

