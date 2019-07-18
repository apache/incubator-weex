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
