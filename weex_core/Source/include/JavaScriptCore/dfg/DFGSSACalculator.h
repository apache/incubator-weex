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

#include "DFGDominators.h"
#include "DFGGraph.h"

namespace JSC { namespace DFG {

// SSACalculator provides a reusable tool for using the Cytron, Ferrante, Rosen, Wegman, and
// Zadeck "Efficiently Computing Static Single Assignment Form and the Control Dependence Graph"
// (TOPLAS'91) algorithm for computing SSA. SSACalculator doesn't magically do everything for you
// but it maintains the major data structures and handles most of the non-local reasoning. Here's
// the workflow of using SSACalculator to execute this algorithm:
//
// 0) Create a fresh SSACalculator instance. You will need this instance only for as long as
//    you're not yet done computing SSA.
//
// 1) Create an SSACalculator::Variable for every variable that you want to do Phi insertion
//    on. SSACalculator::Variable::index() is a dense indexing of the Variables that you
//    created, so you can easily use a Vector to map the SSACalculator::Variables to your
//    variables.
//
// 2) Create a SSACalculator::Def for every assignment to those variables. A Def knows about the
//    variable, the block, and the DFG::Node* that has the value being put into the variable.
//    Note that creating a Def in block B for variable V if block B already has a def for variable
//    V will overwrite the previous Def's DFG::Node* value. This enables you to create Defs by
//    processing basic blocks in forward order. If a block has multiple Defs of a variable, this
//    "just works" because each block will then remember the last Def of each variable.
//
// 3) Call SSACalculator::computePhis(). This takes a functor that will create the Phi nodes. The
//    functor returns either the Phi node it created, or nullptr, if it chooses to prune. (As an
//    aside, it's always sound not to prune, and the safest reason for pruning is liveness.) The
//    computePhis() code will record the created Phi nodes as Defs, and it will separately record
//    the list of Phis inserted at each block. It's OK for the functor you pass here to modify the
//    DFG::Graph on the fly, but the easiest way to write this is to just create the Phi nodes by
//    doing Graph::addNode() and return them. It's then best to insert all Phi nodes for a block
//    in bulk as part of the pass you do below, in step (4).
//
// 4) Modify the graph to create the SSA data flow. For each block, this should:
//
//    4.0) Compute the set of reaching defs (aka available values) for each variable by calling
//         SSACalculator::reachingDefAtHead() for each variable. Record this in a local table that
//         will be incrementally updated as you proceed through the block in forward order in the
//         next steps:
//
//         FIXME: It might be better to compute reaching defs for all live variables in one go, to
//         avoid doing repeated dom tree traversals.
//         https://bugs.webkit.org/show_bug.cgi?id=136610
//
//    4.1) Insert all of the Phi nodes for the block by using SSACalculator::phisForBlock(), and
//         record those Phi nodes as being available values.
//
//    4.2) Process the block in forward order. For each load from a variable, replace it with the
//         available SSA value for that variable. For each store, delete it and record the stored
//         value as being available.
//
//         Note that you have two options of how to replace loads with SSA values. You can replace
//         the load with an Identity node; this will end up working fairly naturally so long as
//         you run GCSE after your phase. Or, you can replace all uses of the load with the SSA
//         value yourself (using the Graph::performSubstitution() idiom), but that requires that
//         your loop over basic blocks proceeds in the appropriate graph order, for example
//         preorder.
//
//         FIXME: Make it easier to do this, that doesn't involve rerunning GCSE.
//         https://bugs.webkit.org/show_bug.cgi?id=136639
//
//    4.3) Insert Upsilons at the end of the current block for the corresponding Phis in each successor block. 
//         Use the available values table to decide the source value for each Phi's variable. Note that 
//         you could also use SSACalculator::reachingDefAtTail() instead of the available values table, 
//         though your local available values table is likely to be more efficient.
//
// The most obvious use of SSACalculator is for the CPS->SSA conversion itself, but it's meant to
// also be used for SSA update and for things like the promotion of heap fields to local SSA
// variables.

class SSACalculator {
public:
    SSACalculator(Graph&);
    ~SSACalculator();
    
    void reset();
    
    class Variable {
    public:
        unsigned index() const { return m_index; }
        
