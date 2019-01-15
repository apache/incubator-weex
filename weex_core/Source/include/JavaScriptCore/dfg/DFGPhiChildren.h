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

#include "DFGNode.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

class Graph;

class PhiChildren {
    WTF_MAKE_FAST_ALLOCATED;
public:
    typedef Vector<Node*, 3> List;
    
    PhiChildren();
    PhiChildren(Graph&);
    ~PhiChildren();
    
    // The list of Upsilons that point to the children of the Phi.
    const List& upsilonsOf(Node*) const;
    
    template<typename Functor>
    void forAllIncomingValues(Node* node, const Functor& functor)
    {
        for (Node* upsilon : upsilonsOf(node))
            functor(upsilon->child1().node());
    }
    
    // This walks the Phi graph.
    template<typename Functor>
    void forAllTransitiveIncomingValues(Node* node, const Functor& functor)
    {
        if (node->op() != Phi) {
            functor(node);
            return;
        }
        HashSet<Node*> seen;
        Vector<Node*> worklist;
        seen.add(node);
        worklist.append(node);
        while (!worklist.isEmpty()) {
            Node* currentNode = worklist.takeLast();
            forAllIncomingValues(
                currentNode,
                [&] (Node* incomingNode) {
                    if (incomingNode->op() == Phi) {
                        if (seen.add(incomingNode).isNewEntry)
                            worklist.append(incomingNode);
                    } else
                        functor(incomingNode);
                });
        }
    }
    
private:
    HashMap<Node*, List> m_children;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
