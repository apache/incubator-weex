/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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
#include "DFGStructureRegistrationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class StructureRegistrationPhase : public Phase {
public:
    StructureRegistrationPhase(Graph& graph)
        : Phase(graph, "structure registration")
    {
    }
    
    bool run()
    {
        // FIXME: This phase shouldn't exist. We should have registered all structures by now, since
        // we may already have done optimizations that rely on structures having been registered.
        // Currently, we still have places where we don't register structures prior to this phase,
        // but structures don't end up being used for optimization prior to this phase. That's a
        // pretty fragile situation and we should fix it eventually.
        // https://bugs.webkit.org/show_bug.cgi?id=147889
        
        // We need to set this before this phase finishes. This phase doesn't do anything
        // conditioned on this field, except for assertIsRegistered() below. We intend for that
        // method to behave as if the phase was already finished. So, we set this up here.
        m_graph.m_structureRegistrationState = AllStructuresAreRegistered;
        
        // These are pretty dumb, but needed to placate subsequent assertions. We don't actually
        // have to watch these because there is no way to transition away from it, but they are
        // watchable and so we will assert if they aren't watched.
        registerStructure(m_graph.m_vm.structureStructure.get());
        registerStructure(m_graph.m_vm.stringStructure.get());
        registerStructure(m_graph.m_vm.symbolStructure.get());
        
        for (FrozenValue* value : m_graph.m_frozenValues)
            assertIsRegistered(value->structure());
        
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
        
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                Node* node = block->at(nodeIndex);
            
                switch (node->op()) {
                case CheckStructure:
                    assertAreRegistered(node->structureSet());
                    break;
                
                case NewObject:
                case ArrayifyToStructure:
                case NewStringObject:
                    registerStructure(node->structure());
                    break;
                
                case PutStructure:
                case AllocatePropertyStorage:
                case ReallocatePropertyStorage:
                    registerStructure(node->transition()->previous);
                    registerStructure(node->transition()->next);
                    break;

                case GetGetterSetterByOffset:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->getterSetterStructure());
                    break;

                case MultiGetByOffset:
                    for (const MultiGetByOffsetCase& getCase : node->multiGetByOffsetData().cases)
                        registerStructures(getCase.set());
                    break;
                    
                case MultiPutByOffset:
                    for (unsigned i = node->multiPutByOffsetData().variants.size(); i--;) {
                        PutByIdVariant& variant = node->multiPutByOffsetData().variants[i];
                        registerStructures(variant.oldStructure());
                        if (variant.kind() == PutByIdVariant::Transition)
                            registerStructure(variant.newStructure());
                    }
                    break;
                    
                case NewArray:
                case NewArrayBuffer:
                case NewArrayWithSize: {
                    JSGlobalObject* globalObject = m_graph.globalObjectFor(node->origin.semantic);
                    registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(node->indexingType()));
                    registerStructure(globalObject->originalArrayStructureForIndexingType(ArrayWithSlowPutArrayStorage));
                    break;
                }

                case CreateRest: {
                    if (m_graph.isWatchingHavingABadTimeWatchpoint(node)) {
                        JSGlobalObject* globalObject = m_graph.globalObjectFor(node->origin.semantic);
                        registerStructure(globalObject->restParameterStructure());
                    }
                    break;
                }
                    
                case NewTypedArray:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->typedArrayStructureConcurrently(node->typedArrayType()));
                    break;
                    
                case ToString:
                case CallStringConstructor:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->stringObjectStructure());
                    break;
                    
                case CreateActivation:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->activationStructure());
                    break;
                    
                case CreateDirectArguments:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->directArgumentsStructure());
                    break;
                    
                case CreateScopedArguments:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->scopedArgumentsStructure());
                    break;

                case CreateClonedArguments:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->clonedArgumentsStructure());
                    break;

                case NewRegexp:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->regExpStructure());
                    break;
                case NewFunction:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->functionStructure());
                    break;
                case NewGeneratorFunction:
                    registerStructure(m_graph.globalObjectFor(node->origin.semantic)->generatorFunctionStructure());
                    break;

                default:
                    break;
                }
            }
        }
        
        return true;
    }

private:
    void registerStructures(const StructureSet& set)
    {
        for (Structure* structure : set)
            registerStructure(structure);
    }
    
    void registerStructure(Structure* structure)
    {
        if (structure)
            m_graph.registerStructure(structure);
    }

    void assertAreRegistered(const StructureSet& set)
    {
        for (Structure* structure : set)
            assertIsRegistered(structure);
    }

    void assertIsRegistered(Structure* structure)
    {
        if (structure)
            m_graph.assertIsRegistered(structure);
    }
};

bool performStructureRegistration(Graph& graph)
{
    return runPhase<StructureRegistrationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

