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
#include "DFGOSREntrypointCreationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGBlockInsertionSet.h"
#include "DFGGraph.h"
#include "DFGLoopPreHeaderCreationPhase.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class OSREntrypointCreationPhase : public Phase {
public:
    OSREntrypointCreationPhase(Graph& graph)
        : Phase(graph, "OSR entrypoint creation")
    {
    }
    
    bool run()
    {
        RELEASE_ASSERT(m_graph.m_plan.mode == FTLForOSREntryMode);
        RELEASE_ASSERT(m_graph.m_form == ThreadedCPS);
        
        unsigned bytecodeIndex = m_graph.m_plan.osrEntryBytecodeIndex;
        RELEASE_ASSERT(bytecodeIndex);
        RELEASE_ASSERT(bytecodeIndex != UINT_MAX);
        
        // Needed by createPreHeader().
        m_graph.ensureDominators();
        
        CodeBlock* baseline = m_graph.m_profiledBlock;
        
        BasicBlock* target = 0;
        for (unsigned blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            unsigned nodeIndex = 0;
            Node* firstNode = block->at(0);
            while (firstNode->isSemanticallySkippable())
                firstNode = block->at(++nodeIndex);
            if (firstNode->op() == LoopHint
                && firstNode->origin.semantic == CodeOrigin(bytecodeIndex)) {
                target = block;
                break;
            }
        }

        if (!target) {
            // This is a terrible outcome. It shouldn't often happen but it might
            // happen and so we should defend against it. If it happens, then this
            // compilation is a failure.
            return false;
        }
        
        BlockInsertionSet insertionSet(m_graph);
        
        // We say that the execution count of the entry block is 1, because we know for sure
        // that this must be the case. Under our definition of executionCount, "1" means "once
        // per invocation". We could have said NaN here, since that would ask any clients of
        // executionCount to use best judgement - but that seems unnecessary since we know for
        // sure what the executionCount should be in this case.
        BasicBlock* newRoot = insertionSet.insert(0, 1);

        // We'd really like to use an unset origin, but ThreadedCPS won't allow that.
        NodeOrigin origin = NodeOrigin(CodeOrigin(0), CodeOrigin(0), false);
        
        Vector<Node*> locals(baseline->m_numCalleeLocals);
        for (int local = 0; local < baseline->m_numCalleeLocals; ++local) {
            Node* previousHead = target->variablesAtHead.local(local);
            if (!previousHead)
                continue;
            VariableAccessData* variable = previousHead->variableAccessData();
            locals[local] = newRoot->appendNode(
                m_graph, variable->prediction(), ExtractOSREntryLocal, origin,
                OpInfo(variable->local().offset()));
            
            newRoot->appendNode(
                m_graph, SpecNone, MovHint, origin, OpInfo(variable->local().offset()),
                Edge(locals[local]));
        }

        // Now use the origin of the target, since it's not OK to exit, and we will probably hoist
        // type checks to here.
        origin = target->at(0)->origin;
        
        for (int argument = 0; argument < baseline->numParameters(); ++argument) {
            Node* oldNode = target->variablesAtHead.argument(argument);
            if (!oldNode) {
                // Just for sanity, always have a SetArgument even if it's not needed.
                oldNode = m_graph.m_arguments[argument];
            }
            Node* node = newRoot->appendNode(
                m_graph, SpecNone, SetArgument, origin,
                OpInfo(oldNode->variableAccessData()));
            m_graph.m_arguments[argument] = node;
        }

        for (int local = 0; local < baseline->m_numCalleeLocals; ++local) {
            Node* previousHead = target->variablesAtHead.local(local);
            if (!previousHead)
                continue;
            VariableAccessData* variable = previousHead->variableAccessData();
            Node* node = locals[local];
            newRoot->appendNode(
                m_graph, SpecNone, SetLocal, origin, OpInfo(variable), Edge(node));
        }
        
        newRoot->appendNode(
            m_graph, SpecNone, Jump, origin,
            OpInfo(createPreHeader(m_graph, insertionSet, target)));
        
        insertionSet.execute();
        m_graph.resetReachability();
        m_graph.killUnreachableBlocks();
        return true;
    }
};

bool performOSREntrypointCreation(Graph& graph)
{
    return runPhase<OSREntrypointCreationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)


