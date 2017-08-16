/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "DFGConstantHoistingPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "DFGPredictionPropagationPhase.h"
#include "DFGVariableAccessDataDump.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

namespace {

class ConstantHoistingPhase : public Phase {
public:
    ConstantHoistingPhase(Graph& graph)
        : Phase(graph, "constant hoisting")
    {
    }
    
    bool run()
    {
        DFG_ASSERT(m_graph, nullptr, m_graph.m_form == SSA);
        
        m_graph.clearReplacements();
        
        HashMap<FrozenValue*, Node*> jsValues;
        HashMap<FrozenValue*, Node*> doubleValues;
        HashMap<FrozenValue*, Node*> int52Values;
        
        auto valuesFor = [&] (NodeType op) -> HashMap<FrozenValue*, Node*>& {
            // Use a roundabout approach because clang thinks that this closure returning a
            // reference to a stack-allocated value in outer scope is a bug. It's not.
            HashMap<FrozenValue*, Node*>* result;
            
            switch (op) {
            case JSConstant:
                result = &jsValues;
                break;
            case DoubleConstant:
                result = &doubleValues;
                break;
            case Int52Constant:
                result = &int52Values;
                break;
            default:
                DFG_CRASH(m_graph, nullptr, "Invalid node type in valuesFor()");
                result = nullptr;
                break;
            }
            
            return *result;
        };
        
        Vector<Node*> toFree;
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            unsigned sourceIndex = 0;
            unsigned targetIndex = 0;
            while (sourceIndex < block->size()) {
                Node* node = block->at(sourceIndex++);
                switch (node->op()) {
                case JSConstant:
                case DoubleConstant:
                case Int52Constant: {
                    HashMap<FrozenValue*, Node*>& values = valuesFor(node->op());
                    auto result = values.add(node->constant(), node);
                    if (result.isNewEntry)
                        node->origin = m_graph.block(0)->at(0)->origin;
                    else {
                        node->setReplacement(result.iterator->value);
                        toFree.append(node);
                    }
                    break;
                }
                default:
                    block->at(targetIndex++) = node;
                    break;
                }
            }
            block->resize(targetIndex);
        }
        
        // Insert the constants into the root block.
        InsertionSet insertionSet(m_graph);
        auto insertConstants = [&] (const HashMap<FrozenValue*, Node*>& values) {
            for (auto& entry : values)
                insertionSet.insert(0, entry.value);
        };
        insertConstants(jsValues);
        insertConstants(doubleValues);
        insertConstants(int52Values);
        insertionSet.execute(m_graph.block(0));
        
        // Perform all of the substitutions. We want all instances of the removed constants to
        // point at their replacements.
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            for (Node* node : *block)
                m_graph.performSubstitution(node);
        }
        
        // And finally free the constants that we removed.
        m_graph.invalidateNodeLiveness();
        for (Node* node : toFree)
            m_graph.deleteNode(node);
        
        return true;
    }
};

} // anonymous namespace
    
bool performConstantHoisting(Graph& graph)
{
    return runPhase<ConstantHoistingPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

