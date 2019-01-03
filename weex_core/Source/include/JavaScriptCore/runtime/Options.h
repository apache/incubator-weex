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

#include "GCLogging.h"
#include "JSExportMacros.h"
#include <stdint.h>
#include <stdio.h>
#include <wtf/PrintStream.h>
#include <wtf/StdLibExtras.h>

namespace WTF {
class StringBuilder;
}
using WTF::StringBuilder;

namespace JSC {

// How do JSC VM options work?
// ===========================
// The JSC_OPTIONS() macro below defines a list of all JSC options in use,
// along with their types and default values. The options values are actually
// realized as an array of Options::Entry elements.
//
//     Options::initialize() will initialize the array of options values with
// the defaults specified in JSC_OPTIONS() below. After that, the values can
// be programmatically read and written to using an accessor method with the
// same name as the option. For example, the option "useJIT" can be read and
// set like so:
//
//     bool jitIsOn = Options::useJIT();  // Get the option value.
//     Options::useJIT() = false;         // Sets the option value.
//
//     If you want to tweak any of these values programmatically for testing
// purposes, you can do so in Options::initialize() after the default values
// are set.
//
//     Alternatively, you can override the default values by specifying
// environment variables of the form: JSC_<name of JSC option>.
//
// Note: Options::initialize() tries to ensure some sanity on the option values
// which are set by doing some range checks, and value corrections. These
// checks are done after the option values are set. If you alter the option
// values after the sanity checks (for your own testing), then you're liable to
// ensure that the new values set are sane and reasonable for your own run.

class OptionRange {
private:
    enum RangeState { Uninitialized, InitError, Normal, Inverted };
public:
    OptionRange& operator= (const int& rhs)
    { // Only needed for initialization
        if (!rhs) {
            m_state = Uninitialized;
            m_rangeString = 0;
            m_lowLimit = 0;
            m_highLimit = 0;
        }
        return *this;
    }

    bool init(const char*);
    bool isInRange(unsigned);
    const char* rangeString() const { return (m_state > InitError) ? m_rangeString : s_nullRangeStr; }
    
    void dump(PrintStream& out) const;

private:
    static const char* const s_nullRangeStr;

