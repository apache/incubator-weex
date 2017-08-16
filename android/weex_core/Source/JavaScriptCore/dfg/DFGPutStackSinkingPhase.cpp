/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
#include "DFGPutStackSinkingPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBlockMapInlines.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "DFGPreciseLocalClobberize.h"
#include "DFGSSACalculator.h"
#include "DFGValidate.h"
#include "JSCInlines.h"
#include "OperandsInlines.h"

namespace JSC { namespace DFG {

namespace {

bool verbose = false;

class PutStackSinkingPhase : public Phase {
public:
    PutStackSinkingPhase(Graph& graph)
        : Phase(graph, "PutStack sinking")
    {
    }
    
    bool run()
    {
        // FIXME: One of the problems of this approach is that it will create a duplicate Phi graph
        // for sunken PutStacks in the presence of interesting control flow merges, and where the
        // value being PutStack'd is also otherwise live in the DFG code. We could work around this
        // by doing the sinking over CPS, or maybe just by doing really smart hoisting. It's also
        // possible that the duplicate Phi graph can be deduplicated by B3. It would be best if we
        // could observe that there is already a Phi graph in place that does what we want. In
        // principle if we have a request to place a Phi at a particular place, we could just check
        // if there is already a Phi that does what we want. Because PutStackSinkingPhase runs just
        // after SSA conversion, we have almost a guarantee that the Phi graph we produce here would
        // be trivially redundant to the one we already have.
        
        // FIXME: This phase doesn't adequately use KillStacks. KillStack can be viewed as a def.
        // This is mostly inconsequential; it would be a bug to have a local live at a KillStack.
        // More important is that KillStack should swallow any deferral. After a KillStack, the
        // local should behave like a TOP deferral because it would be invalid for anyone to trust
        // the stack. It's not clear to me if this is important or not.
        // https://bugs.webkit.org/show_bug.cgi?id=145296
        
        if (verbose) {
            dataLog("Graph before PutStack sinking:\n");
            m_graph.dump();
        }

        m_graph.ensureDominators();
        
        SSACalculator ssaCalculator(m_graph);
        InsertionSet insertionSet(m_graph);
        
        // First figure out where various locals are live.
        BlockMap<Operands<bool>> liveAtHead(m_graph);
        BlockMap<Operands<bool>> liveAtTail(m_graph);
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            liveAtHead[block] = Operands<bool>(OperandsLike, block->variablesAtHead);
            liveAtTail[block] = Operands<bool>(OperandsLike, block->variablesAtHead);
            
            liveAtHead[block].fill(false);
            liveAtTail[block].fill(false);
        }
        
        bool changed;
        do {
            changed = false;
            
            for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
                BasicBlock* block = m_graph.block(blockIndex);
                if (!block)
                    continue;
                
                Operands<bool> live = liveAtTail[block];
                for (unsigned nodeIndex = block->size(); nodeIndex--;) {
                    Node* node = block->at(nodeIndex);
                    if (verbose)
                        dataLog("Live at ", node, ": ", live, "\n");
                    
                    Vector<VirtualRegister, 4> reads;
                    Vector<VirtualRegister, 4> writes;
                    auto escapeHandler = [&] (VirtualRegister operand) {
                        if (operand.isHeader())
                            return;
                        if (verbose)
                            dataLog("    ", operand, " is live at ", node, "\n");
                        reads.append(operand);
                    };

                    auto writeHandler = [&] (VirtualRegister operand) {
                        RELEASE_ASSERT(node->op() == PutStack || node->op() == LoadVarargs || node->op() == ForwardVarargs);
                        writes.append(operand);
                    };

                    preciseLocalClobberize(
                        m_graph, node, escapeHandler, writeHandler,
                        [&] (VirtualRegister, LazyNode) { });

                    for (VirtualRegister operand : writes)
                        live.operand(operand) = false;
                    for (VirtualRegister operand : reads)
                        live.operand(operand) = true;
                }
                
                if (live == liveAtHead[block])
                    continue;
                
                liveAtHead[block] = live;
                changed = true;
                
                for (BasicBlock* predecessor : block->predecessors) {
                    for (size_t i = live.size(); i--;)
                        liveAtTail[predecessor][i] |= live[i];
                }
            }
            
        } while (changed);
        
