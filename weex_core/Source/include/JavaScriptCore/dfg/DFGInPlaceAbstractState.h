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
#include "DFGFlowMap.h"
#include "DFGGraph.h"
#include "DFGNode.h"

namespace JSC { namespace DFG {

class InPlaceAbstractState {
    WTF_MAKE_FAST_ALLOCATED;
public:
    InPlaceAbstractState(Graph&);
    
    ~InPlaceAbstractState();
    
    explicit operator bool() const { return true; }
    
    void createValueForNode(NodeFlowProjection) { }
    
    AbstractValue& forNode(NodeFlowProjection node)
    {
        return m_abstractValues.at(node);
    }
    
    AbstractValue& forNode(Edge edge)
    {
        return forNode(edge.node());
    }
    
    Operands<AbstractValue>& variables()
    {
        return m_variables;
    }
    
    // Call this before beginning CFA to initialize the abstract values of
    // arguments, and to indicate which blocks should be listed for CFA
    // execution.
    void initialize();

    // Start abstractly executing the given basic block. Initializes the
    // notion of abstract state to what we believe it to be at the head
    // of the basic block, according to the basic block's data structures.
    // This method also sets cfaShouldRevisit to false.
    void beginBasicBlock(BasicBlock*);
    
    BasicBlock* block() const { return m_block; }
    
    // Finish abstractly executing a basic block. If MergeToTail or
    // MergeToSuccessors is passed, then this merges everything we have
    // learned about how the state changes during this block's execution into
    // the block's data structures.
    //
    // Returns true if the state of the block at the tail was changed,
    // and, if the state at the heads of successors was changed.
    // A true return means that you must revisit (at least) the successor
    // blocks. This also sets cfaShouldRevisit to true for basic blocks
    // that must be visited next.
    bool endBasicBlock();
    
    // Reset the AbstractState. This throws away any results, and at this point
    // you can safely call beginBasicBlock() on any basic block.
    void reset();
    
    // Did the last executed node clobber the world?
    bool didClobber() const { return m_didClobber; }
    
    // Are structures currently clobbered?
    StructureClobberState structureClobberState() const { return m_structureClobberState; }
    
    // Is the execution state still valid? This will be false if execute() has
    // returned false previously.
    bool isValid() const { return m_isValid; }
    
    // Merge the abstract state stored at the first block's tail into the second
    // block's head. Returns true if the second block's state changed. If so,
    // that block must be abstractly interpreted again. This also sets
    // to->cfaShouldRevisit to true, if it returns true, or if to has not been
    // visited yet.
    bool merge(BasicBlock* from, BasicBlock* to);
    
    // Merge the abstract state stored at the block's tail into all of its
    // successors. Returns true if any of the successors' states changed. Note
    // that this is automatically called in endBasicBlock() if MergeMode is
    // MergeToSuccessors.
    bool mergeToSuccessors(BasicBlock*);
    
    // Methods intended to be called from AbstractInterpreter.
    void setDidClobber(bool didClobber) { m_didClobber = didClobber; }
    void setStructureClobberState(StructureClobberState value) { m_structureClobberState = value; }
    void setIsValid(bool isValid) { m_isValid = isValid; }
    void setBranchDirection(BranchDirection branchDirection) { m_branchDirection = branchDirection; }
    
    // This method is evil - it causes a huge maintenance headache and there is a gross amount of
    // code devoted to it. It would be much nicer to just always run the constant folder on each
    // block. But, the last time we did it, it was a 1% SunSpider regression:
    // https://bugs.webkit.org/show_bug.cgi?id=133947
    // So, we should probably keep this method.
    void setFoundConstants(bool foundConstants) { m_foundConstants = foundConstants; }

private:
    void mergeStateAtTail(AbstractValue& destination, AbstractValue& inVariable, Node*);

    static bool mergeVariableBetweenBlocks(AbstractValue& destination, AbstractValue& source, Node* destinationNode, Node* sourceNode);
    
    Graph& m_graph;

    FlowMap<AbstractValue>& m_abstractValues;
    Operands<AbstractValue> m_variables;
    BasicBlock* m_block;
    
    bool m_foundConstants;
    
    bool m_isValid;
    bool m_didClobber;
    StructureClobberState m_structureClobberState;
    
    BranchDirection m_branchDirection; // This is only set for blocks that end in Branch and that execute to completion (i.e. m_isValid == true).
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