    RangeState m_state;
    const char* m_rangeString;
    unsigned m_lowLimit;
    unsigned m_highLimit;
};

typedef OptionRange optionRange;
typedef const char* optionString;

#define JSC_OPTIONS(v) \
    v(bool, validateOptions, false, Normal, "crashes if mis-typed JSC options were passed to the VM") \
    v(unsigned, dumpOptions, 0, Normal, "dumps JSC options (0 = None, 1 = Overridden only, 2 = All, 3 = Verbose)") \
    v(optionString, configFile, nullptr, Normal, "file to configure JSC options and logging location") \
    \
    v(bool, useLLInt,  true, Normal, "allows the LLINT to be used if true") \
    v(bool, useJIT,    true, Normal, "allows the baseline JIT to be used if true") \
    v(bool, useDFGJIT, true, Normal, "allows the DFG JIT to be used if true") \
    v(bool, useRegExpJIT, true, Normal, "allows the RegExp JIT to be used if true") \
    v(bool, useDOMJIT, true, Normal, "allows the DOMJIT to be used if true") \
    \
    v(bool, reportMustSucceedExecutableAllocations, false, Normal, nullptr) \
    \
    v(unsigned, maxPerThreadStackUsage, 4 * MB, Normal, "Max allowed stack usage by the VM") \
    v(unsigned, softReservedZoneSize, 128 * KB, Normal, "A buffer greater than reservedZoneSize that reserves space for stringifying exceptions.") \
    v(unsigned, reservedZoneSize, 64 * KB, Normal, "The amount of stack space we guarantee to our clients (and to interal VM code that does not call out to clients).") \
    \
    v(bool, crashIfCantAllocateJITMemory, false, Normal, nullptr) \
    v(unsigned, jitMemoryReservationSize, 0, Normal, "Set this number to change the executable allocation size in ExecutableAllocatorFixedVMPool. (In bytes.)") \
    v(bool, useSeparatedWXHeap, false, Normal, nullptr) \
    \
    v(bool, forceCodeBlockLiveness, false, Normal, nullptr) \
    v(bool, forceICFailure, false, Normal, nullptr) \
    \
    v(unsigned, repatchCountForCoolDown, 8, Normal, nullptr) \
    v(unsigned, initialCoolDownCount, 20, Normal, nullptr) \
    v(unsigned, repatchBufferingCountdown, 8, Normal, nullptr) \
    \
    v(bool, dumpGeneratedBytecodes, false, Normal, nullptr) \
    v(bool, dumpBytecodeLivenessResults, false, Normal, nullptr) \
    v(bool, validateBytecode, false, Normal, nullptr) \
    v(bool, forceDebuggerBytecodeGeneration, false, Normal, nullptr) \
    v(bool, dumpBytecodesBeforeGeneratorification, false, Normal, nullptr) \
    \
    v(bool, useFunctionDotArguments, true, Normal, nullptr) \
    v(bool, useTailCalls, true, Normal, nullptr) \
    v(bool, alwaysUseShadowChicken, false, Normal, nullptr) \
    v(unsigned, shadowChickenLogSize, 1000, Normal, nullptr) \
    v(unsigned, shadowChickenMaxTailDeletedFramesSize, 128, Normal, nullptr) \
    \
    /* dumpDisassembly implies dumpDFGDisassembly. */ \
    v(bool, dumpDisassembly, false, Normal, "dumps disassembly of all JIT compiled code upon compilation") \
    v(bool, asyncDisassembly, false, Normal, nullptr) \
    v(bool, dumpDFGDisassembly, false, Normal, "dumps disassembly of DFG function upon compilation") \
    v(bool, dumpFTLDisassembly, false, Normal, "dumps disassembly of FTL function upon compilation") \
    v(bool, dumpAllDFGNodes, false, Normal, nullptr) \
    v(optionRange, bytecodeRangeToJITCompile, 0, Normal, "bytecode size range to allow compilation on, e.g. 1:100") \
    v(optionRange, bytecodeRangeToDFGCompile, 0, Normal, "bytecode size range to allow DFG compilation on, e.g. 1:100") \
    v(optionRange, bytecodeRangeToFTLCompile, 0, Normal, "bytecode size range to allow FTL compilation on, e.g. 1:100") \
    v(optionString, jitWhitelist, nullptr, Normal, "file with list of function signatures to allow compilation on") \
    v(optionString, dfgWhitelist, nullptr, Normal, "file with list of function signatures to allow DFG compilation on") \
    v(bool, dumpSourceAtDFGTime, false, Normal, "dumps source code of JS function being DFG compiled") \
    v(bool, dumpBytecodeAtDFGTime, false, Normal, "dumps bytecode of JS function being DFG compiled") \
    v(bool, dumpGraphAfterParsing, false, Normal, nullptr) \
    v(bool, dumpGraphAtEachPhase, false, Normal, nullptr) \
    v(bool, dumpDFGGraphAtEachPhase, false, Normal, "dumps the DFG graph at each phase of DFG compilation (note this excludes DFG graphs during FTL compilation)") \
    v(bool, dumpDFGFTLGraphAtEachPhase, false, Normal, "dumps the DFG graph at each phase of DFG compilation when compiling FTL code") \
    v(bool, dumpB3GraphAtEachPhase, false, Normal, "dumps the B3 graph at each phase of compilation") \
    v(bool, dumpAirGraphAtEachPhase, false, Normal, "dumps the Air graph at each phase of compilation") \
    v(bool, verboseDFGByteCodeParsing, false, Normal, nullptr) \
    v(bool, safepointBeforeEachPhase, true, Normal, nullptr) \
    v(bool, verboseCompilation, false, Normal, nullptr) \
    v(bool, verboseFTLCompilation, false, Normal, nullptr) \
    v(bool, logCompilationChanges, false, Normal, nullptr) \
    v(bool, printEachOSRExit, false, Normal, nullptr) \
    v(bool, validateGraph, false, Normal, nullptr) \
    v(bool, validateGraphAtEachPhase, false, Normal, nullptr) \
    v(bool, verboseValidationFailure, false, Normal, nullptr) \
    v(bool, verboseOSR, false, Normal, nullptr) \
    v(bool, verboseFTLOSRExit, false, Normal, nullptr) \
    v(bool, verboseCallLink, false, Normal, nullptr) \
    v(bool, verboseCompilationQueue, false, Normal, nullptr) \
    v(bool, reportCompileTimes, false, Normal, "dumps JS function signature and the time it took to compile in all tiers") \
    v(bool, reportBaselineCompileTimes, false, Normal, "dumps JS function signature and the time it took to BaselineJIT compile") \
    v(bool, reportDFGCompileTimes, false, Normal, "dumps JS function signature and the time it took to DFG and FTL compile") \
    v(bool, reportFTLCompileTimes, false, Normal, "dumps JS function signature and the time it took to FTL compile") \
    v(bool, reportDFGPhaseTimes, false, Normal, "dumps JS function name and the time is took for each DFG phase") \
    v(bool, reportTotalCompileTimes, false, Normal, nullptr) \
    v(bool, verboseExitProfile, false, Normal, nullptr) \
    v(bool, verboseCFA, false, Normal, nullptr) \
    v(bool, verboseFTLToJSThunk, false, Normal, nullptr) \
    v(bool, verboseFTLFailure, false, Normal, nullptr) \
    v(bool, alwaysComputeHash, false, Normal, nullptr) \
    v(bool, testTheFTL, false, Normal, nullptr) \
    v(bool, verboseSanitizeStack, false, Normal, nullptr) \
    v(bool, useGenerationalGC, true, Normal, nullptr) \
    v(bool, useConcurrentBarriers, true, Normal, nullptr) \
    v(bool, useConcurrentGC, true, Normal, nullptr) \
    v(bool, collectContinuously, false, Normal, nullptr) \
    v(double, collectContinuouslyPeriodMS, 1, Normal, nullptr) \
    v(bool, forceFencedBarrier, false, Normal, nullptr) \
    v(bool, verboseVisitRace, false, Normal, nullptr) \
    v(bool, optimizeParallelSlotVisitorsForStoppedMutator, false, Normal, nullptr) \
    v(unsigned, largeHeapSize, 32 * 1024 * 1024, Normal, nullptr) \
    v(unsigned, smallHeapSize, 1 * 1024 * 1024, Normal, nullptr) \
    v(double, smallHeapRAMFraction, 0.25, Normal, nullptr) \
    v(double, smallHeapGrowthFactor, 2, Normal, nullptr) \
    v(double, mediumHeapRAMFraction, 0.5, Normal, nullptr) \
    v(double, mediumHeapGrowthFactor, 1.5, Normal, nullptr) \
    v(double, largeHeapGrowthFactor, 1.24, Normal, nullptr) \
    v(double, minimumMutatorUtilization, 0, Normal, nullptr) \
    v(double, maximumMutatorUtilization, 0.7, Normal, nullptr) \
    v(double, epsilonMutatorUtilization, 0.01, Normal, nullptr) \
    v(double, concurrentGCMaxHeadroom, 1.5, Normal, nullptr) \
    v(double, concurrentGCPeriodMS, 2, Normal, nullptr) \
    v(bool, useStochasticMutatorScheduler, true, Normal, nullptr) \
    v(double, minimumGCPauseMS, 0.3, Normal, nullptr) \
    v(double, gcPauseScale, 0.3, Normal, nullptr) \
    v(double, gcIncrementBytes, 10000, Normal, nullptr) \
    v(double, gcIncrementMaxBytes, 100000, Normal, nullptr) \
    v(double, gcIncrementScale, 0, Normal, nullptr) \
    v(bool, scribbleFreeCells, false, Normal, nullptr) \
    v(double, sizeClassProgression, 1.4, Normal, nullptr) \
    v(unsigned, largeAllocationCutoff, 100000, Normal, nullptr) \
    v(bool, dumpSizeClasses, false, Normal, nullptr) \
    v(bool, useBumpAllocator, true, Normal, nullptr) \
    v(bool, stealEmptyBlocksFromOtherAllocators, true, Normal, nullptr) \
    v(bool, eagerlyUpdateTopCallFrame, false, Normal, nullptr) \
    \
    v(bool, useOSREntryToDFG, true, Normal, nullptr) \
    v(bool, useOSREntryToFTL, true, Normal, nullptr) \
    \
    v(bool, useFTLJIT, true, Normal, "allows the FTL JIT to be used if true") \
    v(bool, useFTLTBAA, true, Normal, nullptr) \
    v(bool, validateFTLOSRExitLiveness, false, Normal, nullptr) \
    v(bool, b3AlwaysFailsBeforeCompile, false, Normal, nullptr) \
    v(bool, b3AlwaysFailsBeforeLink, false, Normal, nullptr) \
    v(bool, ftlCrashes, false, Normal, nullptr) /* fool-proof way of checking that you ended up in the FTL. ;-) */\
    v(bool, clobberAllRegsInFTLICSlowPath, !ASSERT_DISABLED, Normal, nullptr) \
    v(bool, useAccessInlining, true, Normal, nullptr) \
    v(unsigned, maxAccessVariantListSize, 8, Normal, nullptr) \
    v(bool, usePolyvariantDevirtualization, true, Normal, nullptr) \
    v(bool, usePolymorphicAccessInlining, true, Normal, nullptr) \
    v(bool, usePolymorphicCallInlining, true, Normal, nullptr) \
    v(bool, usePolymorphicCallInliningForNonStubStatus, false, Normal, nullptr) \
    v(unsigned, maxPolymorphicCallVariantListSize, 15, Normal, nullptr) \
    v(unsigned, maxPolymorphicCallVariantListSizeForTopTier, 5, Normal, nullptr) \
    v(unsigned, maxPolymorphicCallVariantListSizeForWebAssemblyToJS, 5, Normal, nullptr) \
    v(unsigned, maxPolymorphicCallVariantsForInlining, 5, Normal, nullptr) \
    v(unsigned, frequentCallThreshold, 2, Normal, nullptr) \
    v(double, minimumCallToKnownRate, 0.51, Normal, nullptr) \
    v(bool, createPreHeaders, true, Normal, nullptr) \
    v(bool, useMovHintRemoval, true, Normal, nullptr) \
    v(bool, usePutStackSinking, true, Normal, nullptr) \
    v(bool, useObjectAllocationSinking, true, Normal, nullptr) \
    v(bool, logExecutableAllocation, false, Normal, nullptr) \
    \
    v(bool, useConcurrentJIT, true, Normal, "allows the DFG / FTL compilation in threads other than the executing JS thread") \
    v(unsigned, numberOfDFGCompilerThreads, computeNumberOfWorkerThreads(2, 2) - 1, Normal, nullptr) \
    v(unsigned, numberOfFTLCompilerThreads, computeNumberOfWorkerThreads(8, 2) - 1, Normal, nullptr) \
    v(int32, priorityDeltaOfDFGCompilerThreads, computePriorityDeltaOfWorkerThreads(-1, 0), Normal, nullptr) \
    v(int32, priorityDeltaOfFTLCompilerThreads, computePriorityDeltaOfWorkerThreads(-2, 0), Normal, nullptr) \
    \
    v(bool, useProfiler, false, Normal, nullptr) \
    v(bool, disassembleBaselineForProfiler, true, Normal, nullptr) \
    \
    v(bool, useArchitectureSpecificOptimizations, true, Normal, nullptr) \
    \
    v(bool, breakOnThrow, false, Normal, nullptr) \
    \
    v(unsigned, maximumOptimizationCandidateInstructionCount, 100000, Normal, nullptr) \
    \
    v(unsigned, maximumFunctionForCallInlineCandidateInstructionCount, 180, Normal, nullptr) \
    v(unsigned, maximumFunctionForClosureCallInlineCandidateInstructionCount, 100, Normal, nullptr) \
    v(unsigned, maximumFunctionForConstructInlineCandidateInstructionCount, 100, Normal, nullptr) \
    \
    v(unsigned, maximumFTLCandidateInstructionCount, 20000, Normal, nullptr) \
    \
    /* Depth of inline stack, so 1 = no inlining, 2 = one level, etc. */ \
    v(unsigned, maximumInliningDepth, 5, Normal, "maximum allowed inlining depth.  Depth of 1 means no inlining") \
    v(unsigned, maximumInliningRecursion, 2, Normal, nullptr) \
    \
    /* Maximum size of a caller for enabling inlining. This is purely to protect us */\
    /* from super long compiles that take a lot of memory. */\
    v(unsigned, maximumInliningCallerSize, 10000, Normal, nullptr) \
    \
    v(unsigned, maximumVarargsForInlining, 100, Normal, nullptr) \
    \
    v(bool, usePolyvariantCallInlining, true, Normal, nullptr) \
    v(bool, usePolyvariantByIdInlining, true, Normal, nullptr) \
    \
    v(bool, useMaximalFlushInsertionPhase, false, Normal, "Setting to true allows the DFG's MaximalFlushInsertionPhase to run.") \
    \
    v(unsigned, maximumBinaryStringSwitchCaseLength, 50, Normal, nullptr) \
    v(unsigned, maximumBinaryStringSwitchTotalLength, 2000, Normal, nullptr) \
    \
    v(double, jitPolicyScale, 1.0, Normal, "scale JIT thresholds to this specified ratio between 0.0 (compile ASAP) and 1.0 (compile like normal).") \
    v(bool, forceEagerCompilation, false, Normal, nullptr) \
    v(int32, thresholdForJITAfterWarmUp, 500, Normal, nullptr) \
    v(int32, thresholdForJITSoon, 100, Normal, nullptr) \
    \
    v(int32, thresholdForOptimizeAfterWarmUp, 1000, Normal, nullptr) \
    v(int32, thresholdForOptimizeAfterLongWarmUp, 1000, Normal, nullptr) \
    v(int32, thresholdForOptimizeSoon, 1000, Normal, nullptr) \
    v(int32, executionCounterIncrementForLoop, 1, Normal, nullptr) \
    v(int32, executionCounterIncrementForEntry, 15, Normal, nullptr) \
    \
    v(int32, thresholdForFTLOptimizeAfterWarmUp, 100000, Normal, nullptr) \
    v(int32, thresholdForFTLOptimizeSoon, 1000, Normal, nullptr) \
    v(int32, ftlTierUpCounterIncrementForLoop, 1, Normal, nullptr) \
    v(int32, ftlTierUpCounterIncrementForReturn, 15, Normal, nullptr) \
    v(unsigned, ftlOSREntryFailureCountForReoptimization, 15, Normal, nullptr) \
    v(unsigned, ftlOSREntryRetryThreshold, 100, Normal, nullptr) \
    \
    v(int32, evalThresholdMultiplier, 10, Normal, nullptr) \
    v(unsigned, maximumEvalCacheableSourceLength, 256, Normal, nullptr) \
    \
    v(bool, randomizeExecutionCountsBetweenCheckpoints, false, Normal, nullptr) \
    v(int32, maximumExecutionCountsBetweenCheckpointsForBaseline, 1000, Normal, nullptr) \
    v(int32, maximumExecutionCountsBetweenCheckpointsForUpperTiers, 50000, Normal, nullptr) \
    \
    v(unsigned, likelyToTakeSlowCaseMinimumCount, 20, Normal, nullptr) \
    v(unsigned, couldTakeSlowCaseMinimumCount, 10, Normal, nullptr) \
    \
    v(unsigned, osrExitCountForReoptimization, 100, Normal, nullptr) \
    v(unsigned, osrExitCountForReoptimizationFromLoop, 5, Normal, nullptr) \
    \
    v(unsigned, reoptimizationRetryCounterMax, 0, Normal, nullptr)  \
    \
    v(unsigned, minimumOptimizationDelay, 1, Normal, nullptr) \
    v(unsigned, maximumOptimizationDelay, 5, Normal, nullptr) \
    v(double, desiredProfileLivenessRate, 0.75, Normal, nullptr) \
    v(double, desiredProfileFullnessRate, 0.35, Normal, nullptr) \
    \
    v(double, doubleVoteRatioForDoubleFormat, 2, Normal, nullptr) \
    v(double, structureCheckVoteRatioForHoisting, 1, Normal, nullptr) \
    v(double, checkArrayVoteRatioForHoisting, 1, Normal, nullptr) \
    \
    v(unsigned, maximumDirectCallStackSize, 200, Normal, nullptr) \
    \
    v(unsigned, minimumNumberOfScansBetweenRebalance, 100, Normal, nullptr) \
    v(unsigned, numberOfGCMarkers, computeNumberOfGCMarkers(8), Normal, nullptr) \
    v(unsigned, opaqueRootMergeThreshold, 1000, Normal, nullptr) \
    v(double, minHeapUtilization, 0.8, Normal, nullptr) \
    v(double, minMarkedBlockUtilization, 0.9, Normal, nullptr) \
    v(unsigned, slowPathAllocsBetweenGCs, 0, Normal, "force a GC on every Nth slow path alloc, where N is specified by this option") \
    \
    v(double, percentCPUPerMBForFullTimer, 0.0003125, Normal, nullptr) \
    v(double, percentCPUPerMBForEdenTimer, 0.0025, Normal, nullptr) \
    v(double, collectionTimerMaxPercentCPU, 0.05, Normal, nullptr) \
    \
    v(bool, forceWeakRandomSeed, false, Normal, nullptr) \
    v(unsigned, forcedWeakRandomSeed, 0, Normal, nullptr) \
    \
    v(bool, useZombieMode, false, Normal, "debugging option to scribble over dead objects with 0xbadbeef0") \
    v(bool, useImmortalObjects, false, Normal, "debugging option to keep all objects alive forever") \
    v(bool, sweepSynchronously, false, Normal, "debugging option to sweep all dead objects synchronously at GC end before resuming mutator") \
    v(unsigned, maxSingleAllocationSize, 0, Configurable, "debugging option to limit individual allocations to a max size (0 = limit not set, N = limit size in bytes)") \
    \
    v(gcLogLevel, logGC, GCLogging::None, Normal, "debugging option to log GC activity (0 = None, 1 = Basic, 2 = Verbose)") \
    v(bool, useGC, true, Normal, nullptr) \
    v(bool, gcAtEnd, false, Normal, "If true, the jsc CLI will do a GC before exiting") \
    v(bool, forceGCSlowPaths, false, Normal, "If true, we will force all JIT fast allocations down their slow paths.")\
    v(unsigned, gcMaxHeapSize, 0, Normal, nullptr) \
    v(unsigned, forceRAMSize, 0, Normal, nullptr) \
    v(bool, recordGCPauseTimes, false, Normal, nullptr) \
    v(bool, logHeapStatisticsAtExit, false, Normal, nullptr) \
    v(bool, forceCodeBlockToJettisonDueToOldAge, false, Normal, "If true, this means that anytime we can jettison a CodeBlock due to old age, we do.") \
    v(bool, useEagerCodeBlockJettisonTiming, false, Normal, "If true, the time slices for jettisoning a CodeBlock due to old age are shrunk significantly.") \
    \
    v(bool, useTypeProfiler, false, Normal, nullptr) \
    v(bool, useControlFlowProfiler, false, Normal, nullptr) \
    \
    v(bool, useSamplingProfiler, false, Normal, nullptr) \
    v(unsigned, sampleInterval, 1000, Normal, "Time between stack traces in microseconds.") \
    v(bool, collectSamplingProfilerDataForJSCShell, false, Normal, "This corresponds to the JSC shell's --sample option.") \
    v(unsigned, samplingProfilerTopFunctionsCount, 12, Normal, "Number of top functions to report when using the command line interface.") \
    v(unsigned, samplingProfilerTopBytecodesCount, 40, Normal, "Number of top bytecodes to report when using the command line interface.") \
    v(optionString, samplingProfilerPath, nullptr, Normal, "The path to the directory to write sampiling profiler output to. This probably will not work with WK2 unless the path is in the whitelist.") \
    v(bool, sampleCCode, false, Normal, "Causes the sampling profiler to record profiling data for C frames.") \
    \
    v(bool, alwaysGeneratePCToCodeOriginMap, false, Normal, "This will make sure we always generate a PCToCodeOriginMap for JITed code.") \
    \
    v(bool, verifyHeap, false, Normal, nullptr) \
    v(unsigned, numberOfGCCyclesToRecordForVerification, 3, Normal, nullptr) \
    \
    v(bool, useExceptionFuzz, false, Normal, nullptr) \
    v(unsigned, fireExceptionFuzzAt, 0, Normal, nullptr) \
    v(bool, validateDFGExceptionHandling, false, Normal, "Causes the DFG to emit code validating exception handling for each node that can exit") /* This is true by default on Debug builds */\
    v(bool, dumpSimulatedThrows, false, Normal, "Dumps the call stack at each simulated throw for exception scope verification") \
    v(bool, validateExceptionChecks, false, Normal, "Verifies that needed exception checks are performed.") \
    \
    v(bool, useExecutableAllocationFuzz, false, Normal, nullptr) \
    v(unsigned, fireExecutableAllocationFuzzAt, 0, Normal, nullptr) \
    v(unsigned, fireExecutableAllocationFuzzAtOrAfter, 0, Normal, nullptr) \
    v(bool, verboseExecutableAllocationFuzz, false, Normal, nullptr) \
    \
    v(bool, useOSRExitFuzz, false, Normal, nullptr) \
    v(unsigned, fireOSRExitFuzzAtStatic, 0, Normal, nullptr) \
    v(unsigned, fireOSRExitFuzzAt, 0, Normal, nullptr) \
    v(unsigned, fireOSRExitFuzzAtOrAfter, 0, Normal, nullptr) \
    \
    v(bool, logB3PhaseTimes, false, Normal, nullptr) \
    v(double, rareBlockPenalty, 0.001, Normal, nullptr) \
    v(bool, airSpillsEverything, false, Normal, nullptr) \
    v(bool, airForceBriggsAllocator, false, Normal, nullptr) \
    v(bool, airForceIRCAllocator, false, Normal, nullptr) \
    v(bool, logAirRegisterPressure, false, Normal, nullptr) \
    v(unsigned, maxB3TailDupBlockSize, 3, Normal, nullptr) \
    v(unsigned, maxB3TailDupBlockSuccessors, 3, Normal, nullptr) \
    \
    v(bool, useDollarVM, false, Restricted, "installs the $vm debugging tool in global objects") \
    v(optionString, functionOverrides, nullptr, Restricted, "file with debugging overrides for function bodies") \
    v(bool, useSigillCrashAnalyzer, false, Configurable, "logs data about SIGILL crashes") \
    \
    v(unsigned, watchdog, 0, Normal, "watchdog timeout (0 = Disabled, N = a timeout period of N milliseconds)") \
    v(bool, usePollingTraps, false, Normal, "use polling (instead of signalling) VM traps") \
    \
    v(bool, useICStats, false, Normal, nullptr) \
    \
    v(unsigned, prototypeHitCountForLLIntCaching, 2, Normal, "Number of prototype property hits before caching a prototype in the LLInt. A count of 0 means never cache.") \
    \
    v(bool, dumpModuleRecord, false, Normal, nullptr) \
    v(bool, dumpModuleLoadingState, false, Normal, nullptr) \
    v(bool, exposeInternalModuleLoader, false, Normal, "expose the internal module loader object to the global space for debugging") \
    \
    v(bool, useSuperSampler, false, Normal, nullptr) \
    \
    v(bool, reportLLIntStats, false, Configurable, "Reports LLInt statistics") \
    v(optionString, llintStatsFile, nullptr, Configurable, "File to collect LLInt statistics in") \
    \
    v(bool, useSourceProviderCache, true, Normal, "If false, the parser will not use the source provider cache. It's good to verify everything works when this is false. Because the cache is so successful, it can mask bugs.") \
    v(bool, useCodeCache, true, Normal, "If false, the unlinked byte code cache will not be used.") \
    \
    v(bool, useWebAssembly, true, Normal, "Expose the WebAssembly global object.") \
    v(bool, simulateWebAssemblyLowMemory, false, Normal, "If true, the Memory object won't mmap the full 'maximum' range and instead will allocate the minimum required amount.") \
    v(bool, useWebAssemblyFastMemory, true, Normal, "If true, we will try to use a 32-bit address space with a signal handler to bounds check wasm memory.")


enum OptionEquivalence {
    SameOption,
    InvertedOption,
};

#define JSC_ALIASED_OPTIONS(v) \
    v(enableFunctionDotArguments, useFunctionDotArguments, SameOption) \
    v(enableTailCalls, useTailCalls, SameOption) \
    v(showDisassembly, dumpDisassembly, SameOption) \
    v(showDFGDisassembly, dumpDFGDisassembly, SameOption) \
    v(showFTLDisassembly, dumpFTLDisassembly, SameOption) \
    v(showAllDFGNodes, dumpAllDFGNodes, SameOption) \
    v(alwaysDoFullCollection, useGenerationalGC, InvertedOption) \
    v(enableOSREntryToDFG, useOSREntryToDFG, SameOption) \
    v(enableOSREntryToFTL, useOSREntryToFTL, SameOption) \
    v(enableAccessInlining, useAccessInlining, SameOption) \
    v(enablePolyvariantDevirtualization, usePolyvariantDevirtualization, SameOption) \
    v(enablePolymorphicAccessInlining, usePolymorphicAccessInlining, SameOption) \
    v(enablePolymorphicCallInlining, usePolymorphicCallInlining, SameOption) \
    v(enableMovHintRemoval, useMovHintRemoval, SameOption) \
    v(enableObjectAllocationSinking, useObjectAllocationSinking, SameOption) \
    v(enableConcurrentJIT, useConcurrentJIT, SameOption) \
    v(enableProfiler, useProfiler, SameOption) \
    v(enableArchitectureSpecificOptimizations, useArchitectureSpecificOptimizations, SameOption) \
    v(enablePolyvariantCallInlining, usePolyvariantCallInlining, SameOption) \
    v(enablePolyvariantByIdInlining, usePolyvariantByIdInlining, SameOption) \
    v(enableMaximalFlushInsertionPhase, useMaximalFlushInsertionPhase, SameOption) \
    v(objectsAreImmortal, useImmortalObjects, SameOption) \
    v(showObjectStatistics, dumpObjectStatistics, SameOption) \
    v(disableGC, useGC, InvertedOption) \
    v(enableTypeProfiler, useTypeProfiler, SameOption) \
    v(enableControlFlowProfiler, useControlFlowProfiler, SameOption) \
    v(enableExceptionFuzz, useExceptionFuzz, SameOption) \
    v(enableExecutableAllocationFuzz, useExecutableAllocationFuzz, SameOption) \
    v(enableOSRExitFuzz, useOSRExitFuzz, SameOption) \
    v(enableDollarVM, useDollarVM, SameOption) \
    v(enableWebAssembly, useWebAssembly, SameOption) \

class Options {
public:
    enum class DumpLevel {
        None = 0,
        Overridden,
        All,
        Verbose
    };