        // All of the arguments should be live at head of root. Note that we may find that some
        // locals are live at head of root. This seems wrong but isn't. This will happen for example
        // if the function accesses closure variable #42 for some other function and we either don't
        // have variable #42 at all or we haven't set it at root, for whatever reason. Basically this
        // arises since our aliasing for closure variables is conservatively based on variable number
        // and ignores the owning symbol table. We should probably fix this eventually and make our
        // aliasing more precise.
        //
        // For our purposes here, the imprecision in the aliasing is harmless. It just means that we
        // may not do as much Phi pruning as we wanted.
        for (size_t i = liveAtHead.atIndex(0).numberOfArguments(); i--;)
            DFG_ASSERT(m_graph, nullptr, liveAtHead.atIndex(0).argument(i));
        
        // Next identify where we would want to sink PutStacks to. We say that there is a deferred
        // flush if we had a PutStack with a given FlushFormat but it hasn't been materialized yet.
        // Deferrals have the following lattice; but it's worth noting that the TOP part of the
        // lattice serves an entirely different purpose than the rest of the lattice: it just means
        // that we're in a region of code where nobody should have been relying on the value. The
        // rest of the lattice means that we either have a PutStack that is deferred (i.e. still
        // needs to be executed) or there isn't one (because we've alraedy executed it).
        //
        // Bottom:
        //     Represented as DeadFlush. 
        //     Means that all previous PutStacks have been executed so there is nothing deferred.
        //     During merging this is subordinate to the other kinds of deferrals, because it
        //     represents the fact that we've already executed all necessary PutStacks. This implies
        //     that there *had* been some PutStacks that we should have executed.
        //
        // Top:
        //     Represented as ConflictingFlush.
        //     Represents the fact that we know, via forward flow, that there isn't any value in the
        //     given local that anyone should have been relying on. This comes into play at the
        //     prologue (because in SSA form at the prologue no local has any value) or when we merge
        //     deferrals for different formats's. A lexical scope in which a local had some semantic
        //     meaning will by this point share the same format; if we had stores from different
        //     lexical scopes that got merged together then we may have a conflicting format. Hence
        //     a conflicting format proves that we're no longer in an area in which the variable was
        //     in scope. Note that this is all approximate and only precise enough to later answer
        //     questions pertinent to sinking. For example, this doesn't always detect when a local
        //     is no longer semantically relevant - we may well have a deferral from inside some
        //     inlined call survive outside of that inlined code, and this is generally OK. In the
        //     worst case it means that we might think that a deferral that is actually dead must
        //     still be executed. But we usually catch that with liveness. Liveness usually catches
        //     such cases, but that's not guaranteed since liveness is conservative.
        //
        //     What Top does give us is detects situations where we both don't need to care about a
        //     deferral and there is no way that we could reason about it anyway. If we merged
        //     deferrals for different formats then we wouldn't know the format to use. So, we use
        //     Top in that case because that's also a case where we know that we can ignore the
        //     deferral.
        //
        // Deferral with a concrete format:
        //     Represented by format values other than DeadFlush or ConflictingFlush.
        //     Represents the fact that the original code would have done a PutStack but we haven't
        //     identified an operation that would have observed that PutStack.
        //
        // We need to be precise about liveness in this phase because not doing so
        // could cause us to insert a PutStack before a node we thought may escape a 
        // value that it doesn't really escape. Sinking this PutStack above such a node may
        // cause us to insert a GetStack that we forward to the Phi we're feeding into the
        // sunken PutStack. Inserting such a GetStack could cause us to load garbage and
        // can confuse the AI to claim untrue things (like that the program will exit when
        // it really won't).
        BlockMap<Operands<FlushFormat>> deferredAtHead(m_graph);
        BlockMap<Operands<FlushFormat>> deferredAtTail(m_graph);
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            deferredAtHead[block] =
                Operands<FlushFormat>(OperandsLike, block->variablesAtHead);
            deferredAtTail[block] =
                Operands<FlushFormat>(OperandsLike, block->variablesAtHead);
        }

        for (unsigned local = deferredAtHead.atIndex(0).numberOfLocals(); local--;)
            deferredAtHead.atIndex(0).local(local) = ConflictingFlush;
        
