/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DFGStoreBarrierInsertionPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGAbstractInterpreterInlines.h"
#include "DFGBlockMapInlines.h"
#include "DFGDoesGC.h"
#include "DFGGraph.h"
#include "DFGInPlaceAbstractState.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "JSCInlines.h"
#include <wtf/CommaPrinter.h>
#include <wtf/HashSet.h>

namespace JSC { namespace DFG {

namespace {

bool verbose = false;

enum class PhaseMode {
    // Does only a local analysis for store barrier insertion and assumes that pointers live
    // from predecessor blocks may need barriers. Assumes CPS conventions. Does not use AI for
    // eliminating store barriers, but does a best effort to eliminate barriers when you're
    // storing a non-cell value by using Node::result() and by looking at constants. The local
    // analysis is based on GC epochs, so it will eliminate a lot of locally redundant barriers.
    Fast,
        
    // Does a global analysis for store barrier insertion. Reuses the GC-epoch-based analysis
    // used by Fast, but adds a conservative merge rule for propagating information from one
    // block to the next. This will ensure for example that if a value V coming from multiple
    // predecessors in B didn't need any more barriers at the end of each predecessor (either
    // because it was the last allocated object in that predecessor or because it just had a
    // barrier executed), then until we hit another GC point in B, we won't need another barrier
    // on V. Uses AI for eliminating barriers when we know that the value being stored is not a
    // cell. Assumes SSA conventions.
    Global
};

template<PhaseMode mode>
class StoreBarrierInsertionPhase : public Phase {
public:
    StoreBarrierInsertionPhase(Graph& graph)
        : Phase(graph, mode == PhaseMode::Fast ? "fast store barrier insertion" : "global store barrier insertion")
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        if (verbose) {
            dataLog("Starting store barrier insertion:\n");
            m_graph.dump();
        }
        
