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

#include "DFGGraph.h"

namespace JSC { namespace DFG {

class EdgeUsesStructure {
public:
    EdgeUsesStructure()
        : m_result(false)
    {
    }
    
    void operator()(Node*, Edge edge)
    {
        m_result |= usesStructure(edge.useKind());
    }
    
    bool result() const { return m_result; }
    
private:
    bool m_result;
};

inline bool edgesUseStructure(Graph& graph, Node* node)
{
    EdgeUsesStructure edgeUsesStructure;
    DFG_NODE_DO_TO_CHILDREN(graph, node, edgeUsesStructure);
    return edgeUsesStructure.result();
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
