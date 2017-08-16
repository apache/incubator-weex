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

#include "config.h"
#include "DFGPlan.h"

#if ENABLE(DFG_JIT)

#include "DFGArgumentsEliminationPhase.h"
#include "DFGBackwardsPropagationPhase.h"
#include "DFGByteCodeParser.h"
#include "DFGCFAPhase.h"
#include "DFGCFGSimplificationPhase.h"
#include "DFGCPSRethreadingPhase.h"
#include "DFGCSEPhase.h"
#include "DFGCleanUpPhase.h"
#include "DFGConstantFoldingPhase.h"
#include "DFGConstantHoistingPhase.h"
#include "DFGCriticalEdgeBreakingPhase.h"
#include "DFGDCEPhase.h"
#include "DFGFailedFinalizer.h"
#include "DFGFixupPhase.h"
#include "DFGGraphSafepoint.h"
#include "DFGIntegerCheckCombiningPhase.h"
#include "DFGIntegerRangeOptimizationPhase.h"
#include "DFGInvalidationPointInjectionPhase.h"
#include "DFGJITCompiler.h"
#include "DFGLICMPhase.h"
#include "DFGLiveCatchVariablePreservationPhase.h"
#include "DFGLivenessAnalysisPhase.h"
#include "DFGLoopPreHeaderCreationPhase.h"
#include "DFGMaximalFlushInsertionPhase.h"
#include "DFGMovHintRemovalPhase.h"
#include "DFGOSRAvailabilityAnalysisPhase.h"
#include "DFGOSREntrypointCreationPhase.h"
#include "DFGObjectAllocationSinkingPhase.h"
#include "DFGPhantomInsertionPhase.h"
#include "DFGPredictionInjectionPhase.h"
#include "DFGPredictionPropagationPhase.h"
#include "DFGPutStackSinkingPhase.h"
#include "DFGSSAConversionPhase.h"
#include "DFGSSALoweringPhase.h"
#include "DFGStackLayoutPhase.h"
#include "DFGStaticExecutionCountEstimationPhase.h"
#include "DFGStoreBarrierClusteringPhase.h"
#include "DFGStoreBarrierInsertionPhase.h"
#include "DFGStrengthReductionPhase.h"
#include "DFGTierUpCheckInjectionPhase.h"
#include "DFGTypeCheckHoistingPhase.h"
#include "DFGUnificationPhase.h"
#include "DFGValidate.h"
#include "DFGVarargsForwardingPhase.h"
#include "DFGVirtualRegisterAllocationPhase.h"
#include "DFGWatchpointCollectionPhase.h"
#include "JSCInlines.h"
#include "OperandsInlines.h"
#include "ProfilerDatabase.h"
#include "TrackedReferences.h"
#include "VMInlines.h"
#include "Trace.h"
#include <wtf/CurrentTime.h>

#if ENABLE(FTL_JIT)
#include "FTLCapabilities.h"
#include "FTLCompile.h"
#include "FTLFail.h"
#include "FTLLink.h"
#include "FTLLowerDFGToB3.h"
#include "FTLState.h"
#endif

namespace JSC {

extern double totalDFGCompileTime;
extern double totalFTLCompileTime;
extern double totalFTLDFGCompileTime;
extern double totalFTLB3CompileTime;

}

