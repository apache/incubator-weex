/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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

#include "CodeBlock.h"
#include "CompilationResult.h"
#include "DFGCommonData.h"
#include "DFGMinifiedGraph.h"
#include "DFGOSREntry.h"
#include "DFGOSRExit.h"
#include "DFGVariableEventStream.h"
#include "ExecutionCounter.h"
#include "JITCode.h"
#include <wtf/SegmentedVector.h>

namespace JSC {

class TrackedReferences;

namespace DFG {

class JITCompiler;

class JITCode : public DirectJITCode {
public:
    JITCode();
    virtual ~JITCode();
    
    CommonData* dfgCommon() override;
    JITCode* dfg() override;
    
    OSREntryData* appendOSREntryData(unsigned bytecodeIndex, unsigned machineCodeOffset)
    {
        DFG::OSREntryData entry;
        entry.m_bytecodeIndex = bytecodeIndex;
        entry.m_machineCodeOffset = machineCodeOffset;
        osrEntry.append(entry);
        return &osrEntry.last();
    }
    
    OSREntryData* osrEntryDataForBytecodeIndex(unsigned bytecodeIndex)
    {
        return tryBinarySearch<OSREntryData, unsigned>(
            osrEntry, osrEntry.size(), bytecodeIndex,
            getOSREntryDataBytecodeIndex);
    }
    
    unsigned appendOSRExit(const OSRExit& exit)
    {
        unsigned result = osrExit.size();
        osrExit.append(exit);
        return result;
    }
    
    OSRExit& lastOSRExit()
    {
        return osrExit.last();
    }
    
    unsigned appendSpeculationRecovery(const SpeculationRecovery& recovery)
    {
        unsigned result = speculationRecovery.size();
        speculationRecovery.append(recovery);
        return result;
    }
    
    void reconstruct(
        CodeBlock*, CodeOrigin, unsigned streamIndex, Operands<ValueRecovery>& result);
    
    // This is only applicable if we're at a point where all values are spilled to the
    // stack. Currently, it also has the restriction that the values must be in their
    // bytecode-designated stack slots.
    void reconstruct(
        ExecState*, CodeBlock*, CodeOrigin, unsigned streamIndex, Operands<JSValue>& result);

#if ENABLE(FTL_JIT)
    // NB. All of these methods take CodeBlock* because they may want to use
    // CodeBlock's logic about scaling thresholds. It should be a DFG CodeBlock.
    
    bool checkIfOptimizationThresholdReached(CodeBlock*);
    void optimizeNextInvocation(CodeBlock*);
    void dontOptimizeAnytimeSoon(CodeBlock*);
    void optimizeAfterWarmUp(CodeBlock*);
    void optimizeSoon(CodeBlock*);
    void forceOptimizationSlowPathConcurrently(CodeBlock*);
    void setOptimizationThresholdBasedOnCompilationResult(CodeBlock*, CompilationResult);
#endif // ENABLE(FTL_JIT)
    
    void validateReferences(const TrackedReferences&) override;
    
    void shrinkToFit();

    RegisterSet liveRegistersToPreserveAtExceptionHandlingCallSite(CodeBlock*, CallSiteIndex) override;
#if ENABLE(FTL_JIT)
    CodeBlock* osrEntryBlock() { return m_osrEntryBlock.get(); }
    void setOSREntryBlock(VM& vm, const JSCell* owner, CodeBlock* osrEntryBlock) { m_osrEntryBlock.set(vm, owner, osrEntryBlock); }
    void clearOSREntryBlock() { m_osrEntryBlock.clear(); }
#endif

    static ptrdiff_t commonDataOffset() { return OBJECT_OFFSETOF(JITCode, common); }

    std::optional<CodeOrigin> findPC(CodeBlock*, void* pc) override;
    
private:
    friend class JITCompiler; // Allow JITCompiler to call setCodeRef().

public:
    CommonData common;
    Vector<DFG::OSREntryData> osrEntry;
    SegmentedVector<DFG::OSRExit, 8> osrExit;
    Vector<DFG::SpeculationRecovery> speculationRecovery;
    DFG::VariableEventStream variableEventStream;
    DFG::MinifiedGraph minifiedDFG;
#if ENABLE(FTL_JIT)
    uint8_t neverExecutedEntry { 1 };

    UpperTierExecutionCounter tierUpCounter;

    // For osrEntryPoint that are in inner loop, this maps their bytecode to the bytecode
    // of the outerloop entry points in order (from innermost to outermost).
    //
    // The key may not always be a target for OSR Entry but the list in the value is guaranteed
    // to be usable for OSR Entry.
    HashMap<unsigned, Vector<unsigned>> tierUpInLoopHierarchy;

    // Map each bytecode of CheckTierUpAndOSREnter to its stream index.
    HashMap<unsigned, unsigned, WTF::IntHash<unsigned>, WTF::UnsignedWithZeroKeyHashTraits<unsigned>> bytecodeIndexToStreamIndex;

    enum class TriggerReason : uint8_t {
        DontTrigger,
        CompilationDone,
        StartCompilation,
    };

    // Map each bytecode of CheckTierUpAndOSREnter to its trigger forcing OSR Entry.
    // This can never be modified after it has been initialized since the addresses of the triggers
    // are used by the JIT.
    HashMap<unsigned, TriggerReason> tierUpEntryTriggers;

    // Set of bytecode that were the target of a TierUp operation.
    HashSet<unsigned, WTF::IntHash<unsigned>, WTF::UnsignedWithZeroKeyHashTraits<unsigned>> tierUpEntrySeen;

    WriteBarrier<CodeBlock> m_osrEntryBlock;
    unsigned osrEntryRetry;
    bool abandonOSREntry;
#endif // ENABLE(FTL_JIT)
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
