/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#if ENABLE(DFG_JIT)

#include "DFGAbstractValue.h"
#include "DFGBranchDirection.h"
#include "DFGGraph.h"
#include "DFGNode.h"
#include "DFGNodeFlowProjection.h"
#include "DFGPhiChildren.h"

namespace JSC { namespace DFG {

template<typename AbstractStateType>
class AbstractInterpreter {
public:
    AbstractInterpreter(Graph&, AbstractStateType&);
    ~AbstractInterpreter();
    
    AbstractValue& forNode(NodeFlowProjection node)
    {
        return m_state.forNode(node);
    }
    
    AbstractValue& forNode(Edge edge)
    {
        return forNode(edge.node());
    }
    
    Operands<AbstractValue>& variables()
    {
        return m_state.variables();
    }
    
    bool needsTypeCheck(Node* node, SpeculatedType typesPassedThrough)
    {
        return !forNode(node).isType(typesPassedThrough);
    }
    
    bool needsTypeCheck(Edge edge, SpeculatedType typesPassedThrough)
    {
        return needsTypeCheck(edge.node(), typesPassedThrough);
    }
    
    bool needsTypeCheck(Edge edge)
    {
        return needsTypeCheck(edge, typeFilterFor(edge.useKind()));
    }
    
    // Abstractly executes the given node. The new abstract state is stored into an
    // abstract stack stored in *this. Loads of local variables (that span
    // basic blocks) interrogate the basic block's notion of the state at the head.
    // Stores to local variables are handled in endBasicBlock(). This returns true
    // if execution should continue past this node. Notably, it will return true
    // for block terminals, so long as those terminals are not Return or Unreachable.
    //
    // This is guaranteed to be equivalent to doing:
    //
    // state.startExecuting()
    // state.executeEdges(node);
    // result = state.executeEffects(index);
    bool execute(unsigned indexInBlock);
    bool execute(Node*);
    
    // Indicate the start of execution of a node. It resets any state in the node
    // that is progressively built up by executeEdges() and executeEffects().
    void startExecuting();
    
    // Abstractly execute the edges of the given node. This runs filterEdgeByUse()
    // on all edges of the node. You can skip this step, if you have already used
    // filterEdgeByUse() (or some equivalent) on each edge.
    void executeEdges(Node*);

    void executeKnownEdgeTypes(Node*);
    
    ALWAYS_INLINE void filterEdgeByUse(Edge& edge)
    {
        filterByType(edge, typeFilterFor(edge.useKind()));
    }
    
    // Abstractly execute the effects of the given node. This changes the abstract
    // state assuming that edges have already been filtered.
    bool executeEffects(unsigned indexInBlock);
    bool executeEffects(unsigned clobberLimit, Node*);
    
    void dump(PrintStream& out) const;
    void dump(PrintStream& out);
    
    template<typename T>
    FiltrationResult filter(T node, const RegisteredStructureSet& set, SpeculatedType admittedTypes = SpecNone)
    {
        return filter(forNode(node), set, admittedTypes);
    }
    
    template<typename T>
    FiltrationResult filterArrayModes(T node, ArrayModes arrayModes)
    {
        return filterArrayModes(forNode(node), arrayModes);
    }
    
    template<typename T>
    FiltrationResult filter(T node, SpeculatedType type)
    {
        return filter(forNode(node), type);
    }
    
    template<typename T>
    FiltrationResult filterByValue(T node, FrozenValue value)
    {
        return filterByValue(forNode(node), value);
    }
    
    template<typename T>
    FiltrationResult filterClassInfo(T node, const ClassInfo* classInfo)
    {
        return filterClassInfo(forNode(node), classInfo);
    }

    FiltrationResult filter(AbstractValue&, const RegisteredStructureSet&, SpeculatedType admittedTypes = SpecNone);
    FiltrationResult filterArrayModes(AbstractValue&, ArrayModes);
    FiltrationResult filter(AbstractValue&, SpeculatedType);
    FiltrationResult filterByValue(AbstractValue&, FrozenValue);
    FiltrationResult filterClassInfo(AbstractValue&, const ClassInfo*);
    
    PhiChildren* phiChildren() { return m_phiChildren.get(); }
    
private:
    void clobberWorld(const CodeOrigin&, unsigned indexInBlock);
    
    template<typename Functor>
    void forAllValues(unsigned indexInBlock, Functor&);
    
    void clobberStructures(unsigned indexInBlock);
    void observeTransition(unsigned indexInBlock, RegisteredStructure from, RegisteredStructure to);
    void observeTransitions(unsigned indexInBlock, const TransitionVector&);
    void setDidClobber();
    
    enum BooleanResult {
        UnknownBooleanResult,
        DefinitelyFalse,
        DefinitelyTrue
    };
    BooleanResult booleanResult(Node*, AbstractValue&);
    
    void setBuiltInConstant(Node* node, FrozenValue value)
    {
        AbstractValue& abstractValue = forNode(node);
        abstractValue.set(m_graph, value, m_state.structureClobberState());
        abstractValue.fixTypeForRepresentation(m_graph, node);
    }
    
    void setConstant(Node* node, FrozenValue value)
    {
        setBuiltInConstant(node, value);
        m_state.setFoundConstants(true);
    }
    
    ALWAYS_INLINE void filterByType(Edge& edge, SpeculatedType type)
    {
        AbstractValue& value = forNode(edge);
        if (!value.isType(type))
            edge.setProofStatus(NeedsCheck);
        else
            edge.setProofStatus(IsProved);
        
        filter(value, type);
    }
    
    void verifyEdge(Node*, Edge);
    void verifyEdges(Node*);
    void executeDoubleUnaryOpEffects(Node*, double(*equivalentFunction)(double));
    
    CodeBlock* m_codeBlock;
    Graph& m_graph;
    VM& m_vm;
    AbstractStateType& m_state;
    std::unique_ptr<PhiChildren> m_phiChildren;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