namespace JSC { namespace DFG {

namespace {

void dumpAndVerifyGraph(Graph& graph, const char* text, bool forceDump = false)
{
    GraphDumpMode modeForFinalValidate = DumpGraph;
    if (verboseCompilationEnabled(graph.m_plan.mode) || forceDump) {
        dataLog(text, "\n");
        graph.dump();
        modeForFinalValidate = DontDumpGraph;
    }
    if (validationEnabled())
        validate(graph, modeForFinalValidate);
}

Profiler::CompilationKind profilerCompilationKindForMode(CompilationMode mode)
{
    switch (mode) {
    case InvalidCompilationMode:
        RELEASE_ASSERT_NOT_REACHED();
        return Profiler::DFG;
    case DFGMode:
        return Profiler::DFG;
    case FTLMode:
        return Profiler::FTL;
    case FTLForOSREntryMode:
        return Profiler::FTLForOSREntry;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return Profiler::DFG;
}

} // anonymous namespace

Plan::Plan(CodeBlock* passedCodeBlock, CodeBlock* profiledDFGCodeBlock,
    CompilationMode mode, unsigned osrEntryBytecodeIndex,
    const Operands<JSValue>& mustHandleValues)
    : vm(passedCodeBlock->vm())
    , codeBlock(passedCodeBlock)
    , profiledDFGCodeBlock(profiledDFGCodeBlock)
    , mode(mode)
    , osrEntryBytecodeIndex(osrEntryBytecodeIndex)
    , mustHandleValues(mustHandleValues)
    , compilation(vm->m_perBytecodeProfiler ? adoptRef(new Profiler::Compilation(vm->m_perBytecodeProfiler->ensureBytecodesFor(codeBlock), profilerCompilationKindForMode(mode))) : 0)
    , inlineCallFrames(adoptRef(new InlineCallFrameSet()))
    , identifiers(codeBlock)
    , weakReferences(codeBlock)
    , stage(Preparing)
{
}

Plan::~Plan()
{
}

bool Plan::computeCompileTimes() const
{
    return reportCompileTimes()
        || Options::reportTotalCompileTimes()
        || (vm && vm->m_perBytecodeProfiler);
}

bool Plan::reportCompileTimes() const
{
    return Options::reportCompileTimes()
        || Options::reportDFGCompileTimes()
        || (Options::reportFTLCompileTimes() && isFTL(mode));
}

void Plan::compileInThread(LongLivedState& longLivedState, ThreadData* threadData)
{
    this->threadData = threadData;
    
    double before = 0;
    CString codeBlockName;
    if (UNLIKELY(computeCompileTimes()))
        before = monotonicallyIncreasingTimeMS();
    if (UNLIKELY(reportCompileTimes()))
        codeBlockName = toCString(*codeBlock);
    
    CompilationScope compilationScope;

    if (logCompilationChanges(mode) || Options::reportDFGPhaseTimes())
        dataLog("DFG(Plan) compiling ", *codeBlock, " with ", mode, ", number of instructions = ", codeBlock->instructionCount(), "\n");

    CompilationPath path = compileInThreadImpl(longLivedState);

    RELEASE_ASSERT(path == CancelPath || finalizer);
    RELEASE_ASSERT((path == CancelPath) == (stage == Cancelled));
    
    double after = 0;
    if (UNLIKELY(computeCompileTimes())) {
        after = monotonicallyIncreasingTimeMS();
    
        if (Options::reportTotalCompileTimes()) {
            if (isFTL(mode)) {
                totalFTLCompileTime += after - before;
                totalFTLDFGCompileTime += m_timeBeforeFTL - before;
                totalFTLB3CompileTime += after - m_timeBeforeFTL;
            } else
                totalDFGCompileTime += after - before;
        }
    }
    const char* pathName = nullptr;
    switch (path) {
    case FailPath:
        pathName = "N/A (fail)";
        break;
    case DFGPath:
        pathName = "DFG";
        break;
    case FTLPath:
        pathName = "FTL";
        break;
    case CancelPath:
        pathName = "Cancelled";
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
    if (codeBlock) { // codeBlock will be null if the compilation was cancelled.
        if (path == FTLPath)
            CODEBLOCK_LOG_EVENT(codeBlock, "ftlCompile", ("took ", after - before, " ms (DFG: ", m_timeBeforeFTL - before, ", B3: ", after - m_timeBeforeFTL, ") with ", pathName));
        else
            CODEBLOCK_LOG_EVENT(codeBlock, "dfgCompile", ("took ", after - before, " ms with ", pathName));
    }
    if (UNLIKELY(reportCompileTimes())) {
        dataLog("Optimized ", codeBlockName, " using ", mode, " with ", pathName, " into ", finalizer ? finalizer->codeSize() : 0, " bytes in ", after - before, " ms");
        if (path == FTLPath)
            dataLog(" (DFG: ", m_timeBeforeFTL - before, ", B3: ", after - m_timeBeforeFTL, ")");
        dataLog(".\n");
    }
}

Plan::CompilationPath Plan::compileInThreadImpl(LongLivedState& longLivedState)
{
    base::debug::TraceScope traceScope("jsc", "DFG Plan::compileInThreadImpl");
    cleanMustHandleValuesIfNecessary();
    
    if (verboseCompilationEnabled(mode) && osrEntryBytecodeIndex != UINT_MAX) {
        dataLog("\n");
        dataLog("Compiler must handle OSR entry from bc#", osrEntryBytecodeIndex, " with values: ", mustHandleValues, "\n");
        dataLog("\n");
    }
    
    Graph dfg(*vm, *this, longLivedState);
    
    if (!parse(dfg)) {
        finalizer = std::make_unique<FailedFinalizer>(*this);
        return FailPath;
    }

    codeBlock->setCalleeSaveRegisters(RegisterSet::dfgCalleeSaveRegisters());

    bool changed = false;

#define RUN_PHASE(phase)                                         \
    do {                                                         \
        if (Options::safepointBeforeEachPhase()) {               \
            Safepoint::Result safepointResult;                   \
            {                                                    \
                GraphSafepoint safepoint(dfg, safepointResult);  \
            }                                                    \
            if (safepointResult.didGetCancelled())               \
                return CancelPath;                               \
        }                                                        \
        changed |= phase(dfg);                                   \
    } while (false);                                             \

    
    // By this point the DFG bytecode parser will have potentially mutated various tables
    // in the CodeBlock. This is a good time to perform an early shrink, which is more
    // powerful than a late one. It's safe to do so because we haven't generated any code
    // that references any of the tables directly, yet.
    codeBlock->shrinkToFit(CodeBlock::EarlyShrink);

    if (validationEnabled())
        validate(dfg);
    
    if (Options::dumpGraphAfterParsing()) {
        dataLog("Graph after parsing:\n");
        dfg.dump();
    }

    RUN_PHASE(performLiveCatchVariablePreservationPhase);

    if (Options::useMaximalFlushInsertionPhase())
        RUN_PHASE(performMaximalFlushInsertion);
    
    RUN_PHASE(performCPSRethreading);
    RUN_PHASE(performUnification);
    RUN_PHASE(performPredictionInjection);
    
    RUN_PHASE(performStaticExecutionCountEstimation);
    
    if (mode == FTLForOSREntryMode) {
        bool result = performOSREntrypointCreation(dfg);
        if (!result) {
            finalizer = std::make_unique<FailedFinalizer>(*this);
            return FailPath;
        }
        RUN_PHASE(performCPSRethreading);
    }
    
    if (validationEnabled())
        validate(dfg);
    
    RUN_PHASE(performBackwardsPropagation);
    RUN_PHASE(performPredictionPropagation);
    RUN_PHASE(performFixup);
    RUN_PHASE(performInvalidationPointInjection);
    RUN_PHASE(performTypeCheckHoisting);
    
    dfg.m_fixpointState = FixpointNotConverged;
    
    // For now we're back to avoiding a fixpoint. Note that we've ping-ponged on this decision
    // many times. For maximum throughput, it's best to fixpoint. But the throughput benefit is
    // small and not likely to show up in FTL anyway. On the other hand, not fixpointing means
    // that the compiler compiles more quickly. We want the third tier to compile quickly, which
    // not fixpointing accomplishes; and the fourth tier shouldn't need a fixpoint.
    if (validationEnabled())
        validate(dfg);
        
    RUN_PHASE(performStrengthReduction);
    RUN_PHASE(performCPSRethreading);
    RUN_PHASE(performCFA);
    RUN_PHASE(performConstantFolding);
    changed = false;
    RUN_PHASE(performCFGSimplification);
    RUN_PHASE(performLocalCSE);
    
    if (validationEnabled())
        validate(dfg);
    
    RUN_PHASE(performCPSRethreading);
    if (!isFTL(mode)) {
        // Only run this if we're not FTLing, because currently for a LoadVarargs that is forwardable and
        // in a non-varargs inlined call frame, this will generate ForwardVarargs while the FTL
        // ArgumentsEliminationPhase will create a sequence of GetStack+PutStacks. The GetStack+PutStack
        // sequence then gets sunk, eliminating anything that looks like an escape for subsequent phases,
        // while the ForwardVarargs doesn't get simplified until later (or not at all) and looks like an
        // escape for all of the arguments. This then disables object allocation sinking.
        //
        // So, for now, we just disable this phase for the FTL.
        //
        // If we wanted to enable it, we'd have to do any of the following:
        // - Enable ForwardVarargs->GetStack+PutStack strength reduction, and have that run before
        //   PutStack sinking and object allocation sinking.
        // - Make VarargsForwarding emit a GetLocal+SetLocal sequence, that we can later turn into
        //   GetStack+PutStack.
        //
        // But, it's not super valuable to enable those optimizations, since the FTL
        // ArgumentsEliminationPhase does everything that this phase does, and it doesn't introduce this
        // pathology.
        
        RUN_PHASE(performVarargsForwarding); // Do this after CFG simplification and CPS rethreading.
    }
    if (changed) {
        RUN_PHASE(performCFA);
        RUN_PHASE(performConstantFolding);
    }
    
    // If we're doing validation, then run some analyses, to give them an opportunity
    // to self-validate. Now is as good a time as any to do this.
    if (validationEnabled()) {
        dfg.ensureDominators();
        dfg.ensureNaturalLoops();
        dfg.ensurePrePostNumbering();
    }

    switch (mode) {
    case DFGMode: {
        base::debug::TraceScope traceScope("jsc", "DFG Plan::compileInThreadImpl DFG");
        dfg.m_fixpointState = FixpointConverged;
    
        RUN_PHASE(performTierUpCheckInjection);

        RUN_PHASE(performFastStoreBarrierInsertion);
        RUN_PHASE(performStoreBarrierClustering);
        RUN_PHASE(performCleanUp);
        RUN_PHASE(performCPSRethreading);
        RUN_PHASE(performDCE);
        RUN_PHASE(performPhantomInsertion);
        RUN_PHASE(performStackLayout);
        RUN_PHASE(performVirtualRegisterAllocation);
        RUN_PHASE(performWatchpointCollection);
        dumpAndVerifyGraph(dfg, "Graph after optimization:");
        
        JITCompiler dataFlowJIT(dfg);
        if (codeBlock->codeType() == FunctionCode)
            dataFlowJIT.compileFunction();
        else
            dataFlowJIT.compile();
        
        return DFGPath;
    }
    
    case FTLMode:
    case FTLForOSREntryMode: {
#if ENABLE(FTL_JIT)
        base::debug::TraceScope traceScope("jsc", "DFG Plan::compileInThreadImpl FTL");
        if (FTL::canCompile(dfg) == FTL::CannotCompile) {
            finalizer = std::make_unique<FailedFinalizer>(*this);
            return FailPath;
        }
        
        RUN_PHASE(performCleanUp); // Reduce the graph size a bit.
        RUN_PHASE(performCriticalEdgeBreaking);
        if (Options::createPreHeaders())
            RUN_PHASE(performLoopPreHeaderCreation);
        RUN_PHASE(performCPSRethreading);
        RUN_PHASE(performSSAConversion);
        RUN_PHASE(performSSALowering);
        
        // Ideally, these would be run to fixpoint with the object allocation sinking phase.
        RUN_PHASE(performArgumentsElimination);
        if (Options::usePutStackSinking())
            RUN_PHASE(performPutStackSinking);
        
        RUN_PHASE(performConstantHoisting);
        RUN_PHASE(performGlobalCSE);
        RUN_PHASE(performLivenessAnalysis);
        RUN_PHASE(performCFA);
        RUN_PHASE(performConstantFolding);
        RUN_PHASE(performCleanUp); // Reduce the graph size a lot.
        changed = false;
        RUN_PHASE(performStrengthReduction);
        if (Options::useObjectAllocationSinking()) {
            RUN_PHASE(performCriticalEdgeBreaking);
            RUN_PHASE(performObjectAllocationSinking);
        }
        if (changed) {
            // State-at-tail and state-at-head will be invalid if we did strength reduction since
            // it might increase live ranges.
            RUN_PHASE(performLivenessAnalysis);
            RUN_PHASE(performCFA);
            RUN_PHASE(performConstantFolding);
        }
        
        // Currently, this relies on pre-headers still being valid. That precludes running CFG
        // simplification before it, unless we re-created the pre-headers. There wouldn't be anything
        // wrong with running LICM earlier, if we wanted to put other CFG transforms above this point.
        // Alternatively, we could run loop pre-header creation after SSA conversion - but if we did that
        // then we'd need to do some simple SSA fix-up.
        RUN_PHASE(performLivenessAnalysis);
        RUN_PHASE(performCFA);
        RUN_PHASE(performLICM);

        // FIXME: Currently: IntegerRangeOptimization *must* be run after LICM.
        //
        // IntegerRangeOptimization makes changes on nodes based on preceding blocks
        // and nodes. LICM moves nodes which can invalidates assumptions used
        // by IntegerRangeOptimization.
        //
        // Ideally, the dependencies should be explicit. See https://bugs.webkit.org/show_bug.cgi?id=157534.
        RUN_PHASE(performLivenessAnalysis);
        RUN_PHASE(performIntegerRangeOptimization);
        
        RUN_PHASE(performCleanUp);
        RUN_PHASE(performIntegerCheckCombining);
        RUN_PHASE(performGlobalCSE);
        
        // At this point we're not allowed to do any further code motion because our reasoning
        // about code motion assumes that it's OK to insert GC points in random places.
        dfg.m_fixpointState = FixpointConverged;
        
        RUN_PHASE(performLivenessAnalysis);
        RUN_PHASE(performCFA);
        RUN_PHASE(performGlobalStoreBarrierInsertion);
        RUN_PHASE(performStoreBarrierClustering);
        if (Options::useMovHintRemoval())
            RUN_PHASE(performMovHintRemoval);
        RUN_PHASE(performCleanUp);
        RUN_PHASE(performDCE); // We rely on this to kill dead code that won't be recognized as dead by B3.
        RUN_PHASE(performStackLayout);
        RUN_PHASE(performLivenessAnalysis);
        RUN_PHASE(performOSRAvailabilityAnalysis);
        RUN_PHASE(performWatchpointCollection);
        
        if (FTL::canCompile(dfg) == FTL::CannotCompile) {
            finalizer = std::make_unique<FailedFinalizer>(*this);
            return FailPath;
        }

        dumpAndVerifyGraph(dfg, "Graph just before FTL lowering:", shouldDumpDisassembly(mode));

        // Flash a safepoint in case the GC wants some action.
        Safepoint::Result safepointResult;
        {
            GraphSafepoint safepoint(dfg, safepointResult);
        }
        if (safepointResult.didGetCancelled())
            return CancelPath;

        FTL::State state(dfg);
        FTL::lowerDFGToB3(state);
        
        if (UNLIKELY(computeCompileTimes()))
            m_timeBeforeFTL = monotonicallyIncreasingTimeMS();
        
        if (Options::b3AlwaysFailsBeforeCompile()) {
            FTL::fail(state);
            return FTLPath;
        }
        
        FTL::compile(state, safepointResult);
        if (safepointResult.didGetCancelled())
            return CancelPath;
        
        if (Options::b3AlwaysFailsBeforeLink()) {
            FTL::fail(state);
            return FTLPath;
        }
        
        if (state.allocationFailed) {
            FTL::fail(state);
            return FTLPath;
        }

        FTL::link(state);
        
        if (state.allocationFailed) {
            FTL::fail(state);
            return FTLPath;
        }
        
        return FTLPath;
#else
        RELEASE_ASSERT_NOT_REACHED();
        return FailPath;
#endif // ENABLE(FTL_JIT)
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return FailPath;
    }

#undef RUN_PHASE
}

bool Plan::isStillValid()
{
    CodeBlock* replacement = codeBlock->replacement();
    if (!replacement)
        return false;
    // FIXME: This is almost certainly not necessary. There's no way for the baseline
    // code to be replaced during a compilation, except if we delete the plan, in which
    // case we wouldn't be here.
    // https://bugs.webkit.org/show_bug.cgi?id=132707
    if (codeBlock->alternative() != replacement->baselineVersion())
        return false;
    if (!watchpoints.areStillValid())
        return false;
    return true;
}

void Plan::reallyAdd(CommonData* commonData)
{
    watchpoints.reallyAdd(codeBlock, *commonData);
    identifiers.reallyAdd(*vm, commonData);
    weakReferences.reallyAdd(*vm, commonData);
    transitions.reallyAdd(*vm, commonData);
}

void Plan::notifyCompiling()
{
    stage = Compiling;
}

void Plan::notifyReady()
{
    callback->compilationDidBecomeReadyAsynchronously(codeBlock, profiledDFGCodeBlock);
    stage = Ready;
}

CompilationResult Plan::finalizeWithoutNotifyingCallback()
{
    // We will establish new references from the code block to things. So, we need a barrier.
    vm->heap.writeBarrier(codeBlock);
    
    if (!isStillValid()) {
        CODEBLOCK_LOG_EVENT(codeBlock, "dfgFinalize", ("invalidated"));
        return CompilationInvalidated;
    }

    bool result;
    if (codeBlock->codeType() == FunctionCode)
        result = finalizer->finalizeFunction();
    else
        result = finalizer->finalize();
    
    if (!result) {
        CODEBLOCK_LOG_EVENT(codeBlock, "dfgFinalize", ("failed"));
        return CompilationFailed;
    }
    
    reallyAdd(codeBlock->jitCode()->dfgCommon());
    
    if (validationEnabled()) {
        TrackedReferences trackedReferences;
        
        for (WriteBarrier<JSCell>& reference : codeBlock->jitCode()->dfgCommon()->weakReferences)
            trackedReferences.add(reference.get());
        for (WriteBarrier<Structure>& reference : codeBlock->jitCode()->dfgCommon()->weakStructureReferences)
            trackedReferences.add(reference.get());
        for (WriteBarrier<Unknown>& constant : codeBlock->constants())
            trackedReferences.add(constant.get());

        for (auto* inlineCallFrame : *inlineCallFrames) {
            ASSERT(inlineCallFrame->baselineCodeBlock.get());
            trackedReferences.add(inlineCallFrame->baselineCodeBlock.get());
        }
        
        // Check that any other references that we have anywhere in the JITCode are also
        // tracked either strongly or weakly.
        codeBlock->jitCode()->validateReferences(trackedReferences);
    }
    
    CODEBLOCK_LOG_EVENT(codeBlock, "dfgFinalize", ("succeeded"));
    return CompilationSuccessful;
}

void Plan::finalizeAndNotifyCallback()
{
    callback->compilationDidComplete(codeBlock, profiledDFGCodeBlock, finalizeWithoutNotifyingCallback());
}

CompilationKey Plan::key()
{
    return CompilationKey(codeBlock->alternative(), mode);
}

void Plan::checkLivenessAndVisitChildren(SlotVisitor& visitor)
{
    if (!isKnownToBeLiveDuringGC())
        return;

    cleanMustHandleValuesIfNecessary();
    for (unsigned i = mustHandleValues.size(); i--;)
        visitor.appendUnbarriered(mustHandleValues[i]);

    visitor.appendUnbarriered(codeBlock);
    visitor.appendUnbarriered(codeBlock->alternative());
    visitor.appendUnbarriered(profiledDFGCodeBlock);

    if (inlineCallFrames) {
        for (auto* inlineCallFrame : *inlineCallFrames) {
            ASSERT(inlineCallFrame->baselineCodeBlock.get());
            visitor.appendUnbarriered(inlineCallFrame->baselineCodeBlock.get());
        }
    }

    weakReferences.visitChildren(visitor);
    transitions.visitChildren(visitor);
}

bool Plan::isKnownToBeLiveDuringGC()
{
    if (stage == Cancelled)
        return false;
    if (!Heap::isMarked(codeBlock->ownerExecutable()))
        return false;
    if (!Heap::isMarked(codeBlock->alternative()))
        return false;
    if (!!profiledDFGCodeBlock && !Heap::isMarked(profiledDFGCodeBlock))
        return false;
    return true;
}

void Plan::cancel()
{
    vm = nullptr;
    codeBlock = nullptr;
    profiledDFGCodeBlock = nullptr;
    mustHandleValues.clear();
    compilation = nullptr;
    finalizer = nullptr;
    inlineCallFrames = nullptr;
    watchpoints = DesiredWatchpoints();
    identifiers = DesiredIdentifiers();
    weakReferences = DesiredWeakReferences();
    transitions = DesiredTransitions();
    callback = nullptr;
    stage = Cancelled;
}

void Plan::cleanMustHandleValuesIfNecessary()
{
    LockHolder locker(mustHandleValueCleaningLock);
    
    if (!mustHandleValuesMayIncludeGarbage)
        return;
    
    mustHandleValuesMayIncludeGarbage = false;
    
    if (!codeBlock)
        return;
    
    if (!mustHandleValues.numberOfLocals())
        return;
    
    FastBitVector liveness = codeBlock->alternative()->livenessAnalysis().getLivenessInfoAtBytecodeOffset(osrEntryBytecodeIndex);
    
    for (unsigned local = mustHandleValues.numberOfLocals(); local--;) {
        if (!liveness[local])
            mustHandleValues.local(local) = jsUndefined();
    }
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

