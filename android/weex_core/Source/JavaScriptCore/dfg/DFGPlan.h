/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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

#include "CompilationResult.h"
#include "DFGCompilationKey.h"
#include "DFGCompilationMode.h"
#include "DFGDesiredIdentifiers.h"
#include "DFGDesiredTransitions.h"
#include "DFGDesiredWatchpoints.h"
#include "DFGDesiredWeakReferences.h"
#include "DFGFinalizer.h"
#include "DeferredCompilationCallback.h"
#include "Operands.h"
#include "ProfilerCompilation.h"
#include <wtf/HashMap.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/text/CString.h>

namespace JSC {

class CodeBlock;
class SlotVisitor;

namespace DFG {

class LongLivedState;
class ThreadData;

#if ENABLE(DFG_JIT)

struct Plan : public ThreadSafeRefCounted<Plan> {
    Plan(
        CodeBlock* codeBlockToCompile, CodeBlock* profiledDFGCodeBlock,
        CompilationMode, unsigned osrEntryBytecodeIndex,
        const Operands<JSValue>& mustHandleValues);
    ~Plan();

    void compileInThread(LongLivedState&, ThreadData*);
    
    CompilationResult finalizeWithoutNotifyingCallback();
    void finalizeAndNotifyCallback();
    
    void notifyCompiling();
    void notifyReady();
    
    CompilationKey key();
    
    void markCodeBlocks(SlotVisitor&);
    template<typename Func>
    void iterateCodeBlocksForGC(const Func&);
    void checkLivenessAndVisitChildren(SlotVisitor&);
    bool isKnownToBeLiveDuringGC();
    void cancel();

    bool canTierUpAndOSREnter() const { return !tierUpAndOSREnterBytecodes.isEmpty(); }
    
    void cleanMustHandleValuesIfNecessary();
    
    // Warning: pretty much all of the pointer fields in this object get nulled by cancel(). So, if
    // you're writing code that is callable on the cancel path, be sure to null check everything!
    
    VM* vm;

    // These can be raw pointers because we visit them during every GC in checkLivenessAndVisitChildren.
    CodeBlock* codeBlock;
    CodeBlock* profiledDFGCodeBlock;

    CompilationMode mode;
    const unsigned osrEntryBytecodeIndex;
    Operands<JSValue> mustHandleValues;
    bool mustHandleValuesMayIncludeGarbage { true };
    Lock mustHandleValueCleaningLock;
    
    ThreadData* threadData;

    RefPtr<Profiler::Compilation> compilation;

    std::unique_ptr<Finalizer> finalizer;
    
    RefPtr<InlineCallFrameSet> inlineCallFrames;
    DesiredWatchpoints watchpoints;
    DesiredIdentifiers identifiers;
    DesiredWeakReferences weakReferences;
    DesiredTransitions transitions;
    
    bool willTryToTierUp { false };

    HashMap<unsigned, Vector<unsigned>> tierUpInLoopHierarchy;
    Vector<unsigned> tierUpAndOSREnterBytecodes;

    enum Stage { Preparing, Compiling, Ready, Cancelled };
    Stage stage;

    RefPtr<DeferredCompilationCallback> callback;

private:
    bool computeCompileTimes() const;
    bool reportCompileTimes() const;
    
    enum CompilationPath { FailPath, DFGPath, FTLPath, CancelPath };
    CompilationPath compileInThreadImpl(LongLivedState&);
    
    bool isStillValid();
    void reallyAdd(CommonData*);

    double m_timeBeforeFTL;
};

#endif // ENABLE(DFG_JIT)

} } // namespace JSC::DFG