        do {
            changed = false;
            
            for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
                Operands<FlushFormat> deferred = deferredAtHead[block];
                
                for (Node* node : *block) {
                    if (verbose)
                        dataLog("Deferred at ", node, ":", deferred, "\n");
                    
                    if (node->op() == GetStack) {
                        // Handle the case that the input doesn't match our requirements. This is
                        // really a bug, but it's a benign one if we simply don't run this phase.
                        // It usually arises because of patterns like:
                        //
                        // if (thing)
                        //     PutStack()
                        // ...
                        // if (thing)
                        //     GetStack()
                        //
                        // Or:
                        //
                        // if (never happens)
                        //     GetStack()
                        //
                        // Because this phase runs early in SSA, it should be sensible to enforce
                        // that no such code pattern has arisen yet. So, when validation is
                        // enabled, we assert that we aren't seeing this. But with validation
                        // disabled we silently let this fly and we just abort this phase.
                        // FIXME: Get rid of all remaining cases of conflicting GetStacks.
                        // https://bugs.webkit.org/show_bug.cgi?id=150398

                        bool isConflicting =
                            deferred.operand(node->stackAccessData()->local) == ConflictingFlush;
                        
                        if (validationEnabled())
                            DFG_ASSERT(m_graph, node, !isConflicting);

                        if (isConflicting) {
                            // Oh noes! Abort!!
                            return false;
                        }

                        // A GetStack doesn't affect anything, since we know which local we are reading
                        // from.
                        continue;
                    } else if (node->op() == PutStack) {
                        VirtualRegister operand = node->stackAccessData()->local;
                        deferred.operand(operand) = node->stackAccessData()->format;
                        continue;
                    }
                    
                    auto escapeHandler = [&] (VirtualRegister operand) {
                        if (verbose)
                            dataLog("For ", node, " escaping ", operand, "\n");
                        if (operand.isHeader())
                            return;
                        // We will materialize just before any reads.
                        deferred.operand(operand) = DeadFlush;
                    };

                    auto writeHandler = [&] (VirtualRegister operand) {
                        RELEASE_ASSERT(node->op() == LoadVarargs || node->op() == ForwardVarargs);
                        deferred.operand(operand) = DeadFlush;
                    };
                    
                    preciseLocalClobberize(
                        m_graph, node, escapeHandler, writeHandler,
                        [&] (VirtualRegister, LazyNode) { });
                }
                
                if (deferred == deferredAtTail[block])
                    continue;
                
                deferredAtTail[block] = deferred;
                changed = true;
                
                for (BasicBlock* successor : block->successors()) {
                    for (size_t i = deferred.size(); i--;) {
                        if (verbose)
                            dataLog("Considering ", VirtualRegister(deferred.operandForIndex(i)), " at ", pointerDump(block), "->", pointerDump(successor), ": ", deferred[i], " and ", deferredAtHead[successor][i], " merges to ");

                        deferredAtHead[successor][i] =
                            merge(deferredAtHead[successor][i], deferred[i]);
                        
                        if (verbose)
                            dataLog(deferredAtHead[successor][i], "\n");
                    }
                }
            }
            
        } while (changed);
        
        // We wish to insert PutStacks at all of the materialization points, which are defined
        // implicitly as the places where we set deferred to Dead while it was previously not Dead.
        // To do this, we may need to build some Phi functions to handle stuff like this:
        //
        // Before:
        //
        //     if (p)
        //         PutStack(r42, @x)
        //     else
        //         PutStack(r42, @y)
        //
        // After:
        //
        //     if (p)
        //         Upsilon(@x, ^z)
        //     else
        //         Upsilon(@y, ^z)
        //     z: Phi()
        //     PutStack(r42, @z)
        //
        // This means that we have an SSACalculator::Variable for each local, and a Def is any
        // PutStack in the original program. The original PutStacks will simply vanish.
        
        Operands<SSACalculator::Variable*> operandToVariable(
            OperandsLike, m_graph.block(0)->variablesAtHead);
        Vector<VirtualRegister> indexToOperand;
        for (size_t i = m_graph.block(0)->variablesAtHead.size(); i--;) {
            VirtualRegister operand(m_graph.block(0)->variablesAtHead.operandForIndex(i));
            
            SSACalculator::Variable* variable = ssaCalculator.newVariable();
            operandToVariable.operand(operand) = variable;
            ASSERT(indexToOperand.size() == variable->index());
            indexToOperand.append(operand);
        }
        
