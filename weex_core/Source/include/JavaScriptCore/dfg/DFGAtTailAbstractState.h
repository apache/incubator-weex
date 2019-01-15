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
#include "DFGBasicBlock.h"
#include "DFGBlockMap.h"
#include "DFGGraph.h"
#include "DFGNodeFlowProjection.h"

namespace JSC { namespace DFG { 

class AtTailAbstractState {
public:
    AtTailAbstractState(Graph&);
    
    ~AtTailAbstractState();
    
    explicit operator bool() const { return true; }
    
    void initializeTo(BasicBlock* block)
    {
        m_block = block;
    }
    
    void createValueForNode(NodeFlowProjection);
    AbstractValue& forNode(NodeFlowProjection);
    AbstractValue& forNode(Edge edge) { return forNode(edge.node()); }
    Operands<AbstractValue>& variables() { return m_block->valuesAtTail; }
    
    BasicBlock* block() const { return m_block; }
    
    bool isValid() { return m_block->cfaDidFinish; }
    
    StructureClobberState structureClobberState() const { return m_block->cfaStructureClobberStateAtTail; }
    
    void setDidClobber(bool) { }
    void setStructureClobberState(StructureClobberState state) { RELEASE_ASSERT(state == m_block->cfaStructureClobberStateAtTail); }
    void setIsValid(bool isValid) { m_block->cfaDidFinish = isValid; }
    void setBranchDirection(BranchDirection) { }
    void setFoundConstants(bool) { }

private:
    Graph& m_graph;
    BlockMap<HashMap<NodeFlowProjection, AbstractValue>> m_valuesAtTailMap;
    BasicBlock* m_block { nullptr };
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
