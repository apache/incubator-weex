/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
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

#include "DFGCommon.h"
#include "DFGDoubleFormatState.h"
#include "DFGFlushFormat.h"
#include "DFGFlushedAt.h"
#include "DFGNodeFlags.h"
#include "Operands.h"
#include "SpeculatedType.h"
#include "VirtualRegister.h"
#include <wtf/UnionFind.h>

namespace JSC { namespace DFG {

struct Node;

enum DoubleBallot { VoteValue, VoteDouble };

class VariableAccessData : public UnionFind<VariableAccessData> {
public:
    VariableAccessData();
    VariableAccessData(VirtualRegister local);
    
    VirtualRegister local()
    {
        ASSERT(m_local == find()->m_local);
        return m_local;
    }
    
    VirtualRegister& machineLocal()
    {
        ASSERT(find() == this);
        return m_machineLocal;
    }

    bool mergeIsProfitableToUnbox(bool isProfitableToUnbox)
    {
        return checkAndSet(m_isProfitableToUnbox, m_isProfitableToUnbox || isProfitableToUnbox);
    }
    
    bool isProfitableToUnbox()
    {
        return m_isProfitableToUnbox;
    }
    
    bool mergeShouldNeverUnbox(bool shouldNeverUnbox);
    
    // Returns true if it would be unsound to store the value in an unboxed fashion.
    // If this returns false, it simply means that it is sound to unbox; it doesn't
    // mean that we have actually done so.
    bool shouldNeverUnbox()
    {
        return m_shouldNeverUnbox;
    }
    
    // Returns true if we should be unboxing the value provided that the predictions
    // and double format vote say so. This may return false even if shouldNeverUnbox()
    // returns false, since this incorporates heuristics of profitability.
    bool shouldUnboxIfPossible()
    {
        return !shouldNeverUnbox() && isProfitableToUnbox();
    }

    bool mergeStructureCheckHoistingFailed(bool failed)
    {
        return checkAndSet(m_structureCheckHoistingFailed, m_structureCheckHoistingFailed || failed);
    }
    
    bool mergeCheckArrayHoistingFailed(bool failed)
    {
        return checkAndSet(m_checkArrayHoistingFailed, m_checkArrayHoistingFailed || failed);
    }
    
    bool structureCheckHoistingFailed()
    {
        return m_structureCheckHoistingFailed;
    }
    
    bool checkArrayHoistingFailed()
    {
        return m_checkArrayHoistingFailed;
    }
    
    bool mergeIsLoadedFrom(bool isLoadedFrom)
    {
        return checkAndSet(m_isLoadedFrom, m_isLoadedFrom || isLoadedFrom);
    }
    
    void setIsLoadedFrom(bool isLoadedFrom)
    {
        m_isLoadedFrom = isLoadedFrom;
    }
    
    bool isLoadedFrom()
    {
        return m_isLoadedFrom;
    }
    
    bool predict(SpeculatedType prediction);
    
    SpeculatedType nonUnifiedPrediction()
    {
        return m_prediction;
    }
    
    SpeculatedType prediction()
    {
        return find()->m_prediction;
    }
    
    SpeculatedType argumentAwarePrediction()
    {
        return find()->m_argumentAwarePrediction;
    }
    
    bool mergeArgumentAwarePrediction(SpeculatedType prediction);
    
    void clearVotes()
    {
        ASSERT(find() == this);
        m_votes[0] = 0;
        m_votes[1] = 0;
    }
    
    void vote(unsigned ballot, float weight = 1)
    {
        ASSERT(ballot < 2);
        m_votes[ballot] += weight;
    }
    
    double voteRatio()
    {
        ASSERT(find() == this);
        return static_cast<double>(m_votes[1]) / m_votes[0];
    }
    
    bool shouldUseDoubleFormatAccordingToVote();
    
    DoubleFormatState doubleFormatState()
    {
        return find()->m_doubleFormatState;
    }
    
    bool shouldUseDoubleFormat()
    {
        ASSERT(isRoot());
        bool doubleState = m_doubleFormatState == UsingDoubleFormat;
        ASSERT(!(doubleState && shouldNeverUnbox()));
        return doubleState && isProfitableToUnbox();
    }
    
    bool tallyVotesForShouldUseDoubleFormat();
    
    bool mergeDoubleFormatState(DoubleFormatState);
    
    bool makePredictionForDoubleFormat();
    
    NodeFlags flags() const { return m_flags; }
    
    bool mergeFlags(NodeFlags newFlags)
    {
        return checkAndSet(m_flags, m_flags | newFlags);
    }
    
    FlushFormat flushFormat();
    
    bool couldRepresentInt52();
    
    FlushedAt flushedAt()
    {
        return FlushedAt(flushFormat(), machineLocal());
    }

private:
    bool couldRepresentInt52Impl();
    
    // This is slightly space-inefficient, since anything we're unified with
    // will have the same operand and should have the same prediction. But
    // putting them here simplifies the code, and we don't expect DFG space
    // usage for variable access nodes do be significant.

    VirtualRegister m_local;
    VirtualRegister m_machineLocal;
    SpeculatedType m_prediction;
    SpeculatedType m_argumentAwarePrediction;
    NodeFlags m_flags;

    bool m_shouldNeverUnbox;
    bool m_structureCheckHoistingFailed;
    bool m_checkArrayHoistingFailed;
    bool m_isProfitableToUnbox;
    bool m_isLoadedFrom;

    float m_votes[2]; // Used primarily for double voting but may be reused for other purposes.
    DoubleFormatState m_doubleFormatState;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
