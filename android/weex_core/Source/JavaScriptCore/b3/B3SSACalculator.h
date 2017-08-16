/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#if ENABLE(B3_JIT)

#include "B3Dominators.h"
#include "B3ProcedureInlines.h"
#include <wtf/Bag.h>
#include <wtf/IndexMap.h>
#include <wtf/SegmentedVector.h>

namespace JSC { namespace B3 {

// SSACalculator provides a reusable tool for building SSA's. It's modeled after
// DFG::SSACalculator.

class SSACalculator {
public:
    SSACalculator(Procedure&);
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

        Vector<BasicBlock*, 4> m_blocksWithDefs;
        unsigned m_index;
    };

    class Def {
    public:
        Variable* variable() const { return m_variable; }
        BasicBlock* block() const { return m_block; }
        
        Value* value() const { return m_value; }
        
        void dump(PrintStream&) const;
        
    private:
        friend class SSACalculator;
        
        Def()
            : m_variable(nullptr)
            , m_block(nullptr)
            , m_value(nullptr)
        {
        }
        
        Def(Variable* variable, BasicBlock* block, Value* value)
            : m_variable(variable)
            , m_block(block)
            , m_value(value)
        {
        }
        
        Variable* m_variable;
        BasicBlock* m_block;
        Value* m_value;
    };

    Variable* newVariable();
    Def* newDef(Variable*, BasicBlock*, Value*);

    Variable* variable(unsigned index) { return &m_variables[index]; }

    template<typename Functor>
    void computePhis(const Functor& functor)
    {
        m_dominators = &m_proc.dominators();
        for (Variable& variable : m_variables) {
            m_dominators->forAllBlocksInPrunedIteratedDominanceFrontierOf(
                variable.m_blocksWithDefs,
                [&] (BasicBlock* block) -> bool {
                    Value* phi = functor(&variable, block);
                    if (!phi)
                        return false;

                    BlockData& data = m_data[block];
                    Def* phiDef = m_phis.add(Def(&variable, block, phi));
                    data.m_phis.append(phiDef);

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
    
    IndexMap<BasicBlock, BlockData> m_data;

    Dominators* m_dominators { nullptr };
    Procedure& m_proc;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