        void dump(PrintStream&) const;
        void dumpVerbose(PrintStream&) const;
        
    private:
        friend class SSACalculator;
        
        Variable()
            : m_index(UINT_MAX)
        {
        }
        
        Variable(unsigned index)
            : m_index(index)
        {
        }

        BlockList m_blocksWithDefs;
        unsigned m_index;
    };
    
    class Def {
    public:
        Variable* variable() const { return m_variable; }
        BasicBlock* block() const { return m_block; }
        
        Node* value() const { return m_value; }
        
        void dump(PrintStream&) const;
        
    private:
        friend class SSACalculator;
        
        Def()
            : m_variable(nullptr)
            , m_block(nullptr)
            , m_value(nullptr)
        {
        }
        
        Def(Variable* variable, BasicBlock* block, Node* value)
            : m_variable(variable)
            , m_block(block)
            , m_value(value)
        {
        }
        
        Variable* m_variable;
        BasicBlock* m_block;
        Node* m_value;
    };
    
    Variable* newVariable();
    Def* newDef(Variable*, BasicBlock*, Node*);
    
    Variable* variable(unsigned index) { return &m_variables[index]; }
    
    // The PhiInsertionFunctor takes a Variable and a BasicBlock and either inserts a Phi and
    // returns the Node for that Phi, or it decides that it's not worth it to insert a Phi at that
    // block because of some additional pruning condition (typically liveness) and returns
    // nullptr. If a non-null Node* is returned, a new Def is created, so that
    // nonLocalReachingDef() will find it later. Note that it is generally always sound to not
    // prune any Phis (that is, to always have the functor insert a Phi and never return nullptr).
    template<typename PhiInsertionFunctor>
    void computePhis(const PhiInsertionFunctor& functor)
    {
        DFG_ASSERT(m_graph, nullptr, m_graph.m_dominators);
        
        for (Variable& variable : m_variables) {
            m_graph.m_dominators->forAllBlocksInPrunedIteratedDominanceFrontierOf(
                variable.m_blocksWithDefs,
                [&] (BasicBlock* block) -> bool {
                    Node* phiNode = functor(&variable, block);
                    if (!phiNode)
                        return false;
                    
                    BlockData& data = m_data[block];
                    Def* phiDef = m_phis.add(Def(&variable, block, phiNode));
                    data.m_phis.append(phiDef);
                    
                    // Note that it's possible to have a block that looks like this before SSA
                    // conversion:
                    //
                    // label:
                    //     print(x);
                    //     ...
                    //     x = 42;
                    //     goto label;
                    //
                    // And it may look like this after SSA conversion:
                    //
                    // label:
                    //     x1: Phi()
                    //     ...
                    //     Upsilon(42, ^x1)
                    //     goto label;
                    //
                    // In this case, we will want to insert a Phi in this block, and the block
                    // will already have a Def for the variable. When this happens, we don't want
                    // the Phi to override the original Def, since the Phi is at the top, the
                    // original Def in the m_defs table would have been at the bottom, and we want
                    // m_defs to tell us about defs at tail.
                    //
                    // So, we rely on the fact that HashMap::add() does nothing if the key was
                    // already present.
                    data.m_defs.add(&variable, phiDef);
                    return true;
                });
        }
    }
    
    const Vector<Def*>& phisForBlock(BasicBlock* block)
    {
        return m_data[block].m_phis;
    }
    
    // Ignores defs within the given block; it assumes that you've taken care of those
    // yourself.
    Def* nonLocalReachingDef(BasicBlock*, Variable*);
    Def* reachingDefAtHead(BasicBlock* block, Variable* variable)
    {
        return nonLocalReachingDef(block, variable);
    }
    
    // Considers the def within the given block, but only works at the tail of the block.
    Def* reachingDefAtTail(BasicBlock*, Variable*);
    
    void dump(PrintStream&) const;
    
private:
    SegmentedVector<Variable> m_variables;
    Bag<Def> m_defs;
    
    Bag<Def> m_phis;
    
    struct BlockData {
        HashMap<Variable*, Def*> m_defs;
        Vector<Def*> m_phis;
    };
    
    BlockMap<BlockData> m_data;
    
    Graph& m_graph;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
