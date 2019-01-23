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