        HashSet<Node*> putStacksToSink;
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            for (Node* node : *block) {
                switch (node->op()) {
                case PutStack:
                    putStacksToSink.add(node);
                    ssaCalculator.newDef(
                        operandToVariable.operand(node->stackAccessData()->local),
                        block, node->child1().node());
                    break;
                case GetStack:
                    ssaCalculator.newDef(
                        operandToVariable.operand(node->stackAccessData()->local),
                        block, node);
                    break;
                default:
                    break;
                }
            }
        }
        
        ssaCalculator.computePhis(
            [&] (SSACalculator::Variable* variable, BasicBlock* block) -> Node* {
                VirtualRegister operand = indexToOperand[variable->index()];
                
                if (!liveAtHead[block].operand(operand))
                    return nullptr;
                
                FlushFormat format = deferredAtHead[block].operand(operand);

                // We could have an invalid deferral because liveness is imprecise.
                if (!isConcrete(format))
                    return nullptr;

                if (verbose)
                    dataLog("Adding Phi for ", operand, " at ", pointerDump(block), "\n");
                
                Node* phiNode = m_graph.addNode(SpecHeapTop, Phi, block->at(0)->origin.withInvalidExit());
                phiNode->mergeFlags(resultFor(format));
                return phiNode;
            });
        
        Operands<Node*> mapping(OperandsLike, m_graph.block(0)->variablesAtHead);
        Operands<FlushFormat> deferred;
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            mapping.fill(nullptr);
            
            for (size_t i = mapping.size(); i--;) {
                VirtualRegister operand(mapping.operandForIndex(i));
                
                SSACalculator::Variable* variable = operandToVariable.operand(operand);
                SSACalculator::Def* def = ssaCalculator.reachingDefAtHead(block, variable);
                if (!def)
                    continue;
                
                mapping.operand(operand) = def->value();
            }
            
            if (verbose)
                dataLog("Mapping at top of ", pointerDump(block), ": ", mapping, "\n");
            
            for (SSACalculator::Def* phiDef : ssaCalculator.phisForBlock(block)) {
                VirtualRegister operand = indexToOperand[phiDef->variable()->index()];
                
                insertionSet.insert(0, phiDef->value());
                
                if (verbose)
                    dataLog("   Mapping ", operand, " to ", phiDef->value(), "\n");
                mapping.operand(operand) = phiDef->value();
            }
            
            deferred = deferredAtHead[block];
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                Node* node = block->at(nodeIndex);
                if (verbose)
                    dataLog("Deferred at ", node, ":", deferred, "\n");
                
                switch (node->op()) {
                case PutStack: {
                    StackAccessData* data = node->stackAccessData();
                    VirtualRegister operand = data->local;
                    deferred.operand(operand) = data->format;
                    if (verbose)
                        dataLog("   Mapping ", operand, " to ", node->child1().node(), " at ", node, "\n");
                    mapping.operand(operand) = node->child1().node();
                    break;
                }
                    
                case GetStack: {
                    StackAccessData* data = node->stackAccessData();
                    FlushFormat format = deferred.operand(data->local);
                    if (!isConcrete(format)) {
                        DFG_ASSERT(
                            m_graph, node,
                            deferred.operand(data->local) != ConflictingFlush);
                        
                        // This means there is no deferral. No deferral means that the most
                        // authoritative value for this stack slot is what is stored in the stack. So,
                        // keep the GetStack.
                        mapping.operand(data->local) = node;
                        break;
                    }
                    
                    // We have a concrete deferral, which means a PutStack that hasn't executed yet. It
                    // would have stored a value with a certain format. That format must match our
                    // format. But more importantly, we can simply use the value that the PutStack would
                    // have stored and get rid of the GetStack.
                    DFG_ASSERT(m_graph, node, format == data->format);
                    
                    Node* incoming = mapping.operand(data->local);
                    node->child1() = incoming->defaultEdge();
                    node->convertToIdentity();
                    break;
                }
                
                default: {
                    auto escapeHandler = [&] (VirtualRegister operand) {
                        if (verbose)
                            dataLog("For ", node, " escaping ", operand, "\n");

                        if (operand.isHeader())
                            return;
                    
                        FlushFormat format = deferred.operand(operand);
                        if (!isConcrete(format)) {
                            // It's dead now, rather than conflicting.
                            deferred.operand(operand) = DeadFlush;
                            return;
                        }
                    
                        // Gotta insert a PutStack.
                        if (verbose)
                            dataLog("Inserting a PutStack for ", operand, " at ", node, "\n");

                        Node* incoming = mapping.operand(operand);
                        DFG_ASSERT(m_graph, node, incoming);
                    
                        insertionSet.insertNode(
                            nodeIndex, SpecNone, PutStack, node->origin,
                            OpInfo(m_graph.m_stackAccessData.add(operand, format)),
                            Edge(incoming, uncheckedUseKindFor(format)));
                    
                        deferred.operand(operand) = DeadFlush;
                    };

                    auto writeHandler = [&] (VirtualRegister operand) {
                        // LoadVarargs and ForwardVarargs are unconditional writes to the stack
                        // locations they claim to write to. They do not read from the stack 
                        // locations they write to. This makes those stack locations dead right 
                        // before a LoadVarargs/ForwardVarargs. This means we should never sink
                        // PutStacks right to this point.
                        RELEASE_ASSERT(node->op() == LoadVarargs || node->op() == ForwardVarargs);
                        deferred.operand(operand) = DeadFlush;
                    };

                    preciseLocalClobberize(
                        m_graph, node, escapeHandler, writeHandler,
                        [&] (VirtualRegister, LazyNode) { });
                    break;
                } }
            }
            
            NodeAndIndex terminal = block->findTerminal();
            size_t upsilonInsertionPoint = terminal.index;
            NodeOrigin upsilonOrigin = terminal.node->origin;
            for (BasicBlock* successorBlock : block->successors()) {
                for (SSACalculator::Def* phiDef : ssaCalculator.phisForBlock(successorBlock)) {
                    Node* phiNode = phiDef->value();
                    SSACalculator::Variable* variable = phiDef->variable();
                    VirtualRegister operand = indexToOperand[variable->index()];
                    if (verbose)
                        dataLog("Creating Upsilon for ", operand, " at ", pointerDump(block), "->", pointerDump(successorBlock), "\n");
                    FlushFormat format = deferredAtHead[successorBlock].operand(operand);
                    DFG_ASSERT(m_graph, nullptr, isConcrete(format));
                    UseKind useKind = uncheckedUseKindFor(format);
                    
                    // We need to get a value for the stack slot. This phase doesn't really have a
                    // good way of determining if a stack location got clobbered. It just knows if
                    // there is a deferral. The lack of a deferral might mean that a PutStack or
                    // GetStack had never happened, or it might mean that the value was read, or
                    // that it was written. It's OK for us to make some bad decisions here, since
                    // GCSE will clean it up anyway.
                    Node* incoming;
                    if (isConcrete(deferred.operand(operand))) {
                        incoming = mapping.operand(operand);
                        DFG_ASSERT(m_graph, phiNode, incoming);
                    } else {
                        // Issue a GetStack to get the value. This might introduce some redundancy
                        // into the code, but if it's bad enough, GCSE will clean it up.
                        incoming = insertionSet.insertNode(
                            upsilonInsertionPoint, SpecNone, GetStack, upsilonOrigin,
                            OpInfo(m_graph.m_stackAccessData.add(operand, format)));
                        incoming->setResult(resultFor(format));
                    }
                    
                    insertionSet.insertNode(
                        upsilonInsertionPoint, SpecNone, Upsilon, upsilonOrigin,
                        OpInfo(phiNode), Edge(incoming, useKind));
                }
            }
            
            insertionSet.execute(block);
        }
        
        // Finally eliminate the sunken PutStacks by turning them into Checks. This keeps whatever
        // type check they were doing.
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            for (auto* node : *block) {
                if (!putStacksToSink.contains(node))
                    continue;
                
                node->remove();
            }
        }
        
        if (verbose) {
            dataLog("Graph after PutStack sinking:\n");
            m_graph.dump();
        }
        
        return true;
    }
};

} // anonymous namespace
    
bool performPutStackSinking(Graph& graph)
{
    return runPhase<PutStackSinkingPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

