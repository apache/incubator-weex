/*
 * Copyright (C) 2008-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Cameron Zwarich <cwzwarich@uwaterloo.ca>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "ArrayProfile.h"
#include "ByValInfo.h"
#include "BytecodeConventions.h"
#include "CallLinkInfo.h"
#include "CallReturnOffsetToBytecodeOffset.h"
#include "CodeBlockHash.h"
#include "CodeOrigin.h"
#include "CodeType.h"
#include "CompactJITCodeMap.h"
#include "ConcurrentJSLock.h"
#include "DFGCommon.h"
#include "DFGExitProfile.h"
#include "DeferredCompilationCallback.h"
#include "DirectEvalCodeCache.h"
#include "EvalExecutable.h"
#include "ExecutionCounter.h"
#include "ExpressionRangeInfo.h"
#include "FunctionExecutable.h"
#include "HandlerInfo.h"
#include "Instruction.h"
#include "JITCode.h"
#include "JITMathICForwards.h"
#include "JSCell.h"
#include "JSGlobalObject.h"
#include "JumpTable.h"
#include "LLIntCallLinkInfo.h"
#include "LLIntPrototypeLoadAdaptiveStructureWatchpoint.h"
#include "LazyOperandValueProfile.h"
#include "ModuleProgramExecutable.h"
#include "ObjectAllocationProfile.h"
#include "Options.h"
#include "ProfilerJettisonReason.h"
#include "ProgramExecutable.h"
#include "PutPropertySlot.h"
#include "UnconditionalFinalizer.h"
#include "ValueProfile.h"
#include "VirtualRegister.h"
#include "Watchpoint.h"
#include <wtf/Bag.h>
#include <wtf/FastBitVector.h>
#include <wtf/FastMalloc.h>
#include <wtf/RefCountedArray.h>
#include <wtf/RefPtr.h>
#include <wtf/SegmentedVector.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

namespace JSC {

class BytecodeLivenessAnalysis;
class CodeBlockSet;
class ExecState;
class JSModuleEnvironment;
class LLIntOffsetsExtractor;
class PCToCodeOriginMap;
class RegisterAtOffsetList;
class StructureStubInfo;

enum class AccessType : int8_t;

struct ArithProfile;

typedef HashMap<CodeOrigin, StructureStubInfo*, CodeOriginApproximateHash> StubInfoMap;

enum ReoptimizationMode { DontCountReoptimization, CountReoptimization };

class CodeBlock : public JSCell {
    typedef JSCell Base;
    friend class BytecodeLivenessAnalysis;
    friend class JIT;
    friend class LLIntOffsetsExtractor;

    class UnconditionalFinalizer : public JSC::UnconditionalFinalizer { 
        void finalizeUnconditionally() override;
    };

    class WeakReferenceHarvester : public JSC::WeakReferenceHarvester {
        void visitWeakReferences(SlotVisitor&) override;
    };

public:
    enum CopyParsedBlockTag { CopyParsedBlock };

    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    DECLARE_INFO;

protected:
    CodeBlock(VM*, Structure*, CopyParsedBlockTag, CodeBlock& other);
    CodeBlock(VM*, Structure*, ScriptExecutable* ownerExecutable, UnlinkedCodeBlock*, JSScope*, RefPtr<SourceProvider>&&, unsigned sourceOffset, unsigned firstLineColumnOffset);

    void finishCreation(VM&, CopyParsedBlockTag, CodeBlock& other);
    bool finishCreation(VM&, ScriptExecutable* ownerExecutable, UnlinkedCodeBlock*, JSScope*);

    WriteBarrier<JSGlobalObject> m_globalObject;

public:
    JS_EXPORT_PRIVATE ~CodeBlock();

    UnlinkedCodeBlock* unlinkedCodeBlock() const { return m_unlinkedCode.get(); }

    CString inferredName() const;
    CodeBlockHash hash() const;
    bool hasHash() const;
    bool isSafeToComputeHash() const;
    CString hashAsStringIfPossible() const;
    CString sourceCodeForTools() const; // Not quite the actual source we parsed; this will do things like prefix the source for a function with a reified signature.
    CString sourceCodeOnOneLine() const; // As sourceCodeForTools(), but replaces all whitespace runs with a single space.
    void dumpAssumingJITType(PrintStream&, JITCode::JITType) const;
    JS_EXPORT_PRIVATE void dump(PrintStream&) const;

    int numParameters() const { return m_numParameters; }
    void setNumParameters(int newValue);

    int numCalleeLocals() const { return m_numCalleeLocals; }

    int* addressOfNumParameters() { return &m_numParameters; }
    static ptrdiff_t offsetOfNumParameters() { return OBJECT_OFFSETOF(CodeBlock, m_numParameters); }

    CodeBlock* alternative() const { return static_cast<CodeBlock*>(m_alternative.get()); }
    void setAlternative(VM&, CodeBlock*);

    template <typename Functor> void forEachRelatedCodeBlock(Functor&& functor)
    {
        Functor f(std::forward<Functor>(functor));
        Vector<CodeBlock*, 4> codeBlocks;
        codeBlocks.append(this);

        while (!codeBlocks.isEmpty()) {
            CodeBlock* currentCodeBlock = codeBlocks.takeLast();
            f(currentCodeBlock);

            if (CodeBlock* alternative = currentCodeBlock->alternative())
                codeBlocks.append(alternative);
            if (CodeBlock* osrEntryBlock = currentCodeBlock->specialOSREntryBlockOrNull())
                codeBlocks.append(osrEntryBlock);
        }
    }
    
    CodeSpecializationKind specializationKind() const
    {
        return specializationFromIsConstruct(m_isConstructor);
    }

    CodeBlock* alternativeForJettison();    
    JS_EXPORT_PRIVATE CodeBlock* baselineAlternative();
    
    // FIXME: Get rid of this.
    // https://bugs.webkit.org/show_bug.cgi?id=123677
    CodeBlock* baselineVersion();

    static size_t estimatedSize(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);
    void visitChildren(SlotVisitor&);
    void visitWeakly(SlotVisitor&);
    void clearVisitWeaklyHasBeenCalled();

    void dumpSource();
    void dumpSource(PrintStream&);

    void dumpBytecode();
    void dumpBytecode(PrintStream&);
    void dumpBytecode(PrintStream& out, const Instruction* begin, const Instruction*& it, const StubInfoMap& = StubInfoMap(), const CallLinkInfoMap& = CallLinkInfoMap());
    void dumpBytecode(PrintStream& out, unsigned bytecodeOffset, const StubInfoMap& = StubInfoMap(), const CallLinkInfoMap& = CallLinkInfoMap());

    void dumpExceptionHandlers(PrintStream&);
    void printStructures(PrintStream&, const Instruction*);
    void printStructure(PrintStream&, const char* name, const Instruction*, int operand);

    void dumpMathICStats();

    bool isStrictMode() const { return m_isStrictMode; }
    ECMAMode ecmaMode() const { return isStrictMode() ? StrictMode : NotStrictMode; }

    bool hasInstalledVMTrapBreakpoints() const;
    bool installVMTrapBreakpoints();

    inline bool isKnownNotImmediate(int index)
    {
        if (index == m_thisRegister.offset() && !m_isStrictMode)
            return true;

        if (isConstantRegisterIndex(index))
            return getConstant(index).isCell();

        return false;
    }

    ALWAYS_INLINE bool isTemporaryRegisterIndex(int index)
    {
        return index >= m_numVars;
    }

    HandlerInfo* handlerForBytecodeOffset(unsigned bytecodeOffset, RequiredHandler = RequiredHandler::AnyHandler);
    HandlerInfo* handlerForIndex(unsigned, RequiredHandler = RequiredHandler::AnyHandler);
    void removeExceptionHandlerForCallSite(CallSiteIndex);
    unsigned lineNumberForBytecodeOffset(unsigned bytecodeOffset);
    unsigned columnNumberForBytecodeOffset(unsigned bytecodeOffset);
    void expressionRangeForBytecodeOffset(unsigned bytecodeOffset, int& divot,
        int& startOffset, int& endOffset, unsigned& line, unsigned& column) const;

    std::optional<unsigned> bytecodeOffsetFromCallSiteIndex(CallSiteIndex);

    void getStubInfoMap(const ConcurrentJSLocker&, StubInfoMap& result);
    void getStubInfoMap(StubInfoMap& result);
    
    void getCallLinkInfoMap(const ConcurrentJSLocker&, CallLinkInfoMap& result);
    void getCallLinkInfoMap(CallLinkInfoMap& result);

    void getByValInfoMap(const ConcurrentJSLocker&, ByValInfoMap& result);
    void getByValInfoMap(ByValInfoMap& result);
    
#if ENABLE(JIT)
    StructureStubInfo* addStubInfo(AccessType);
    JITAddIC* addJITAddIC(ArithProfile*);
    JITMulIC* addJITMulIC(ArithProfile*);
    JITNegIC* addJITNegIC(ArithProfile*);
    JITSubIC* addJITSubIC(ArithProfile*);
    Bag<StructureStubInfo>::iterator stubInfoBegin() { return m_stubInfos.begin(); }
    Bag<StructureStubInfo>::iterator stubInfoEnd() { return m_stubInfos.end(); }
    
    // O(n) operation. Use getStubInfoMap() unless you really only intend to get one
    // stub info.
    StructureStubInfo* findStubInfo(CodeOrigin);

    ByValInfo* addByValInfo();

    CallLinkInfo* addCallLinkInfo();
    Bag<CallLinkInfo>::iterator callLinkInfosBegin() { return m_callLinkInfos.begin(); }
    Bag<CallLinkInfo>::iterator callLinkInfosEnd() { return m_callLinkInfos.end(); }

    // This is a slow function call used primarily for compiling OSR exits in the case
    // that there had been inlining. Chances are if you want to use this, you're really
    // looking for a CallLinkInfoMap to amortize the cost of calling this.
    CallLinkInfo* getCallLinkInfoForBytecodeIndex(unsigned bytecodeIndex);
    
    // We call this when we want to reattempt compiling something with the baseline JIT. Ideally
    // the baseline JIT would not add data to CodeBlock, but instead it would put its data into
    // a newly created JITCode, which could be thrown away if we bail on JIT compilation. Then we
    // would be able to get rid of this silly function.
    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=159061
    void resetJITData();
#endif // ENABLE(JIT)

    void unlinkIncomingCalls();

#if ENABLE(JIT)
    void linkIncomingCall(ExecState* callerFrame, CallLinkInfo*);
    void linkIncomingPolymorphicCall(ExecState* callerFrame, PolymorphicCallNode*);
#endif // ENABLE(JIT)

    void linkIncomingCall(ExecState* callerFrame, LLIntCallLinkInfo*);

    void setJITCodeMap(std::unique_ptr<CompactJITCodeMap> jitCodeMap)
    {
        m_jitCodeMap = WTFMove(jitCodeMap);
    }
    CompactJITCodeMap* jitCodeMap()
    {
        return m_jitCodeMap.get();
    }
    
    static void clearLLIntGetByIdCache(Instruction*);

    unsigned bytecodeOffset(Instruction* returnAddress)
    {
        RELEASE_ASSERT(returnAddress >= instructions().begin() && returnAddress < instructions().end());
        return static_cast<Instruction*>(returnAddress) - instructions().begin();
    }

    typedef JSC::Instruction Instruction;
    typedef RefCountedArray<Instruction>& UnpackedInstructions;

    unsigned numberOfInstructions() const { return m_instructions.size(); }
    RefCountedArray<Instruction>& instructions() { return m_instructions; }
    const RefCountedArray<Instruction>& instructions() const { return m_instructions; }

    size_t predictedMachineCodeSize();

    bool usesOpcode(OpcodeID);

    unsigned instructionCount() const { return m_instructions.size(); }

    // Exactly equivalent to codeBlock->ownerExecutable()->newReplacementCodeBlockFor(codeBlock->specializationKind())
    CodeBlock* newReplacement();
    
    void setJITCode(Ref<JITCode>&& code)
    {
        ASSERT(heap()->isDeferred());
        heap()->reportExtraMemoryAllocated(code->size());
        ConcurrentJSLocker locker(m_lock);
        WTF::storeStoreFence(); // This is probably not needed because the lock will also do something similar, but it's good to be paranoid.
        m_jitCode = WTFMove(code);
    }
    RefPtr<JITCode> jitCode() { return m_jitCode; }
    static ptrdiff_t jitCodeOffset() { return OBJECT_OFFSETOF(CodeBlock, m_jitCode); }
    JITCode::JITType jitType() const
    {
        JITCode* jitCode = m_jitCode.get();
        WTF::loadLoadFence();
        JITCode::JITType result = JITCode::jitTypeFor(jitCode);
        WTF::loadLoadFence(); // This probably isn't needed. Oh well, paranoia is good.
        return result;
    }

    bool hasBaselineJITProfiling() const
    {
        return jitType() == JITCode::BaselineJIT;
    }
    
#if ENABLE(JIT)
    CodeBlock* replacement();

    DFG::CapabilityLevel computeCapabilityLevel();
    DFG::CapabilityLevel capabilityLevel();
    DFG::CapabilityLevel capabilityLevelState() { return static_cast<DFG::CapabilityLevel>(m_capabilityLevelState); }

    bool hasOptimizedReplacement(JITCode::JITType typeToReplace);
    bool hasOptimizedReplacement(); // the typeToReplace is my JITType
#endif

    void jettison(Profiler::JettisonReason, ReoptimizationMode = DontCountReoptimization, const FireDetail* = nullptr);
    
    ExecutableBase* ownerExecutable() const { return m_ownerExecutable.get(); }
    ScriptExecutable* ownerScriptExecutable() const { return jsCast<ScriptExecutable*>(m_ownerExecutable.get()); }

    VM* vm() const { return m_vm; }

    void setThisRegister(VirtualRegister thisRegister) { m_thisRegister = thisRegister; }
    VirtualRegister thisRegister() const { return m_thisRegister; }

    bool usesEval() const { return m_unlinkedCode->usesEval(); }

    void setScopeRegister(VirtualRegister scopeRegister)
    {
        ASSERT(scopeRegister.isLocal() || !scopeRegister.isValid());
        m_scopeRegister = scopeRegister;
    }

    VirtualRegister scopeRegister() const
    {
        return m_scopeRegister;
    }
    
    CodeType codeType() const
    {
        return static_cast<CodeType>(m_codeType);
    }

    PutPropertySlot::Context putByIdContext() const
    {
        if (codeType() == EvalCode)
            return PutPropertySlot::PutByIdEval;
        return PutPropertySlot::PutById;
    }

    SourceProvider* source() const { return m_source.get(); }
    unsigned sourceOffset() const { return m_sourceOffset; }
    unsigned firstLineColumnOffset() const { return m_firstLineColumnOffset; }

    size_t numberOfJumpTargets() const { return m_unlinkedCode->numberOfJumpTargets(); }
    unsigned jumpTarget(int index) const { return m_unlinkedCode->jumpTarget(index); }

    String nameForRegister(VirtualRegister);

    unsigned numberOfArgumentValueProfiles()
    {
        ASSERT(m_numParameters >= 0);
        ASSERT(m_argumentValueProfiles.size() == static_cast<unsigned>(m_numParameters));
        return m_argumentValueProfiles.size();
    }
    ValueProfile* valueProfileForArgument(unsigned argumentIndex)
    {
        ValueProfile* result = &m_argumentValueProfiles[argumentIndex];
        ASSERT(result->m_bytecodeOffset == -1);
        return result;
    }

    unsigned numberOfValueProfiles() { return m_valueProfiles.size(); }
    ValueProfile* valueProfile(int index) { return &m_valueProfiles[index]; }
    ValueProfile* valueProfileForBytecodeOffset(int bytecodeOffset);
    SpeculatedType valueProfilePredictionForBytecodeOffset(const ConcurrentJSLocker& locker, int bytecodeOffset)
    {
        if (ValueProfile* valueProfile = valueProfileForBytecodeOffset(bytecodeOffset))
            return valueProfile->computeUpdatedPrediction(locker);
        return SpecNone;
    }

    unsigned totalNumberOfValueProfiles()
    {
        return numberOfArgumentValueProfiles() + numberOfValueProfiles();
    }
    ValueProfile* getFromAllValueProfiles(unsigned index)
    {
        if (index < numberOfArgumentValueProfiles())
            return valueProfileForArgument(index);
        return valueProfile(index - numberOfArgumentValueProfiles());
    }

    RareCaseProfile* addRareCaseProfile(int bytecodeOffset);
    unsigned numberOfRareCaseProfiles() { return m_rareCaseProfiles.size(); }
    RareCaseProfile* rareCaseProfileForBytecodeOffset(int bytecodeOffset);
    unsigned rareCaseProfileCountForBytecodeOffset(int bytecodeOffset);

    bool likelyToTakeSlowCase(int bytecodeOffset)
    {
        if (!hasBaselineJITProfiling())
            return false;
        unsigned value = rareCaseProfileCountForBytecodeOffset(bytecodeOffset);
        return value >= Options::likelyToTakeSlowCaseMinimumCount();
    }

    bool couldTakeSlowCase(int bytecodeOffset)
    {
        if (!hasBaselineJITProfiling())
            return false;
        unsigned value = rareCaseProfileCountForBytecodeOffset(bytecodeOffset);
        return value >= Options::couldTakeSlowCaseMinimumCount();
    }

    ArithProfile* arithProfileForBytecodeOffset(int bytecodeOffset);
    ArithProfile* arithProfileForPC(Instruction*);

    bool couldTakeSpecialFastCase(int bytecodeOffset);

    unsigned numberOfArrayProfiles() const { return m_arrayProfiles.size(); }
    const ArrayProfileVector& arrayProfiles() { return m_arrayProfiles; }
    ArrayProfile* addArrayProfile(const ConcurrentJSLocker&, unsigned bytecodeOffset);
    ArrayProfile* addArrayProfile(unsigned bytecodeOffset);
    ArrayProfile* getArrayProfile(const ConcurrentJSLocker&, unsigned bytecodeOffset);
    ArrayProfile* getArrayProfile(unsigned bytecodeOffset);
    ArrayProfile* getOrAddArrayProfile(const ConcurrentJSLocker&, unsigned bytecodeOffset);
    ArrayProfile* getOrAddArrayProfile(unsigned bytecodeOffset);

    // Exception handling support

    size_t numberOfExceptionHandlers() const { return m_rareData ? m_rareData->m_exceptionHandlers.size() : 0; }
    HandlerInfo& exceptionHandler(int index) { RELEASE_ASSERT(m_rareData); return m_rareData->m_exceptionHandlers[index]; }

    bool hasExpressionInfo() { return m_unlinkedCode->hasExpressionInfo(); }

#if ENABLE(DFG_JIT)
    Vector<CodeOrigin, 0, UnsafeVectorOverflow>& codeOrigins();
    
    // Having code origins implies that there has been some inlining.
    bool hasCodeOrigins()
    {
        return JITCode::isOptimizingJIT(jitType());
    }
        
    bool canGetCodeOrigin(CallSiteIndex index)
    {
        if (!hasCodeOrigins())
            return false;
        return index.bits() < codeOrigins().size();
    }

    CodeOrigin codeOrigin(CallSiteIndex index)
    {
        return codeOrigins()[index.bits()];
    }

    bool addFrequentExitSite(const DFG::FrequentExitSite& site)
    {
        ASSERT(JITCode::isBaselineCode(jitType()));
        ConcurrentJSLocker locker(m_lock);
        return m_exitProfile.add(locker, this, site);
    }

    bool hasExitSite(const ConcurrentJSLocker& locker, const DFG::FrequentExitSite& site) const
    {
        return m_exitProfile.hasExitSite(locker, site);
    }
    bool hasExitSite(const DFG::FrequentExitSite& site) const
    {
        ConcurrentJSLocker locker(m_lock);
        return hasExitSite(locker, site);
    }

    DFG::ExitProfile& exitProfile() { return m_exitProfile; }

    CompressedLazyOperandValueProfileHolder& lazyOperandValueProfiles()
    {
        return m_lazyOperandValueProfiles;
    }
#endif // ENABLE(DFG_JIT)

    // Constant Pool
#if ENABLE(DFG_JIT)
    size_t numberOfIdentifiers() const { return m_unlinkedCode->numberOfIdentifiers() + numberOfDFGIdentifiers(); }
    size_t numberOfDFGIdentifiers() const;
    const Identifier& identifier(int index) const;
#else
    size_t numberOfIdentifiers() const { return m_unlinkedCode->numberOfIdentifiers(); }
    const Identifier& identifier(int index) const { return m_unlinkedCode->identifier(index); }
#endif

    Vector<WriteBarrier<Unknown>>& constants() { return m_constantRegisters; }
    Vector<SourceCodeRepresentation>& constantsSourceCodeRepresentation() { return m_constantsSourceCodeRepresentation; }
    unsigned addConstant(JSValue v)
    {
        unsigned result = m_constantRegisters.size();
        m_constantRegisters.append(WriteBarrier<Unknown>());
        m_constantRegisters.last().set(m_globalObject->vm(), this, v);
        m_constantsSourceCodeRepresentation.append(SourceCodeRepresentation::Other);
        return result;
    }

    unsigned addConstantLazily()
    {
        unsigned result = m_constantRegisters.size();
        m_constantRegisters.append(WriteBarrier<Unknown>());
        m_constantsSourceCodeRepresentation.append(SourceCodeRepresentation::Other);
        return result;
    }

    const Vector<WriteBarrier<Unknown>>& constantRegisters() { return m_constantRegisters; }
    WriteBarrier<Unknown>& constantRegister(int index) { return m_constantRegisters[index - FirstConstantRegisterIndex]; }
    static ALWAYS_INLINE bool isConstantRegisterIndex(int index) { return index >= FirstConstantRegisterIndex; }
    ALWAYS_INLINE JSValue getConstant(int index) const { return m_constantRegisters[index - FirstConstantRegisterIndex].get(); }
    ALWAYS_INLINE SourceCodeRepresentation constantSourceCodeRepresentation(int index) const { return m_constantsSourceCodeRepresentation[index - FirstConstantRegisterIndex]; }

    FunctionExecutable* functionDecl(int index) { return m_functionDecls[index].get(); }
    int numberOfFunctionDecls() { return m_functionDecls.size(); }
    FunctionExecutable* functionExpr(int index) { return m_functionExprs[index].get(); }
    
    RegExp* regexp(int index) const { return m_unlinkedCode->regexp(index); }
    unsigned numberOfRegExps() const { return m_unlinkedCode->numberOfRegExps(); }

    const Vector<BitVector>& bitVectors() const { return m_unlinkedCode->bitVectors(); }
    const BitVector& bitVector(size_t i) { return m_unlinkedCode->bitVector(i); }

    unsigned numberOfConstantBuffers() const
    {
        if (!m_rareData)
            return 0;
        return m_rareData->m_constantBuffers.size();
    }
    unsigned addConstantBuffer(const Vector<JSValue>& buffer)
    {
        createRareDataIfNecessary();
        unsigned size = m_rareData->m_constantBuffers.size();
        m_rareData->m_constantBuffers.append(buffer);
        return size;
    }

    Vector<JSValue>& constantBufferAsVector(unsigned index)
    {
        ASSERT(m_rareData);
        return m_rareData->m_constantBuffers[index];
    }
    JSValue* constantBuffer(unsigned index)
    {
        return constantBufferAsVector(index).data();
    }

    Heap* heap() const { return &m_vm->heap; }
    JSGlobalObject* globalObject() { return m_globalObject.get(); }

    JSGlobalObject* globalObjectFor(CodeOrigin);

    BytecodeLivenessAnalysis& livenessAnalysis()
    {
        {
            ConcurrentJSLocker locker(m_lock);
            if (!!m_livenessAnalysis)
                return *m_livenessAnalysis;
        }
        return livenessAnalysisSlow();
    }
    
    void validate();

    // Jump Tables

    size_t numberOfSwitchJumpTables() const { return m_rareData ? m_rareData->m_switchJumpTables.size() : 0; }
    SimpleJumpTable& addSwitchJumpTable() { createRareDataIfNecessary(); m_rareData->m_switchJumpTables.append(SimpleJumpTable()); return m_rareData->m_switchJumpTables.last(); }
    SimpleJumpTable& switchJumpTable(int tableIndex) { RELEASE_ASSERT(m_rareData); return m_rareData->m_switchJumpTables[tableIndex]; }
    void clearSwitchJumpTables()
    {
        if (!m_rareData)
            return;
        m_rareData->m_switchJumpTables.clear();
    }

    size_t numberOfStringSwitchJumpTables() const { return m_rareData ? m_rareData->m_stringSwitchJumpTables.size() : 0; }
    StringJumpTable& addStringSwitchJumpTable() { createRareDataIfNecessary(); m_rareData->m_stringSwitchJumpTables.append(StringJumpTable()); return m_rareData->m_stringSwitchJumpTables.last(); }
    StringJumpTable& stringSwitchJumpTable(int tableIndex) { RELEASE_ASSERT(m_rareData); return m_rareData->m_stringSwitchJumpTables[tableIndex]; }

    DirectEvalCodeCache& directEvalCodeCache() { createRareDataIfNecessary(); return m_rareData->m_directEvalCodeCache; }

    enum ShrinkMode {
        // Shrink prior to generating machine code that may point directly into vectors.
        EarlyShrink,

        // Shrink after generating machine code, and after possibly creating new vectors
        // and appending to others. At this time it is not safe to shrink certain vectors
        // because we would have generated machine code that references them directly.
        LateShrink
    };
    void shrinkToFit(ShrinkMode);

    // Functions for controlling when JITting kicks in, in a mixed mode
    // execution world.

    bool checkIfJITThresholdReached()
    {
        return m_llintExecuteCounter.checkIfThresholdCrossedAndSet(this);
    }

    void dontJITAnytimeSoon()
    {
        m_llintExecuteCounter.deferIndefinitely();
    }

    int32_t thresholdForJIT(int32_t threshold);
    void jitAfterWarmUp();
    void jitSoon();

    const BaselineExecutionCounter& llintExecuteCounter() const
    {
        return m_llintExecuteCounter;
    }

    typedef HashMap<Structure*, Bag<LLIntPrototypeLoadAdaptiveStructureWatchpoint>> StructureWatchpointMap;
    StructureWatchpointMap& llintGetByIdWatchpointMap() { return m_llintGetByIdWatchpointMap; }

    // Functions for controlling when tiered compilation kicks in. This
    // controls both when the optimizing compiler is invoked and when OSR
    // entry happens. Two triggers exist: the loop trigger and the return
    // trigger. In either case, when an addition to m_jitExecuteCounter
    // causes it to become non-negative, the optimizing compiler is
    // invoked. This includes a fast check to see if this CodeBlock has
    // already been optimized (i.e. replacement() returns a CodeBlock
    // that was optimized with a higher tier JIT than this one). In the
    // case of the loop trigger, if the optimized compilation succeeds
    // (or has already succeeded in the past) then OSR is attempted to
    // redirect program flow into the optimized code.

    // These functions are called from within the optimization triggers,
    // and are used as a single point at which we define the heuristics
    // for how much warm-up is mandated before the next optimization
    // trigger files. All CodeBlocks start out with optimizeAfterWarmUp(),
    // as this is called from the CodeBlock constructor.

    // When we observe a lot of speculation failures, we trigger a
    // reoptimization. But each time, we increase the optimization trigger
    // to avoid thrashing.
    JS_EXPORT_PRIVATE unsigned reoptimizationRetryCounter() const;
    void countReoptimization();
#if ENABLE(JIT)
    static unsigned numberOfLLIntBaselineCalleeSaveRegisters() { return RegisterSet::llintBaselineCalleeSaveRegisters().numberOfSetRegisters(); }
    static size_t llintBaselineCalleeSaveSpaceAsVirtualRegisters();
    size_t calleeSaveSpaceAsVirtualRegisters();

    unsigned numberOfDFGCompiles();

    int32_t codeTypeThresholdMultiplier() const;

    int32_t adjustedCounterValue(int32_t desiredThreshold);

    int32_t* addressOfJITExecuteCounter()
    {
        return &m_jitExecuteCounter.m_counter;
    }

    static ptrdiff_t offsetOfJITExecuteCounter() { return OBJECT_OFFSETOF(CodeBlock, m_jitExecuteCounter) + OBJECT_OFFSETOF(BaselineExecutionCounter, m_counter); }
    static ptrdiff_t offsetOfJITExecutionActiveThreshold() { return OBJECT_OFFSETOF(CodeBlock, m_jitExecuteCounter) + OBJECT_OFFSETOF(BaselineExecutionCounter, m_activeThreshold); }
    static ptrdiff_t offsetOfJITExecutionTotalCount() { return OBJECT_OFFSETOF(CodeBlock, m_jitExecuteCounter) + OBJECT_OFFSETOF(BaselineExecutionCounter, m_totalCount); }

    const BaselineExecutionCounter& jitExecuteCounter() const { return m_jitExecuteCounter; }

    unsigned optimizationDelayCounter() const { return m_optimizationDelayCounter; }

    // Check if the optimization threshold has been reached, and if not,
    // adjust the heuristics accordingly. Returns true if the threshold has
    // been reached.
    bool checkIfOptimizationThresholdReached();

    // Call this to force the next optimization trigger to fire. This is
    // rarely wise, since optimization triggers are typically more
    // expensive than executing baseline code.
    void optimizeNextInvocation();

    // Call this to prevent optimization from happening again. Note that
    // optimization will still happen after roughly 2^29 invocations,
    // so this is really meant to delay that as much as possible. This
    // is called if optimization failed, and we expect it to fail in
    // the future as well.
    void dontOptimizeAnytimeSoon();

    // Call this to reinitialize the counter to its starting state,
    // forcing a warm-up to happen before the next optimization trigger
    // fires. This is called in the CodeBlock constructor. It also
    // makes sense to call this if an OSR exit occurred. Note that
    // OSR exit code is code generated, so the value of the execute
    // counter that this corresponds to is also available directly.
    void optimizeAfterWarmUp();

    // Call this to force an optimization trigger to fire only after
    // a lot of warm-up.
    void optimizeAfterLongWarmUp();

    // Call this to cause an optimization trigger to fire soon, but
    // not necessarily the next one. This makes sense if optimization
    // succeeds. Successfuly optimization means that all calls are
    // relinked to the optimized code, so this only affects call
    // frames that are still executing this CodeBlock. The value here
    // is tuned to strike a balance between the cost of OSR entry
    // (which is too high to warrant making every loop back edge to
    // trigger OSR immediately) and the cost of executing baseline
    // code (which is high enough that we don't necessarily want to
    // have a full warm-up). The intuition for calling this instead of
    // optimizeNextInvocation() is for the case of recursive functions
    // with loops. Consider that there may be N call frames of some
    // recursive function, for a reasonably large value of N. The top
    // one triggers optimization, and then returns, and then all of
    // the others return. We don't want optimization to be triggered on
    // each return, as that would be superfluous. It only makes sense
    // to trigger optimization if one of those functions becomes hot
    // in the baseline code.
    void optimizeSoon();

    void forceOptimizationSlowPathConcurrently();

    void setOptimizationThresholdBasedOnCompilationResult(CompilationResult);
    
    uint32_t osrExitCounter() const { return m_osrExitCounter; }

    void countOSRExit() { m_osrExitCounter++; }

    uint32_t* addressOfOSRExitCounter() { return &m_osrExitCounter; }

    static ptrdiff_t offsetOfOSRExitCounter() { return OBJECT_OFFSETOF(CodeBlock, m_osrExitCounter); }

    uint32_t adjustedExitCountThreshold(uint32_t desiredThreshold);
    uint32_t exitCountThresholdForReoptimization();
    uint32_t exitCountThresholdForReoptimizationFromLoop();
    bool shouldReoptimizeNow();
    bool shouldReoptimizeFromLoopNow();

    void setCalleeSaveRegisters(RegisterSet);
    void setCalleeSaveRegisters(std::unique_ptr<RegisterAtOffsetList>);
    
    RegisterAtOffsetList* calleeSaveRegisters() const { return m_calleeSaveRegisters.get(); }
#else // No JIT
    static unsigned numberOfLLIntBaselineCalleeSaveRegisters() { return 0; }
    static size_t llintBaselineCalleeSaveSpaceAsVirtualRegisters() { return 0; };
    void optimizeAfterWarmUp() { }
    unsigned numberOfDFGCompiles() { return 0; }
#endif

    bool shouldOptimizeNow();
    void updateAllValueProfilePredictions();
    void updateAllArrayPredictions();
    void updateAllPredictions();

    unsigned frameRegisterCount();
    int stackPointerOffset();

    bool hasOpDebugForLineAndColumn(unsigned line, unsigned column);

    bool hasDebuggerRequests() const { return m_debuggerRequests; }
    void* debuggerRequestsAddress() { return &m_debuggerRequests; }

    void addBreakpoint(unsigned numBreakpoints);
    void removeBreakpoint(unsigned numBreakpoints)
    {
        ASSERT(m_numBreakpoints >= numBreakpoints);
        m_numBreakpoints -= numBreakpoints;
    }

    enum SteppingMode {
        SteppingModeDisabled,
        SteppingModeEnabled
    };
    void setSteppingMode(SteppingMode);

    void clearDebuggerRequests()
    {
        m_steppingMode = SteppingModeDisabled;
        m_numBreakpoints = 0;
    }

    bool wasCompiledWithDebuggingOpcodes() const { return m_unlinkedCode->wasCompiledWithDebuggingOpcodes(); }
    
    // FIXME: Make these remaining members private.

    int m_numCalleeLocals;
    int m_numVars;
    
    // This is intentionally public; it's the responsibility of anyone doing any
    // of the following to hold the lock:
    //
    // - Modifying any inline cache in this code block.
    //
    // - Quering any inline cache in this code block, from a thread other than
    //   the main thread.
    //
    // Additionally, it's only legal to modify the inline cache on the main
    // thread. This means that the main thread can query the inline cache without
    // locking. This is crucial since executing the inline cache is effectively
    // "querying" it.
    //
    // Another exception to the rules is that the GC can do whatever it wants
    // without holding any locks, because the GC is guaranteed to wait until any
    // concurrent compilation threads finish what they're doing.
    mutable ConcurrentJSLock m_lock;

    bool m_visitWeaklyHasBeenCalled;

    bool m_shouldAlwaysBeInlined; // Not a bitfield because the JIT wants to store to it.

#if ENABLE(JIT)
    unsigned m_capabilityLevelState : 2; // DFG::CapabilityLevel
#endif

    bool m_allTransitionsHaveBeenMarked : 1; // Initialized and used on every GC.

    bool m_didFailJITCompilation : 1;
    bool m_didFailFTLCompilation : 1;
    bool m_hasBeenCompiledWithFTL : 1;
    bool m_isConstructor : 1;
    bool m_isStrictMode : 1;
    unsigned m_codeType : 2; // CodeType

    // Internal methods for use by validation code. It would be private if it wasn't
    // for the fact that we use it from anonymous namespaces.
    void beginValidationDidFail();
    NO_RETURN_DUE_TO_CRASH void endValidationDidFail();

    struct RareData {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        Vector<HandlerInfo> m_exceptionHandlers;

        // Buffers used for large array literals
        Vector<Vector<JSValue>> m_constantBuffers;

        // Jump Tables
        Vector<SimpleJumpTable> m_switchJumpTables;
        Vector<StringJumpTable> m_stringSwitchJumpTables;

        DirectEvalCodeCache m_directEvalCodeCache;
    };

    void clearExceptionHandlers()
    {
        if (m_rareData)
            m_rareData->m_exceptionHandlers.clear();
    }

    void appendExceptionHandler(const HandlerInfo& handler)
    {
        createRareDataIfNecessary(); // We may be handling the exception of an inlined call frame.
        m_rareData->m_exceptionHandlers.append(handler);
    }

    CallSiteIndex newExceptionHandlingCallSiteIndex(CallSiteIndex originalCallSite);

#if ENABLE(JIT)
    void setPCToCodeOriginMap(std::unique_ptr<PCToCodeOriginMap>&&);
    std::optional<CodeOrigin> findPC(void* pc);
#endif

protected:
    void finalizeLLIntInlineCaches();
    void finalizeBaselineJITInlineCaches();

#if ENABLE(DFG_JIT)
    void tallyFrequentExitSites();
#else
    void tallyFrequentExitSites() { }
#endif

private:
    friend class CodeBlockSet;

    BytecodeLivenessAnalysis& livenessAnalysisSlow();
    
    CodeBlock* specialOSREntryBlockOrNull();
    
    void noticeIncomingCall(ExecState* callerFrame);
    
    double optimizationThresholdScalingFactor();

    void updateAllPredictionsAndCountLiveness(unsigned& numberOfLiveNonArgumentValueProfiles, unsigned& numberOfSamplesInProfiles);

    bool setConstantIdentifierSetRegisters(VM&, const Vector<ConstantIndentifierSetEntry>& constants);

    void setConstantRegisters(const Vector<WriteBarrier<Unknown>>& constants, const Vector<SourceCodeRepresentation>& constantsSourceCodeRepresentation);

    void replaceConstant(int index, JSValue value)
    {
        ASSERT(isConstantRegisterIndex(index) && static_cast<size_t>(index - FirstConstantRegisterIndex) < m_constantRegisters.size());
        m_constantRegisters[index - FirstConstantRegisterIndex].set(m_globalObject->vm(), this, value);
    }

    bool shouldVisitStrongly(const ConcurrentJSLocker&);
    bool shouldJettisonDueToWeakReference();
    bool shouldJettisonDueToOldAge(const ConcurrentJSLocker&);
    
    void propagateTransitions(const ConcurrentJSLocker&, SlotVisitor&);
    void determineLiveness(const ConcurrentJSLocker&, SlotVisitor&);
        
    void stronglyVisitStrongReferences(const ConcurrentJSLocker&, SlotVisitor&);
    void stronglyVisitWeakReferences(const ConcurrentJSLocker&, SlotVisitor&);
    void visitOSRExitTargets(const ConcurrentJSLocker&, SlotVisitor&);

    std::chrono::milliseconds timeSinceCreation()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_creationTime);
    }

    void createRareDataIfNecessary()
    {
        if (!m_rareData)
            m_rareData = std::make_unique<RareData>();
    }

    void insertBasicBlockBoundariesForControlFlowProfiler(RefCountedArray<Instruction>&);

    WriteBarrier<UnlinkedCodeBlock> m_unlinkedCode;
    int m_numParameters;
    union {
        unsigned m_debuggerRequests;
        struct {
            unsigned m_hasDebuggerStatement : 1;
            unsigned m_steppingMode : 1;
            unsigned m_numBreakpoints : 30;
        };
    };
    WriteBarrier<ExecutableBase> m_ownerExecutable;
    VM* m_vm;

    RefCountedArray<Instruction> m_instructions;
    VirtualRegister m_thisRegister;
    VirtualRegister m_scopeRegister;
    mutable CodeBlockHash m_hash;

    RefPtr<SourceProvider> m_source;
    unsigned m_sourceOffset;
    unsigned m_firstLineColumnOffset;

    RefCountedArray<LLIntCallLinkInfo> m_llintCallLinkInfos;
    SentinelLinkedList<LLIntCallLinkInfo, BasicRawSentinelNode<LLIntCallLinkInfo>> m_incomingLLIntCalls;
    StructureWatchpointMap m_llintGetByIdWatchpointMap;
    RefPtr<JITCode> m_jitCode;
#if ENABLE(JIT)
    std::unique_ptr<RegisterAtOffsetList> m_calleeSaveRegisters;
    Bag<StructureStubInfo> m_stubInfos;
    Bag<JITAddIC> m_addICs;
    Bag<JITMulIC> m_mulICs;
    Bag<JITNegIC> m_negICs;
    Bag<JITSubIC> m_subICs;
    Bag<ByValInfo> m_byValInfos;
    Bag<CallLinkInfo> m_callLinkInfos;
    SentinelLinkedList<CallLinkInfo, BasicRawSentinelNode<CallLinkInfo>> m_incomingCalls;
    SentinelLinkedList<PolymorphicCallNode, BasicRawSentinelNode<PolymorphicCallNode>> m_incomingPolymorphicCalls;
    std::unique_ptr<PCToCodeOriginMap> m_pcToCodeOriginMap;
#endif
    std::unique_ptr<CompactJITCodeMap> m_jitCodeMap;
#if ENABLE(DFG_JIT)
    // This is relevant to non-DFG code blocks that serve as the profiled code block
    // for DFG code blocks.
    DFG::ExitProfile m_exitProfile;
    CompressedLazyOperandValueProfileHolder m_lazyOperandValueProfiles;
#endif
    RefCountedArray<ValueProfile> m_argumentValueProfiles;
    RefCountedArray<ValueProfile> m_valueProfiles;
    SegmentedVector<RareCaseProfile, 8> m_rareCaseProfiles;
    RefCountedArray<ArrayAllocationProfile> m_arrayAllocationProfiles;
    ArrayProfileVector m_arrayProfiles;
    RefCountedArray<ObjectAllocationProfile> m_objectAllocationProfiles;

    // Constant Pool
    COMPILE_ASSERT(sizeof(Register) == sizeof(WriteBarrier<Unknown>), Register_must_be_same_size_as_WriteBarrier_Unknown);
    // TODO: This could just be a pointer to m_unlinkedCodeBlock's data, but the DFG mutates
    // it, so we're stuck with it for now.
    Vector<WriteBarrier<Unknown>> m_constantRegisters;
    Vector<SourceCodeRepresentation> m_constantsSourceCodeRepresentation;
    RefCountedArray<WriteBarrier<FunctionExecutable>> m_functionDecls;
    RefCountedArray<WriteBarrier<FunctionExecutable>> m_functionExprs;

    WriteBarrier<CodeBlock> m_alternative;
    
    BaselineExecutionCounter m_llintExecuteCounter;

    BaselineExecutionCounter m_jitExecuteCounter;
    uint32_t m_osrExitCounter;
    uint16_t m_optimizationDelayCounter;
    uint16_t m_reoptimizationRetryCounter;

    std::chrono::steady_clock::time_point m_creationTime;

    std::unique_ptr<BytecodeLivenessAnalysis> m_livenessAnalysis;

    std::unique_ptr<RareData> m_rareData;

    UnconditionalFinalizer m_unconditionalFinalizer;
    WeakReferenceHarvester m_weakReferenceHarvester;
};

inline Register& ExecState::r(int index)
{
    CodeBlock* codeBlock = this->codeBlock();
    if (codeBlock->isConstantRegisterIndex(index))
        return *reinterpret_cast<Register*>(&codeBlock->constantRegister(index));
    return this[index];
}

inline Register& ExecState::r(VirtualRegister reg)
{
    return r(reg.offset());
}

inline Register& ExecState::uncheckedR(int index)
{
    RELEASE_ASSERT(index < FirstConstantRegisterIndex);
    return this[index];
}

inline Register& ExecState::uncheckedR(VirtualRegister reg)
{
    return uncheckedR(reg.offset());
}

inline void CodeBlock::clearVisitWeaklyHasBeenCalled()
{
    m_visitWeaklyHasBeenCalled = false;
}

template <typename ExecutableType>
JSObject* ScriptExecutable::prepareForExecution(VM& vm, JSFunction* function, JSScope* scope, CodeSpecializationKind kind, CodeBlock*& resultCodeBlock)
{
    if (hasJITCodeFor(kind)) {
        if (std::is_same<ExecutableType, EvalExecutable>::value)
            resultCodeBlock = jsCast<CodeBlock*>(jsCast<EvalExecutable*>(this)->codeBlock());
        else if (std::is_same<ExecutableType, ProgramExecutable>::value)
            resultCodeBlock = jsCast<CodeBlock*>(jsCast<ProgramExecutable*>(this)->codeBlock());
        else if (std::is_same<ExecutableType, ModuleProgramExecutable>::value)
            resultCodeBlock = jsCast<CodeBlock*>(jsCast<ModuleProgramExecutable*>(this)->codeBlock());
        else if (std::is_same<ExecutableType, FunctionExecutable>::value)
            resultCodeBlock = jsCast<CodeBlock*>(jsCast<FunctionExecutable*>(this)->codeBlockFor(kind));
        else
            RELEASE_ASSERT_NOT_REACHED();
        return nullptr;
    }
    return prepareForExecutionImpl(vm, function, scope, kind, resultCodeBlock);
}

#define CODEBLOCK_LOG_EVENT(codeBlock, summary, details) \
    (codeBlock->vm()->logEvent(codeBlock, summary, [&] () { return toCString details; }))

} // namespace JSC
