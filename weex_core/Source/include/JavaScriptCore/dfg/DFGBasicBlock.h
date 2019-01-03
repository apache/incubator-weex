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
#include "DFGAvailability.h"
#include "DFGAvailabilityMap.h"
#include "DFGBranchDirection.h"
#include "DFGFlushedAt.h"
#include "DFGNode.h"
#include "DFGNodeAbstractValuePair.h"
#include "DFGNodeOrigin.h"
#include "DFGStructureClobberState.h"
#include "Operands.h"
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

class Graph;
class InsertionSet;

typedef Vector<BasicBlock*, 2> PredecessorList;
typedef Vector<Node*, 8> BlockNodeList;

struct BasicBlock : RefCounted<BasicBlock> {
    BasicBlock(
        unsigned bytecodeBegin, unsigned numArguments, unsigned numLocals,
        float executionCount);
    ~BasicBlock();
    
    void ensureLocals(unsigned newNumLocals);
    
    size_t size() const { return m_nodes.size(); }
    bool isEmpty() const { return !size(); }
    Node*& at(size_t i) { return m_nodes[i]; }
    Node* at(size_t i) const { return m_nodes[i]; }
    Node* tryAt(size_t i) const
    {
        if (i >= size())
            return nullptr;
        return at(i);
    }
    Node*& operator[](size_t i) { return at(i); }
    Node* operator[](size_t i) const { return at(i); }
    
    // Use this to find both the index of the terminal and the terminal itself in one go. May
    // return a clear NodeAndIndex if the basic block currently lacks a terminal. That may happen
    // in the middle of IR transformations within a phase but should never be the case in between
    // phases.
    //
    // The reason why this is more than just "at(size() - 1)" is that we may place non-terminal
    // liveness marking instructions after the terminal. This is supposed to happen infrequently
    // but some basic blocks - most notably return blocks - will have liveness markers for all of
    // the flushed variables right after the return.
    //
    // It turns out that doing this linear search is basically perf-neutral, so long as we force
    // the method to be inlined. Hence the ALWAYS_INLINE.
    ALWAYS_INLINE NodeAndIndex findTerminal() const
    {
        size_t i = size();
        while (i--) {
            Node* node = at(i);
            switch (node->op()) {
            case Jump:
            case Branch:
            case Switch:
            case Return:
            case TailCall:
            case DirectTailCall:
            case TailCallVarargs:
            case TailCallForwardVarargs:
            case Unreachable:
                return NodeAndIndex(node, i);
            // The bitter end can contain Phantoms and the like. There will probably only be one or two nodes after the terminal. They are all no-ops and will not have any checked children.
            case Check: // This is here because it's our universal no-op.
            case Phantom:
            case PhantomLocal:
            case Flush:
                break;
            default:
                return NodeAndIndex();
            }
        }
        return NodeAndIndex();
    }
    
    ALWAYS_INLINE Node* terminal() const
    {
        return findTerminal().node;
    }
    
    void resize(size_t size) { m_nodes.resize(size); }
    void grow(size_t size) { m_nodes.grow(size); }
    
    void append(Node* node) { m_nodes.append(node); }
    void insertBeforeTerminal(Node* node)
    {
        NodeAndIndex result = findTerminal();
        if (!result)
            append(node);
        else
            m_nodes.insert(result.index, node);
    }
    
    void replaceTerminal(Node*);
    
    size_t numNodes() const { return phis.size() + size(); }
    Node* node(size_t i) const
    {
        if (i < phis.size())
            return phis[i];
        return at(i - phis.size());
    }
    bool isPhiIndex(size_t i) const { return i < phis.size(); }
    
    bool isInPhis(Node* node) const;
    bool isInBlock(Node* myNode) const;
    
    BlockNodeList::iterator begin() { return m_nodes.begin(); }
    BlockNodeList::iterator end() { return m_nodes.end(); }

    unsigned numSuccessors() { return terminal()->numSuccessors(); }
    
    BasicBlock*& successor(unsigned index)
    {
        return terminal()->successor(index);
    }
    BasicBlock*& successorForCondition(bool condition)
    {
        return terminal()->successorForCondition(condition);
    }

    Node::SuccessorsIterable successors()
    {
        return terminal()->successors();
    }
    
    void removePredecessor(BasicBlock* block);
    void replacePredecessor(BasicBlock* from, BasicBlock* to);