    enum class Availability {
        Normal = 0,
        Restricted,
        Configurable
    };

    // This typedef is to allow us to eliminate the '_' in the field name in
    // union inside Entry. This is needed to keep the style checker happy.
    typedef int32_t int32;

    // Declare the option IDs:
    enum ID {
#define FOR_EACH_OPTION(type_, name_, defaultValue_, availability_, description_) \
        name_##ID,
        JSC_OPTIONS(FOR_EACH_OPTION)
#undef FOR_EACH_OPTION
        numberOfOptions
    };

    enum class Type {
        boolType,
        unsignedType,
        doubleType,
        int32Type,
        optionRangeType,
        optionStringType,
        gcLogLevelType,
    };

    JS_EXPORT_PRIVATE static void initialize();

    // Parses a string of options where each option is of the format "--<optionName>=<value>"
    // and are separated by a space. The leading "--" is optional and will be ignored.
    JS_EXPORT_PRIVATE static bool setOptions(const char* optionsList);

    // Parses a single command line option in the format "<optionName>=<value>"
    // (no spaces allowed) and set the specified option if appropriate.
    JS_EXPORT_PRIVATE static bool setOption(const char* arg);

    JS_EXPORT_PRIVATE static void dumpAllOptions(FILE*, DumpLevel, const char* title = nullptr);
    JS_EXPORT_PRIVATE static void dumpAllOptionsInALine(StringBuilder&);

