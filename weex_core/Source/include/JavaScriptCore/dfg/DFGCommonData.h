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

#include "CodeBlockJettisoningWatchpoint.h"
#include "DFGAdaptiveInferredPropertyValueWatchpoint.h"
#include "DFGAdaptiveStructureWatchpoint.h"
#include "DFGJumpReplacement.h"
#include "InlineCallFrameSet.h"
#include "JSCell.h"
#include "ProfilerCompilation.h"
#include "SymbolTable.h"
#include <wtf/Bag.h>
#include <wtf/Noncopyable.h>

namespace JSC {

class CodeBlock;
class Identifier;
class TrackedReferences;

namespace DFG {

struct Node;
struct Plan;

// CommonData holds the set of data that both DFG and FTL code blocks need to know
// about themselves.

struct WeakReferenceTransition {
    WeakReferenceTransition() { }
    
    WeakReferenceTransition(VM& vm, JSCell* owner, JSCell* codeOrigin, JSCell* from, JSCell* to)
        : m_from(vm, owner, from)
        , m_to(vm, owner, to)
    {
        if (!!codeOrigin)
            m_codeOrigin.set(vm, owner, codeOrigin);
    }
    
    WriteBarrier<JSCell> m_codeOrigin;
    WriteBarrier<JSCell> m_from;
    WriteBarrier<JSCell> m_to;
};
        
class CommonData {
    WTF_MAKE_NONCOPYABLE(CommonData);
public:
    CommonData()
        : isStillValid(true)
        , frameRegisterCount(std::numeric_limits<unsigned>::max())
        , requiredRegisterCountForExit(std::numeric_limits<unsigned>::max())
    { }
    
    void notifyCompilingStructureTransition(Plan&, CodeBlock*, Node*);
    CallSiteIndex addCodeOrigin(CodeOrigin);
    CallSiteIndex addUniqueCallSiteIndex(CodeOrigin);
    CallSiteIndex lastCallSite() const;
    void removeCallSiteIndex(CallSiteIndex);
    
    void shrinkToFit();
    
    bool invalidate(); // Returns true if we did invalidate, or false if the code block was already invalidated.
    bool hasInstalledVMTrapsBreakpoints() const { return isStillValid && hasVMTrapsBreakpointsInstalled; }
    void installVMTrapBreakpoints();
    bool isVMTrapBreakpoint(void* address);

    unsigned requiredRegisterCountForExecutionAndExit() const
    {
        return std::max(frameRegisterCount, requiredRegisterCountForExit);
    }
    
    void validateReferences(const TrackedReferences&);

    static ptrdiff_t frameRegisterCountOffset() { return OBJECT_OFFSETOF(CommonData, frameRegisterCount); }

    RefPtr<InlineCallFrameSet> inlineCallFrames;
    Vector<CodeOrigin, 0, UnsafeVectorOverflow> codeOrigins;
    
    Vector<Identifier> dfgIdentifiers;
    Vector<WeakReferenceTransition> transitions;
    Vector<WriteBarrier<JSCell>> weakReferences;
    Vector<WriteBarrier<Structure>> weakStructureReferences;
    Bag<CodeBlockJettisoningWatchpoint> watchpoints;
    Bag<AdaptiveStructureWatchpoint> adaptiveStructureWatchpoints;
    Bag<AdaptiveInferredPropertyValueWatchpoint> adaptiveInferredPropertyValueWatchpoints;
    Vector<JumpReplacement> jumpReplacements;
    
    RefPtr<Profiler::Compilation> compilation;
    bool livenessHasBeenProved; // Initialized and used on every GC.
    bool allTransitionsHaveBeenMarked; // Initialized and used on every GC.
    bool isStillValid;
    bool hasVMTrapsBreakpointsInstalled { false };
    
#if USE(JSVALUE32_64)
    std::unique_ptr<Bag<double>> doubleConstants;
#endif
    
    unsigned frameRegisterCount;
    unsigned requiredRegisterCountForExit;

private:
    HashSet<unsigned, WTF::IntHash<unsigned>, WTF::UnsignedWithZeroKeyHashTraits<unsigned>> callSiteIndexFreeList;

};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