    template<typename... Params>
    Node* appendNode(Graph&, SpeculatedType, Params...);
    
    template<typename... Params>
    Node* appendNonTerminal(Graph&, SpeculatedType, Params...);
    
    template<typename... Params>
    Node* replaceTerminal(Graph&, SpeculatedType, Params...);
    
    void dump(PrintStream& out) const;
    
    void didLink()
    {
#if !ASSERT_DISABLED
        isLinked = true;
#endif
    }
    
    // This value is used internally for block linking and OSR entry. It is mostly meaningless
    // for other purposes due to inlining.
    unsigned bytecodeBegin;
    
    BlockIndex index;
    
    bool isOSRTarget;
    bool cfaHasVisited;
    bool cfaShouldRevisit;
    bool cfaFoundConstants;
    bool cfaDidFinish;
    StructureClobberState cfaStructureClobberStateAtHead;
    StructureClobberState cfaStructureClobberStateAtTail;
    BranchDirection cfaBranchDirection;
#if !ASSERT_DISABLED
    bool isLinked;
#endif
    bool isReachable;
    
    Vector<Node*> phis;
    PredecessorList predecessors;
    
    Operands<Node*> variablesAtHead;
    Operands<Node*> variablesAtTail;
    
    Operands<AbstractValue> valuesAtHead;
    Operands<AbstractValue> valuesAtTail;
    
    // The intersection of assumptions we have made previously at the head of this block. Note
    // that under normal circumstances, each time we run the CFA, we will get strictly more precise
    // results. But we don't actually require this to be the case. It's fine for the CFA to loosen
    // up for any odd reason. It's fine when this happens, because anything that the CFA proves
    // must be true from that point forward, except if some registered watchpoint fires, in which
    // case the code won't ever run. So, the CFA proving something less precise later on is just an
    // outcome of the CFA being imperfect; the more precise thing that it had proved earlier is no
    // less true.
    //
    // But for the purpose of OSR entry, we need to make sure that we remember what assumptions we
    // had used for optimizing any given basic block. That's what this is for.
    //
    // It's interesting that we could use this to make the CFA more precise: all future CFAs could
    // filter their results with this thing to sort of maintain maximal precision. Because we
    // expect CFA to usually be monotonically more precise each time we run it to fixpoint, this
    // would not be a productive optimization: it would make setting up a basic block more
    // expensive and would only benefit bizarre pathological cases.
    Operands<AbstractValue> intersectionOfPastValuesAtHead;
    bool intersectionOfCFAHasVisited;
    
    float executionCount;
    
    // These fields are reserved for NaturalLoops.
    static const unsigned numberOfInnerMostLoopIndices = 2;
    unsigned innerMostLoopIndices[numberOfInnerMostLoopIndices];

    struct SSAData {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        void invalidate()
        {
            liveAtTail.clear();
            liveAtHead.clear();
            valuesAtHead.clear();
            valuesAtTail.clear();
        }

        AvailabilityMap availabilityAtHead;
        AvailabilityMap availabilityAtTail;

        Vector<NodeFlowProjection> liveAtHead;
        Vector<NodeFlowProjection> liveAtTail;
        Vector<NodeAbstractValuePair> valuesAtHead;
        Vector<NodeAbstractValuePair> valuesAtTail;
        
        SSAData(BasicBlock*);
        ~SSAData();
    };
    std::unique_ptr<SSAData> ssa;
    
private:
    friend class InsertionSet;
    BlockNodeList m_nodes;
};

typedef Vector<BasicBlock*, 5> BlockList;

struct UnlinkedBlock {
    BasicBlock* m_block;
    bool m_needsNormalLinking;
    bool m_needsEarlyReturnLinking;
    
    UnlinkedBlock() { }
    
    explicit UnlinkedBlock(BasicBlock* block)
        : m_block(block)
        , m_needsNormalLinking(true)
        , m_needsEarlyReturnLinking(false)
    {
    }
};
    
static inline unsigned getBytecodeBeginForBlock(BasicBlock** basicBlock)
{
    return (*basicBlock)->bytecodeBegin;
}

static inline BasicBlock* blockForBytecodeOffset(Vector<BasicBlock*>& linkingTargets, unsigned bytecodeBegin)
{
    return *binarySearch<BasicBlock*, unsigned>(linkingTargets, linkingTargets.size(), bytecodeBegin, getBytecodeBeginForBlock);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
