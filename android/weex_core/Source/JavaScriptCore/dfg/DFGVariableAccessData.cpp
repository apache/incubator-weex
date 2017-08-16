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
#include "DFGVariableAccessData.h"

#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

VariableAccessData::VariableAccessData()
    : m_local(static_cast<VirtualRegister>(std::numeric_limits<int>::min()))
    , m_prediction(SpecNone)
    , m_argumentAwarePrediction(SpecNone)
    , m_flags(0)
    , m_shouldNeverUnbox(false)
    , m_structureCheckHoistingFailed(false)
    , m_checkArrayHoistingFailed(false)
    , m_isProfitableToUnbox(false)
    , m_isLoadedFrom(false)
    , m_doubleFormatState(EmptyDoubleFormatState)
{
    clearVotes();
}

VariableAccessData::VariableAccessData(VirtualRegister local)
    : m_local(local)
    , m_prediction(SpecNone)
    , m_argumentAwarePrediction(SpecNone)
    , m_flags(0)
    , m_shouldNeverUnbox(false)
    , m_structureCheckHoistingFailed(false)
    , m_checkArrayHoistingFailed(false)
    , m_isProfitableToUnbox(false)
    , m_isLoadedFrom(false)
    , m_doubleFormatState(EmptyDoubleFormatState)
{
    clearVotes();
}

bool VariableAccessData::mergeShouldNeverUnbox(bool shouldNeverUnbox)
{
    bool newShouldNeverUnbox = m_shouldNeverUnbox | shouldNeverUnbox;
    if (newShouldNeverUnbox == m_shouldNeverUnbox)
        return false;
    m_shouldNeverUnbox = newShouldNeverUnbox;
    return true;
}

bool VariableAccessData::predict(SpeculatedType prediction)
{
    VariableAccessData* self = find();
    bool result = mergeSpeculation(self->m_prediction, prediction);
    if (result)
        mergeSpeculation(m_argumentAwarePrediction, m_prediction);
    return result;
}

bool VariableAccessData::mergeArgumentAwarePrediction(SpeculatedType prediction)
{
    return mergeSpeculation(find()->m_argumentAwarePrediction, prediction);
}

bool VariableAccessData::shouldUseDoubleFormatAccordingToVote()
{
    // We don't support this facility for arguments, yet.
    // FIXME: make this work for arguments.
    if (local().isArgument())
        return false;
        
    // If the variable is not a number prediction, then this doesn't
    // make any sense.
    if (!isFullNumberSpeculation(prediction())) {
        // FIXME: we may end up forcing a local in inlined argument position to be a double even
        // if it is sometimes not even numeric, since this never signals the fact that it doesn't
        // want doubles. https://bugs.webkit.org/show_bug.cgi?id=109511
        return false;
    }
        
    // If the variable is predicted to hold only doubles, then it's a
    // no-brainer: it should be formatted as a double.
    if (isDoubleSpeculation(prediction()))
        return true;
        
    // If the variable is known to be used as an integer, then be safe -
    // don't force it to be a double.
    if (flags() & NodeBytecodeUsesAsInt)
        return false;
        
    // If the variable has been voted to become a double, then make it a
    // double.
    if (voteRatio() >= Options::doubleVoteRatioForDoubleFormat())
        return true;
        
    return false;
}

bool VariableAccessData::tallyVotesForShouldUseDoubleFormat()
{
    ASSERT(isRoot());
        
    if (local().isArgument() || shouldNeverUnbox()
        || (flags() & NodeBytecodeUsesAsArrayIndex))
        return DFG::mergeDoubleFormatState(m_doubleFormatState, NotUsingDoubleFormat);
    
    if (m_doubleFormatState == CantUseDoubleFormat)
        return false;
        
    bool newValueOfShouldUseDoubleFormat = shouldUseDoubleFormatAccordingToVote();
    if (!newValueOfShouldUseDoubleFormat) {
        // We monotonically convert to double. Hence, if the fixpoint leads us to conclude that we should
        // switch back to int, we instead ignore this and stick with double.
        return false;
    }
        
    if (m_doubleFormatState == UsingDoubleFormat)
        return false;
        
    return DFG::mergeDoubleFormatState(m_doubleFormatState, UsingDoubleFormat);
}

bool VariableAccessData::mergeDoubleFormatState(DoubleFormatState doubleFormatState)
{
    return DFG::mergeDoubleFormatState(find()->m_doubleFormatState, doubleFormatState);
}

bool VariableAccessData::makePredictionForDoubleFormat()
{
    ASSERT(isRoot());
    
    if (m_doubleFormatState != UsingDoubleFormat)
        return false;
    
    SpeculatedType type = m_prediction;
    if (type & ~SpecBytecodeNumber)
        type |= SpecDoublePureNaN;
    if (type & SpecAnyInt)
        type |= SpecAnyIntAsDouble;
    return checkAndSet(m_prediction, type);
}

bool VariableAccessData::couldRepresentInt52()
{
    if (shouldNeverUnbox())
        return false;
    
    return couldRepresentInt52Impl();
}

bool VariableAccessData::couldRepresentInt52Impl()
{
    // The hardware has to support it.
    if (!enableInt52())
        return false;
    
    // We punt for machine arguments.
    if (m_local.isArgument())
        return false;
    
    // The argument-aware prediction -- which merges all of an (inlined or machine)
    // argument's variable access datas' predictions -- must possibly be AnyInt.
    return !(argumentAwarePrediction() & ~SpecAnyInt);
}

FlushFormat VariableAccessData::flushFormat()
{
    ASSERT(find() == this);
    
    if (!shouldUnboxIfPossible())
        return FlushedJSValue;
    
    if (shouldUseDoubleFormat())
        return FlushedDouble;
    
    SpeculatedType prediction = argumentAwarePrediction();
    
    // This guard is here to protect the call to couldRepresentInt52(), which will return
    // true for !prediction.
    if (!prediction)
        return FlushedJSValue;
    
    if (isInt32Speculation(prediction))
        return FlushedInt32;
    
    if (couldRepresentInt52Impl())
        return FlushedInt52;
    
    if (isCellSpeculation(prediction))
        return FlushedCell;
    
    if (isBooleanSpeculation(prediction))
        return FlushedBoolean;
    
    return FlushedJSValue;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