        switch (mode) {
        case PhaseMode::Fast: {
            DFG_ASSERT(m_graph, nullptr, m_graph.m_form != SSA);
            
            m_graph.clearEpochs();
            for (BasicBlock* block : m_graph.blocksInNaturalOrder())
                handleBlock(block);
            return true;
        }
            
        case PhaseMode::Global: {
            DFG_ASSERT(m_graph, nullptr, m_graph.m_form == SSA);

            m_state = std::make_unique<InPlaceAbstractState>(m_graph);
            m_interpreter = std::make_unique<AbstractInterpreter<InPlaceAbstractState>>(m_graph, *m_state);
            
            m_isConverged = false;
            
            // First run the analysis. Inside basic blocks we use an epoch-based analysis that
            // is very precise. At block boundaries, we just propagate which nodes may need a
            // barrier. This gives us a very nice bottom->top fixpoint: we start out assuming
            // that no node needs any barriers at block boundaries, and then we converge
            // towards believing that all nodes need barriers. "Needing a barrier" is like
            // saying that the node is in a past epoch. "Not needing a barrier" is like saying
            // that the node is in the current epoch.
            m_stateAtHead = std::make_unique<BlockMap<HashSet<Node*>>>(m_graph);
            m_stateAtTail = std::make_unique<BlockMap<HashSet<Node*>>>(m_graph);
            
            BlockList postOrder = m_graph.blocksInPostOrder();
            
            bool changed = true;
            while (changed) {
                changed = false;
                
                // Intentional backwards loop because we are using RPO.
                for (unsigned blockIndex = postOrder.size(); blockIndex--;) {
                    BasicBlock* block = postOrder[blockIndex];
                    
                    if (!handleBlock(block)) {
                        // If the block didn't finish, then it cannot affect the fixpoint.
                        continue;
                    }
                    
                    // Construct the state-at-tail based on the epochs of live nodes and the
                    // current epoch. We grow state-at-tail monotonically to ensure convergence.
                    bool thisBlockChanged = false;
                    for (NodeFlowProjection node : block->ssa->liveAtTail) {
                        if (node.kind() == NodeFlowProjection::Shadow)
                            continue;
                        if (node->epoch() != m_currentEpoch) {
                            // If the node is older than the current epoch, then we may need to
                            // run a barrier on it in the future. So, add it to the state.
                            thisBlockChanged |= m_stateAtTail->at(block).add(node.node()).isNewEntry;
                        }
                    }
                    
                    if (!thisBlockChanged) {
                        // This iteration didn't learn anything new about this block.
                        continue;
                    }
                    
                    // Changed things. Make sure that we loop one more time.
                    changed = true;
                    
                    for (BasicBlock* successor : block->successors()) {
                        for (Node* node : m_stateAtTail->at(block))
                            m_stateAtHead->at(successor).add(node);
                    }
                }
            }
            
            // Tell handleBlock() that it's time to actually insert barriers for real.
            m_isConverged = true;
            
            for (BasicBlock* block : m_graph.blocksInNaturalOrder())
                handleBlock(block);
            
            return true;
        } }
        
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }

private:
    bool handleBlock(BasicBlock* block)
    {
        if (verbose) {
            dataLog("Dealing with block ", pointerDump(block), "\n");
            if (reallyInsertBarriers())
                dataLog("    Really inserting barriers.\n");
        }
        
        m_currentEpoch = Epoch::first();
        
        if (mode == PhaseMode::Global) {
            if (!block->cfaHasVisited)
                return false;
            m_state->beginBasicBlock(block);
            
            for (NodeFlowProjection node : block->ssa->liveAtHead) {
                if (node.kind() == NodeFlowProjection::Shadow)
                    continue;
                if (m_stateAtHead->at(block).contains(node.node())) {
                    // If previous blocks tell us that this node may need a barrier in the
                    // future, then put it in the ancient primordial epoch. This forces us to
                    // emit a barrier on any possibly-cell store, regardless of the epoch of the
                    // stored value.
                    node->setEpoch(Epoch());
                } else {
                    // If previous blocks aren't requiring us to run a barrier on this node,
                    // then put it in the current epoch. This means that we will skip barriers
                    // on this node so long as we don't allocate. It also means that we won't
                    // run barriers on stores to on one such node into another such node. That's
                    // fine, because nodes would be excluded from the state set if at the tails
                    // of all predecessors they always had the current epoch.
                    node->setEpoch(m_currentEpoch);
                }
            }
        }

        bool result = true;
        
        for (m_nodeIndex = 0; m_nodeIndex < block->size(); ++m_nodeIndex) {
            m_node = block->at(m_nodeIndex);
            
            if (verbose) {
                dataLog(
                    "    ", m_currentEpoch, ": Looking at node ", m_node, " with children: ");
                CommaPrinter comma;
                m_graph.doToChildren(
                    m_node,
                    [&] (Edge edge) {
                        dataLog(comma, edge, " (", edge->epoch(), ")");
                    });
                dataLog("\n");
            }
            
            if (mode == PhaseMode::Global) {
                // Execute edges separately because we don't want to insert barriers if the
                // operation doing the store does a check that ensures that the child is not
                // a cell.
                m_interpreter->startExecuting();
                m_interpreter->executeEdges(m_node);
            }
            
            switch (m_node->op()) {
            case PutByValDirect:
            case PutByVal:
            case PutByValAlias: {
                switch (m_node->arrayMode().modeForPut().type()) {
                case Array::Contiguous:
                case Array::ArrayStorage:
                case Array::SlowPutArrayStorage: {
                    Edge child1 = m_graph.varArgChild(m_node, 0);
                    Edge child3 = m_graph.varArgChild(m_node, 2);
                    considerBarrier(child1, child3);
                    break;
                }
                default:
                    break;
                }
                break;
            }
                
            case ArrayPush: {
                switch (m_node->arrayMode().type()) {
                case Array::Contiguous:
                case Array::ArrayStorage:
                    considerBarrier(m_node->child1(), m_node->child2());
                    break;
                default:
                    break;
                }
                break;
            }
                
            case PutById:
            case PutByIdFlush:
            case PutByIdDirect:
            case PutStructure: {
                considerBarrier(m_node->child1());
                break;
            }

            case RecordRegExpCachedResult: {
                considerBarrier(m_graph.varArgChild(m_node, 0));
                break;
            }

            case PutClosureVar:
            case PutToArguments:
            case SetRegExpObjectLastIndex: {
                considerBarrier(m_node->child1(), m_node->child2());
                break;
            }
                
            case MultiPutByOffset: {
                considerBarrier(m_node->child1());
                break;
            }
                
            case PutByOffset: {
                considerBarrier(m_node->child2(), m_node->child3());
                break;
            }
                
            case PutGlobalVariable: {
                considerBarrier(m_node->child1(), m_node->child2());
                break;
            }
                
            case SetFunctionName: {
                considerBarrier(m_node->child1(), m_node->child2());
                break;
            }
                
            case NukeStructureAndSetButterfly: {
                considerBarrier(m_node->child1());
                break;
            }

            default:
                break;
            }
            
            if (doesGC(m_graph, m_node))
                m_currentEpoch.bump();
            
            switch (m_node->op()) {
            case NewObject:
            case NewArray:
            case NewArrayWithSize:
            case NewArrayBuffer:
            case NewTypedArray:
            case NewRegexp:
            case MaterializeNewObject:
            case MaterializeCreateActivation:
            case NewStringObject:
            case MakeRope:
            case CreateActivation:
            case CreateDirectArguments:
            case CreateScopedArguments:
            case CreateClonedArguments:
            case NewFunction:
            case NewGeneratorFunction:
            case NewAsyncFunction:
            case AllocatePropertyStorage:
            case ReallocatePropertyStorage:
                // Nodes that allocate get to set their epoch because for those nodes we know
                // that they will be the newest object in the heap.
                m_node->setEpoch(m_currentEpoch);
                break;
                
            case Upsilon:
                // Assume the worst for Phis so that we don't have to worry about Phi shadows.
                m_node->phi()->setEpoch(Epoch());
                m_node->setEpoch(Epoch());
                break;
                
            default:
                // For nodes that aren't guaranteed to allocate, we say that their return value
                // (if there is one) could be arbitrarily old.
                m_node->setEpoch(Epoch());
                break;
            }
            
            if (verbose) {
                dataLog(
                    "    ", m_currentEpoch, ": Done with node ", m_node, " (", m_node->epoch(),
                    ") with children: ");
                CommaPrinter comma;
                m_graph.doToChildren(
                    m_node,
                    [&] (Edge edge) {
                        dataLog(comma, edge, " (", edge->epoch(), ")");
                    });
                dataLog("\n");
            }
            
            if (mode == PhaseMode::Global) {
                if (!m_interpreter->executeEffects(m_nodeIndex, m_node)) {
                    result = false;
                    break;
                }
            }
        }
        
        if (mode == PhaseMode::Global)
            m_state->reset();

        if (reallyInsertBarriers())
            m_insertionSet.execute(block);
        
        return result;
    }
    
