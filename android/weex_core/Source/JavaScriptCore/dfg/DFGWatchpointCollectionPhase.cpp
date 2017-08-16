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
#include "DFGWatchpointCollectionPhase.h"

#if ENABLE(DFG_JIT)

#include "ArrayPrototype.h"
#include "DFGClobberize.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

// FIXME: Remove this phase entirely by moving the addLazily() calls into either the backend or
// into the phase that performs the optimization. Moving the calls into the backend makes the most
// sense when the intermediate phases don't need to know that the watchpoint was set. Moving the
// calls earlier usually only makes sense if the node's only purpose was to convey the need for
// the watchpoint (like VarInjectionWatchpoint). But, it can also make sense if the fact that the
// watchpoint was set enables other optimizations.
// https://bugs.webkit.org/show_bug.cgi?id=144669

namespace JSC { namespace DFG {

class WatchpointCollectionPhase : public Phase {
    static const bool verbose = false;
    
public:
    WatchpointCollectionPhase(Graph& graph)
        : Phase(graph, "watchpoint collection")
    {
    }
    
    bool run()
    {
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            
            for (unsigned nodeIndex = block->size(); nodeIndex--;) {
                m_node = block->at(nodeIndex);
                handle();
            }
        }
        
        return true;
    }

private:
    void handle()
    {
        switch (m_node->op()) {
        case IsUndefined:
            handleMasqueradesAsUndefined();
            break;
            
        case CompareEq:
            if (m_node->isBinaryUseKind(ObjectUse)
                || (m_node->child1().useKind() == ObjectUse && m_node->child2().useKind() == ObjectOrOtherUse)
                || (m_node->child1().useKind() == ObjectOrOtherUse && m_node->child2().useKind() == ObjectUse)
                || (m_node->child1().useKind() == OtherUse || m_node->child2().useKind() == OtherUse))
                handleMasqueradesAsUndefined();
            break;
            
        case LogicalNot:
        case Branch:
            switch (m_node->child1().useKind()) {
            case ObjectOrOtherUse:
            case UntypedUse:
                handleMasqueradesAsUndefined();
                break;
            default:
                break;
            }
            break;
            
        default:
            break;
        }
    }
    
    void handleMasqueradesAsUndefined()
    {
        if (m_graph.masqueradesAsUndefinedWatchpointIsStillValid(m_node->origin.semantic))
            addLazily(globalObject()->masqueradesAsUndefinedWatchpoint());
    }
    
    void addLazily(WatchpointSet* set)
    {
        m_graph.watchpoints().addLazily(set);
    }
    void addLazily(InlineWatchpointSet& set)
    {
        m_graph.watchpoints().addLazily(set);
    }
    
    JSGlobalObject* globalObject()
    {
        return m_graph.globalObjectFor(m_node->origin.semantic);
    }
    
    Node* m_node;
};

bool performWatchpointCollection(Graph& graph)
{
    return runPhase<WatchpointCollectionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