    JS_EXPORT_PRIVATE static void ensureOptionsAreCoherent();

    JS_EXPORT_PRIVATE static void enableRestrictedOptions(bool enableOrNot);

    // Declare accessors for each option:
#define FOR_EACH_OPTION(type_, name_, defaultValue_, availability_, description_) \
    ALWAYS_INLINE static type_& name_() { return s_options[name_##ID].type_##Val; } \
    ALWAYS_INLINE static type_& name_##Default() { return s_defaultOptions[name_##ID].type_##Val; }

    JSC_OPTIONS(FOR_EACH_OPTION)
#undef FOR_EACH_OPTION

    static bool isAvailable(ID, Availability);

private:
    // For storing for an option value:
    union Entry {
        bool boolVal;
        unsigned unsignedVal;
        double doubleVal;
        int32 int32Val;
        OptionRange optionRangeVal;
        const char* optionStringVal;
        GCLogging::Level gcLogLevelVal;
    };

    // For storing constant meta data about each option:
    struct EntryInfo {
        const char* name;
        const char* description;
        Type type;
        Availability availability;
    };

    Options();

    enum DumpDefaultsOption {
        DontDumpDefaults,
        DumpDefaults
    };
    static void dumpOptionsIfNeeded();
    static void dumpAllOptions(StringBuilder&, DumpLevel, const char* title,
        const char* separator, const char* optionHeader, const char* optionFooter, DumpDefaultsOption);
    static void dumpOption(StringBuilder&, DumpLevel, ID,
        const char* optionHeader, const char* optionFooter, DumpDefaultsOption);

    static bool setOptionWithoutAlias(const char* arg);
    static bool setAliasedOption(const char* arg);
    static bool overrideAliasedOptionWithHeuristic(const char* name);

    // Declare the singleton instance of the options store:
    JS_EXPORTDATA static Entry s_options[numberOfOptions];
    static Entry s_defaultOptions[numberOfOptions];
    static const EntryInfo s_optionsInfo[numberOfOptions];

    friend class Option;
};

class Option {
public:
    Option(Options::ID id)
        : m_id(id)
        , m_entry(Options::s_options[m_id])
    {
    }
    
    void dump(StringBuilder&) const;

    bool operator==(const Option& other) const;
    bool operator!=(const Option& other) const { return !(*this == other); }
    
    Options::ID id() const { return m_id; }
    const char* name() const;
    const char* description() const;
    Options::Type type() const;
    Options::Availability availability() const;
    bool isOverridden() const;
    const Option defaultOption() const;
    
    bool& boolVal();
    unsigned& unsignedVal();
    double& doubleVal();
    int32_t& int32Val();
    OptionRange optionRangeVal();
    const char* optionStringVal();
    GCLogging::Level& gcLogLevelVal();
    
private:
    // Only used for constructing default Options.
    Option(Options::ID id, Options::Entry& entry)
        : m_id(id)
        , m_entry(entry)
    {
    }
    
    Options::ID m_id;
    Options::Entry& m_entry;
};

inline const char* Option::name() const
{
    return Options::s_optionsInfo[m_id].name;
}

inline const char* Option::description() const
{
    return Options::s_optionsInfo[m_id].description;
}

inline Options::Type Option::type() const
{
    return Options::s_optionsInfo[m_id].type;
}

inline Options::Availability Option::availability() const
{
    return Options::s_optionsInfo[m_id].availability;
}

inline bool Option::isOverridden() const
{
    return *this != defaultOption();
}

inline const Option Option::defaultOption() const
{
    return Option(m_id, Options::s_defaultOptions[m_id]);
}

inline bool& Option::boolVal()
{
    return m_entry.boolVal;
}

inline unsigned& Option::unsignedVal()
{
    return m_entry.unsignedVal;
}

inline double& Option::doubleVal()
{
    return m_entry.doubleVal;
}

inline int32_t& Option::int32Val()
{
    return m_entry.int32Val;
}

inline OptionRange Option::optionRangeVal()
{
    return m_entry.optionRangeVal;
}

inline const char* Option::optionStringVal()
{
    return m_entry.optionStringVal;
}

inline GCLogging::Level& Option::gcLogLevelVal()
{
    return m_entry.gcLogLevelVal;
}

} // namespace JSC