    void considerBarrier(Edge base, Edge child)
    {
        if (verbose)
            dataLog("        Considering adding barrier ", base, " => ", child, "\n");
        
        // We don't need a store barrier if the child is guaranteed to not be a cell.
        switch (mode) {
        case PhaseMode::Fast: {
            // Don't try too hard because it's too expensive to run AI.
            if (child->hasConstant()) {
                if (!child->asJSValue().isCell()) {
                    if (verbose)
                        dataLog("            Rejecting because of constant type.\n");
                    return;
                }
            } else {
                switch (child->result()) {
                case NodeResultNumber:
                case NodeResultDouble:
                case NodeResultInt32:
                case NodeResultInt52:
                case NodeResultBoolean:
                    if (verbose)
                        dataLog("            Rejecting because of result type.\n");
                    return;
                default:
                    break;
                }
            }
            break;
        }
            
        case PhaseMode::Global: {
            // Go into rage mode to eliminate any chance of a barrier with a non-cell child. We
            // can afford to keep around AI in Global mode.
            if (!m_interpreter->needsTypeCheck(child, ~SpecCell)) {
                if (verbose)
                    dataLog("            Rejecting because of AI type.\n");
                return;
            }
            break;
        } }
        
        considerBarrier(base);
    }
    
    void considerBarrier(Edge base)
    {
        if (verbose)
            dataLog("        Considering adding barrier on ", base, "\n");
        
        // We don't need a store barrier if the epoch of the base is identical to the current
        // epoch. That means that we either just allocated the object and so it's guaranteed to
        // be in newgen, or we just ran a barrier on it so it's guaranteed to be remembered
        // already.
        if (base->epoch() == m_currentEpoch) {
            if (verbose)
                dataLog("            Rejecting because it's in the current epoch.\n");
            return;
        }
        
        if (verbose)
            dataLog("            Inserting barrier.\n");
        insertBarrier(m_nodeIndex + 1, base);
    }

    void insertBarrier(unsigned nodeIndex, Edge base)
    {
        // This is just our way of saying that barriers are not redundant with each other according
        // to forward analysis: if we proved one time that a barrier was necessary then it'll for
        // sure be necessary next time.
        base->setEpoch(Epoch());

        // If we're in global mode, we should only insert the barriers once we have converged.
        if (!reallyInsertBarriers())
            return;
        
        // FIXME: We could support StoreBarrier(UntypedUse:). That would be sort of cool.
        // But right now we don't need it.

        DFG_ASSERT(m_graph, m_node, isCell(base.useKind()));
        
        // Barriers are always inserted after the node that they service. Therefore, we always know
        // that the thing is a cell now.
        base.setUseKind(KnownCellUse);
        
        NodeOrigin origin = m_node->origin;
        if (clobbersExitState(m_graph, m_node))
            origin = origin.withInvalidExit();
        
        NodeType type;
        if (Options::useConcurrentBarriers())
            type = FencedStoreBarrier;
        else
            type = StoreBarrier;
        
        m_insertionSet.insertNode(nodeIndex, SpecNone, type, origin, base);
    }
    
    bool reallyInsertBarriers()
    {
        return mode == PhaseMode::Fast || m_isConverged;
    }
    
    InsertionSet m_insertionSet;
    Epoch m_currentEpoch;
    unsigned m_nodeIndex;
    Node* m_node;
    
    // Things we only use in Global mode.
    std::unique_ptr<InPlaceAbstractState> m_state;
    std::unique_ptr<AbstractInterpreter<InPlaceAbstractState>> m_interpreter;
    std::unique_ptr<BlockMap<HashSet<Node*>>> m_stateAtHead;
    std::unique_ptr<BlockMap<HashSet<Node*>>> m_stateAtTail;
    bool m_isConverged;
};

} // anonymous namespace

bool performFastStoreBarrierInsertion(Graph& graph)
{
    return runPhase<StoreBarrierInsertionPhase<PhaseMode::Fast>>(graph);
}

bool performGlobalStoreBarrierInsertion(Graph& graph)
{
    return runPhase<StoreBarrierInsertionPhase<PhaseMode::Global>>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

