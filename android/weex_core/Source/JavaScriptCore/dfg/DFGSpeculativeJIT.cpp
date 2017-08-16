/*
 * Copyright (C) 2011-2017 Apple Inc. All rights reserved.
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
#include "DFGSpeculativeJIT.h"

#if ENABLE(DFG_JIT)

#include "BinarySwitch.h"
#include "DFGAbstractInterpreterInlines.h"
#include "DFGArrayifySlowPathGenerator.h"
#include "DFGCallArrayAllocatorSlowPathGenerator.h"
#include "DFGCallCreateDirectArgumentsSlowPathGenerator.h"
#include "DFGCapabilities.h"
#include "DFGDOMJITPatchpointParams.h"
#include "DFGMayExit.h"
#include "DFGOSRExitFuzz.h"
#include "DFGSaneStringGetByValSlowPathGenerator.h"
#include "DFGSlowPathGenerator.h"
#include "DOMJITPatchpoint.h"
#include "DirectArguments.h"
#include "JITAddGenerator.h"
#include "JITBitAndGenerator.h"
#include "JITBitOrGenerator.h"
#include "JITBitXorGenerator.h"
#include "JITDivGenerator.h"
#include "JITLeftShiftGenerator.h"
#include "JITMulGenerator.h"
#include "JITRightShiftGenerator.h"
#include "JITSubGenerator.h"
#include "JSAsyncFunction.h"
#include "JSCInlines.h"
#include "JSEnvironmentRecord.h"
#include "JSFixedArray.h"
#include "JSGeneratorFunction.h"
#include "JSLexicalEnvironment.h"
#include "LinkBuffer.h"
#include "RegExpConstructor.h"
#include "ScopedArguments.h"
#include "ScratchRegisterAllocator.h"
#include <wtf/BitVector.h>
#include <wtf/Box.h>
#include <wtf/MathExtras.h>

namespace JSC { namespace DFG {

SpeculativeJIT::SpeculativeJIT(JITCompiler& jit)
    : m_compileOkay(true)
    , m_jit(jit)
    , m_currentNode(0)
    , m_lastGeneratedNode(LastNodeType)
    , m_indexInBlock(0)
    , m_generationInfo(m_jit.graph().frameRegisterCount())
    , m_state(m_jit.graph())
    , m_interpreter(m_jit.graph(), m_state)
    , m_stream(&jit.jitCode()->variableEventStream)
    , m_minifiedGraph(&jit.jitCode()->minifiedDFG)
{
}

SpeculativeJIT::~SpeculativeJIT()
{
}

void SpeculativeJIT::emitAllocateRawObject(GPRReg resultGPR, RegisteredStructure structure, GPRReg storageGPR, unsigned numElements, unsigned vectorLength)
{
    IndexingType indexingType = structure->indexingType();
    bool hasIndexingHeader = hasIndexedProperties(indexingType);

    unsigned inlineCapacity = structure->inlineCapacity();
    unsigned outOfLineCapacity = structure->outOfLineCapacity();
    
    GPRTemporary scratch(this);
    GPRTemporary scratch2(this);
    GPRReg scratchGPR = scratch.gpr();
    GPRReg scratch2GPR = scratch2.gpr();

    ASSERT(vectorLength >= numElements);
    vectorLength = Butterfly::optimalContiguousVectorLength(structure.get(), vectorLength);
    
    JITCompiler::JumpList slowCases;

    size_t size = 0;
    if (hasIndexingHeader)
        size += vectorLength * sizeof(JSValue) + sizeof(IndexingHeader);
    size += outOfLineCapacity * sizeof(JSValue);

    m_jit.move(TrustedImmPtr(0), storageGPR);
    
    if (size) {
        if (MarkedAllocator* allocator = m_jit.vm()->auxiliarySpace.allocatorFor(size)) {
            m_jit.move(TrustedImmPtr(allocator), scratchGPR);
            m_jit.emitAllocate(storageGPR, allocator, scratchGPR, scratch2GPR, slowCases);
            
            m_jit.addPtr(
                TrustedImm32(outOfLineCapacity * sizeof(JSValue) + sizeof(IndexingHeader)),
                storageGPR);
            
            if (hasIndexingHeader)
                m_jit.store32(TrustedImm32(vectorLength), MacroAssembler::Address(storageGPR, Butterfly::offsetOfVectorLength()));
        } else
            slowCases.append(m_jit.jump());
    }

    size_t allocationSize = JSFinalObject::allocationSize(inlineCapacity);
    MarkedAllocator* allocatorPtr = subspaceFor<JSFinalObject>(*m_jit.vm())->allocatorFor(allocationSize);
    if (allocatorPtr) {
        m_jit.move(TrustedImmPtr(allocatorPtr), scratchGPR);
        emitAllocateJSObject(resultGPR, allocatorPtr, scratchGPR, TrustedImmPtr(structure), storageGPR, scratch2GPR, slowCases);
        m_jit.emitInitializeInlineStorage(resultGPR, structure->inlineCapacity());
    } else
        slowCases.append(m_jit.jump());

    // I want a slow path that also loads out the storage pointer, and that's
    // what this custom CallArrayAllocatorSlowPathGenerator gives me. It's a lot
    // of work for a very small piece of functionality. :-/
    addSlowPathGenerator(std::make_unique<CallArrayAllocatorSlowPathGenerator>(
        slowCases, this, operationNewRawObject, resultGPR, storageGPR,
        structure, vectorLength));

    if (numElements < vectorLength) {
#if USE(JSVALUE64)
        if (hasDouble(structure->indexingType()))
            m_jit.move(TrustedImm64(bitwise_cast<int64_t>(PNaN)), scratchGPR);
        else
            m_jit.move(TrustedImm64(JSValue::encode(JSValue())), scratchGPR);
        for (unsigned i = numElements; i < vectorLength; ++i)
            m_jit.store64(scratchGPR, MacroAssembler::Address(storageGPR, sizeof(double) * i));
#else
        EncodedValueDescriptor value;
        if (hasDouble(structure->indexingType()))
            value.asInt64 = JSValue::encode(JSValue(JSValue::EncodeAsDouble, PNaN));
        else
            value.asInt64 = JSValue::encode(JSValue());
        for (unsigned i = numElements; i < vectorLength; ++i) {
            m_jit.store32(TrustedImm32(value.asBits.tag), MacroAssembler::Address(storageGPR, sizeof(double) * i + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
            m_jit.store32(TrustedImm32(value.asBits.payload), MacroAssembler::Address(storageGPR, sizeof(double) * i + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
        }
#endif
    }
    
    if (hasIndexingHeader)
        m_jit.store32(TrustedImm32(numElements), MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()));
    
    m_jit.emitInitializeOutOfLineStorage(storageGPR, structure->outOfLineCapacity());
    
    m_jit.mutatorFence();
}

void SpeculativeJIT::emitGetLength(InlineCallFrame* inlineCallFrame, GPRReg lengthGPR, bool includeThis)
{
    if (inlineCallFrame && !inlineCallFrame->isVarargs())
        m_jit.move(TrustedImm32(inlineCallFrame->arguments.size() - !includeThis), lengthGPR);
    else {
        VirtualRegister argumentCountRegister = m_jit.argumentCount(inlineCallFrame);
        m_jit.load32(JITCompiler::payloadFor(argumentCountRegister), lengthGPR);
        if (!includeThis)
            m_jit.sub32(TrustedImm32(1), lengthGPR);
    }
}

void SpeculativeJIT::emitGetLength(CodeOrigin origin, GPRReg lengthGPR, bool includeThis)
{
    emitGetLength(origin.inlineCallFrame, lengthGPR, includeThis);
}

void SpeculativeJIT::emitGetCallee(CodeOrigin origin, GPRReg calleeGPR)
{
    if (origin.inlineCallFrame) {
        if (origin.inlineCallFrame->isClosureCall) {
            m_jit.loadPtr(
                JITCompiler::addressFor(origin.inlineCallFrame->calleeRecovery.virtualRegister()),
                calleeGPR);
        } else {
            m_jit.move(
                TrustedImmPtr::weakPointer(m_jit.graph(), origin.inlineCallFrame->calleeRecovery.constant().asCell()),
                calleeGPR);
        }
    } else
        m_jit.loadPtr(JITCompiler::addressFor(CallFrameSlot::callee), calleeGPR);
}

void SpeculativeJIT::emitGetArgumentStart(CodeOrigin origin, GPRReg startGPR)
{
    m_jit.addPtr(
        TrustedImm32(
            JITCompiler::argumentsStart(origin).offset() * static_cast<int>(sizeof(Register))),
        GPRInfo::callFrameRegister, startGPR);
}

MacroAssembler::Jump SpeculativeJIT::emitOSRExitFuzzCheck()
{
    if (!Options::useOSRExitFuzz()
        || !canUseOSRExitFuzzing(m_jit.graph().baselineCodeBlockFor(m_origin.semantic))
        || !doOSRExitFuzzing())
        return MacroAssembler::Jump();
    
    MacroAssembler::Jump result;
    
    m_jit.pushToSave(GPRInfo::regT0);
    m_jit.load32(&g_numberOfOSRExitFuzzChecks, GPRInfo::regT0);
    m_jit.add32(TrustedImm32(1), GPRInfo::regT0);
    m_jit.store32(GPRInfo::regT0, &g_numberOfOSRExitFuzzChecks);
    unsigned atOrAfter = Options::fireOSRExitFuzzAtOrAfter();
    unsigned at = Options::fireOSRExitFuzzAt();
    if (at || atOrAfter) {
        unsigned threshold;
        MacroAssembler::RelationalCondition condition;
        if (atOrAfter) {
            threshold = atOrAfter;
            condition = MacroAssembler::Below;
        } else {
            threshold = at;
            condition = MacroAssembler::NotEqual;
        }
        MacroAssembler::Jump ok = m_jit.branch32(
            condition, GPRInfo::regT0, MacroAssembler::TrustedImm32(threshold));
        m_jit.popToRestore(GPRInfo::regT0);
        result = m_jit.jump();
        ok.link(&m_jit);
    }
    m_jit.popToRestore(GPRInfo::regT0);
    
    return result;
}

void SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Node* node, MacroAssembler::Jump jumpToFail)
{
    if (!m_compileOkay)
        return;
    JITCompiler::Jump fuzzJump = emitOSRExitFuzzCheck();
    if (fuzzJump.isSet()) {
        JITCompiler::JumpList jumpsToFail;
        jumpsToFail.append(fuzzJump);
        jumpsToFail.append(jumpToFail);
        m_jit.appendExitInfo(jumpsToFail);
    } else
        m_jit.appendExitInfo(jumpToFail);
    m_jit.jitCode()->appendOSRExit(OSRExit(kind, jsValueSource, m_jit.graph().methodOfGettingAValueProfileFor(m_currentNode, node), this, m_stream->size()));
}

void SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Node* node, const MacroAssembler::JumpList& jumpsToFail)
{
    if (!m_compileOkay)
        return;
    JITCompiler::Jump fuzzJump = emitOSRExitFuzzCheck();
    if (fuzzJump.isSet()) {
        JITCompiler::JumpList myJumpsToFail;
        myJumpsToFail.append(jumpsToFail);
        myJumpsToFail.append(fuzzJump);
        m_jit.appendExitInfo(myJumpsToFail);
    } else
        m_jit.appendExitInfo(jumpsToFail);
    m_jit.jitCode()->appendOSRExit(OSRExit(kind, jsValueSource, m_jit.graph().methodOfGettingAValueProfileFor(m_currentNode, node), this, m_stream->size()));
}

OSRExitJumpPlaceholder SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Node* node)
{
    if (!m_compileOkay)
        return OSRExitJumpPlaceholder();
    unsigned index = m_jit.jitCode()->osrExit.size();
    m_jit.appendExitInfo();
    m_jit.jitCode()->appendOSRExit(OSRExit(kind, jsValueSource, m_jit.graph().methodOfGettingAValueProfileFor(m_currentNode, node), this, m_stream->size()));
    return OSRExitJumpPlaceholder(index);
}

OSRExitJumpPlaceholder SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Edge nodeUse)
{
    return speculationCheck(kind, jsValueSource, nodeUse.node());
}

void SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Edge nodeUse, MacroAssembler::Jump jumpToFail)
{
    speculationCheck(kind, jsValueSource, nodeUse.node(), jumpToFail);
}

void SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Edge nodeUse, const MacroAssembler::JumpList& jumpsToFail)
{
    speculationCheck(kind, jsValueSource, nodeUse.node(), jumpsToFail);
}

void SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Node* node, MacroAssembler::Jump jumpToFail, const SpeculationRecovery& recovery)
{
    if (!m_compileOkay)
        return;
    unsigned recoveryIndex = m_jit.jitCode()->appendSpeculationRecovery(recovery);
    m_jit.appendExitInfo(jumpToFail);
    m_jit.jitCode()->appendOSRExit(OSRExit(kind, jsValueSource, m_jit.graph().methodOfGettingAValueProfileFor(m_currentNode, node), this, m_stream->size(), recoveryIndex));
}

void SpeculativeJIT::speculationCheck(ExitKind kind, JSValueSource jsValueSource, Edge nodeUse, MacroAssembler::Jump jumpToFail, const SpeculationRecovery& recovery)
{
    speculationCheck(kind, jsValueSource, nodeUse.node(), jumpToFail, recovery);
}

void SpeculativeJIT::emitInvalidationPoint(Node* node)
{
    if (!m_compileOkay)
        return;
    OSRExitCompilationInfo& info = m_jit.appendExitInfo(JITCompiler::JumpList());
    m_jit.jitCode()->appendOSRExit(OSRExit(
        UncountableInvalidation, JSValueSource(), MethodOfGettingAValueProfile(),
        this, m_stream->size()));
    info.m_replacementSource = m_jit.watchpointLabel();
    ASSERT(info.m_replacementSource.isSet());
    noResult(node);
}

void SpeculativeJIT::unreachable(Node* node)
{
    m_compileOkay = false;
    m_jit.abortWithReason(DFGUnreachableNode, node->op());
}

void SpeculativeJIT::terminateSpeculativeExecution(ExitKind kind, JSValueRegs jsValueRegs, Node* node)
{
    if (!m_compileOkay)
        return;
    speculationCheck(kind, jsValueRegs, node, m_jit.jump());
    m_compileOkay = false;
    if (verboseCompilationEnabled())
        dataLog("Bailing compilation.\n");
}

void SpeculativeJIT::terminateSpeculativeExecution(ExitKind kind, JSValueRegs jsValueRegs, Edge nodeUse)
{
    terminateSpeculativeExecution(kind, jsValueRegs, nodeUse.node());
}

void SpeculativeJIT::typeCheck(JSValueSource source, Edge edge, SpeculatedType typesPassedThrough, MacroAssembler::Jump jumpToFail, ExitKind exitKind)
{
    ASSERT(needsTypeCheck(edge, typesPassedThrough));
    m_interpreter.filter(edge, typesPassedThrough);
    speculationCheck(exitKind, source, edge.node(), jumpToFail);
}

RegisterSet SpeculativeJIT::usedRegisters()
{
    RegisterSet result;
    
    for (unsigned i = GPRInfo::numberOfRegisters; i--;) {
        GPRReg gpr = GPRInfo::toRegister(i);
        if (m_gprs.isInUse(gpr))
            result.set(gpr);
    }
    for (unsigned i = FPRInfo::numberOfRegisters; i--;) {
        FPRReg fpr = FPRInfo::toRegister(i);
        if (m_fprs.isInUse(fpr))
            result.set(fpr);
    }
    
    result.merge(RegisterSet::stubUnavailableRegisters());
    
    return result;
}

void SpeculativeJIT::addSlowPathGenerator(std::unique_ptr<SlowPathGenerator> slowPathGenerator)
{
    m_slowPathGenerators.append(WTFMove(slowPathGenerator));
}

void SpeculativeJIT::addSlowPathGenerator(std::function<void()> lambda)
{
    m_slowPathLambdas.append(SlowPathLambda{ lambda, m_currentNode, static_cast<unsigned>(m_stream->size()) });
}

void SpeculativeJIT::runSlowPathGenerators(PCToCodeOriginMapBuilder& pcToCodeOriginMapBuilder)
{
    for (auto& slowPathGenerator : m_slowPathGenerators) {
        pcToCodeOriginMapBuilder.appendItem(m_jit.labelIgnoringWatchpoints(), slowPathGenerator->origin().semantic);
        slowPathGenerator->generate(this);
    }
    for (auto& slowPathLambda : m_slowPathLambdas) {
        Node* currentNode = slowPathLambda.currentNode;
        m_currentNode = currentNode;
        m_outOfLineStreamIndex = slowPathLambda.streamIndex;
        pcToCodeOriginMapBuilder.appendItem(m_jit.labelIgnoringWatchpoints(), currentNode->origin.semantic);
        slowPathLambda.generator();
        m_outOfLineStreamIndex = std::nullopt;
    }
}

void SpeculativeJIT::clearGenerationInfo()
{
    for (unsigned i = 0; i < m_generationInfo.size(); ++i)
        m_generationInfo[i] = GenerationInfo();
    m_gprs = RegisterBank<GPRInfo>();
    m_fprs = RegisterBank<FPRInfo>();
}

SilentRegisterSavePlan SpeculativeJIT::silentSavePlanForGPR(VirtualRegister spillMe, GPRReg source)
{
    GenerationInfo& info = generationInfoFromVirtualRegister(spillMe);
    Node* node = info.node();
    DataFormat registerFormat = info.registerFormat();
    ASSERT(registerFormat != DataFormatNone);
    ASSERT(registerFormat != DataFormatDouble);
        
    SilentSpillAction spillAction;
    SilentFillAction fillAction;
        
    if (!info.needsSpill())
        spillAction = DoNothingForSpill;
    else {
#if USE(JSVALUE64)
        ASSERT(info.gpr() == source);
        if (registerFormat == DataFormatInt32)
            spillAction = Store32Payload;
        else if (registerFormat == DataFormatCell || registerFormat == DataFormatStorage)
            spillAction = StorePtr;
        else if (registerFormat == DataFormatInt52 || registerFormat == DataFormatStrictInt52)
            spillAction = Store64;
        else {
            ASSERT(registerFormat & DataFormatJS);
            spillAction = Store64;
        }
#elif USE(JSVALUE32_64)
        if (registerFormat & DataFormatJS) {
            ASSERT(info.tagGPR() == source || info.payloadGPR() == source);
            spillAction = source == info.tagGPR() ? Store32Tag : Store32Payload;
        } else {
            ASSERT(info.gpr() == source);
            spillAction = Store32Payload;
        }
#endif
    }
        
    if (registerFormat == DataFormatInt32) {
        ASSERT(info.gpr() == source);
        ASSERT(isJSInt32(info.registerFormat()));
        if (node->hasConstant()) {
            ASSERT(node->isInt32Constant());
            fillAction = SetInt32Constant;
        } else
            fillAction = Load32Payload;
    } else if (registerFormat == DataFormatBoolean) {
#if USE(JSVALUE64)
        RELEASE_ASSERT_NOT_REACHED();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
        fillAction = DoNothingForFill;
#endif
#elif USE(JSVALUE32_64)
        ASSERT(info.gpr() == source);
        if (node->hasConstant()) {
            ASSERT(node->isBooleanConstant());
            fillAction = SetBooleanConstant;
        } else
            fillAction = Load32Payload;
#endif
    } else if (registerFormat == DataFormatCell) {
        ASSERT(info.gpr() == source);
        if (node->hasConstant()) {
            DFG_ASSERT(m_jit.graph(), m_currentNode, node->isCellConstant());
            node->asCell(); // To get the assertion.
            fillAction = SetCellConstant;
        } else {
#if USE(JSVALUE64)
            fillAction = LoadPtr;
#else
            fillAction = Load32Payload;
#endif
        }
    } else if (registerFormat == DataFormatStorage) {
        ASSERT(info.gpr() == source);
        fillAction = LoadPtr;
    } else if (registerFormat == DataFormatInt52) {
        if (node->hasConstant())
            fillAction = SetInt52Constant;
        else if (info.spillFormat() == DataFormatInt52)
            fillAction = Load64;
        else if (info.spillFormat() == DataFormatStrictInt52)
            fillAction = Load64ShiftInt52Left;
        else if (info.spillFormat() == DataFormatNone)
            fillAction = Load64;
        else {
            RELEASE_ASSERT_NOT_REACHED();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
            fillAction = Load64; // Make GCC happy.
#endif
        }
    } else if (registerFormat == DataFormatStrictInt52) {
        if (node->hasConstant())
            fillAction = SetStrictInt52Constant;
        else if (info.spillFormat() == DataFormatInt52)
            fillAction = Load64ShiftInt52Right;
        else if (info.spillFormat() == DataFormatStrictInt52)
            fillAction = Load64;
        else if (info.spillFormat() == DataFormatNone)
            fillAction = Load64;
        else {
            RELEASE_ASSERT_NOT_REACHED();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
            fillAction = Load64; // Make GCC happy.
#endif
        }
    } else {
        ASSERT(registerFormat & DataFormatJS);
#if USE(JSVALUE64)
        ASSERT(info.gpr() == source);
        if (node->hasConstant()) {
            if (node->isCellConstant())
                fillAction = SetTrustedJSConstant;
            else
                fillAction = SetJSConstant;
        } else if (info.spillFormat() == DataFormatInt32) {
            ASSERT(registerFormat == DataFormatJSInt32);
            fillAction = Load32PayloadBoxInt;
        } else
            fillAction = Load64;
#else
        ASSERT(info.tagGPR() == source || info.payloadGPR() == source);
        if (node->hasConstant())
            fillAction = info.tagGPR() == source ? SetJSConstantTag : SetJSConstantPayload;
        else if (info.payloadGPR() == source)
            fillAction = Load32Payload;
        else { // Fill the Tag
            switch (info.spillFormat()) {
            case DataFormatInt32:
                ASSERT(registerFormat == DataFormatJSInt32);
                fillAction = SetInt32Tag;
                break;
            case DataFormatCell:
                ASSERT(registerFormat == DataFormatJSCell);
                fillAction = SetCellTag;
                break;
            case DataFormatBoolean:
                ASSERT(registerFormat == DataFormatJSBoolean);
                fillAction = SetBooleanTag;
                break;
            default:
                fillAction = Load32Tag;
                break;
            }
        }
#endif
    }
        
    return SilentRegisterSavePlan(spillAction, fillAction, node, source);
}
    
SilentRegisterSavePlan SpeculativeJIT::silentSavePlanForFPR(VirtualRegister spillMe, FPRReg source)
{
    GenerationInfo& info = generationInfoFromVirtualRegister(spillMe);
    Node* node = info.node();
    ASSERT(info.registerFormat() == DataFormatDouble);

    SilentSpillAction spillAction;
    SilentFillAction fillAction;
        
    if (!info.needsSpill())
        spillAction = DoNothingForSpill;
    else {
        ASSERT(!node->hasConstant());
        ASSERT(info.spillFormat() == DataFormatNone);
        ASSERT(info.fpr() == source);
        spillAction = StoreDouble;
    }
        
#if USE(JSVALUE64)
    if (node->hasConstant()) {
        node->asNumber(); // To get the assertion.
        fillAction = SetDoubleConstant;
    } else {
        ASSERT(info.spillFormat() == DataFormatNone || info.spillFormat() == DataFormatDouble);
        fillAction = LoadDouble;
    }
#elif USE(JSVALUE32_64)
    ASSERT(info.registerFormat() == DataFormatDouble);
    if (node->hasConstant()) {
        node->asNumber(); // To get the assertion.
        fillAction = SetDoubleConstant;
    } else
        fillAction = LoadDouble;
#endif

    return SilentRegisterSavePlan(spillAction, fillAction, node, source);
}
    
void SpeculativeJIT::silentSpill(const SilentRegisterSavePlan& plan)
{
    switch (plan.spillAction()) {
    case DoNothingForSpill:
        break;
    case Store32Tag:
        m_jit.store32(plan.gpr(), JITCompiler::tagFor(plan.node()->virtualRegister()));
        break;
    case Store32Payload:
        m_jit.store32(plan.gpr(), JITCompiler::payloadFor(plan.node()->virtualRegister()));
        break;
    case StorePtr:
        m_jit.storePtr(plan.gpr(), JITCompiler::addressFor(plan.node()->virtualRegister()));
        break;
#if USE(JSVALUE64)
    case Store64:
        m_jit.store64(plan.gpr(), JITCompiler::addressFor(plan.node()->virtualRegister()));
        break;
#endif
    case StoreDouble:
        m_jit.storeDouble(plan.fpr(), JITCompiler::addressFor(plan.node()->virtualRegister()));
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
}
    
void SpeculativeJIT::silentFill(const SilentRegisterSavePlan& plan, GPRReg canTrample)
{
#if USE(JSVALUE32_64)
    UNUSED_PARAM(canTrample);
#endif
    switch (plan.fillAction()) {
    case DoNothingForFill:
        break;
    case SetInt32Constant:
        m_jit.move(Imm32(plan.node()->asInt32()), plan.gpr());
        break;
#if USE(JSVALUE64)
    case SetInt52Constant:
        m_jit.move(Imm64(plan.node()->asAnyInt() << JSValue::int52ShiftAmount), plan.gpr());
        break;
    case SetStrictInt52Constant:
        m_jit.move(Imm64(plan.node()->asAnyInt()), plan.gpr());
        break;
#endif // USE(JSVALUE64)
    case SetBooleanConstant:
        m_jit.move(TrustedImm32(plan.node()->asBoolean()), plan.gpr());
        break;
    case SetCellConstant:
        ASSERT(plan.node()->constant()->value().isCell());
        m_jit.move(TrustedImmPtr(plan.node()->constant()), plan.gpr());
        break;
#if USE(JSVALUE64)
    case SetTrustedJSConstant:
        m_jit.move(valueOfJSConstantAsImm64(plan.node()).asTrustedImm64(), plan.gpr());
        break;
    case SetJSConstant:
        m_jit.move(valueOfJSConstantAsImm64(plan.node()), plan.gpr());
        break;
    case SetDoubleConstant:
        m_jit.move(Imm64(reinterpretDoubleToInt64(plan.node()->asNumber())), canTrample);
        m_jit.move64ToDouble(canTrample, plan.fpr());
        break;
    case Load32PayloadBoxInt:
        m_jit.load32(JITCompiler::payloadFor(plan.node()->virtualRegister()), plan.gpr());
        m_jit.or64(GPRInfo::tagTypeNumberRegister, plan.gpr());
        break;
    case Load32PayloadConvertToInt52:
        m_jit.load32(JITCompiler::payloadFor(plan.node()->virtualRegister()), plan.gpr());
        m_jit.signExtend32ToPtr(plan.gpr(), plan.gpr());
        m_jit.lshift64(TrustedImm32(JSValue::int52ShiftAmount), plan.gpr());
        break;
    case Load32PayloadSignExtend:
        m_jit.load32(JITCompiler::payloadFor(plan.node()->virtualRegister()), plan.gpr());
        m_jit.signExtend32ToPtr(plan.gpr(), plan.gpr());
        break;
#else
    case SetJSConstantTag:
        m_jit.move(Imm32(plan.node()->asJSValue().tag()), plan.gpr());
        break;
    case SetJSConstantPayload:
        m_jit.move(Imm32(plan.node()->asJSValue().payload()), plan.gpr());
        break;
    case SetInt32Tag:
        m_jit.move(TrustedImm32(JSValue::Int32Tag), plan.gpr());
        break;
    case SetCellTag:
        m_jit.move(TrustedImm32(JSValue::CellTag), plan.gpr());
        break;
    case SetBooleanTag:
        m_jit.move(TrustedImm32(JSValue::BooleanTag), plan.gpr());
        break;
    case SetDoubleConstant:
        m_jit.loadDouble(TrustedImmPtr(m_jit.addressOfDoubleConstant(plan.node())), plan.fpr());
        break;
#endif
    case Load32Tag:
        m_jit.load32(JITCompiler::tagFor(plan.node()->virtualRegister()), plan.gpr());
        break;
    case Load32Payload:
        m_jit.load32(JITCompiler::payloadFor(plan.node()->virtualRegister()), plan.gpr());
        break;
    case LoadPtr:
        m_jit.loadPtr(JITCompiler::addressFor(plan.node()->virtualRegister()), plan.gpr());
        break;
#if USE(JSVALUE64)
    case Load64:
        m_jit.load64(JITCompiler::addressFor(plan.node()->virtualRegister()), plan.gpr());
        break;
    case Load64ShiftInt52Right:
        m_jit.load64(JITCompiler::addressFor(plan.node()->virtualRegister()), plan.gpr());
        m_jit.rshift64(TrustedImm32(JSValue::int52ShiftAmount), plan.gpr());
        break;
    case Load64ShiftInt52Left:
        m_jit.load64(JITCompiler::addressFor(plan.node()->virtualRegister()), plan.gpr());
        m_jit.lshift64(TrustedImm32(JSValue::int52ShiftAmount), plan.gpr());
        break;
#endif
    case LoadDouble:
        m_jit.loadDouble(JITCompiler::addressFor(plan.node()->virtualRegister()), plan.fpr());
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
}
    
JITCompiler::Jump SpeculativeJIT::jumpSlowForUnwantedArrayMode(GPRReg tempGPR, ArrayMode arrayMode, IndexingType shape)
{
    switch (arrayMode.arrayClass()) {
    case Array::OriginalArray: {
        CRASH();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
        JITCompiler::Jump result; // I already know that VC++ takes unkindly to the expression "return Jump()", so I'm doing it this way in anticipation of someone eventually using VC++ to compile the DFG.
        return result;
#endif
    }
        
    case Array::Array:
        m_jit.and32(TrustedImm32(IsArray | IndexingShapeMask), tempGPR);
        return m_jit.branch32(
            MacroAssembler::NotEqual, tempGPR, TrustedImm32(IsArray | shape));
        
    case Array::NonArray:
    case Array::OriginalNonArray:
        m_jit.and32(TrustedImm32(IsArray | IndexingShapeMask), tempGPR);
        return m_jit.branch32(
            MacroAssembler::NotEqual, tempGPR, TrustedImm32(shape));
        
    case Array::PossiblyArray:
        m_jit.and32(TrustedImm32(IndexingShapeMask), tempGPR);
        return m_jit.branch32(MacroAssembler::NotEqual, tempGPR, TrustedImm32(shape));
    }
    
    RELEASE_ASSERT_NOT_REACHED();
    return JITCompiler::Jump();
}

JITCompiler::JumpList SpeculativeJIT::jumpSlowForUnwantedArrayMode(GPRReg tempGPR, ArrayMode arrayMode)
{
    JITCompiler::JumpList result;
    
    switch (arrayMode.type()) {
    case Array::Int32:
        return jumpSlowForUnwantedArrayMode(tempGPR, arrayMode, Int32Shape);

    case Array::Double:
        return jumpSlowForUnwantedArrayMode(tempGPR, arrayMode, DoubleShape);

    case Array::Contiguous:
        return jumpSlowForUnwantedArrayMode(tempGPR, arrayMode, ContiguousShape);

    case Array::Undecided:
        return jumpSlowForUnwantedArrayMode(tempGPR, arrayMode, UndecidedShape);

    case Array::ArrayStorage:
    case Array::SlowPutArrayStorage: {
        ASSERT(!arrayMode.isJSArrayWithOriginalStructure());
        
        if (arrayMode.isJSArray()) {
            if (arrayMode.isSlowPut()) {
                result.append(
                    m_jit.branchTest32(
                        MacroAssembler::Zero, tempGPR, MacroAssembler::TrustedImm32(IsArray)));
                m_jit.and32(TrustedImm32(IndexingShapeMask), tempGPR);
                m_jit.sub32(TrustedImm32(ArrayStorageShape), tempGPR);
                result.append(
                    m_jit.branch32(
                        MacroAssembler::Above, tempGPR,
                        TrustedImm32(SlowPutArrayStorageShape - ArrayStorageShape)));
                break;
            }
            m_jit.and32(TrustedImm32(IsArray | IndexingShapeMask), tempGPR);
            result.append(
                m_jit.branch32(MacroAssembler::NotEqual, tempGPR, TrustedImm32(IsArray | ArrayStorageShape)));
            break;
        }
        m_jit.and32(TrustedImm32(IndexingShapeMask), tempGPR);
        if (arrayMode.isSlowPut()) {
            m_jit.sub32(TrustedImm32(ArrayStorageShape), tempGPR);
            result.append(
                m_jit.branch32(
                    MacroAssembler::Above, tempGPR,
                    TrustedImm32(SlowPutArrayStorageShape - ArrayStorageShape)));
            break;
        }
        result.append(
            m_jit.branch32(MacroAssembler::NotEqual, tempGPR, TrustedImm32(ArrayStorageShape)));
        break;
    }
    default:
        CRASH();
        break;
    }
    
    return result;
}

void SpeculativeJIT::checkArray(Node* node)
{
    ASSERT(node->arrayMode().isSpecific());
    ASSERT(!node->arrayMode().doesConversion());
    
    SpeculateCellOperand base(this, node->child1());
    GPRReg baseReg = base.gpr();
    
    if (node->arrayMode().alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1()))) {
        noResult(m_currentNode);
        return;
    }
    
    const ClassInfo* expectedClassInfo = 0;
    
    switch (node->arrayMode().type()) {
    case Array::AnyTypedArray:
    case Array::String:
        RELEASE_ASSERT_NOT_REACHED(); // Should have been a Phantom(String:)
        break;
    case Array::Int32:
    case Array::Double:
    case Array::Contiguous:
    case Array::Undecided:
    case Array::ArrayStorage:
    case Array::SlowPutArrayStorage: {
        GPRTemporary temp(this);
        GPRReg tempGPR = temp.gpr();
        m_jit.load8(MacroAssembler::Address(baseReg, JSCell::indexingTypeAndMiscOffset()), tempGPR);
        speculationCheck(
            BadIndexingType, JSValueSource::unboxedCell(baseReg), 0,
            jumpSlowForUnwantedArrayMode(tempGPR, node->arrayMode()));
        
        noResult(m_currentNode);
        return;
    }
    case Array::DirectArguments:
        speculateCellTypeWithoutTypeFiltering(node->child1(), baseReg, DirectArgumentsType);
        noResult(m_currentNode);
        return;
    case Array::ScopedArguments:
        speculateCellTypeWithoutTypeFiltering(node->child1(), baseReg, ScopedArgumentsType);
        noResult(m_currentNode);
        return;
    default:
        speculateCellTypeWithoutTypeFiltering(
            node->child1(), baseReg,
            typeForTypedArrayType(node->arrayMode().typedArrayType()));
        noResult(m_currentNode);
        return;
    }
    
    RELEASE_ASSERT(expectedClassInfo);
    
    GPRTemporary temp(this);
    GPRTemporary temp2(this);
    m_jit.emitLoadStructure(baseReg, temp.gpr(), temp2.gpr());
    speculationCheck(
        BadType, JSValueSource::unboxedCell(baseReg), node,
        m_jit.branchPtr(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(temp.gpr(), Structure::classInfoOffset()),
            TrustedImmPtr(expectedClassInfo)));
    
    noResult(m_currentNode);
}

void SpeculativeJIT::arrayify(Node* node, GPRReg baseReg, GPRReg propertyReg)
{
    ASSERT(node->arrayMode().doesConversion());
    
    GPRTemporary temp(this);
    GPRTemporary structure;
    GPRReg tempGPR = temp.gpr();
    GPRReg structureGPR = InvalidGPRReg;
    
    if (node->op() != ArrayifyToStructure) {
        GPRTemporary realStructure(this);
        structure.adopt(realStructure);
        structureGPR = structure.gpr();
    }
        
    // We can skip all that comes next if we already have array storage.
    MacroAssembler::JumpList slowPath;
    
    if (node->op() == ArrayifyToStructure) {
        slowPath.append(m_jit.branchWeakStructure(
            JITCompiler::NotEqual,
            JITCompiler::Address(baseReg, JSCell::structureIDOffset()),
            node->structure()));
    } else {
        m_jit.load8(
            MacroAssembler::Address(baseReg, JSCell::indexingTypeAndMiscOffset()), tempGPR);
        
        slowPath.append(jumpSlowForUnwantedArrayMode(tempGPR, node->arrayMode()));
    }
    
    addSlowPathGenerator(std::make_unique<ArrayifySlowPathGenerator>(
        slowPath, this, node, baseReg, propertyReg, tempGPR, structureGPR));
    
    noResult(m_currentNode);
}

void SpeculativeJIT::arrayify(Node* node)
{
    ASSERT(node->arrayMode().isSpecific());
    
    SpeculateCellOperand base(this, node->child1());
    
    if (!node->child2()) {
        arrayify(node, base.gpr(), InvalidGPRReg);
        return;
    }
    
    SpeculateInt32Operand property(this, node->child2());
    
    arrayify(node, base.gpr(), property.gpr());
}

GPRReg SpeculativeJIT::fillStorage(Edge edge)
{
    VirtualRegister virtualRegister = edge->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);
    
    switch (info.registerFormat()) {
    case DataFormatNone: {
        if (info.spillFormat() == DataFormatStorage) {
            GPRReg gpr = allocate();
            m_gprs.retain(gpr, virtualRegister, SpillOrderSpilled);
            m_jit.loadPtr(JITCompiler::addressFor(virtualRegister), gpr);
            info.fillStorage(*m_stream, gpr);
            return gpr;
        }
        
        // Must be a cell; fill it as a cell and then return the pointer.
        return fillSpeculateCell(edge);
    }
        
    case DataFormatStorage: {
        GPRReg gpr = info.gpr();
        m_gprs.lock(gpr);
        return gpr;
    }
        
    default:
        return fillSpeculateCell(edge);
    }
}

void SpeculativeJIT::useChildren(Node* node)
{
    if (node->flags() & NodeHasVarArgs) {
        for (unsigned childIdx = node->firstChild(); childIdx < node->firstChild() + node->numChildren(); childIdx++) {
            if (!!m_jit.graph().m_varArgChildren[childIdx])
                use(m_jit.graph().m_varArgChildren[childIdx]);
        }
    } else {
        Edge child1 = node->child1();
        if (!child1) {
            ASSERT(!node->child2() && !node->child3());
            return;
        }
        use(child1);
        
        Edge child2 = node->child2();
        if (!child2) {
            ASSERT(!node->child3());
            return;
        }
        use(child2);
        
        Edge child3 = node->child3();
        if (!child3)
            return;
        use(child3);
    }
}

void SpeculativeJIT::compileTryGetById(Node* node)
{
    switch (node->child1().useKind()) {
    case CellUse: {
        SpeculateCellOperand base(this, node->child1());
        JSValueRegsTemporary result(this, Reuse, base);

        JSValueRegs baseRegs = JSValueRegs::payloadOnly(base.gpr());
        JSValueRegs resultRegs = result.regs();

        base.use();

        cachedGetById(node->origin.semantic, baseRegs, resultRegs, node->identifierNumber(), JITCompiler::Jump(), NeedToSpill, AccessType::TryGet);

        jsValueResult(resultRegs, node, DataFormatJS, UseChildrenCalledExplicitly);
        break;
    }

    case UntypedUse: {
        JSValueOperand base(this, node->child1());
        JSValueRegsTemporary result(this, Reuse, base);

        JSValueRegs baseRegs = base.jsValueRegs();
        JSValueRegs resultRegs = result.regs();

        base.use();

        JITCompiler::Jump notCell = m_jit.branchIfNotCell(baseRegs);

        cachedGetById(node->origin.semantic, baseRegs, resultRegs, node->identifierNumber(), notCell, NeedToSpill, AccessType::TryGet);

        jsValueResult(resultRegs, node, DataFormatJS, UseChildrenCalledExplicitly);
        break;
    }

    default:
        DFG_CRASH(m_jit.graph(), node, "Bad use kind");
        break;
    } 
}

void SpeculativeJIT::compileIn(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    GPRReg baseGPR = base.gpr();
    
    if (JSString* string = node->child2()->dynamicCastConstant<JSString*>(*m_jit.vm())) {
        if (string->tryGetValueImpl() && string->tryGetValueImpl()->isAtomic()) {
            StructureStubInfo* stubInfo = m_jit.codeBlock()->addStubInfo(AccessType::In);
            
            GPRTemporary result(this);
            GPRReg resultGPR = result.gpr();

            use(node->child2());
            
            MacroAssembler::PatchableJump jump = m_jit.patchableJump();
            MacroAssembler::Label done = m_jit.label();
            
            // Since this block is executed only when the result of string->tryGetValueImpl() is atomic,
            // we can cast it to const AtomicStringImpl* safely.
            auto slowPath = slowPathCall(
                jump.m_jump, this, operationInOptimize,
                JSValueRegs::payloadOnly(resultGPR), stubInfo, baseGPR,
                static_cast<const AtomicStringImpl*>(string->tryGetValueImpl()));
            
            stubInfo->callSiteIndex = m_jit.addCallSite(node->origin.semantic);
            stubInfo->codeOrigin = node->origin.semantic;
            stubInfo->patch.baseGPR = static_cast<int8_t>(baseGPR);
            stubInfo->patch.valueGPR = static_cast<int8_t>(resultGPR);
            stubInfo->patch.thisGPR = static_cast<int8_t>(InvalidGPRReg);
#if USE(JSVALUE32_64)
            stubInfo->patch.valueTagGPR = static_cast<int8_t>(InvalidGPRReg);
            stubInfo->patch.baseTagGPR = static_cast<int8_t>(InvalidGPRReg);
            stubInfo->patch.thisTagGPR = static_cast<int8_t>(InvalidGPRReg);
#endif
            stubInfo->patch.usedRegisters = usedRegisters();

            m_jit.addIn(InRecord(jump, done, slowPath.get(), stubInfo));
            addSlowPathGenerator(WTFMove(slowPath));

            base.use();

            blessedBooleanResult(resultGPR, node, UseChildrenCalledExplicitly);
            return;
        }
    }

    JSValueOperand key(this, node->child2());
    JSValueRegs regs = key.jsValueRegs();
        
    GPRFlushedCallResult result(this);
    GPRReg resultGPR = result.gpr();
        
    base.use();
    key.use();
        
    flushRegisters();
    callOperation(
        operationGenericIn, extractResult(JSValueRegs::payloadOnly(resultGPR)),
        baseGPR, regs);
    m_jit.exceptionCheck();
    blessedBooleanResult(resultGPR, node, UseChildrenCalledExplicitly);
}

void SpeculativeJIT::compileDeleteById(Node* node)
{
    JSValueOperand value(this, node->child1());
    GPRFlushedCallResult result(this);

    JSValueRegs valueRegs = value.jsValueRegs();
    GPRReg resultGPR = result.gpr();

    value.use();

    flushRegisters();
    callOperation(operationDeleteById, resultGPR, valueRegs, identifierUID(node->identifierNumber()));
    m_jit.exceptionCheck();

    unblessedBooleanResult(resultGPR, node, UseChildrenCalledExplicitly);
}

void SpeculativeJIT::compileDeleteByVal(Node* node)
{
    JSValueOperand base(this, node->child1());
    JSValueOperand key(this, node->child2());
    GPRFlushedCallResult result(this);

    JSValueRegs baseRegs = base.jsValueRegs();
    JSValueRegs keyRegs = key.jsValueRegs();
    GPRReg resultGPR = result.gpr();

    base.use();
    key.use();

    flushRegisters();
    callOperation(operationDeleteByVal, resultGPR, baseRegs, keyRegs);
    m_jit.exceptionCheck();

    unblessedBooleanResult(resultGPR, node, UseChildrenCalledExplicitly);
}

bool SpeculativeJIT::nonSpeculativeCompare(Node* node, MacroAssembler::RelationalCondition cond, S_JITOperation_EJJ helperFunction)
{
    unsigned branchIndexInBlock = detectPeepHoleBranch();
    if (branchIndexInBlock != UINT_MAX) {
        Node* branchNode = m_block->at(branchIndexInBlock);

        ASSERT(node->adjustedRefCount() == 1);
        
        nonSpeculativePeepholeBranch(node, branchNode, cond, helperFunction);
    
        m_indexInBlock = branchIndexInBlock;
        m_currentNode = branchNode;
        
        return true;
    }
    
    nonSpeculativeNonPeepholeCompare(node, cond, helperFunction);
    
    return false;
}

bool SpeculativeJIT::nonSpeculativeStrictEq(Node* node, bool invert)
{
    unsigned branchIndexInBlock = detectPeepHoleBranch();
    if (branchIndexInBlock != UINT_MAX) {
        Node* branchNode = m_block->at(branchIndexInBlock);

        ASSERT(node->adjustedRefCount() == 1);
        
        nonSpeculativePeepholeStrictEq(node, branchNode, invert);
    
        m_indexInBlock = branchIndexInBlock;
        m_currentNode = branchNode;
        
        return true;
    }
    
    nonSpeculativeNonPeepholeStrictEq(node, invert);
    
    return false;
}

static const char* dataFormatString(DataFormat format)
{
    // These values correspond to the DataFormat enum.
    const char* strings[] = {
        "[  ]",
        "[ i]",
        "[ d]",
        "[ c]",
        "Err!",
        "Err!",
        "Err!",
        "Err!",
        "[J ]",
        "[Ji]",
        "[Jd]",
        "[Jc]",
        "Err!",
        "Err!",
        "Err!",
        "Err!",
    };
    return strings[format];
}

void SpeculativeJIT::dump(const char* label)
{
    if (label)
        dataLogF("<%s>\n", label);

    dataLogF("  gprs:\n");
    m_gprs.dump();
    dataLogF("  fprs:\n");
    m_fprs.dump();
    dataLogF("  VirtualRegisters:\n");
    for (unsigned i = 0; i < m_generationInfo.size(); ++i) {
        GenerationInfo& info = m_generationInfo[i];
        if (info.alive())
            dataLogF("    % 3d:%s%s", i, dataFormatString(info.registerFormat()), dataFormatString(info.spillFormat()));
        else
            dataLogF("    % 3d:[__][__]", i);
        if (info.registerFormat() == DataFormatDouble)
            dataLogF(":fpr%d\n", info.fpr());
        else if (info.registerFormat() != DataFormatNone
#if USE(JSVALUE32_64)
            && !(info.registerFormat() & DataFormatJS)
#endif
            ) {
            ASSERT(info.gpr() != InvalidGPRReg);
            dataLogF(":%s\n", GPRInfo::debugName(info.gpr()));
        } else
            dataLogF("\n");
    }
    if (label)
        dataLogF("</%s>\n", label);
}

GPRTemporary::GPRTemporary()
    : m_jit(0)
    , m_gpr(InvalidGPRReg)
{
}

GPRTemporary::GPRTemporary(SpeculativeJIT* jit)
    : m_jit(jit)
    , m_gpr(InvalidGPRReg)
{
    m_gpr = m_jit->allocate();
}

GPRTemporary::GPRTemporary(SpeculativeJIT* jit, GPRReg specific)
    : m_jit(jit)
    , m_gpr(InvalidGPRReg)
{
    m_gpr = m_jit->allocate(specific);
}

#if USE(JSVALUE32_64)
GPRTemporary::GPRTemporary(
    SpeculativeJIT* jit, ReuseTag, JSValueOperand& op1, WhichValueWord which)
    : m_jit(jit)
    , m_gpr(InvalidGPRReg)
{
    if (!op1.isDouble() && m_jit->canReuse(op1.node()))
        m_gpr = m_jit->reuse(op1.gpr(which));
    else
        m_gpr = m_jit->allocate();
}
#endif // USE(JSVALUE32_64)

JSValueRegsTemporary::JSValueRegsTemporary() { }

JSValueRegsTemporary::JSValueRegsTemporary(SpeculativeJIT* jit)
#if USE(JSVALUE64)
    : m_gpr(jit)
#else
    : m_payloadGPR(jit)
    , m_tagGPR(jit)
#endif
{
}

#if USE(JSVALUE64)
template<typename T>
JSValueRegsTemporary::JSValueRegsTemporary(SpeculativeJIT* jit, ReuseTag, T& operand, WhichValueWord)
    : m_gpr(jit, Reuse, operand)
{
}
#else
template<typename T>
JSValueRegsTemporary::JSValueRegsTemporary(SpeculativeJIT* jit, ReuseTag, T& operand, WhichValueWord resultWord)
{
    if (resultWord == PayloadWord) {
        m_payloadGPR = GPRTemporary(jit, Reuse, operand);
        m_tagGPR = GPRTemporary(jit);
    } else {
        m_payloadGPR = GPRTemporary(jit);
        m_tagGPR = GPRTemporary(jit, Reuse, operand);
    }
}
#endif

#if USE(JSVALUE64)
JSValueRegsTemporary::JSValueRegsTemporary(SpeculativeJIT* jit, ReuseTag, JSValueOperand& operand)
{
    m_gpr = GPRTemporary(jit, Reuse, operand);
}
#else
JSValueRegsTemporary::JSValueRegsTemporary(SpeculativeJIT* jit, ReuseTag, JSValueOperand& operand)
{
    if (jit->canReuse(operand.node())) {
        m_payloadGPR = GPRTemporary(jit, Reuse, operand, PayloadWord);
        m_tagGPR = GPRTemporary(jit, Reuse, operand, TagWord);
    } else {
        m_payloadGPR = GPRTemporary(jit);
        m_tagGPR = GPRTemporary(jit);
    }
}
#endif

JSValueRegsTemporary::~JSValueRegsTemporary() { }

JSValueRegs JSValueRegsTemporary::regs()
{
#if USE(JSVALUE64)
    return JSValueRegs(m_gpr.gpr());
#else
    return JSValueRegs(m_tagGPR.gpr(), m_payloadGPR.gpr());
#endif
}

void GPRTemporary::adopt(GPRTemporary& other)
{
    ASSERT(!m_jit);
    ASSERT(m_gpr == InvalidGPRReg);
    ASSERT(other.m_jit);
    ASSERT(other.m_gpr != InvalidGPRReg);
    m_jit = other.m_jit;
    m_gpr = other.m_gpr;
    other.m_jit = 0;
    other.m_gpr = InvalidGPRReg;
}

FPRTemporary::FPRTemporary(FPRTemporary&& other)
{
    ASSERT(other.m_jit);
    ASSERT(other.m_fpr != InvalidFPRReg);
    m_jit = other.m_jit;
    m_fpr = other.m_fpr;

    other.m_jit = nullptr;
}

FPRTemporary::FPRTemporary(SpeculativeJIT* jit)
    : m_jit(jit)
    , m_fpr(InvalidFPRReg)
{
    m_fpr = m_jit->fprAllocate();
}

FPRTemporary::FPRTemporary(SpeculativeJIT* jit, SpeculateDoubleOperand& op1)
    : m_jit(jit)
    , m_fpr(InvalidFPRReg)
{
    if (m_jit->canReuse(op1.node()))
        m_fpr = m_jit->reuse(op1.fpr());
    else
        m_fpr = m_jit->fprAllocate();
}

FPRTemporary::FPRTemporary(SpeculativeJIT* jit, SpeculateDoubleOperand& op1, SpeculateDoubleOperand& op2)
    : m_jit(jit)
    , m_fpr(InvalidFPRReg)
{
    if (m_jit->canReuse(op1.node()))
        m_fpr = m_jit->reuse(op1.fpr());
    else if (m_jit->canReuse(op2.node()))
        m_fpr = m_jit->reuse(op2.fpr());
    else if (m_jit->canReuse(op1.node(), op2.node()) && op1.fpr() == op2.fpr())
        m_fpr = m_jit->reuse(op1.fpr());
    else
        m_fpr = m_jit->fprAllocate();
}

#if USE(JSVALUE32_64)
FPRTemporary::FPRTemporary(SpeculativeJIT* jit, JSValueOperand& op1)
    : m_jit(jit)
    , m_fpr(InvalidFPRReg)
{
    if (op1.isDouble() && m_jit->canReuse(op1.node()))
        m_fpr = m_jit->reuse(op1.fpr());
    else
        m_fpr = m_jit->fprAllocate();
}
#endif

void SpeculativeJIT::compilePeepHoleDoubleBranch(Node* node, Node* branchNode, JITCompiler::DoubleCondition condition)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    if (taken == nextBlock()) {
        condition = MacroAssembler::invert(condition);
        std::swap(taken, notTaken);
    }

    SpeculateDoubleOperand op1(this, node->child1());
    SpeculateDoubleOperand op2(this, node->child2());
    
    branchDouble(condition, op1.fpr(), op2.fpr(), taken);
    jump(notTaken);
}

void SpeculativeJIT::compilePeepHoleObjectEquality(Node* node, Node* branchNode)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    MacroAssembler::RelationalCondition condition = MacroAssembler::Equal;
    
    if (taken == nextBlock()) {
        condition = MacroAssembler::NotEqual;
        BasicBlock* tmp = taken;
        taken = notTaken;
        notTaken = tmp;
    }

    SpeculateCellOperand op1(this, node->child1());
    SpeculateCellOperand op2(this, node->child2());
    
    GPRReg op1GPR = op1.gpr();
    GPRReg op2GPR = op2.gpr();
    
    if (masqueradesAsUndefinedWatchpointIsStillValid()) {
        if (m_state.forNode(node->child1()).m_type & ~SpecObject) {
            speculationCheck(
                BadType, JSValueSource::unboxedCell(op1GPR), node->child1(), m_jit.branchIfNotObject(op1GPR));
        }
        if (m_state.forNode(node->child2()).m_type & ~SpecObject) {
            speculationCheck(
                BadType, JSValueSource::unboxedCell(op2GPR), node->child2(), m_jit.branchIfNotObject(op2GPR));
        }
    } else {
        if (m_state.forNode(node->child1()).m_type & ~SpecObject) {
            speculationCheck(
                BadType, JSValueSource::unboxedCell(op1GPR), node->child1(),
                m_jit.branchIfNotObject(op1GPR));
        }
        speculationCheck(BadType, JSValueSource::unboxedCell(op1GPR), node->child1(),
            m_jit.branchTest8(
                MacroAssembler::NonZero, 
                MacroAssembler::Address(op1GPR, JSCell::typeInfoFlagsOffset()), 
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));

        if (m_state.forNode(node->child2()).m_type & ~SpecObject) {
            speculationCheck(
                BadType, JSValueSource::unboxedCell(op2GPR), node->child2(),
                m_jit.branchIfNotObject(op2GPR));
        }
        speculationCheck(BadType, JSValueSource::unboxedCell(op2GPR), node->child2(),
            m_jit.branchTest8(
                MacroAssembler::NonZero, 
                MacroAssembler::Address(op2GPR, JSCell::typeInfoFlagsOffset()), 
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));
    }

    branchPtr(condition, op1GPR, op2GPR, taken);
    jump(notTaken);
}

void SpeculativeJIT::compilePeepHoleBooleanBranch(Node* node, Node* branchNode, JITCompiler::RelationalCondition condition)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    // The branch instruction will branch to the taken block.
    // If taken is next, switch taken with notTaken & invert the branch condition so we can fall through.
    if (taken == nextBlock()) {
        condition = JITCompiler::invert(condition);
        BasicBlock* tmp = taken;
        taken = notTaken;
        notTaken = tmp;
    }

    if (node->child1()->isInt32Constant()) {
        int32_t imm = node->child1()->asInt32();
        SpeculateBooleanOperand op2(this, node->child2());
        branch32(condition, JITCompiler::Imm32(imm), op2.gpr(), taken);
    } else if (node->child2()->isInt32Constant()) {
        SpeculateBooleanOperand op1(this, node->child1());
        int32_t imm = node->child2()->asInt32();
        branch32(condition, op1.gpr(), JITCompiler::Imm32(imm), taken);
    } else {
        SpeculateBooleanOperand op1(this, node->child1());
        SpeculateBooleanOperand op2(this, node->child2());
        branch32(condition, op1.gpr(), op2.gpr(), taken);
    }

    jump(notTaken);
}

void SpeculativeJIT::compileToLowerCase(Node* node)
{
    ASSERT(node->op() == ToLowerCase);
    SpeculateCellOperand string(this, node->child1());
    GPRTemporary temp(this);
    GPRTemporary index(this);
    GPRTemporary charReg(this);
    GPRTemporary length(this);

    GPRReg stringGPR = string.gpr();
    GPRReg tempGPR = temp.gpr();
    GPRReg indexGPR = index.gpr();
    GPRReg charGPR = charReg.gpr();
    GPRReg lengthGPR = length.gpr();

    speculateString(node->child1(), stringGPR);

    CCallHelpers::JumpList slowPath;

    m_jit.move(TrustedImmPtr(0), indexGPR);

    m_jit.loadPtr(MacroAssembler::Address(stringGPR, JSString::offsetOfValue()), tempGPR);
    slowPath.append(m_jit.branchTestPtr(MacroAssembler::Zero, tempGPR));

    slowPath.append(m_jit.branchTest32(
        MacroAssembler::Zero, MacroAssembler::Address(tempGPR, StringImpl::flagsOffset()),
        MacroAssembler::TrustedImm32(StringImpl::flagIs8Bit())));
    m_jit.load32(MacroAssembler::Address(tempGPR, StringImpl::lengthMemoryOffset()), lengthGPR);
    m_jit.loadPtr(MacroAssembler::Address(tempGPR, StringImpl::dataOffset()), tempGPR);

    auto loopStart = m_jit.label();
    auto loopDone = m_jit.branch32(CCallHelpers::AboveOrEqual, indexGPR, lengthGPR);
    m_jit.load8(MacroAssembler::BaseIndex(tempGPR, indexGPR, MacroAssembler::TimesOne), charGPR);
    slowPath.append(m_jit.branchTest32(CCallHelpers::NonZero, charGPR, TrustedImm32(~0x7F)));
    m_jit.sub32(TrustedImm32('A'), charGPR);
    slowPath.append(m_jit.branch32(CCallHelpers::BelowOrEqual, charGPR, TrustedImm32('Z' - 'A')));

    m_jit.add32(TrustedImm32(1), indexGPR);
    m_jit.jump().linkTo(loopStart, &m_jit);
    
    slowPath.link(&m_jit);
    silentSpillAllRegisters(lengthGPR);
    callOperation(operationToLowerCase, lengthGPR, stringGPR, indexGPR);
    silentFillAllRegisters(lengthGPR);
    m_jit.exceptionCheck();
    auto done = m_jit.jump();

    loopDone.link(&m_jit);
    m_jit.move(stringGPR, lengthGPR);

    done.link(&m_jit);
    cellResult(lengthGPR, node);
}

void SpeculativeJIT::compilePeepHoleInt32Branch(Node* node, Node* branchNode, JITCompiler::RelationalCondition condition)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    // The branch instruction will branch to the taken block.
    // If taken is next, switch taken with notTaken & invert the branch condition so we can fall through.
    if (taken == nextBlock()) {
        condition = JITCompiler::invert(condition);
        BasicBlock* tmp = taken;
        taken = notTaken;
        notTaken = tmp;
    }

    if (node->child1()->isInt32Constant()) {
        int32_t imm = node->child1()->asInt32();
        SpeculateInt32Operand op2(this, node->child2());
        branch32(condition, JITCompiler::Imm32(imm), op2.gpr(), taken);
    } else if (node->child2()->isInt32Constant()) {
        SpeculateInt32Operand op1(this, node->child1());
        int32_t imm = node->child2()->asInt32();
        branch32(condition, op1.gpr(), JITCompiler::Imm32(imm), taken);
    } else {
        SpeculateInt32Operand op1(this, node->child1());
        SpeculateInt32Operand op2(this, node->child2());
        branch32(condition, op1.gpr(), op2.gpr(), taken);
    }

    jump(notTaken);
}

// Returns true if the compare is fused with a subsequent branch.
bool SpeculativeJIT::compilePeepHoleBranch(Node* node, MacroAssembler::RelationalCondition condition, MacroAssembler::DoubleCondition doubleCondition, S_JITOperation_EJJ operation)
{
    // Fused compare & branch.
    unsigned branchIndexInBlock = detectPeepHoleBranch();
    if (branchIndexInBlock != UINT_MAX) {
        Node* branchNode = m_block->at(branchIndexInBlock);

        // detectPeepHoleBranch currently only permits the branch to be the very next node,
        // so can be no intervening nodes to also reference the compare. 
        ASSERT(node->adjustedRefCount() == 1);

        if (node->isBinaryUseKind(Int32Use))
            compilePeepHoleInt32Branch(node, branchNode, condition);
#if USE(JSVALUE64)
        else if (node->isBinaryUseKind(Int52RepUse))
            compilePeepHoleInt52Branch(node, branchNode, condition);
#endif // USE(JSVALUE64)
        else if (node->isBinaryUseKind(StringUse) || node->isBinaryUseKind(StringIdentUse)) {
            // Use non-peephole comparison, for now.
            return false;
        } else if (node->isBinaryUseKind(DoubleRepUse))
            compilePeepHoleDoubleBranch(node, branchNode, doubleCondition);
        else if (node->op() == CompareEq) {
            if (node->isBinaryUseKind(BooleanUse))
                compilePeepHoleBooleanBranch(node, branchNode, condition);
            else if (node->isBinaryUseKind(SymbolUse))
                compilePeepHoleSymbolEquality(node, branchNode);
            else if (node->isBinaryUseKind(ObjectUse))
                compilePeepHoleObjectEquality(node, branchNode);
            else if (node->isBinaryUseKind(ObjectUse, ObjectOrOtherUse))
                compilePeepHoleObjectToObjectOrOtherEquality(node->child1(), node->child2(), branchNode);
            else if (node->isBinaryUseKind(ObjectOrOtherUse, ObjectUse))
                compilePeepHoleObjectToObjectOrOtherEquality(node->child2(), node->child1(), branchNode);
            else if (!needsTypeCheck(node->child1(), SpecOther))
                nonSpeculativePeepholeBranchNullOrUndefined(node->child2(), branchNode);
            else if (!needsTypeCheck(node->child2(), SpecOther))
                nonSpeculativePeepholeBranchNullOrUndefined(node->child1(), branchNode);
            else {
                nonSpeculativePeepholeBranch(node, branchNode, condition, operation);
                return true;
            }
        } else {
            nonSpeculativePeepholeBranch(node, branchNode, condition, operation);
            return true;
        }

        use(node->child1());
        use(node->child2());
        m_indexInBlock = branchIndexInBlock;
        m_currentNode = branchNode;
        return true;
    }
    return false;
}

void SpeculativeJIT::noticeOSRBirth(Node* node)
{
    if (!node->hasVirtualRegister())
        return;
    
    VirtualRegister virtualRegister = node->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);
    
    info.noticeOSRBirth(*m_stream, node, virtualRegister);
}

void SpeculativeJIT::compileMovHint(Node* node)
{
    ASSERT(node->containsMovHint() && node->op() != ZombieHint);
    
    Node* child = node->child1().node();
    noticeOSRBirth(child);
    
    m_stream->appendAndLog(VariableEvent::movHint(MinifiedID(child), node->unlinkedLocal()));
}

void SpeculativeJIT::bail(AbortReason reason)
{
    if (verboseCompilationEnabled())
        dataLog("Bailing compilation.\n");
    m_compileOkay = true;
    m_jit.abortWithReason(reason, m_lastGeneratedNode);
    clearGenerationInfo();
}

void SpeculativeJIT::compileCurrentBlock()
{
    ASSERT(m_compileOkay);
    
    if (!m_block)
        return;
    
    ASSERT(m_block->isReachable);
    
    m_jit.blockHeads()[m_block->index] = m_jit.label();

    if (!m_block->intersectionOfCFAHasVisited) {
        // Don't generate code for basic blocks that are unreachable according to CFA.
        // But to be sure that nobody has generated a jump to this block, drop in a
        // breakpoint here.
        m_jit.abortWithReason(DFGUnreachableBasicBlock);
        return;
    }

    m_stream->appendAndLog(VariableEvent::reset());
    
    m_jit.jitAssertHasValidCallFrame();
    m_jit.jitAssertTagsInPlace();
    m_jit.jitAssertArgumentCountSane();

    m_state.reset();
    m_state.beginBasicBlock(m_block);
    
    for (size_t i = m_block->variablesAtHead.size(); i--;) {
        int operand = m_block->variablesAtHead.operandForIndex(i);
        Node* node = m_block->variablesAtHead[i];
        if (!node)
            continue; // No need to record dead SetLocal's.
        
        VariableAccessData* variable = node->variableAccessData();
        DataFormat format;
        if (!node->refCount())
            continue; // No need to record dead SetLocal's.
        format = dataFormatFor(variable->flushFormat());
        m_stream->appendAndLog(
            VariableEvent::setLocal(
                VirtualRegister(operand),
                variable->machineLocal(),
                format));
    }

    m_origin = NodeOrigin();
    
    for (m_indexInBlock = 0; m_indexInBlock < m_block->size(); ++m_indexInBlock) {
        m_currentNode = m_block->at(m_indexInBlock);
        
        // We may have hit a contradiction that the CFA was aware of but that the JIT
        // didn't cause directly.
        if (!m_state.isValid()) {
            bail(DFGBailedAtTopOfBlock);
            return;
        }

        m_interpreter.startExecuting();
        m_interpreter.executeKnownEdgeTypes(m_currentNode);
        m_jit.setForNode(m_currentNode);
        m_origin = m_currentNode->origin;
        if (validationEnabled())
            m_origin.exitOK &= mayExit(m_jit.graph(), m_currentNode) == Exits;
        m_lastGeneratedNode = m_currentNode->op();
        
        ASSERT(m_currentNode->shouldGenerate());
        
        if (verboseCompilationEnabled()) {
            dataLogF(
                "SpeculativeJIT generating Node @%d (bc#%u) at JIT offset 0x%x",
                (int)m_currentNode->index(),
                m_currentNode->origin.semantic.bytecodeIndex, m_jit.debugOffset());
            dataLog("\n");
        }

        if (Options::validateDFGExceptionHandling() && (mayExit(m_jit.graph(), m_currentNode) != DoesNotExit || m_currentNode->isTerminal()))
            m_jit.jitReleaseAssertNoException();

        m_jit.pcToCodeOriginMapBuilder().appendItem(m_jit.labelIgnoringWatchpoints(), m_origin.semantic);

        compile(m_currentNode);
        
        if (belongsInMinifiedGraph(m_currentNode->op()))
            m_minifiedGraph->append(MinifiedNode::fromNode(m_currentNode));
        
#if ENABLE(DFG_REGISTER_ALLOCATION_VALIDATION)
        m_jit.clearRegisterAllocationOffsets();
#endif
        
        if (!m_compileOkay) {
            bail(DFGBailedAtEndOfNode);
            return;
        }
        
        // Make sure that the abstract state is rematerialized for the next node.
        m_interpreter.executeEffects(m_indexInBlock);
    }
    
    // Perform the most basic verification that children have been used correctly.
    if (!ASSERT_DISABLED) {
        for (auto& info : m_generationInfo)
            RELEASE_ASSERT(!info.alive());
    }
}

// If we are making type predictions about our arguments then
// we need to check that they are correct on function entry.
void SpeculativeJIT::checkArgumentTypes()
{
    ASSERT(!m_currentNode);
    m_origin = NodeOrigin(CodeOrigin(0), CodeOrigin(0), true);

    for (int i = 0; i < m_jit.codeBlock()->numParameters(); ++i) {
        Node* node = m_jit.graph().m_arguments[i];
        if (!node) {
            // The argument is dead. We don't do any checks for such arguments.
            continue;
        }
        
        ASSERT(node->op() == SetArgument);
        ASSERT(node->shouldGenerate());

        VariableAccessData* variableAccessData = node->variableAccessData();
        FlushFormat format = variableAccessData->flushFormat();
        
        if (format == FlushedJSValue)
            continue;
        
        VirtualRegister virtualRegister = variableAccessData->local();

        JSValueSource valueSource = JSValueSource(JITCompiler::addressFor(virtualRegister));
        
#if USE(JSVALUE64)
        switch (format) {
        case FlushedInt32: {
            speculationCheck(BadType, valueSource, node, m_jit.branch64(MacroAssembler::Below, JITCompiler::addressFor(virtualRegister), GPRInfo::tagTypeNumberRegister));
            break;
        }
        case FlushedBoolean: {
            GPRTemporary temp(this);
            m_jit.load64(JITCompiler::addressFor(virtualRegister), temp.gpr());
            m_jit.xor64(TrustedImm32(static_cast<int32_t>(ValueFalse)), temp.gpr());
            speculationCheck(BadType, valueSource, node, m_jit.branchTest64(MacroAssembler::NonZero, temp.gpr(), TrustedImm32(static_cast<int32_t>(~1))));
            break;
        }
        case FlushedCell: {
            speculationCheck(BadType, valueSource, node, m_jit.branchTest64(MacroAssembler::NonZero, JITCompiler::addressFor(virtualRegister), GPRInfo::tagMaskRegister));
            break;
        }
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
#else
        switch (format) {
        case FlushedInt32: {
            speculationCheck(BadType, valueSource, node, m_jit.branch32(MacroAssembler::NotEqual, JITCompiler::tagFor(virtualRegister), TrustedImm32(JSValue::Int32Tag)));
            break;
        }
        case FlushedBoolean: {
            speculationCheck(BadType, valueSource, node, m_jit.branch32(MacroAssembler::NotEqual, JITCompiler::tagFor(virtualRegister), TrustedImm32(JSValue::BooleanTag)));
            break;
        }
        case FlushedCell: {
            speculationCheck(BadType, valueSource, node, m_jit.branch32(MacroAssembler::NotEqual, JITCompiler::tagFor(virtualRegister), TrustedImm32(JSValue::CellTag)));
            break;
        }
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
#endif
    }

    m_origin = NodeOrigin();
}

bool SpeculativeJIT::compile()
{
    checkArgumentTypes();
    
    ASSERT(!m_currentNode);
    for (BlockIndex blockIndex = 0; blockIndex < m_jit.graph().numBlocks(); ++blockIndex) {
        m_jit.setForBlockIndex(blockIndex);
        m_block = m_jit.graph().block(blockIndex);
        compileCurrentBlock();
    }
    linkBranches();
    return true;
}

void SpeculativeJIT::createOSREntries()
{
    for (BlockIndex blockIndex = 0; blockIndex < m_jit.graph().numBlocks(); ++blockIndex) {
        BasicBlock* block = m_jit.graph().block(blockIndex);
        if (!block)
            continue;
        if (!block->isOSRTarget)
            continue;
        
        // Currently we don't have OSR entry trampolines. We could add them
        // here if need be.
        m_osrEntryHeads.append(m_jit.blockHeads()[blockIndex]);
    }
}

void SpeculativeJIT::linkOSREntries(LinkBuffer& linkBuffer)
{
    unsigned osrEntryIndex = 0;
    for (BlockIndex blockIndex = 0; blockIndex < m_jit.graph().numBlocks(); ++blockIndex) {
        BasicBlock* block = m_jit.graph().block(blockIndex);
        if (!block)
            continue;
        if (!block->isOSRTarget)
            continue;
        m_jit.noticeOSREntry(*block, m_osrEntryHeads[osrEntryIndex++], linkBuffer);
    }
    ASSERT(osrEntryIndex == m_osrEntryHeads.size());
    
    if (verboseCompilationEnabled()) {
        DumpContext dumpContext;
        dataLog("OSR Entries:\n");
        for (OSREntryData& entryData : m_jit.jitCode()->osrEntry)
            dataLog("    ", inContext(entryData, &dumpContext), "\n");
        if (!dumpContext.isEmpty())
            dumpContext.dump(WTF::dataFile());
    }
}
    
void SpeculativeJIT::compileCheckTraps(Node*)
{
    ASSERT(Options::usePollingTraps());
    GPRTemporary unused(this);
    GPRReg unusedGPR = unused.gpr();

    JITCompiler::Jump needTrapHandling = m_jit.branchTest8(JITCompiler::NonZero,
        JITCompiler::AbsoluteAddress(m_jit.vm()->needTrapHandlingAddress()));

    addSlowPathGenerator(slowPathCall(needTrapHandling, this, operationHandleTraps, unusedGPR));
}

void SpeculativeJIT::compileDoublePutByVal(Node* node, SpeculateCellOperand& base, SpeculateStrictInt32Operand& property)
{
    Edge child3 = m_jit.graph().varArgChild(node, 2);
    Edge child4 = m_jit.graph().varArgChild(node, 3);

    ArrayMode arrayMode = node->arrayMode();
    
    GPRReg baseReg = base.gpr();
    GPRReg propertyReg = property.gpr();
    
    SpeculateDoubleOperand value(this, child3);

    FPRReg valueReg = value.fpr();
    
    DFG_TYPE_CHECK(
        JSValueRegs(), child3, SpecFullRealNumber,
        m_jit.branchDouble(
            MacroAssembler::DoubleNotEqualOrUnordered, valueReg, valueReg));
    
    if (!m_compileOkay)
        return;
    
    StorageOperand storage(this, child4);
    GPRReg storageReg = storage.gpr();

    if (node->op() == PutByValAlias) {
        // Store the value to the array.
        GPRReg propertyReg = property.gpr();
        FPRReg valueReg = value.fpr();
        m_jit.storeDouble(valueReg, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight));
        
        noResult(m_currentNode);
        return;
    }
    
    GPRTemporary temporary;
    GPRReg temporaryReg = temporaryRegisterForPutByVal(temporary, node);

    MacroAssembler::Jump slowCase;
    
    if (arrayMode.isInBounds()) {
        speculationCheck(
            OutOfBounds, JSValueRegs(), 0,
            m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength())));
    } else {
        MacroAssembler::Jump inBounds = m_jit.branch32(MacroAssembler::Below, propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength()));
        
        slowCase = m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfVectorLength()));
        
        if (!arrayMode.isOutOfBounds())
            speculationCheck(OutOfBounds, JSValueRegs(), 0, slowCase);
        
        m_jit.add32(TrustedImm32(1), propertyReg, temporaryReg);
        m_jit.store32(temporaryReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength()));
        
        inBounds.link(&m_jit);
    }
    
    m_jit.storeDouble(valueReg, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight));

    base.use();
    property.use();
    value.use();
    storage.use();
    
    if (arrayMode.isOutOfBounds()) {
        addSlowPathGenerator(
            slowPathCall(
                slowCase, this,
                m_jit.codeBlock()->isStrictMode() ? operationPutDoubleByValBeyondArrayBoundsStrict : operationPutDoubleByValBeyondArrayBoundsNonStrict,
                NoResult, baseReg, propertyReg, valueReg));
    }

    noResult(m_currentNode, UseChildrenCalledExplicitly);
}

void SpeculativeJIT::compileGetCharCodeAt(Node* node)
{
    SpeculateCellOperand string(this, node->child1());
    SpeculateStrictInt32Operand index(this, node->child2());
    StorageOperand storage(this, node->child3());

    GPRReg stringReg = string.gpr();
    GPRReg indexReg = index.gpr();
    GPRReg storageReg = storage.gpr();
    
    ASSERT(speculationChecked(m_state.forNode(node->child1()).m_type, SpecString));

    // unsigned comparison so we can filter out negative indices and indices that are too large
    speculationCheck(Uncountable, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::AboveOrEqual, indexReg, MacroAssembler::Address(stringReg, JSString::offsetOfLength())));

    GPRTemporary scratch(this);
    GPRReg scratchReg = scratch.gpr();

    m_jit.loadPtr(MacroAssembler::Address(stringReg, JSString::offsetOfValue()), scratchReg);

    // Load the character into scratchReg
    JITCompiler::Jump is16Bit = m_jit.branchTest32(MacroAssembler::Zero, MacroAssembler::Address(scratchReg, StringImpl::flagsOffset()), TrustedImm32(StringImpl::flagIs8Bit()));

    m_jit.load8(MacroAssembler::BaseIndex(storageReg, indexReg, MacroAssembler::TimesOne, 0), scratchReg);
    JITCompiler::Jump cont8Bit = m_jit.jump();

    is16Bit.link(&m_jit);

    m_jit.load16(MacroAssembler::BaseIndex(storageReg, indexReg, MacroAssembler::TimesTwo, 0), scratchReg);

    cont8Bit.link(&m_jit);

    int32Result(scratchReg, m_currentNode);
}

void SpeculativeJIT::compileGetByValOnString(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    SpeculateStrictInt32Operand property(this, node->child2());
    StorageOperand storage(this, node->child3());
    GPRReg baseReg = base.gpr();
    GPRReg propertyReg = property.gpr();
    GPRReg storageReg = storage.gpr();

    GPRTemporary scratch(this);
    GPRReg scratchReg = scratch.gpr();
#if USE(JSVALUE32_64)
    GPRTemporary resultTag;
    GPRReg resultTagReg = InvalidGPRReg;
    if (node->arrayMode().isOutOfBounds()) {
        GPRTemporary realResultTag(this);
        resultTag.adopt(realResultTag);
        resultTagReg = resultTag.gpr();
    }
#endif

    ASSERT(ArrayMode(Array::String).alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1())));

    // unsigned comparison so we can filter out negative indices and indices that are too large
    JITCompiler::Jump outOfBounds = m_jit.branch32(
        MacroAssembler::AboveOrEqual, propertyReg,
        MacroAssembler::Address(baseReg, JSString::offsetOfLength()));
    if (node->arrayMode().isInBounds())
        speculationCheck(OutOfBounds, JSValueRegs(), 0, outOfBounds);

    m_jit.loadPtr(MacroAssembler::Address(baseReg, JSString::offsetOfValue()), scratchReg);

    // Load the character into scratchReg
    JITCompiler::Jump is16Bit = m_jit.branchTest32(MacroAssembler::Zero, MacroAssembler::Address(scratchReg, StringImpl::flagsOffset()), TrustedImm32(StringImpl::flagIs8Bit()));

    m_jit.load8(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesOne, 0), scratchReg);
    JITCompiler::Jump cont8Bit = m_jit.jump();

    is16Bit.link(&m_jit);

    m_jit.load16(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesTwo, 0), scratchReg);

    JITCompiler::Jump bigCharacter =
        m_jit.branch32(MacroAssembler::AboveOrEqual, scratchReg, TrustedImm32(0x100));

    // 8 bit string values don't need the isASCII check.
    cont8Bit.link(&m_jit);

    m_jit.lshift32(MacroAssembler::TrustedImm32(sizeof(void*) == 4 ? 2 : 3), scratchReg);
    m_jit.addPtr(TrustedImmPtr(m_jit.vm()->smallStrings.singleCharacterStrings()), scratchReg);
    m_jit.loadPtr(scratchReg, scratchReg);

    addSlowPathGenerator(
        slowPathCall(
            bigCharacter, this, operationSingleCharacterString, scratchReg, scratchReg));

    if (node->arrayMode().isOutOfBounds()) {
#if USE(JSVALUE32_64)
        m_jit.move(TrustedImm32(JSValue::CellTag), resultTagReg);
#endif

        JSGlobalObject* globalObject = m_jit.globalObjectFor(node->origin.semantic);
        bool prototypeChainIsSane = false;
        if (globalObject->stringPrototypeChainIsSane()) {
            // FIXME: This could be captured using a Speculation mode that means "out-of-bounds
            // loads return a trivial value". Something like SaneChainOutOfBounds. This should
            // speculate that we don't take negative out-of-bounds, or better yet, it should rely
            // on a stringPrototypeChainIsSane() guaranteeing that the prototypes have no negative
            // indexed properties either.
            // https://bugs.webkit.org/show_bug.cgi?id=144668
            m_jit.graph().watchpoints().addLazily(globalObject->stringPrototype()->structure()->transitionWatchpointSet());
            m_jit.graph().watchpoints().addLazily(globalObject->objectPrototype()->structure()->transitionWatchpointSet());
            prototypeChainIsSane = globalObject->stringPrototypeChainIsSane();
        }
        if (prototypeChainIsSane) {
            m_jit.graph().watchpoints().addLazily(globalObject->stringPrototype()->structure()->transitionWatchpointSet());
            m_jit.graph().watchpoints().addLazily(globalObject->objectPrototype()->structure()->transitionWatchpointSet());
            
#if USE(JSVALUE64)
            addSlowPathGenerator(std::make_unique<SaneStringGetByValSlowPathGenerator>(
                outOfBounds, this, JSValueRegs(scratchReg), baseReg, propertyReg));
#else
            addSlowPathGenerator(std::make_unique<SaneStringGetByValSlowPathGenerator>(
                outOfBounds, this, JSValueRegs(resultTagReg, scratchReg),
                baseReg, propertyReg));
#endif
        } else {
#if USE(JSVALUE64)
            addSlowPathGenerator(
                slowPathCall(
                    outOfBounds, this, operationGetByValStringInt,
                    scratchReg, baseReg, propertyReg));
#else
            addSlowPathGenerator(
                slowPathCall(
                    outOfBounds, this, operationGetByValStringInt,
                    JSValueRegs(resultTagReg, scratchReg), baseReg, propertyReg));
#endif
        }
        
#if USE(JSVALUE64)
        jsValueResult(scratchReg, m_currentNode);
#else
        jsValueResult(resultTagReg, scratchReg, m_currentNode);
#endif
    } else
        cellResult(scratchReg, m_currentNode);
}

void SpeculativeJIT::compileFromCharCode(Node* node)
{
    Edge& child = node->child1();
    if (child.useKind() == UntypedUse) {
        JSValueOperand opr(this, child);
        JSValueRegs oprRegs = opr.jsValueRegs();
#if USE(JSVALUE64)
        GPRTemporary result(this);
        JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
        GPRTemporary resultTag(this);
        GPRTemporary resultPayload(this);
        JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
#endif
        flushRegisters();
        callOperation(operationStringFromCharCodeUntyped, resultRegs, oprRegs);
        m_jit.exceptionCheck();
        
        jsValueResult(resultRegs, node);
        return;
    }

    SpeculateStrictInt32Operand property(this, child);
    GPRReg propertyReg = property.gpr();
    GPRTemporary smallStrings(this);
    GPRTemporary scratch(this);
    GPRReg scratchReg = scratch.gpr();
    GPRReg smallStringsReg = smallStrings.gpr();

    JITCompiler::JumpList slowCases;
    slowCases.append(m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, TrustedImm32(0xff)));
    m_jit.move(TrustedImmPtr(m_jit.vm()->smallStrings.singleCharacterStrings()), smallStringsReg);
    m_jit.loadPtr(MacroAssembler::BaseIndex(smallStringsReg, propertyReg, MacroAssembler::ScalePtr, 0), scratchReg);

    slowCases.append(m_jit.branchTest32(MacroAssembler::Zero, scratchReg));
    addSlowPathGenerator(slowPathCall(slowCases, this, operationStringFromCharCode, scratchReg, propertyReg));
    cellResult(scratchReg, m_currentNode);
}

GeneratedOperandType SpeculativeJIT::checkGeneratedTypeForToInt32(Node* node)
{
    VirtualRegister virtualRegister = node->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);

    switch (info.registerFormat()) {
    case DataFormatStorage:
        RELEASE_ASSERT_NOT_REACHED();

    case DataFormatBoolean:
    case DataFormatCell:
        terminateSpeculativeExecution(Uncountable, JSValueRegs(), 0);
        return GeneratedOperandTypeUnknown;

    case DataFormatNone:
    case DataFormatJSCell:
    case DataFormatJS:
    case DataFormatJSBoolean:
    case DataFormatJSDouble:
        return GeneratedOperandJSValue;

    case DataFormatJSInt32:
    case DataFormatInt32:
        return GeneratedOperandInteger;

    default:
        RELEASE_ASSERT_NOT_REACHED();
        return GeneratedOperandTypeUnknown;
    }
}

void SpeculativeJIT::compileValueToInt32(Node* node)
{
    switch (node->child1().useKind()) {
#if USE(JSVALUE64)
    case Int52RepUse: {
        SpeculateStrictInt52Operand op1(this, node->child1());
        GPRTemporary result(this, Reuse, op1);
        GPRReg op1GPR = op1.gpr();
        GPRReg resultGPR = result.gpr();
        m_jit.zeroExtend32ToPtr(op1GPR, resultGPR);
        int32Result(resultGPR, node, DataFormatInt32);
        return;
    }
#endif // USE(JSVALUE64)
        
    case DoubleRepUse: {
        GPRTemporary result(this);
        SpeculateDoubleOperand op1(this, node->child1());
        FPRReg fpr = op1.fpr();
        GPRReg gpr = result.gpr();
        JITCompiler::Jump notTruncatedToInteger = m_jit.branchTruncateDoubleToInt32(fpr, gpr, JITCompiler::BranchIfTruncateFailed);
        
        addSlowPathGenerator(slowPathCall(notTruncatedToInteger, this,
            hasSensibleDoubleToInt() ? operationToInt32SensibleSlow : operationToInt32, NeedToSpill, ExceptionCheckRequirement::CheckNotNeeded, gpr, fpr));
        
        int32Result(gpr, node);
        return;
    }
    
    case NumberUse:
    case NotCellUse: {
        switch (checkGeneratedTypeForToInt32(node->child1().node())) {
        case GeneratedOperandInteger: {
            SpeculateInt32Operand op1(this, node->child1(), ManualOperandSpeculation);
            GPRTemporary result(this, Reuse, op1);
            m_jit.move(op1.gpr(), result.gpr());
            int32Result(result.gpr(), node, op1.format());
            return;
        }
        case GeneratedOperandJSValue: {
            GPRTemporary result(this);
#if USE(JSVALUE64)
            JSValueOperand op1(this, node->child1(), ManualOperandSpeculation);

            GPRReg gpr = op1.gpr();
            GPRReg resultGpr = result.gpr();
            FPRTemporary tempFpr(this);
            FPRReg fpr = tempFpr.fpr();

            JITCompiler::Jump isInteger = m_jit.branch64(MacroAssembler::AboveOrEqual, gpr, GPRInfo::tagTypeNumberRegister);
            JITCompiler::JumpList converted;

            if (node->child1().useKind() == NumberUse) {
                DFG_TYPE_CHECK(
                    JSValueRegs(gpr), node->child1(), SpecBytecodeNumber,
                    m_jit.branchTest64(
                        MacroAssembler::Zero, gpr, GPRInfo::tagTypeNumberRegister));
            } else {
                JITCompiler::Jump isNumber = m_jit.branchTest64(MacroAssembler::NonZero, gpr, GPRInfo::tagTypeNumberRegister);
                
                DFG_TYPE_CHECK(
                    JSValueRegs(gpr), node->child1(), ~SpecCell, m_jit.branchIfCell(JSValueRegs(gpr)));
                
                // It's not a cell: so true turns into 1 and all else turns into 0.
                m_jit.compare64(JITCompiler::Equal, gpr, TrustedImm32(ValueTrue), resultGpr);
                converted.append(m_jit.jump());
                
                isNumber.link(&m_jit);
            }

            // First, if we get here we have a double encoded as a JSValue
            unboxDouble(gpr, resultGpr, fpr);

            silentSpillAllRegisters(resultGpr);
            callOperation(operationToInt32, resultGpr, fpr);
            silentFillAllRegisters(resultGpr);

            converted.append(m_jit.jump());

            isInteger.link(&m_jit);
            m_jit.zeroExtend32ToPtr(gpr, resultGpr);

            converted.link(&m_jit);
#else
            Node* childNode = node->child1().node();
            VirtualRegister virtualRegister = childNode->virtualRegister();
            GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);

            JSValueOperand op1(this, node->child1(), ManualOperandSpeculation);

            GPRReg payloadGPR = op1.payloadGPR();
            GPRReg resultGpr = result.gpr();
        
            JITCompiler::JumpList converted;

            if (info.registerFormat() == DataFormatJSInt32)
                m_jit.move(payloadGPR, resultGpr);
            else {
                GPRReg tagGPR = op1.tagGPR();
                FPRTemporary tempFpr(this);
                FPRReg fpr = tempFpr.fpr();
                FPRTemporary scratch(this);

                JITCompiler::Jump isInteger = m_jit.branch32(MacroAssembler::Equal, tagGPR, TrustedImm32(JSValue::Int32Tag));

                if (node->child1().useKind() == NumberUse) {
                    DFG_TYPE_CHECK(
                        op1.jsValueRegs(), node->child1(), SpecBytecodeNumber,
                        m_jit.branch32(
                            MacroAssembler::AboveOrEqual, tagGPR,
                            TrustedImm32(JSValue::LowestTag)));
                } else {
                    JITCompiler::Jump isNumber = m_jit.branch32(MacroAssembler::Below, tagGPR, TrustedImm32(JSValue::LowestTag));
                    
                    DFG_TYPE_CHECK(
                        op1.jsValueRegs(), node->child1(), ~SpecCell,
                        m_jit.branchIfCell(op1.jsValueRegs()));
                    
                    // It's not a cell: so true turns into 1 and all else turns into 0.
                    JITCompiler::Jump isBoolean = m_jit.branch32(JITCompiler::Equal, tagGPR, TrustedImm32(JSValue::BooleanTag));
                    m_jit.move(TrustedImm32(0), resultGpr);
                    converted.append(m_jit.jump());
                    
                    isBoolean.link(&m_jit);
                    m_jit.move(payloadGPR, resultGpr);
                    converted.append(m_jit.jump());
                    
                    isNumber.link(&m_jit);
                }

                unboxDouble(tagGPR, payloadGPR, fpr, scratch.fpr());

                silentSpillAllRegisters(resultGpr);
                callOperation(operationToInt32, resultGpr, fpr);
                silentFillAllRegisters(resultGpr);

                converted.append(m_jit.jump());

                isInteger.link(&m_jit);
                m_jit.move(payloadGPR, resultGpr);

                converted.link(&m_jit);
            }
#endif
            int32Result(resultGpr, node);
            return;
        }
        case GeneratedOperandTypeUnknown:
            RELEASE_ASSERT(!m_compileOkay);
            return;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
    
    default:
        ASSERT(!m_compileOkay);
        return;
    }
}

void SpeculativeJIT::compileUInt32ToNumber(Node* node)
{
    if (doesOverflow(node->arithMode())) {
        if (enableInt52()) {
            SpeculateInt32Operand op1(this, node->child1());
            GPRTemporary result(this, Reuse, op1);
            m_jit.zeroExtend32ToPtr(op1.gpr(), result.gpr());
            strictInt52Result(result.gpr(), node);
            return;
        }
        SpeculateInt32Operand op1(this, node->child1());
        FPRTemporary result(this);
            
        GPRReg inputGPR = op1.gpr();
        FPRReg outputFPR = result.fpr();
            
        m_jit.convertInt32ToDouble(inputGPR, outputFPR);
            
        JITCompiler::Jump positive = m_jit.branch32(MacroAssembler::GreaterThanOrEqual, inputGPR, TrustedImm32(0));
        m_jit.addDouble(JITCompiler::AbsoluteAddress(&AssemblyHelpers::twoToThe32), outputFPR);
        positive.link(&m_jit);
            
        doubleResult(outputFPR, node);
        return;
    }
    
    RELEASE_ASSERT(node->arithMode() == Arith::CheckOverflow);

    SpeculateInt32Operand op1(this, node->child1());
    GPRTemporary result(this);

    m_jit.move(op1.gpr(), result.gpr());

    speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::LessThan, result.gpr(), TrustedImm32(0)));

    int32Result(result.gpr(), node, op1.format());
}

void SpeculativeJIT::compileDoubleAsInt32(Node* node)
{
    SpeculateDoubleOperand op1(this, node->child1());
    FPRTemporary scratch(this);
    GPRTemporary result(this);
    
    FPRReg valueFPR = op1.fpr();
    FPRReg scratchFPR = scratch.fpr();
    GPRReg resultGPR = result.gpr();

    JITCompiler::JumpList failureCases;
    RELEASE_ASSERT(shouldCheckOverflow(node->arithMode()));
    m_jit.branchConvertDoubleToInt32(
        valueFPR, resultGPR, failureCases, scratchFPR,
        shouldCheckNegativeZero(node->arithMode()));
    speculationCheck(Overflow, JSValueRegs(), 0, failureCases);

    int32Result(resultGPR, node);
}

void SpeculativeJIT::compileDoubleRep(Node* node)
{
    switch (node->child1().useKind()) {
    case RealNumberUse: {
        JSValueOperand op1(this, node->child1(), ManualOperandSpeculation);
        FPRTemporary result(this);
        
        JSValueRegs op1Regs = op1.jsValueRegs();
        FPRReg resultFPR = result.fpr();
        
#if USE(JSVALUE64)
        GPRTemporary temp(this);
        GPRReg tempGPR = temp.gpr();
        m_jit.unboxDoubleWithoutAssertions(op1Regs.gpr(), tempGPR, resultFPR);
#else
        FPRTemporary temp(this);
        FPRReg tempFPR = temp.fpr();
        unboxDouble(op1Regs.tagGPR(), op1Regs.payloadGPR(), resultFPR, tempFPR);
#endif
        
        JITCompiler::Jump done = m_jit.branchDouble(
            JITCompiler::DoubleEqual, resultFPR, resultFPR);
        
        DFG_TYPE_CHECK(
            op1Regs, node->child1(), SpecBytecodeRealNumber, m_jit.branchIfNotInt32(op1Regs));
        m_jit.convertInt32ToDouble(op1Regs.payloadGPR(), resultFPR);
        
        done.link(&m_jit);
        
        doubleResult(resultFPR, node);
        return;
    }
    
    case NotCellUse:
    case NumberUse: {
        ASSERT(!node->child1()->isNumberConstant()); // This should have been constant folded.

        SpeculatedType possibleTypes = m_state.forNode(node->child1()).m_type;
        if (isInt32Speculation(possibleTypes)) {
            SpeculateInt32Operand op1(this, node->child1(), ManualOperandSpeculation);
            FPRTemporary result(this);
            m_jit.convertInt32ToDouble(op1.gpr(), result.fpr());
            doubleResult(result.fpr(), node);
            return;
        }

        JSValueOperand op1(this, node->child1(), ManualOperandSpeculation);
        FPRTemporary result(this);

#if USE(JSVALUE64)
        GPRTemporary temp(this);

        GPRReg op1GPR = op1.gpr();
        GPRReg tempGPR = temp.gpr();
        FPRReg resultFPR = result.fpr();
        JITCompiler::JumpList done;

        JITCompiler::Jump isInteger = m_jit.branch64(
            MacroAssembler::AboveOrEqual, op1GPR, GPRInfo::tagTypeNumberRegister);

        if (node->child1().useKind() == NotCellUse) {
            JITCompiler::Jump isNumber = m_jit.branchTest64(MacroAssembler::NonZero, op1GPR, GPRInfo::tagTypeNumberRegister);
            JITCompiler::Jump isUndefined = m_jit.branch64(JITCompiler::Equal, op1GPR, TrustedImm64(ValueUndefined));

            static const double zero = 0;
            m_jit.loadDouble(TrustedImmPtr(&zero), resultFPR);

            JITCompiler::Jump isNull = m_jit.branch64(JITCompiler::Equal, op1GPR, TrustedImm64(ValueNull));
            done.append(isNull);

            DFG_TYPE_CHECK(JSValueRegs(op1GPR), node->child1(), ~SpecCell,
                m_jit.branchTest64(JITCompiler::Zero, op1GPR, TrustedImm32(static_cast<int32_t>(TagBitBool))));

            JITCompiler::Jump isFalse = m_jit.branch64(JITCompiler::Equal, op1GPR, TrustedImm64(ValueFalse));
            static const double one = 1;
            m_jit.loadDouble(TrustedImmPtr(&one), resultFPR);
            done.append(m_jit.jump());
            done.append(isFalse);

            isUndefined.link(&m_jit);
            static const double NaN = PNaN;
            m_jit.loadDouble(TrustedImmPtr(&NaN), resultFPR);
            done.append(m_jit.jump());

            isNumber.link(&m_jit);
        } else if (needsTypeCheck(node->child1(), SpecBytecodeNumber)) {
            typeCheck(
                JSValueRegs(op1GPR), node->child1(), SpecBytecodeNumber,
                m_jit.branchTest64(MacroAssembler::Zero, op1GPR, GPRInfo::tagTypeNumberRegister));
        }

        unboxDouble(op1GPR, tempGPR, resultFPR);
        done.append(m_jit.jump());
    
        isInteger.link(&m_jit);
        m_jit.convertInt32ToDouble(op1GPR, resultFPR);
        done.link(&m_jit);
#else // USE(JSVALUE64) -> this is the 32_64 case
        FPRTemporary temp(this);
    
        GPRReg op1TagGPR = op1.tagGPR();
        GPRReg op1PayloadGPR = op1.payloadGPR();
        FPRReg tempFPR = temp.fpr();
        FPRReg resultFPR = result.fpr();
        JITCompiler::JumpList done;
    
        JITCompiler::Jump isInteger = m_jit.branch32(
            MacroAssembler::Equal, op1TagGPR, TrustedImm32(JSValue::Int32Tag));

        if (node->child1().useKind() == NotCellUse) {
            JITCompiler::Jump isNumber = m_jit.branch32(JITCompiler::Below, op1TagGPR, JITCompiler::TrustedImm32(JSValue::LowestTag + 1));
            JITCompiler::Jump isUndefined = m_jit.branch32(JITCompiler::Equal, op1TagGPR, TrustedImm32(JSValue::UndefinedTag));

            static const double zero = 0;
            m_jit.loadDouble(TrustedImmPtr(&zero), resultFPR);

            JITCompiler::Jump isNull = m_jit.branch32(JITCompiler::Equal, op1TagGPR, TrustedImm32(JSValue::NullTag));
            done.append(isNull);

            DFG_TYPE_CHECK(JSValueRegs(op1TagGPR, op1PayloadGPR), node->child1(), ~SpecCell, m_jit.branch32(JITCompiler::NotEqual, op1TagGPR, TrustedImm32(JSValue::BooleanTag)));

            JITCompiler::Jump isFalse = m_jit.branchTest32(JITCompiler::Zero, op1PayloadGPR, TrustedImm32(1));
            static const double one = 1;
            m_jit.loadDouble(TrustedImmPtr(&one), resultFPR);
            done.append(m_jit.jump());
            done.append(isFalse);

            isUndefined.link(&m_jit);
            static const double NaN = PNaN;
            m_jit.loadDouble(TrustedImmPtr(&NaN), resultFPR);
            done.append(m_jit.jump());

            isNumber.link(&m_jit);
        } else if (needsTypeCheck(node->child1(), SpecBytecodeNumber)) {
            typeCheck(
                JSValueRegs(op1TagGPR, op1PayloadGPR), node->child1(), SpecBytecodeNumber,
                m_jit.branch32(MacroAssembler::AboveOrEqual, op1TagGPR, TrustedImm32(JSValue::LowestTag)));
        }

        unboxDouble(op1TagGPR, op1PayloadGPR, resultFPR, tempFPR);
        done.append(m_jit.jump());
    
        isInteger.link(&m_jit);
        m_jit.convertInt32ToDouble(op1PayloadGPR, resultFPR);
        done.link(&m_jit);
#endif // USE(JSVALUE64)
    
        doubleResult(resultFPR, node);
        return;
    }
        
#if USE(JSVALUE64)
    case Int52RepUse: {
        SpeculateStrictInt52Operand value(this, node->child1());
        FPRTemporary result(this);
        
        GPRReg valueGPR = value.gpr();
        FPRReg resultFPR = result.fpr();

        m_jit.convertInt64ToDouble(valueGPR, resultFPR);
        
        doubleResult(resultFPR, node);
        return;
    }
#endif // USE(JSVALUE64)
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
}

void SpeculativeJIT::compileValueRep(Node* node)
{
    switch (node->child1().useKind()) {
    case DoubleRepUse: {
        SpeculateDoubleOperand value(this, node->child1());
        JSValueRegsTemporary result(this);
        
        FPRReg valueFPR = value.fpr();
        JSValueRegs resultRegs = result.regs();
        
        // It's very tempting to in-place filter the value to indicate that it's not impure NaN
        // anymore. Unfortunately, this would be unsound. If it's a GetLocal or if the value was
        // subject to a prior SetLocal, filtering the value would imply that the corresponding
        // local was purified.
        if (needsTypeCheck(node->child1(), ~SpecDoubleImpureNaN))
            m_jit.purifyNaN(valueFPR);

        boxDouble(valueFPR, resultRegs);
        
        jsValueResult(resultRegs, node);
        return;
    }
        
#if USE(JSVALUE64)
    case Int52RepUse: {
        SpeculateStrictInt52Operand value(this, node->child1());
        GPRTemporary result(this);
        
        GPRReg valueGPR = value.gpr();
        GPRReg resultGPR = result.gpr();
        
        boxInt52(valueGPR, resultGPR, DataFormatStrictInt52);
        
        jsValueResult(resultGPR, node);
        return;
    }
#endif // USE(JSVALUE64)
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
}

static double clampDoubleToByte(double d)
{
    d += 0.5;
    if (!(d > 0))
        d = 0;
    else if (d > 255)
        d = 255;
    return d;
}

static void compileClampIntegerToByte(JITCompiler& jit, GPRReg result)
{
    MacroAssembler::Jump inBounds = jit.branch32(MacroAssembler::BelowOrEqual, result, JITCompiler::TrustedImm32(0xff));
    MacroAssembler::Jump tooBig = jit.branch32(MacroAssembler::GreaterThan, result, JITCompiler::TrustedImm32(0xff));
    jit.xorPtr(result, result);
    MacroAssembler::Jump clamped = jit.jump();
    tooBig.link(&jit);
    jit.move(JITCompiler::TrustedImm32(255), result);
    clamped.link(&jit);
    inBounds.link(&jit);
}

static void compileClampDoubleToByte(JITCompiler& jit, GPRReg result, FPRReg source, FPRReg scratch)
{
    // Unordered compare so we pick up NaN
    static const double zero = 0;
    static const double byteMax = 255;
    static const double half = 0.5;
    jit.loadDouble(JITCompiler::TrustedImmPtr(&zero), scratch);
    MacroAssembler::Jump tooSmall = jit.branchDouble(MacroAssembler::DoubleLessThanOrEqualOrUnordered, source, scratch);
    jit.loadDouble(JITCompiler::TrustedImmPtr(&byteMax), scratch);
    MacroAssembler::Jump tooBig = jit.branchDouble(MacroAssembler::DoubleGreaterThan, source, scratch);
    
    jit.loadDouble(JITCompiler::TrustedImmPtr(&half), scratch);
    // FIXME: This should probably just use a floating point round!
    // https://bugs.webkit.org/show_bug.cgi?id=72054
    jit.addDouble(source, scratch);
    jit.truncateDoubleToInt32(scratch, result);   
    MacroAssembler::Jump truncatedInt = jit.jump();
    
    tooSmall.link(&jit);
    jit.xorPtr(result, result);
    MacroAssembler::Jump zeroed = jit.jump();
    
    tooBig.link(&jit);
    jit.move(JITCompiler::TrustedImm32(255), result);
    
    truncatedInt.link(&jit);
    zeroed.link(&jit);

}

JITCompiler::Jump SpeculativeJIT::jumpForTypedArrayOutOfBounds(Node* node, GPRReg baseGPR, GPRReg indexGPR)
{
    if (node->op() == PutByValAlias)
        return JITCompiler::Jump();
    JSArrayBufferView* view = m_jit.graph().tryGetFoldableView(
        m_state.forNode(m_jit.graph().child(node, 0)).m_value, node->arrayMode());
    if (view) {
        uint32_t length = view->length();
        Node* indexNode = m_jit.graph().child(node, 1).node();
        if (indexNode->isInt32Constant() && indexNode->asUInt32() < length)
            return JITCompiler::Jump();
        return m_jit.branch32(
            MacroAssembler::AboveOrEqual, indexGPR, MacroAssembler::Imm32(length));
    }
    return m_jit.branch32(
        MacroAssembler::AboveOrEqual, indexGPR,
        MacroAssembler::Address(baseGPR, JSArrayBufferView::offsetOfLength()));
}

void SpeculativeJIT::emitTypedArrayBoundsCheck(Node* node, GPRReg baseGPR, GPRReg indexGPR)
{
    JITCompiler::Jump jump = jumpForTypedArrayOutOfBounds(node, baseGPR, indexGPR);
    if (!jump.isSet())
        return;
    speculationCheck(OutOfBounds, JSValueRegs(), 0, jump);
}

JITCompiler::Jump SpeculativeJIT::jumpForTypedArrayIsNeuteredIfOutOfBounds(Node* node, GPRReg base, JITCompiler::Jump outOfBounds)
{
    JITCompiler::Jump done;
    if (outOfBounds.isSet()) {
        done = m_jit.jump();
        if (node->arrayMode().isInBounds())
            speculationCheck(OutOfBounds, JSValueSource(), 0, outOfBounds);
        else {
            outOfBounds.link(&m_jit);

            JITCompiler::Jump notWasteful = m_jit.branch32(
                MacroAssembler::NotEqual,
                MacroAssembler::Address(base, JSArrayBufferView::offsetOfMode()),
                TrustedImm32(WastefulTypedArray));

            JITCompiler::Jump hasNullVector = m_jit.branchTestPtr(
                MacroAssembler::Zero,
                MacroAssembler::Address(base, JSArrayBufferView::offsetOfVector()));
            speculationCheck(Uncountable, JSValueSource(), node, hasNullVector);
            notWasteful.link(&m_jit);
        }
    }
    return done;
}

void SpeculativeJIT::compileGetByValOnIntTypedArray(Node* node, TypedArrayType type)
{
    ASSERT(isInt(type));
    
    SpeculateCellOperand base(this, node->child1());
    SpeculateStrictInt32Operand property(this, node->child2());
    StorageOperand storage(this, node->child3());

    GPRReg baseReg = base.gpr();
    GPRReg propertyReg = property.gpr();
    GPRReg storageReg = storage.gpr();

    GPRTemporary result(this);
    GPRReg resultReg = result.gpr();

    ASSERT(node->arrayMode().alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1())));

    emitTypedArrayBoundsCheck(node, baseReg, propertyReg);
    switch (elementSize(type)) {
    case 1:
        if (isSigned(type))
            m_jit.load8SignedExtendTo32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesOne), resultReg);
        else
            m_jit.load8(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesOne), resultReg);
        break;
    case 2:
        if (isSigned(type))
            m_jit.load16SignedExtendTo32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesTwo), resultReg);
        else
            m_jit.load16(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesTwo), resultReg);
        break;
    case 4:
        m_jit.load32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesFour), resultReg);
        break;
    default:
        CRASH();
    }
    if (elementSize(type) < 4 || isSigned(type)) {
        int32Result(resultReg, node);
        return;
    }
    
    ASSERT(elementSize(type) == 4 && !isSigned(type));
    if (node->shouldSpeculateInt32()) {
        speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::LessThan, resultReg, TrustedImm32(0)));
        int32Result(resultReg, node);
        return;
    }
    
#if USE(JSVALUE64)
    if (node->shouldSpeculateAnyInt()) {
        m_jit.zeroExtend32ToPtr(resultReg, resultReg);
        strictInt52Result(resultReg, node);
        return;
    }
#endif
    
    FPRTemporary fresult(this);
    m_jit.convertInt32ToDouble(resultReg, fresult.fpr());
    JITCompiler::Jump positive = m_jit.branch32(MacroAssembler::GreaterThanOrEqual, resultReg, TrustedImm32(0));
    m_jit.addDouble(JITCompiler::AbsoluteAddress(&AssemblyHelpers::twoToThe32), fresult.fpr());
    positive.link(&m_jit);
    doubleResult(fresult.fpr(), node);
}

void SpeculativeJIT::compilePutByValForIntTypedArray(GPRReg base, GPRReg property, Node* node, TypedArrayType type)
{
    ASSERT(isInt(type));
    
    StorageOperand storage(this, m_jit.graph().varArgChild(node, 3));
    GPRReg storageReg = storage.gpr();
    
    Edge valueUse = m_jit.graph().varArgChild(node, 2);
    
    GPRTemporary value;
#if USE(JSVALUE32_64)
    GPRTemporary propertyTag;
    GPRTemporary valueTag;
#endif

    GPRReg valueGPR = InvalidGPRReg;
#if USE(JSVALUE32_64)
    GPRReg propertyTagGPR = InvalidGPRReg;
    GPRReg valueTagGPR = InvalidGPRReg;
#endif

    JITCompiler::JumpList slowPathCases;
    
    bool isAppropriateConstant = false;
    if (valueUse->isConstant()) {
        JSValue jsValue = valueUse->asJSValue();
        SpeculatedType expectedType = typeFilterFor(valueUse.useKind());
        SpeculatedType actualType = speculationFromValue(jsValue);
        isAppropriateConstant = (expectedType | actualType) == expectedType;
    }

    if (isAppropriateConstant) {
        JSValue jsValue = valueUse->asJSValue();
        if (!jsValue.isNumber()) {
            terminateSpeculativeExecution(Uncountable, JSValueRegs(), 0);
            noResult(node);
            return;
        }
        double d = jsValue.asNumber();
        if (isClamped(type)) {
            ASSERT(elementSize(type) == 1);
            d = clampDoubleToByte(d);
        }
        GPRTemporary scratch(this);
        GPRReg scratchReg = scratch.gpr();
        m_jit.move(Imm32(toInt32(d)), scratchReg);
        value.adopt(scratch);
        valueGPR = scratchReg;
    } else {
        switch (valueUse.useKind()) {
        case Int32Use: {
            SpeculateInt32Operand valueOp(this, valueUse);
            GPRTemporary scratch(this);
            GPRReg scratchReg = scratch.gpr();
            m_jit.move(valueOp.gpr(), scratchReg);
            if (isClamped(type)) {
                ASSERT(elementSize(type) == 1);
                compileClampIntegerToByte(m_jit, scratchReg);
            }
            value.adopt(scratch);
            valueGPR = scratchReg;
            break;
        }
            
#if USE(JSVALUE64)
        case Int52RepUse: {
            SpeculateStrictInt52Operand valueOp(this, valueUse);
            GPRTemporary scratch(this);
            GPRReg scratchReg = scratch.gpr();
            m_jit.move(valueOp.gpr(), scratchReg);
            if (isClamped(type)) {
                ASSERT(elementSize(type) == 1);
                MacroAssembler::Jump inBounds = m_jit.branch64(
                    MacroAssembler::BelowOrEqual, scratchReg, JITCompiler::TrustedImm64(0xff));
                MacroAssembler::Jump tooBig = m_jit.branch64(
                    MacroAssembler::GreaterThan, scratchReg, JITCompiler::TrustedImm64(0xff));
                m_jit.move(TrustedImm32(0), scratchReg);
                MacroAssembler::Jump clamped = m_jit.jump();
                tooBig.link(&m_jit);
                m_jit.move(JITCompiler::TrustedImm32(255), scratchReg);
                clamped.link(&m_jit);
                inBounds.link(&m_jit);
            }
            value.adopt(scratch);
            valueGPR = scratchReg;
            break;
        }
#endif // USE(JSVALUE64)
            
        case DoubleRepUse: {
            if (isClamped(type)) {
                ASSERT(elementSize(type) == 1);
                SpeculateDoubleOperand valueOp(this, valueUse);
                GPRTemporary result(this);
                FPRTemporary floatScratch(this);
                FPRReg fpr = valueOp.fpr();
                GPRReg gpr = result.gpr();
                compileClampDoubleToByte(m_jit, gpr, fpr, floatScratch.fpr());
                value.adopt(result);
                valueGPR = gpr;
            } else {
#if USE(JSVALUE32_64)
                GPRTemporary realPropertyTag(this);
                propertyTag.adopt(realPropertyTag);
                propertyTagGPR = propertyTag.gpr();

                GPRTemporary realValueTag(this);
                valueTag.adopt(realValueTag);
                valueTagGPR = valueTag.gpr();
#endif
                SpeculateDoubleOperand valueOp(this, valueUse);
                GPRTemporary result(this);
                FPRReg fpr = valueOp.fpr();
                GPRReg gpr = result.gpr();
                MacroAssembler::Jump notNaN = m_jit.branchDouble(MacroAssembler::DoubleEqual, fpr, fpr);
                m_jit.xorPtr(gpr, gpr);
                MacroAssembler::JumpList fixed(m_jit.jump());
                notNaN.link(&m_jit);

                fixed.append(m_jit.branchTruncateDoubleToInt32(
                    fpr, gpr, MacroAssembler::BranchIfTruncateSuccessful));

#if USE(JSVALUE64)
                m_jit.or64(GPRInfo::tagTypeNumberRegister, property);
                boxDouble(fpr, gpr);
#else
                m_jit.move(TrustedImm32(JSValue::Int32Tag), propertyTagGPR);
                boxDouble(fpr, valueTagGPR, gpr);
#endif
                slowPathCases.append(m_jit.jump());

                fixed.link(&m_jit);
                value.adopt(result);
                valueGPR = gpr;
            }
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
    }
    
    ASSERT_UNUSED(valueGPR, valueGPR != property);
    ASSERT(valueGPR != base);
    ASSERT(valueGPR != storageReg);
    JITCompiler::Jump outOfBounds = jumpForTypedArrayOutOfBounds(node, base, property);

    switch (elementSize(type)) {
    case 1:
        m_jit.store8(value.gpr(), MacroAssembler::BaseIndex(storageReg, property, MacroAssembler::TimesOne));
        break;
    case 2:
        m_jit.store16(value.gpr(), MacroAssembler::BaseIndex(storageReg, property, MacroAssembler::TimesTwo));
        break;
    case 4:
        m_jit.store32(value.gpr(), MacroAssembler::BaseIndex(storageReg, property, MacroAssembler::TimesFour));
        break;
    default:
        CRASH();
    }

    JITCompiler::Jump done = jumpForTypedArrayIsNeuteredIfOutOfBounds(node, base, outOfBounds);
    if (done.isSet())
        done.link(&m_jit);

    if (!slowPathCases.empty()) {
#if USE(JSVALUE64)
        if (node->op() == PutByValDirect) {
            addSlowPathGenerator(slowPathCall(
                slowPathCases, this,
                m_jit.isStrictModeFor(node->origin.semantic) ? operationPutByValDirectStrict : operationPutByValDirectNonStrict,
                NoResult, base, property, valueGPR));
        } else {
            addSlowPathGenerator(slowPathCall(
                slowPathCases, this,
                m_jit.isStrictModeFor(node->origin.semantic) ? operationPutByValStrict : operationPutByValNonStrict,
                NoResult, base, property, valueGPR));
        }
#else // not USE(JSVALUE64)
        if (node->op() == PutByValDirect) {
            addSlowPathGenerator(slowPathCall(
                slowPathCases, this,
                m_jit.codeBlock()->isStrictMode() ? operationPutByValDirectCellStrict : operationPutByValDirectCellNonStrict,
                NoResult, base, JSValueRegs(propertyTagGPR, property), JSValueRegs(valueTagGPR, valueGPR)));
        } else {
            addSlowPathGenerator(slowPathCall(
                slowPathCases, this,
                m_jit.codeBlock()->isStrictMode() ? operationPutByValCellStrict : operationPutByValCellNonStrict,
                NoResult, base, JSValueRegs(propertyTagGPR, property), JSValueRegs(valueTagGPR, valueGPR)));
        }
#endif
    }
    noResult(node);
}

void SpeculativeJIT::compileGetByValOnFloatTypedArray(Node* node, TypedArrayType type)
{
    ASSERT(isFloat(type));
    
    SpeculateCellOperand base(this, node->child1());
    SpeculateStrictInt32Operand property(this, node->child2());
    StorageOperand storage(this, node->child3());

    GPRReg baseReg = base.gpr();
    GPRReg propertyReg = property.gpr();
    GPRReg storageReg = storage.gpr();

    ASSERT(node->arrayMode().alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1())));

    FPRTemporary result(this);
    FPRReg resultReg = result.fpr();
    emitTypedArrayBoundsCheck(node, baseReg, propertyReg);
    switch (elementSize(type)) {
    case 4:
        m_jit.loadFloat(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesFour), resultReg);
        m_jit.convertFloatToDouble(resultReg, resultReg);
        break;
    case 8: {
        m_jit.loadDouble(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight), resultReg);
        break;
    }
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    doubleResult(resultReg, node);
}

void SpeculativeJIT::compilePutByValForFloatTypedArray(GPRReg base, GPRReg property, Node* node, TypedArrayType type)
{
    ASSERT(isFloat(type));
    
    StorageOperand storage(this, m_jit.graph().varArgChild(node, 3));
    GPRReg storageReg = storage.gpr();
    
    Edge baseUse = m_jit.graph().varArgChild(node, 0);
    Edge valueUse = m_jit.graph().varArgChild(node, 2);

    SpeculateDoubleOperand valueOp(this, valueUse);
    FPRTemporary scratch(this);
    FPRReg valueFPR = valueOp.fpr();
    FPRReg scratchFPR = scratch.fpr();

    ASSERT_UNUSED(baseUse, node->arrayMode().alreadyChecked(m_jit.graph(), node, m_state.forNode(baseUse)));
    
    MacroAssembler::Jump outOfBounds = jumpForTypedArrayOutOfBounds(node, base, property);
    
    switch (elementSize(type)) {
    case 4: {
        m_jit.moveDouble(valueFPR, scratchFPR);
        m_jit.convertDoubleToFloat(valueFPR, scratchFPR);
        m_jit.storeFloat(scratchFPR, MacroAssembler::BaseIndex(storageReg, property, MacroAssembler::TimesFour));
        break;
    }
    case 8:
        m_jit.storeDouble(valueFPR, MacroAssembler::BaseIndex(storageReg, property, MacroAssembler::TimesEight));
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }

    JITCompiler::Jump done = jumpForTypedArrayIsNeuteredIfOutOfBounds(node, base, outOfBounds);
    if (done.isSet())
        done.link(&m_jit);
    noResult(node);
}

void SpeculativeJIT::compileInstanceOfForObject(Node*, GPRReg valueReg, GPRReg prototypeReg, GPRReg scratchReg, GPRReg scratch2Reg)
{
    // Check that prototype is an object.
    speculationCheck(BadType, JSValueRegs(), 0, m_jit.branchIfNotObject(prototypeReg));
    
    // Initialize scratchReg with the value being checked.
    m_jit.move(valueReg, scratchReg);
    
    // Walk up the prototype chain of the value (in scratchReg), comparing to prototypeReg.
    MacroAssembler::Label loop(&m_jit);
    MacroAssembler::Jump performDefaultHasInstance = m_jit.branch8(MacroAssembler::Equal,
        MacroAssembler::Address(scratchReg, JSCell::typeInfoTypeOffset()), TrustedImm32(ProxyObjectType));
    m_jit.emitLoadStructure(scratchReg, scratchReg, scratch2Reg);
    m_jit.loadPtr(MacroAssembler::Address(scratchReg, Structure::prototypeOffset() + CellPayloadOffset), scratchReg);
    MacroAssembler::Jump isInstance = m_jit.branchPtr(MacroAssembler::Equal, scratchReg, prototypeReg);
#if USE(JSVALUE64)
    m_jit.branchIfCell(JSValueRegs(scratchReg)).linkTo(loop, &m_jit);
#else
    m_jit.branchTestPtr(MacroAssembler::NonZero, scratchReg).linkTo(loop, &m_jit);
#endif
    
    // No match - result is false.
#if USE(JSVALUE64)
    m_jit.move(MacroAssembler::TrustedImm64(JSValue::encode(jsBoolean(false))), scratchReg);
#else
    m_jit.move(MacroAssembler::TrustedImm32(0), scratchReg);
#endif
    MacroAssembler::JumpList doneJumps; 
    doneJumps.append(m_jit.jump());

    performDefaultHasInstance.link(&m_jit);
    silentSpillAllRegisters(scratchReg);
    callOperation(operationDefaultHasInstance, scratchReg, valueReg, prototypeReg); 
    silentFillAllRegisters(scratchReg);
    m_jit.exceptionCheck();
#if USE(JSVALUE64)
    m_jit.or32(TrustedImm32(ValueFalse), scratchReg);
#endif
    doneJumps.append(m_jit.jump());
    
    isInstance.link(&m_jit);
#if USE(JSVALUE64)
    m_jit.move(MacroAssembler::TrustedImm64(JSValue::encode(jsBoolean(true))), scratchReg);
#else
    m_jit.move(MacroAssembler::TrustedImm32(1), scratchReg);
#endif
    
    doneJumps.link(&m_jit);
}

void SpeculativeJIT::compileCheckTypeInfoFlags(Node* node)
{
    SpeculateCellOperand base(this, node->child1());

    GPRReg baseGPR = base.gpr();

    speculationCheck(BadTypeInfoFlags, JSValueRegs(), 0, m_jit.branchTest8(MacroAssembler::Zero, MacroAssembler::Address(baseGPR, JSCell::typeInfoFlagsOffset()), MacroAssembler::TrustedImm32(node->typeInfoOperand())));

    noResult(node);
}

void SpeculativeJIT::compileParseInt(Node* node)
{
    RELEASE_ASSERT(node->child1().useKind() == UntypedUse || node->child1().useKind() == StringUse);

    GPRFlushedCallResult resultPayload(this);
    GPRReg resultPayloadGPR = resultPayload.gpr();
#if USE(JSVALUE64)
    JSValueRegs resultRegs(resultPayloadGPR);
#else
    GPRFlushedCallResult2 resultTag(this);
    GPRReg resultTagGPR = resultTag.gpr();
    JSValueRegs resultRegs(resultTagGPR, resultPayloadGPR);
#endif

    if (node->child2()) {
        SpeculateInt32Operand radix(this, node->child2());
        GPRReg radixGPR = radix.gpr();
        if (node->child1().useKind() == UntypedUse) {
            JSValueOperand value(this, node->child1());

            flushRegisters();
#if USE(JSVALUE64)
            callOperation(operationParseIntGeneric, resultRegs.gpr(), value.gpr(), radixGPR);
#else
            callOperation(operationParseIntGeneric, resultRegs, value.jsValueRegs(), radixGPR);
#endif
            m_jit.exceptionCheck();
        } else {
            SpeculateCellOperand value(this, node->child1());
            GPRReg valueGPR = value.gpr();
            speculateString(node->child1(), valueGPR);

            flushRegisters();
#if USE(JSVALUE64)
            callOperation(operationParseIntString, resultRegs.gpr(), valueGPR, radixGPR);
#else
            callOperation(operationParseIntString, resultRegs, valueGPR, radixGPR);
#endif
            m_jit.exceptionCheck();
        }
    } else {
        if (node->child1().useKind() == UntypedUse) {
            JSValueOperand value(this, node->child1());

            flushRegisters();
#if USE(JSVALUE64)
            callOperation(operationParseIntNoRadixGeneric, resultRegs.gpr(), value.jsValueRegs());
#else
            callOperation(operationParseIntNoRadixGeneric, resultRegs, value.jsValueRegs());
#endif
            m_jit.exceptionCheck();
        } else {
            SpeculateCellOperand value(this, node->child1());
            GPRReg valueGPR = value.gpr();
            speculateString(node->child1(), valueGPR);

            flushRegisters();
            callOperation(operationParseIntStringNoRadix, resultRegs, valueGPR);
            m_jit.exceptionCheck();
        }
    }

    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileInstanceOf(Node* node)
{
    if (node->child1().useKind() == UntypedUse) {
        // It might not be a cell. Speculate less aggressively.
        // Or: it might only be used once (i.e. by us), so we get zero benefit
        // from speculating any more aggressively than we absolutely need to.
        
        JSValueOperand value(this, node->child1());
        SpeculateCellOperand prototype(this, node->child2());
        GPRTemporary scratch(this);
        GPRTemporary scratch2(this);
        
        GPRReg prototypeReg = prototype.gpr();
        GPRReg scratchReg = scratch.gpr();
        GPRReg scratch2Reg = scratch2.gpr();
        
        MacroAssembler::Jump isCell = m_jit.branchIfCell(value.jsValueRegs());
        GPRReg valueReg = value.jsValueRegs().payloadGPR();
        moveFalseTo(scratchReg);

        MacroAssembler::Jump done = m_jit.jump();
        
        isCell.link(&m_jit);
        
        compileInstanceOfForObject(node, valueReg, prototypeReg, scratchReg, scratch2Reg);
        
        done.link(&m_jit);

        blessedBooleanResult(scratchReg, node);
        return;
    }
    
    SpeculateCellOperand value(this, node->child1());
    SpeculateCellOperand prototype(this, node->child2());
    
    GPRTemporary scratch(this);
    GPRTemporary scratch2(this);
    
    GPRReg valueReg = value.gpr();
    GPRReg prototypeReg = prototype.gpr();
    GPRReg scratchReg = scratch.gpr();
    GPRReg scratch2Reg = scratch2.gpr();
    
    compileInstanceOfForObject(node, valueReg, prototypeReg, scratchReg, scratch2Reg);

    blessedBooleanResult(scratchReg, node);
}

template<typename SnippetGenerator, J_JITOperation_EJJ snippetSlowPathFunction>
void SpeculativeJIT::emitUntypedBitOp(Node* node)
{
    Edge& leftChild = node->child1();
    Edge& rightChild = node->child2();

    if (isKnownNotNumber(leftChild.node()) || isKnownNotNumber(rightChild.node())) {
        JSValueOperand left(this, leftChild);
        JSValueOperand right(this, rightChild);
        JSValueRegs leftRegs = left.jsValueRegs();
        JSValueRegs rightRegs = right.jsValueRegs();
#if USE(JSVALUE64)
        GPRTemporary result(this);
        JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
        GPRTemporary resultTag(this);
        GPRTemporary resultPayload(this);
        JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
#endif
        flushRegisters();
        callOperation(snippetSlowPathFunction, resultRegs, leftRegs, rightRegs);
        m_jit.exceptionCheck();

        jsValueResult(resultRegs, node);
        return;
    }

    std::optional<JSValueOperand> left;
    std::optional<JSValueOperand> right;

    JSValueRegs leftRegs;
    JSValueRegs rightRegs;

#if USE(JSVALUE64)
    GPRTemporary result(this);
    JSValueRegs resultRegs = JSValueRegs(result.gpr());
    GPRTemporary scratch(this);
    GPRReg scratchGPR = scratch.gpr();
#else
    GPRTemporary resultTag(this);
    GPRTemporary resultPayload(this);
    JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
    GPRReg scratchGPR = resultTag.gpr();
#endif

    SnippetOperand leftOperand;
    SnippetOperand rightOperand;

    // The snippet generator does not support both operands being constant. If the left
    // operand is already const, we'll ignore the right operand's constness.
    if (leftChild->isInt32Constant())
        leftOperand.setConstInt32(leftChild->asInt32());
    else if (rightChild->isInt32Constant())
        rightOperand.setConstInt32(rightChild->asInt32());

    RELEASE_ASSERT(!leftOperand.isConst() || !rightOperand.isConst());

    if (!leftOperand.isConst()) {
        left.emplace(this, leftChild);
        leftRegs = left->jsValueRegs();
    }
    if (!rightOperand.isConst()) {
        right.emplace(this, rightChild);
        rightRegs = right->jsValueRegs();
    }

    SnippetGenerator gen(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs, scratchGPR);
    gen.generateFastPath(m_jit);

    ASSERT(gen.didEmitFastPath());
    gen.endJumpList().append(m_jit.jump());

    gen.slowPathJumpList().link(&m_jit);
    silentSpillAllRegisters(resultRegs);

    if (leftOperand.isConst()) {
        leftRegs = resultRegs;
        m_jit.moveValue(leftChild->asJSValue(), leftRegs);
    } else if (rightOperand.isConst()) {
        rightRegs = resultRegs;
        m_jit.moveValue(rightChild->asJSValue(), rightRegs);
    }

    callOperation(snippetSlowPathFunction, resultRegs, leftRegs, rightRegs);

    silentFillAllRegisters(resultRegs);
    m_jit.exceptionCheck();

    gen.endJumpList().link(&m_jit);
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileBitwiseOp(Node* node)
{
    NodeType op = node->op();
    Edge& leftChild = node->child1();
    Edge& rightChild = node->child2();

    if (leftChild.useKind() == UntypedUse || rightChild.useKind() == UntypedUse) {
        switch (op) {
        case BitAnd:
            emitUntypedBitOp<JITBitAndGenerator, operationValueBitAnd>(node);
            return;
        case BitOr:
            emitUntypedBitOp<JITBitOrGenerator, operationValueBitOr>(node);
            return;
        case BitXor:
            emitUntypedBitOp<JITBitXorGenerator, operationValueBitXor>(node);
            return;
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    if (leftChild->isInt32Constant()) {
        SpeculateInt32Operand op2(this, rightChild);
        GPRTemporary result(this, Reuse, op2);

        bitOp(op, leftChild->asInt32(), op2.gpr(), result.gpr());

        int32Result(result.gpr(), node);

    } else if (rightChild->isInt32Constant()) {
        SpeculateInt32Operand op1(this, leftChild);
        GPRTemporary result(this, Reuse, op1);

        bitOp(op, rightChild->asInt32(), op1.gpr(), result.gpr());

        int32Result(result.gpr(), node);

    } else {
        SpeculateInt32Operand op1(this, leftChild);
        SpeculateInt32Operand op2(this, rightChild);
        GPRTemporary result(this, Reuse, op1, op2);
        
        GPRReg reg1 = op1.gpr();
        GPRReg reg2 = op2.gpr();
        bitOp(op, reg1, reg2, result.gpr());
        
        int32Result(result.gpr(), node);
    }
}

void SpeculativeJIT::emitUntypedRightShiftBitOp(Node* node)
{
    J_JITOperation_EJJ snippetSlowPathFunction = node->op() == BitRShift
        ? operationValueBitRShift : operationValueBitURShift;
    JITRightShiftGenerator::ShiftType shiftType = node->op() == BitRShift
        ? JITRightShiftGenerator::SignedShift : JITRightShiftGenerator::UnsignedShift;

    Edge& leftChild = node->child1();
    Edge& rightChild = node->child2();

    if (isKnownNotNumber(leftChild.node()) || isKnownNotNumber(rightChild.node())) {
        JSValueOperand left(this, leftChild);
        JSValueOperand right(this, rightChild);
        JSValueRegs leftRegs = left.jsValueRegs();
        JSValueRegs rightRegs = right.jsValueRegs();
#if USE(JSVALUE64)
        GPRTemporary result(this);
        JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
        GPRTemporary resultTag(this);
        GPRTemporary resultPayload(this);
        JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
#endif
        flushRegisters();
        callOperation(snippetSlowPathFunction, resultRegs, leftRegs, rightRegs);
        m_jit.exceptionCheck();

        jsValueResult(resultRegs, node);
        return;
    }

    std::optional<JSValueOperand> left;
    std::optional<JSValueOperand> right;

    JSValueRegs leftRegs;
    JSValueRegs rightRegs;

    FPRTemporary leftNumber(this);
    FPRReg leftFPR = leftNumber.fpr();

#if USE(JSVALUE64)
    GPRTemporary result(this);
    JSValueRegs resultRegs = JSValueRegs(result.gpr());
    GPRTemporary scratch(this);
    GPRReg scratchGPR = scratch.gpr();
    FPRReg scratchFPR = InvalidFPRReg;
#else
    GPRTemporary resultTag(this);
    GPRTemporary resultPayload(this);
    JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
    GPRReg scratchGPR = resultTag.gpr();
    FPRTemporary fprScratch(this);
    FPRReg scratchFPR = fprScratch.fpr();
#endif

    SnippetOperand leftOperand;
    SnippetOperand rightOperand;

    // The snippet generator does not support both operands being constant. If the left
    // operand is already const, we'll ignore the right operand's constness.
    if (leftChild->isInt32Constant())
        leftOperand.setConstInt32(leftChild->asInt32());
    else if (rightChild->isInt32Constant())
        rightOperand.setConstInt32(rightChild->asInt32());

    RELEASE_ASSERT(!leftOperand.isConst() || !rightOperand.isConst());

    if (!leftOperand.isConst()) {
        left.emplace(this, leftChild);
        leftRegs = left->jsValueRegs();
    }
    if (!rightOperand.isConst()) {
        right.emplace(this, rightChild);
        rightRegs = right->jsValueRegs();
    }

    JITRightShiftGenerator gen(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs,
        leftFPR, scratchGPR, scratchFPR, shiftType);
    gen.generateFastPath(m_jit);

    ASSERT(gen.didEmitFastPath());
    gen.endJumpList().append(m_jit.jump());

    gen.slowPathJumpList().link(&m_jit);
    silentSpillAllRegisters(resultRegs);

    if (leftOperand.isConst()) {
        leftRegs = resultRegs;
        m_jit.moveValue(leftChild->asJSValue(), leftRegs);
    } else if (rightOperand.isConst()) {
        rightRegs = resultRegs;
        m_jit.moveValue(rightChild->asJSValue(), rightRegs);
    }

    callOperation(snippetSlowPathFunction, resultRegs, leftRegs, rightRegs);

    silentFillAllRegisters(resultRegs);
    m_jit.exceptionCheck();

    gen.endJumpList().link(&m_jit);
    jsValueResult(resultRegs, node);
    return;
}

void SpeculativeJIT::compileShiftOp(Node* node)
{
    NodeType op = node->op();
    Edge& leftChild = node->child1();
    Edge& rightChild = node->child2();

    if (leftChild.useKind() == UntypedUse || rightChild.useKind() == UntypedUse) {
        switch (op) {
        case BitLShift:
            emitUntypedBitOp<JITLeftShiftGenerator, operationValueBitLShift>(node);
            return;
        case BitRShift:
        case BitURShift:
            emitUntypedRightShiftBitOp(node);
            return;
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    if (rightChild->isInt32Constant()) {
        SpeculateInt32Operand op1(this, leftChild);
        GPRTemporary result(this, Reuse, op1);

        shiftOp(op, op1.gpr(), rightChild->asInt32() & 0x1f, result.gpr());

        int32Result(result.gpr(), node);
    } else {
        // Do not allow shift amount to be used as the result, MacroAssembler does not permit this.
        SpeculateInt32Operand op1(this, leftChild);
        SpeculateInt32Operand op2(this, rightChild);
        GPRTemporary result(this, Reuse, op1);

        GPRReg reg1 = op1.gpr();
        GPRReg reg2 = op2.gpr();
        shiftOp(op, reg1, reg2, result.gpr());

        int32Result(result.gpr(), node);
    }
}

void SpeculativeJIT::compileValueAdd(Node* node)
{
    Edge& leftChild = node->child1();
    Edge& rightChild = node->child2();

    if (isKnownNotNumber(leftChild.node()) || isKnownNotNumber(rightChild.node())) {
        JSValueOperand left(this, leftChild);
        JSValueOperand right(this, rightChild);
        JSValueRegs leftRegs = left.jsValueRegs();
        JSValueRegs rightRegs = right.jsValueRegs();
#if USE(JSVALUE64)
        GPRTemporary result(this);
        JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
        GPRTemporary resultTag(this);
        GPRTemporary resultPayload(this);
        JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
#endif
        flushRegisters();
        callOperation(operationValueAddNotNumber, resultRegs, leftRegs, rightRegs);
        m_jit.exceptionCheck();
    
        jsValueResult(resultRegs, node);
        return;
    }

#if USE(JSVALUE64)
    bool needsScratchGPRReg = true;
    bool needsScratchFPRReg = false;
#else
    bool needsScratchGPRReg = true;
    bool needsScratchFPRReg = true;
#endif

    ArithProfile* arithProfile = m_jit.graph().baselineCodeBlockFor(node->origin.semantic)->arithProfileForBytecodeOffset(node->origin.semantic.bytecodeIndex);
    JITAddIC* addIC = m_jit.codeBlock()->addJITAddIC(arithProfile);
    auto repatchingFunction = operationValueAddOptimize;
    auto nonRepatchingFunction = operationValueAdd;
    
    compileMathIC(node, addIC, needsScratchGPRReg, needsScratchFPRReg, repatchingFunction, nonRepatchingFunction);
}

template <typename Generator, typename RepatchingFunction, typename NonRepatchingFunction>
void SpeculativeJIT::compileMathIC(Node* node, JITBinaryMathIC<Generator>* mathIC, bool needsScratchGPRReg, bool needsScratchFPRReg, RepatchingFunction repatchingFunction, NonRepatchingFunction nonRepatchingFunction)
{
    Edge& leftChild = node->child1();
    Edge& rightChild = node->child2();

    std::optional<JSValueOperand> left;
    std::optional<JSValueOperand> right;

    JSValueRegs leftRegs;
    JSValueRegs rightRegs;

    FPRTemporary leftNumber(this);
    FPRTemporary rightNumber(this);
    FPRReg leftFPR = leftNumber.fpr();
    FPRReg rightFPR = rightNumber.fpr();

    GPRReg scratchGPR = InvalidGPRReg;
    FPRReg scratchFPR = InvalidFPRReg;

    std::optional<FPRTemporary> fprScratch;
    if (needsScratchFPRReg) {
        fprScratch.emplace(this);
        scratchFPR = fprScratch->fpr();
    }

#if USE(JSVALUE64)
    std::optional<GPRTemporary> gprScratch;
    if (needsScratchGPRReg) {
        gprScratch.emplace(this);
        scratchGPR = gprScratch->gpr();
    }
    GPRTemporary result(this);
    JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
    GPRTemporary resultTag(this);
    GPRTemporary resultPayload(this);
    JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
    if (needsScratchGPRReg)
        scratchGPR = resultRegs.tagGPR();
#endif

    SnippetOperand leftOperand(m_state.forNode(leftChild).resultType());
    SnippetOperand rightOperand(m_state.forNode(rightChild).resultType());

    // The snippet generator does not support both operands being constant. If the left
    // operand is already const, we'll ignore the right operand's constness.
    if (leftChild->isInt32Constant())
        leftOperand.setConstInt32(leftChild->asInt32());
    else if (rightChild->isInt32Constant())
        rightOperand.setConstInt32(rightChild->asInt32());

    ASSERT(!leftOperand.isConst() || !rightOperand.isConst());
    ASSERT(!(Generator::isLeftOperandValidConstant(leftOperand) && Generator::isRightOperandValidConstant(rightOperand)));

    if (!Generator::isLeftOperandValidConstant(leftOperand)) {
        left.emplace(this, leftChild);
        leftRegs = left->jsValueRegs();
    }
    if (!Generator::isRightOperandValidConstant(rightOperand)) {
        right.emplace(this, rightChild);
        rightRegs = right->jsValueRegs();
    }

#if ENABLE(MATH_IC_STATS)
    auto inlineStart = m_jit.label();
#endif

    Box<MathICGenerationState> addICGenerationState = Box<MathICGenerationState>::create();
    mathIC->m_generator = Generator(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs, leftFPR, rightFPR, scratchGPR, scratchFPR);

    bool shouldEmitProfiling = false;
    bool generatedInline = mathIC->generateInline(m_jit, *addICGenerationState, shouldEmitProfiling);

    if (generatedInline) {
        ASSERT(!addICGenerationState->slowPathJumps.empty());

        Vector<SilentRegisterSavePlan> savePlans;
        silentSpillAllRegistersImpl(false, savePlans, resultRegs);

        auto done = m_jit.label();

        addSlowPathGenerator([=, savePlans = WTFMove(savePlans)] () {
            addICGenerationState->slowPathJumps.link(&m_jit);
            addICGenerationState->slowPathStart = m_jit.label();
#if ENABLE(MATH_IC_STATS)
            auto slowPathStart = m_jit.label();
#endif

            silentSpill(savePlans);

            auto innerLeftRegs = leftRegs;
            auto innerRightRegs = rightRegs;
            if (Generator::isLeftOperandValidConstant(leftOperand)) {
                innerLeftRegs = resultRegs;
                m_jit.moveValue(leftChild->asJSValue(), innerLeftRegs);
            } else if (Generator::isRightOperandValidConstant(rightOperand)) {
                innerRightRegs = resultRegs;
                m_jit.moveValue(rightChild->asJSValue(), innerRightRegs);
            }

            if (addICGenerationState->shouldSlowPathRepatch)
                addICGenerationState->slowPathCall = callOperation(bitwise_cast<J_JITOperation_EJJMic>(repatchingFunction), resultRegs, innerLeftRegs, innerRightRegs, TrustedImmPtr(mathIC));
            else
                addICGenerationState->slowPathCall = callOperation(nonRepatchingFunction, resultRegs, innerLeftRegs, innerRightRegs);

            silentFill(savePlans);
            m_jit.exceptionCheck();
            m_jit.jump().linkTo(done, &m_jit);

            m_jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
                mathIC->finalizeInlineCode(*addICGenerationState, linkBuffer);
            });

#if ENABLE(MATH_IC_STATS)
            auto slowPathEnd = m_jit.label();
            m_jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
                size_t size = static_cast<char*>(linkBuffer.locationOf(slowPathEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(slowPathStart).executableAddress());
                mathIC->m_generatedCodeSize += size;
            });
#endif

        });
    } else {
        if (Generator::isLeftOperandValidConstant(leftOperand)) {
            left.emplace(this, leftChild);
            leftRegs = left->jsValueRegs();
        } else if (Generator::isRightOperandValidConstant(rightOperand)) {
            right.emplace(this, rightChild);
            rightRegs = right->jsValueRegs();
        }

        flushRegisters();
        callOperation(nonRepatchingFunction, resultRegs, leftRegs, rightRegs);
        m_jit.exceptionCheck();
    }

#if ENABLE(MATH_IC_STATS)
    auto inlineEnd = m_jit.label();
    m_jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
        size_t size = static_cast<char*>(linkBuffer.locationOf(inlineEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(inlineStart).executableAddress());
        mathIC->m_generatedCodeSize += size;
    });
#endif

    jsValueResult(resultRegs, node);
    return;
}

void SpeculativeJIT::compileInstanceOfCustom(Node* node)
{
    // We could do something smarter here but this case is currently super rare and unless
    // Symbol.hasInstance becomes popular will likely remain that way.

    JSValueOperand value(this, node->child1());
    SpeculateCellOperand constructor(this, node->child2());
    JSValueOperand hasInstanceValue(this, node->child3());
    GPRTemporary result(this);

    JSValueRegs valueRegs = value.jsValueRegs();
    GPRReg constructorGPR = constructor.gpr();
    JSValueRegs hasInstanceRegs = hasInstanceValue.jsValueRegs();
    GPRReg resultGPR = result.gpr();

    MacroAssembler::Jump slowCase = m_jit.jump();

    addSlowPathGenerator(slowPathCall(slowCase, this, operationInstanceOfCustom, resultGPR, valueRegs, constructorGPR, hasInstanceRegs));

    unblessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compileIsCellWithType(Node* node)
{
    switch (node->child1().useKind()) {
    case UntypedUse: {
        JSValueOperand value(this, node->child1());
#if USE(JSVALUE64)
        GPRTemporary result(this, Reuse, value);
#else
        GPRTemporary result(this, Reuse, value, PayloadWord);
#endif

        JSValueRegs valueRegs = value.jsValueRegs();
        GPRReg resultGPR = result.gpr();

        JITCompiler::Jump isNotCell = m_jit.branchIfNotCell(valueRegs);

        m_jit.compare8(JITCompiler::Equal,
            JITCompiler::Address(valueRegs.payloadGPR(), JSCell::typeInfoTypeOffset()),
            TrustedImm32(node->queriedType()),
            resultGPR);
        blessBoolean(resultGPR);
        JITCompiler::Jump done = m_jit.jump();

        isNotCell.link(&m_jit);
        moveFalseTo(resultGPR);

        done.link(&m_jit);
        blessedBooleanResult(resultGPR, node);
        return;
    }

    case CellUse: {
        SpeculateCellOperand cell(this, node->child1());
        GPRTemporary result(this, Reuse, cell);

        GPRReg cellGPR = cell.gpr();
        GPRReg resultGPR = result.gpr();

        m_jit.compare8(JITCompiler::Equal,
            JITCompiler::Address(cellGPR, JSCell::typeInfoTypeOffset()),
            TrustedImm32(node->queriedType()),
            resultGPR);
        blessBoolean(resultGPR);
        blessedBooleanResult(resultGPR, node);
        return;
    }

    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

void SpeculativeJIT::compileIsTypedArrayView(Node* node)
{
    JSValueOperand value(this, node->child1());
#if USE(JSVALUE64)
    GPRTemporary result(this, Reuse, value);
#else
    GPRTemporary result(this, Reuse, value, PayloadWord);
#endif

    JSValueRegs valueRegs = value.jsValueRegs();
    GPRReg resultGPR = result.gpr();

    JITCompiler::Jump isNotCell = m_jit.branchIfNotCell(valueRegs);

    m_jit.load8(JITCompiler::Address(valueRegs.payloadGPR(), JSCell::typeInfoTypeOffset()), resultGPR);
    m_jit.sub32(TrustedImm32(Int8ArrayType), resultGPR);
    m_jit.compare32(JITCompiler::BelowOrEqual,
        resultGPR,
        TrustedImm32(Float64ArrayType - Int8ArrayType),
        resultGPR);
    blessBoolean(resultGPR);
    JITCompiler::Jump done = m_jit.jump();

    isNotCell.link(&m_jit);
    moveFalseTo(resultGPR);

    done.link(&m_jit);
    blessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compileCallObjectConstructor(Node* node)
{
    RELEASE_ASSERT(node->child1().useKind() == UntypedUse);
    JSValueOperand value(this, node->child1());
#if USE(JSVALUE64)
    GPRTemporary result(this, Reuse, value);
#else
    GPRTemporary result(this, Reuse, value, PayloadWord);
#endif

    JSValueRegs valueRegs = value.jsValueRegs();
    GPRReg resultGPR = result.gpr();

    MacroAssembler::JumpList slowCases;
    slowCases.append(m_jit.branchIfNotCell(valueRegs));
    slowCases.append(m_jit.branchIfNotObject(valueRegs.payloadGPR()));
    m_jit.move(valueRegs.payloadGPR(), resultGPR);

    addSlowPathGenerator(slowPathCall(slowCases, this, operationObjectConstructor, resultGPR, m_jit.globalObjectFor(node->origin.semantic), valueRegs));
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileArithAdd(Node* node)
{
    switch (node->binaryUseKind()) {
    case Int32Use: {
        ASSERT(!shouldCheckNegativeZero(node->arithMode()));

        if (node->child2()->isInt32Constant()) {
            SpeculateInt32Operand op1(this, node->child1());
            GPRTemporary result(this, Reuse, op1);

            GPRReg gpr1 = op1.gpr();
            int32_t imm2 = node->child2()->asInt32();
            GPRReg gprResult = result.gpr();

            if (!shouldCheckOverflow(node->arithMode())) {
                m_jit.add32(Imm32(imm2), gpr1, gprResult);
                int32Result(gprResult, node);
                return;
            }

            MacroAssembler::Jump check = m_jit.branchAdd32(MacroAssembler::Overflow, gpr1, Imm32(imm2), gprResult);
            if (gpr1 == gprResult) {
                speculationCheck(Overflow, JSValueRegs(), 0, check,
                    SpeculationRecovery(SpeculativeAddImmediate, gpr1, imm2));
            } else
                speculationCheck(Overflow, JSValueRegs(), 0, check);

            int32Result(gprResult, node);
            return;
        }
                
        SpeculateInt32Operand op1(this, node->child1());
        SpeculateInt32Operand op2(this, node->child2());
        GPRTemporary result(this, Reuse, op1, op2);

        GPRReg gpr1 = op1.gpr();
        GPRReg gpr2 = op2.gpr();
        GPRReg gprResult = result.gpr();

        if (!shouldCheckOverflow(node->arithMode()))
            m_jit.add32(gpr1, gpr2, gprResult);
        else {
            MacroAssembler::Jump check = m_jit.branchAdd32(MacroAssembler::Overflow, gpr1, gpr2, gprResult);
                
            if (gpr1 == gprResult)
                speculationCheck(Overflow, JSValueRegs(), 0, check, SpeculationRecovery(SpeculativeAdd, gprResult, gpr2));
            else if (gpr2 == gprResult)
                speculationCheck(Overflow, JSValueRegs(), 0, check, SpeculationRecovery(SpeculativeAdd, gprResult, gpr1));
            else
                speculationCheck(Overflow, JSValueRegs(), 0, check);
        }

        int32Result(gprResult, node);
        return;
    }
        
#if USE(JSVALUE64)
    case Int52RepUse: {
        ASSERT(shouldCheckOverflow(node->arithMode()));
        ASSERT(!shouldCheckNegativeZero(node->arithMode()));

        // Will we need an overflow check? If we can prove that neither input can be
        // Int52 then the overflow check will not be necessary.
        if (!m_state.forNode(node->child1()).couldBeType(SpecInt52Only)
            && !m_state.forNode(node->child2()).couldBeType(SpecInt52Only)) {
            SpeculateWhicheverInt52Operand op1(this, node->child1());
            SpeculateWhicheverInt52Operand op2(this, node->child2(), op1);
            GPRTemporary result(this, Reuse, op1);
            m_jit.add64(op1.gpr(), op2.gpr(), result.gpr());
            int52Result(result.gpr(), node, op1.format());
            return;
        }
        
        SpeculateInt52Operand op1(this, node->child1());
        SpeculateInt52Operand op2(this, node->child2());
        GPRTemporary result(this);
        m_jit.move(op1.gpr(), result.gpr());
        speculationCheck(
            Int52Overflow, JSValueRegs(), 0,
            m_jit.branchAdd64(MacroAssembler::Overflow, op2.gpr(), result.gpr()));
        int52Result(result.gpr(), node);
        return;
    }
#endif // USE(JSVALUE64)
    
    case DoubleRepUse: {
        SpeculateDoubleOperand op1(this, node->child1());
        SpeculateDoubleOperand op2(this, node->child2());
        FPRTemporary result(this, op1, op2);

        FPRReg reg1 = op1.fpr();
        FPRReg reg2 = op2.fpr();
        m_jit.addDouble(reg1, reg2, result.fpr());

        doubleResult(result.fpr(), node);
        return;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

void SpeculativeJIT::compileMakeRope(Node* node)
{
    ASSERT(node->child1().useKind() == KnownStringUse);
    ASSERT(node->child2().useKind() == KnownStringUse);
    ASSERT(!node->child3() || node->child3().useKind() == KnownStringUse);
    
    SpeculateCellOperand op1(this, node->child1());
    SpeculateCellOperand op2(this, node->child2());
    SpeculateCellOperand op3(this, node->child3());
    GPRTemporary result(this);
    GPRTemporary allocator(this);
    GPRTemporary scratch(this);
    
    GPRReg opGPRs[3];
    unsigned numOpGPRs;
    opGPRs[0] = op1.gpr();
    opGPRs[1] = op2.gpr();
    if (node->child3()) {
        opGPRs[2] = op3.gpr();
        numOpGPRs = 3;
    } else {
        opGPRs[2] = InvalidGPRReg;
        numOpGPRs = 2;
    }
    GPRReg resultGPR = result.gpr();
    GPRReg allocatorGPR = allocator.gpr();
    GPRReg scratchGPR = scratch.gpr();
    
    JITCompiler::JumpList slowPath;
    MarkedAllocator* markedAllocator = subspaceFor<JSString>(*m_jit.vm())->allocatorFor(sizeof(JSRopeString));
    RELEASE_ASSERT(markedAllocator);
    m_jit.move(TrustedImmPtr(markedAllocator), allocatorGPR);
    emitAllocateJSCell(resultGPR, markedAllocator, allocatorGPR, TrustedImmPtr(m_jit.graph().registerStructure(m_jit.vm()->stringStructure.get())), scratchGPR, slowPath);
        
    m_jit.storePtr(TrustedImmPtr(0), JITCompiler::Address(resultGPR, JSString::offsetOfValue()));
    for (unsigned i = 0; i < numOpGPRs; ++i)
        m_jit.storePtr(opGPRs[i], JITCompiler::Address(resultGPR, JSRopeString::offsetOfFibers() + sizeof(WriteBarrier<JSString>) * i));
    for (unsigned i = numOpGPRs; i < JSRopeString::s_maxInternalRopeLength; ++i)
        m_jit.storePtr(TrustedImmPtr(0), JITCompiler::Address(resultGPR, JSRopeString::offsetOfFibers() + sizeof(WriteBarrier<JSString>) * i));
    m_jit.load32(JITCompiler::Address(opGPRs[0], JSString::offsetOfFlags()), scratchGPR);
    m_jit.load32(JITCompiler::Address(opGPRs[0], JSString::offsetOfLength()), allocatorGPR);
    if (!ASSERT_DISABLED) {
        JITCompiler::Jump ok = m_jit.branch32(
            JITCompiler::GreaterThanOrEqual, allocatorGPR, TrustedImm32(0));
        m_jit.abortWithReason(DFGNegativeStringLength);
        ok.link(&m_jit);
    }
    for (unsigned i = 1; i < numOpGPRs; ++i) {
        m_jit.and32(JITCompiler::Address(opGPRs[i], JSString::offsetOfFlags()), scratchGPR);
        speculationCheck(
            Uncountable, JSValueSource(), nullptr,
            m_jit.branchAdd32(
                JITCompiler::Overflow,
                JITCompiler::Address(opGPRs[i], JSString::offsetOfLength()), allocatorGPR));
    }
    m_jit.and32(JITCompiler::TrustedImm32(JSString::Is8Bit), scratchGPR);
    m_jit.store32(scratchGPR, JITCompiler::Address(resultGPR, JSString::offsetOfFlags()));
    if (!ASSERT_DISABLED) {
        JITCompiler::Jump ok = m_jit.branch32(
            JITCompiler::GreaterThanOrEqual, allocatorGPR, TrustedImm32(0));
        m_jit.abortWithReason(DFGNegativeStringLength);
        ok.link(&m_jit);
    }
    m_jit.store32(allocatorGPR, JITCompiler::Address(resultGPR, JSString::offsetOfLength()));
    
    m_jit.mutatorFence();
    
    switch (numOpGPRs) {
    case 2:
        addSlowPathGenerator(slowPathCall(
            slowPath, this, operationMakeRope2, resultGPR, opGPRs[0], opGPRs[1]));
        break;
    case 3:
        addSlowPathGenerator(slowPathCall(
            slowPath, this, operationMakeRope3, resultGPR, opGPRs[0], opGPRs[1], opGPRs[2]));
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
    
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileArithAbs(Node* node)
{
    switch (node->child1().useKind()) {
    case Int32Use: {
        SpeculateStrictInt32Operand op1(this, node->child1());
        GPRTemporary result(this, Reuse, op1);
        GPRTemporary scratch(this);

        m_jit.move(op1.gpr(), result.gpr());
        m_jit.rshift32(result.gpr(), MacroAssembler::TrustedImm32(31), scratch.gpr());
        m_jit.add32(scratch.gpr(), result.gpr());
        m_jit.xor32(scratch.gpr(), result.gpr());
        if (shouldCheckOverflow(node->arithMode()))
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(MacroAssembler::Signed, result.gpr()));
        int32Result(result.gpr(), node);
        break;
    }

    case DoubleRepUse: {
        SpeculateDoubleOperand op1(this, node->child1());
        FPRTemporary result(this);

        m_jit.absDouble(op1.fpr(), result.fpr());
        doubleResult(result.fpr(), node);
        break;
    }

    default: {
        DFG_ASSERT(m_jit.graph(), node, node->child1().useKind() == UntypedUse);
        JSValueOperand op1(this, node->child1());
        JSValueRegs op1Regs = op1.jsValueRegs();
        flushRegisters();
        FPRResult result(this);
        callOperation(operationArithAbs, result.fpr(), op1Regs);
        m_jit.exceptionCheck();
        doubleResult(result.fpr(), node);
        break;
    }
    }
}

void SpeculativeJIT::compileArithClz32(Node* node)
{
    if (node->child1().useKind() == Int32Use || node->child1().useKind() == KnownInt32Use) {
        SpeculateInt32Operand value(this, node->child1());
        GPRTemporary result(this, Reuse, value);
        GPRReg valueReg = value.gpr();
        GPRReg resultReg = result.gpr();
        m_jit.countLeadingZeros32(valueReg, resultReg);
        int32Result(resultReg, node);
        return;
    }
    JSValueOperand op1(this, node->child1());
    JSValueRegs op1Regs = op1.jsValueRegs();
    GPRTemporary result(this);
    GPRReg resultReg = result.gpr();
    flushRegisters();
    callOperation(operationArithClz32, resultReg, op1Regs);
    m_jit.exceptionCheck();
    int32Result(resultReg, node);
}

void SpeculativeJIT::compileArithDoubleUnaryOp(Node* node, double (*doubleFunction)(double), double (*operation)(ExecState*, EncodedJSValue))
{
    if (node->child1().useKind() == DoubleRepUse) {
        SpeculateDoubleOperand op1(this, node->child1());
        FPRReg op1FPR = op1.fpr();

        flushRegisters();

        FPRResult result(this);
        callOperation(doubleFunction, result.fpr(), op1FPR);
        doubleResult(result.fpr(), node);
        return;
    }

    JSValueOperand op1(this, node->child1());
    JSValueRegs op1Regs = op1.jsValueRegs();
    flushRegisters();
    FPRResult result(this);
    callOperation(operation, result.fpr(), op1Regs);
    m_jit.exceptionCheck();
    doubleResult(result.fpr(), node);
}

void SpeculativeJIT::compileArithCos(Node* node)
{
    compileArithDoubleUnaryOp(node, cos, operationArithCos);
}

void SpeculativeJIT::compileArithTan(Node* node)
{
    compileArithDoubleUnaryOp(node, tan, operationArithTan);
}

void SpeculativeJIT::compileArithSub(Node* node)
{
    switch (node->binaryUseKind()) {
    case Int32Use: {
        ASSERT(!shouldCheckNegativeZero(node->arithMode()));
        
        if (node->child2()->isInt32Constant()) {
            SpeculateInt32Operand op1(this, node->child1());
            int32_t imm2 = node->child2()->asInt32();
            GPRTemporary result(this);

            if (!shouldCheckOverflow(node->arithMode())) {
                m_jit.move(op1.gpr(), result.gpr());
                m_jit.sub32(Imm32(imm2), result.gpr());
            } else {
                GPRTemporary scratch(this);
                speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchSub32(MacroAssembler::Overflow, op1.gpr(), Imm32(imm2), result.gpr(), scratch.gpr()));
            }

            int32Result(result.gpr(), node);
            return;
        }
            
        if (node->child1()->isInt32Constant()) {
            int32_t imm1 = node->child1()->asInt32();
            SpeculateInt32Operand op2(this, node->child2());
            GPRTemporary result(this);
                
            m_jit.move(Imm32(imm1), result.gpr());
            if (!shouldCheckOverflow(node->arithMode()))
                m_jit.sub32(op2.gpr(), result.gpr());
            else
                speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchSub32(MacroAssembler::Overflow, op2.gpr(), result.gpr()));
                
            int32Result(result.gpr(), node);
            return;
        }
            
        SpeculateInt32Operand op1(this, node->child1());
        SpeculateInt32Operand op2(this, node->child2());
        GPRTemporary result(this);

        if (!shouldCheckOverflow(node->arithMode())) {
            m_jit.move(op1.gpr(), result.gpr());
            m_jit.sub32(op2.gpr(), result.gpr());
        } else
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchSub32(MacroAssembler::Overflow, op1.gpr(), op2.gpr(), result.gpr()));

        int32Result(result.gpr(), node);
        return;
    }
        
#if USE(JSVALUE64)
    case Int52RepUse: {
        ASSERT(shouldCheckOverflow(node->arithMode()));
        ASSERT(!shouldCheckNegativeZero(node->arithMode()));

        // Will we need an overflow check? If we can prove that neither input can be
        // Int52 then the overflow check will not be necessary.
        if (!m_state.forNode(node->child1()).couldBeType(SpecInt52Only)
            && !m_state.forNode(node->child2()).couldBeType(SpecInt52Only)) {
            SpeculateWhicheverInt52Operand op1(this, node->child1());
            SpeculateWhicheverInt52Operand op2(this, node->child2(), op1);
            GPRTemporary result(this, Reuse, op1);
            m_jit.move(op1.gpr(), result.gpr());
            m_jit.sub64(op2.gpr(), result.gpr());
            int52Result(result.gpr(), node, op1.format());
            return;
        }
        
        SpeculateInt52Operand op1(this, node->child1());
        SpeculateInt52Operand op2(this, node->child2());
        GPRTemporary result(this);
        m_jit.move(op1.gpr(), result.gpr());
        speculationCheck(
            Int52Overflow, JSValueRegs(), 0,
            m_jit.branchSub64(MacroAssembler::Overflow, op2.gpr(), result.gpr()));
        int52Result(result.gpr(), node);
        return;
    }
#endif // USE(JSVALUE64)

    case DoubleRepUse: {
        SpeculateDoubleOperand op1(this, node->child1());
        SpeculateDoubleOperand op2(this, node->child2());
        FPRTemporary result(this, op1);

        FPRReg reg1 = op1.fpr();
        FPRReg reg2 = op2.fpr();
        m_jit.subDouble(reg1, reg2, result.fpr());

        doubleResult(result.fpr(), node);
        return;
    }

    case UntypedUse: {
#if USE(JSVALUE64)
        bool needsScratchGPRReg = true;
        bool needsScratchFPRReg = false;
#else
        bool needsScratchGPRReg = true;
        bool needsScratchFPRReg = true;
#endif

        ArithProfile* arithProfile = m_jit.graph().baselineCodeBlockFor(node->origin.semantic)->arithProfileForBytecodeOffset(node->origin.semantic.bytecodeIndex);
        JITSubIC* subIC = m_jit.codeBlock()->addJITSubIC(arithProfile);
        auto repatchingFunction = operationValueSubOptimize;
        auto nonRepatchingFunction = operationValueSub;
        
        compileMathIC(node, subIC, needsScratchGPRReg, needsScratchFPRReg, repatchingFunction, nonRepatchingFunction);
        return;
    }

    default:
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
}

void SpeculativeJIT::compileArithNegate(Node* node)
{
    switch (node->child1().useKind()) {
    case Int32Use: {
        SpeculateInt32Operand op1(this, node->child1());
        GPRTemporary result(this);

        m_jit.move(op1.gpr(), result.gpr());

        // Note: there is no notion of being not used as a number, but someone
        // caring about negative zero.
        
        if (!shouldCheckOverflow(node->arithMode()))
            m_jit.neg32(result.gpr());
        else if (!shouldCheckNegativeZero(node->arithMode()))
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchNeg32(MacroAssembler::Overflow, result.gpr()));
        else {
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(MacroAssembler::Zero, result.gpr(), TrustedImm32(0x7fffffff)));
            m_jit.neg32(result.gpr());
        }

        int32Result(result.gpr(), node);
        return;
    }

#if USE(JSVALUE64)
    case Int52RepUse: {
        ASSERT(shouldCheckOverflow(node->arithMode()));
        
        if (!m_state.forNode(node->child1()).couldBeType(SpecInt52Only)) {
            SpeculateWhicheverInt52Operand op1(this, node->child1());
            GPRTemporary result(this);
            GPRReg op1GPR = op1.gpr();
            GPRReg resultGPR = result.gpr();
            m_jit.move(op1GPR, resultGPR);
            m_jit.neg64(resultGPR);
            if (shouldCheckNegativeZero(node->arithMode())) {
                speculationCheck(
                    NegativeZero, JSValueRegs(), 0,
                    m_jit.branchTest64(MacroAssembler::Zero, resultGPR));
            }
            int52Result(resultGPR, node, op1.format());
            return;
        }
        
        SpeculateInt52Operand op1(this, node->child1());
        GPRTemporary result(this);
        GPRReg op1GPR = op1.gpr();
        GPRReg resultGPR = result.gpr();
        m_jit.move(op1GPR, resultGPR);
        speculationCheck(
            Int52Overflow, JSValueRegs(), 0,
            m_jit.branchNeg64(MacroAssembler::Overflow, resultGPR));
        if (shouldCheckNegativeZero(node->arithMode())) {
            speculationCheck(
                NegativeZero, JSValueRegs(), 0,
                m_jit.branchTest64(MacroAssembler::Zero, resultGPR));
        }
        int52Result(resultGPR, node);
        return;
    }
#endif // USE(JSVALUE64)
        
    case DoubleRepUse: {
        SpeculateDoubleOperand op1(this, node->child1());
        FPRTemporary result(this);
        
        m_jit.negateDouble(op1.fpr(), result.fpr());
        
        doubleResult(result.fpr(), node);
        return;
    }
        
    default: {
        ArithProfile* arithProfile = m_jit.graph().baselineCodeBlockFor(node->origin.semantic)->arithProfileForBytecodeOffset(node->origin.semantic.bytecodeIndex);
        JITNegIC* negIC = m_jit.codeBlock()->addJITNegIC(arithProfile);
        auto repatchingFunction = operationArithNegateOptimize;
        auto nonRepatchingFunction = operationArithNegate;
        bool needsScratchGPRReg = true;
        compileMathIC(node, negIC, needsScratchGPRReg, repatchingFunction, nonRepatchingFunction);
        return;
    }
    }
}

template <typename Generator, typename RepatchingFunction, typename NonRepatchingFunction>
void SpeculativeJIT::compileMathIC(Node* node, JITUnaryMathIC<Generator>* mathIC, bool needsScratchGPRReg, RepatchingFunction repatchingFunction, NonRepatchingFunction nonRepatchingFunction)
{
    GPRReg scratchGPR = InvalidGPRReg;
    std::optional<GPRTemporary> gprScratch;
    if (needsScratchGPRReg) {
        gprScratch.emplace(this);
        scratchGPR = gprScratch->gpr();
    }
    JSValueOperand childOperand(this, node->child1());
    JSValueRegs childRegs = childOperand.jsValueRegs();
#if USE(JSVALUE64)
    GPRTemporary result(this, Reuse, childOperand);
    JSValueRegs resultRegs(result.gpr());
#else
    GPRTemporary resultTag(this);
    GPRTemporary resultPayload(this);
    JSValueRegs resultRegs(resultPayload.gpr(), resultTag.gpr());
#endif

#if ENABLE(MATH_IC_STATS)
    auto inlineStart = m_jit.label();
#endif

    Box<MathICGenerationState> icGenerationState = Box<MathICGenerationState>::create();
    mathIC->m_generator = Generator(resultRegs, childRegs, scratchGPR);

    bool shouldEmitProfiling = false;
    bool generatedInline = mathIC->generateInline(m_jit, *icGenerationState, shouldEmitProfiling);

    if (generatedInline) {
        ASSERT(!icGenerationState->slowPathJumps.empty());

        Vector<SilentRegisterSavePlan> savePlans;
        silentSpillAllRegistersImpl(false, savePlans, resultRegs);

        auto done = m_jit.label();

        addSlowPathGenerator([=, savePlans = WTFMove(savePlans)] () {
            icGenerationState->slowPathJumps.link(&m_jit);
            icGenerationState->slowPathStart = m_jit.label();
#if ENABLE(MATH_IC_STATS)
            auto slowPathStart = m_jit.label();
#endif

            silentSpill(savePlans);

            if (icGenerationState->shouldSlowPathRepatch)
                icGenerationState->slowPathCall = callOperation(bitwise_cast<J_JITOperation_EJMic>(repatchingFunction), resultRegs, childRegs, TrustedImmPtr(mathIC));
            else
                icGenerationState->slowPathCall = callOperation(nonRepatchingFunction, resultRegs, childRegs);

            silentFill(savePlans);
            m_jit.exceptionCheck();
            m_jit.jump().linkTo(done, &m_jit);

            m_jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
                mathIC->finalizeInlineCode(*icGenerationState, linkBuffer);
            });

#if ENABLE(MATH_IC_STATS)
            auto slowPathEnd = m_jit.label();
            m_jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
                size_t size = static_cast<char*>(linkBuffer.locationOf(slowPathEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(slowPathStart).executableAddress());
                mathIC->m_generatedCodeSize += size;
            });
#endif

        });
    } else {
        flushRegisters();
        callOperation(nonRepatchingFunction, resultRegs, childRegs);
        m_jit.exceptionCheck();
    }

#if ENABLE(MATH_IC_STATS)
    auto inlineEnd = m_jit.label();
    m_jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
        size_t size = static_cast<char*>(linkBuffer.locationOf(inlineEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(inlineStart).executableAddress());
        mathIC->m_generatedCodeSize += size;
    });
#endif

    jsValueResult(resultRegs, node);
    return;
}


void SpeculativeJIT::compileArithMul(Node* node)
{
    switch (node->binaryUseKind()) {
    case Int32Use: {
        if (node->child2()->isInt32Constant()) {
            SpeculateInt32Operand op1(this, node->child1());
            GPRTemporary result(this);

            int32_t imm = node->child2()->asInt32();
            GPRReg op1GPR = op1.gpr();
            GPRReg resultGPR = result.gpr();

            if (!shouldCheckOverflow(node->arithMode()))
                m_jit.mul32(Imm32(imm), op1GPR, resultGPR);
            else {
                speculationCheck(Overflow, JSValueRegs(), 0,
                    m_jit.branchMul32(MacroAssembler::Overflow, op1GPR, Imm32(imm), resultGPR));
            }

            // The only way to create negative zero with a constant is:
            // -negative-op1 * 0.
            // -zero-op1 * negative constant.
            if (shouldCheckNegativeZero(node->arithMode())) {
                if (!imm)
                    speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branchTest32(MacroAssembler::Signed, op1GPR));
                else if (imm < 0) {
                    if (shouldCheckOverflow(node->arithMode()))
                        speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branchTest32(MacroAssembler::Zero, resultGPR));
                    else
                        speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branchTest32(MacroAssembler::Zero, op1GPR));
                }
            }

            int32Result(resultGPR, node);
            return;
        }
        SpeculateInt32Operand op1(this, node->child1());
        SpeculateInt32Operand op2(this, node->child2());
        GPRTemporary result(this);

        GPRReg reg1 = op1.gpr();
        GPRReg reg2 = op2.gpr();

        // We can perform truncated multiplications if we get to this point, because if the
        // fixup phase could not prove that it would be safe, it would have turned us into
        // a double multiplication.
        if (!shouldCheckOverflow(node->arithMode()))
            m_jit.mul32(reg1, reg2, result.gpr());
        else {
            speculationCheck(
                Overflow, JSValueRegs(), 0,
                m_jit.branchMul32(MacroAssembler::Overflow, reg1, reg2, result.gpr()));
        }
            
        // Check for negative zero, if the users of this node care about such things.
        if (shouldCheckNegativeZero(node->arithMode())) {
            MacroAssembler::Jump resultNonZero = m_jit.branchTest32(MacroAssembler::NonZero, result.gpr());
            speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branchTest32(MacroAssembler::Signed, reg1));
            speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branchTest32(MacroAssembler::Signed, reg2));
            resultNonZero.link(&m_jit);
        }

        int32Result(result.gpr(), node);
        return;
    }

#if USE(JSVALUE64)
    case Int52RepUse: {
        ASSERT(shouldCheckOverflow(node->arithMode()));
        
        // This is super clever. We want to do an int52 multiplication and check the
        // int52 overflow bit. There is no direct hardware support for this, but we do
        // have the ability to do an int64 multiplication and check the int64 overflow
        // bit. We leverage that. Consider that a, b are int52 numbers inside int64
        // registers, with the high 12 bits being sign-extended. We can do:
        //
        //     (a * (b << 12))
        //
        // This will give us a left-shifted int52 (value is in high 52 bits, low 16
        // bits are zero) plus the int52 overflow bit. I.e. whether this 64-bit
        // multiplication overflows is identical to whether the 'a * b' 52-bit
        // multiplication overflows.
        //
        // In our nomenclature, this is:
        //
        //     strictInt52(a) * int52(b) => int52
        //
        // That is "strictInt52" means unshifted and "int52" means left-shifted by 16
        // bits.
        //
        // We don't care which of op1 or op2 serves as the left-shifted operand, so
        // we just do whatever is more convenient for op1 and have op2 do the
        // opposite. This ensures that we do at most one shift.

        SpeculateWhicheverInt52Operand op1(this, node->child1());
        SpeculateWhicheverInt52Operand op2(this, node->child2(), OppositeShift, op1);
        GPRTemporary result(this);
        
        GPRReg op1GPR = op1.gpr();
        GPRReg op2GPR = op2.gpr();
        GPRReg resultGPR = result.gpr();
        
        m_jit.move(op1GPR, resultGPR);
        speculationCheck(
            Int52Overflow, JSValueRegs(), 0,
            m_jit.branchMul64(MacroAssembler::Overflow, op2GPR, resultGPR));
        
        if (shouldCheckNegativeZero(node->arithMode())) {
            MacroAssembler::Jump resultNonZero = m_jit.branchTest64(
                MacroAssembler::NonZero, resultGPR);
            speculationCheck(
                NegativeZero, JSValueRegs(), 0,
                m_jit.branch64(MacroAssembler::LessThan, op1GPR, TrustedImm32(0)));
            speculationCheck(
                NegativeZero, JSValueRegs(), 0,
                m_jit.branch64(MacroAssembler::LessThan, op2GPR, TrustedImm32(0)));
            resultNonZero.link(&m_jit);
        }
        
        int52Result(resultGPR, node);
        return;
    }
#endif // USE(JSVALUE64)
        
    case DoubleRepUse: {
        SpeculateDoubleOperand op1(this, node->child1());
        SpeculateDoubleOperand op2(this, node->child2());
        FPRTemporary result(this, op1, op2);
        
        FPRReg reg1 = op1.fpr();
        FPRReg reg2 = op2.fpr();
        
        m_jit.mulDouble(reg1, reg2, result.fpr());
        
        doubleResult(result.fpr(), node);
        return;
    }

    case UntypedUse: {
        Edge& leftChild = node->child1();
        Edge& rightChild = node->child2();

        if (isKnownNotNumber(leftChild.node()) || isKnownNotNumber(rightChild.node())) {
            JSValueOperand left(this, leftChild);
            JSValueOperand right(this, rightChild);
            JSValueRegs leftRegs = left.jsValueRegs();
            JSValueRegs rightRegs = right.jsValueRegs();
#if USE(JSVALUE64)
            GPRTemporary result(this);
            JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
#endif
            flushRegisters();
            callOperation(operationValueMul, resultRegs, leftRegs, rightRegs);
            m_jit.exceptionCheck();

            jsValueResult(resultRegs, node);
            return;
        }

#if USE(JSVALUE64)
        bool needsScratchGPRReg = true;
        bool needsScratchFPRReg = false;
#else
        bool needsScratchGPRReg = true;
        bool needsScratchFPRReg = true;
#endif

        ArithProfile* arithProfile = m_jit.graph().baselineCodeBlockFor(node->origin.semantic)->arithProfileForBytecodeOffset(node->origin.semantic.bytecodeIndex);
        JITMulIC* mulIC = m_jit.codeBlock()->addJITMulIC(arithProfile);
        auto repatchingFunction = operationValueMulOptimize;
        auto nonRepatchingFunction = operationValueMul;
        
        compileMathIC(node, mulIC, needsScratchGPRReg, needsScratchFPRReg, repatchingFunction, nonRepatchingFunction);
        return;
    }

    default:
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
}

void SpeculativeJIT::compileArithDiv(Node* node)
{
    switch (node->binaryUseKind()) {
    case Int32Use: {
#if CPU(X86) || CPU(X86_64)
        SpeculateInt32Operand op1(this, node->child1());
        SpeculateInt32Operand op2(this, node->child2());
        GPRTemporary eax(this, X86Registers::eax);
        GPRTemporary edx(this, X86Registers::edx);
        GPRReg op1GPR = op1.gpr();
        GPRReg op2GPR = op2.gpr();
    
        GPRReg op2TempGPR;
        GPRReg temp;
        if (op2GPR == X86Registers::eax || op2GPR == X86Registers::edx) {
            op2TempGPR = allocate();
            temp = op2TempGPR;
        } else {
            op2TempGPR = InvalidGPRReg;
            if (op1GPR == X86Registers::eax)
                temp = X86Registers::edx;
            else
                temp = X86Registers::eax;
        }
    
        ASSERT(temp != op1GPR);
        ASSERT(temp != op2GPR);
    
        m_jit.add32(JITCompiler::TrustedImm32(1), op2GPR, temp);
    
        JITCompiler::Jump safeDenominator = m_jit.branch32(JITCompiler::Above, temp, JITCompiler::TrustedImm32(1));
    
        JITCompiler::JumpList done;
        if (shouldCheckOverflow(node->arithMode())) {
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::Zero, op2GPR));
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branch32(JITCompiler::Equal, op1GPR, TrustedImm32(-2147483647-1)));
        } else {
            // This is the case where we convert the result to an int after we're done, and we
            // already know that the denominator is either -1 or 0. So, if the denominator is
            // zero, then the result should be zero. If the denominator is not zero (i.e. it's
            // -1) and the numerator is -2^31 then the result should be -2^31. Otherwise we
            // are happy to fall through to a normal division, since we're just dividing
            // something by negative 1.
        
            JITCompiler::Jump notZero = m_jit.branchTest32(JITCompiler::NonZero, op2GPR);
            m_jit.move(TrustedImm32(0), eax.gpr());
            done.append(m_jit.jump());
        
            notZero.link(&m_jit);
            JITCompiler::Jump notNeg2ToThe31 =
                m_jit.branch32(JITCompiler::NotEqual, op1GPR, TrustedImm32(-2147483647-1));
            m_jit.zeroExtend32ToPtr(op1GPR, eax.gpr());
            done.append(m_jit.jump());
        
            notNeg2ToThe31.link(&m_jit);
        }
    
        safeDenominator.link(&m_jit);
    
        // If the user cares about negative zero, then speculate that we're not about
        // to produce negative zero.
        if (shouldCheckNegativeZero(node->arithMode())) {
            MacroAssembler::Jump numeratorNonZero = m_jit.branchTest32(MacroAssembler::NonZero, op1GPR);
            speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::LessThan, op2GPR, TrustedImm32(0)));
            numeratorNonZero.link(&m_jit);
        }
    
        if (op2TempGPR != InvalidGPRReg) {
            m_jit.move(op2GPR, op2TempGPR);
            op2GPR = op2TempGPR;
        }
            
        m_jit.move(op1GPR, eax.gpr());
        m_jit.x86ConvertToDoubleWord32();
        m_jit.x86Div32(op2GPR);
            
        if (op2TempGPR != InvalidGPRReg)
            unlock(op2TempGPR);

        // Check that there was no remainder. If there had been, then we'd be obligated to
        // produce a double result instead.
        if (shouldCheckOverflow(node->arithMode()))
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::NonZero, edx.gpr()));
        
        done.link(&m_jit);
        int32Result(eax.gpr(), node);
#elif HAVE(ARM_IDIV_INSTRUCTIONS) || CPU(ARM64)
        SpeculateInt32Operand op1(this, node->child1());
        SpeculateInt32Operand op2(this, node->child2());
        GPRReg op1GPR = op1.gpr();
        GPRReg op2GPR = op2.gpr();
        GPRTemporary quotient(this);
        GPRTemporary multiplyAnswer(this);

        // If the user cares about negative zero, then speculate that we're not about
        // to produce negative zero.
        if (shouldCheckNegativeZero(node->arithMode())) {
            MacroAssembler::Jump numeratorNonZero = m_jit.branchTest32(MacroAssembler::NonZero, op1GPR);
            speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::LessThan, op2GPR, TrustedImm32(0)));
            numeratorNonZero.link(&m_jit);
        }

        if (shouldCheckOverflow(node->arithMode()))
            speculationCheck(Overflow, JSValueRegs(), nullptr, m_jit.branchTest32(MacroAssembler::Zero, op2GPR));

        m_jit.assembler().sdiv<32>(quotient.gpr(), op1GPR, op2GPR);

        // Check that there was no remainder. If there had been, then we'd be obligated to
        // produce a double result instead.
        if (shouldCheckOverflow(node->arithMode())) {
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchMul32(JITCompiler::Overflow, quotient.gpr(), op2GPR, multiplyAnswer.gpr()));
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branch32(JITCompiler::NotEqual, multiplyAnswer.gpr(), op1GPR));
        }

        int32Result(quotient.gpr(), node);
#else
        RELEASE_ASSERT_NOT_REACHED();
#endif
        break;
    }
        
    case DoubleRepUse: {
        SpeculateDoubleOperand op1(this, node->child1());
        SpeculateDoubleOperand op2(this, node->child2());
        FPRTemporary result(this, op1);
        
        FPRReg reg1 = op1.fpr();
        FPRReg reg2 = op2.fpr();
        m_jit.divDouble(reg1, reg2, result.fpr());
        
        doubleResult(result.fpr(), node);
        break;
    }

    case UntypedUse: {
        Edge& leftChild = node->child1();
        Edge& rightChild = node->child2();

        if (isKnownNotNumber(leftChild.node()) || isKnownNotNumber(rightChild.node())) {
            JSValueOperand left(this, leftChild);
            JSValueOperand right(this, rightChild);
            JSValueRegs leftRegs = left.jsValueRegs();
            JSValueRegs rightRegs = right.jsValueRegs();
#if USE(JSVALUE64)
            GPRTemporary result(this);
            JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
#endif
            flushRegisters();
            callOperation(operationValueDiv, resultRegs, leftRegs, rightRegs);
            m_jit.exceptionCheck();

            jsValueResult(resultRegs, node);
            return;
        }

        std::optional<JSValueOperand> left;
        std::optional<JSValueOperand> right;

        JSValueRegs leftRegs;
        JSValueRegs rightRegs;

        FPRTemporary leftNumber(this);
        FPRTemporary rightNumber(this);
        FPRReg leftFPR = leftNumber.fpr();
        FPRReg rightFPR = rightNumber.fpr();
        FPRTemporary fprScratch(this);
        FPRReg scratchFPR = fprScratch.fpr();

#if USE(JSVALUE64)
        GPRTemporary result(this);
        JSValueRegs resultRegs = JSValueRegs(result.gpr());
        GPRTemporary scratch(this);
        GPRReg scratchGPR = scratch.gpr();
#else
        GPRTemporary resultTag(this);
        GPRTemporary resultPayload(this);
        JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
        GPRReg scratchGPR = resultTag.gpr();
#endif

        SnippetOperand leftOperand(m_state.forNode(leftChild).resultType());
        SnippetOperand rightOperand(m_state.forNode(rightChild).resultType());

        if (leftChild->isInt32Constant())
            leftOperand.setConstInt32(leftChild->asInt32());
#if USE(JSVALUE64)
        else if (leftChild->isDoubleConstant())
            leftOperand.setConstDouble(leftChild->asNumber());
#endif

        if (leftOperand.isConst()) {
            // The snippet generator only supports 1 argument as a constant.
            // Ignore the rightChild's const-ness.
        } else if (rightChild->isInt32Constant())
            rightOperand.setConstInt32(rightChild->asInt32());
#if USE(JSVALUE64)
        else if (rightChild->isDoubleConstant())
            rightOperand.setConstDouble(rightChild->asNumber());
#endif

        RELEASE_ASSERT(!leftOperand.isConst() || !rightOperand.isConst());

        if (!leftOperand.isConst()) {
            left.emplace(this, leftChild);
            leftRegs = left->jsValueRegs();
        }
        if (!rightOperand.isConst()) {
            right.emplace(this, rightChild);
            rightRegs = right->jsValueRegs();
        }

        JITDivGenerator gen(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs,
            leftFPR, rightFPR, scratchGPR, scratchFPR);
        gen.generateFastPath(m_jit);

        ASSERT(gen.didEmitFastPath());
        gen.endJumpList().append(m_jit.jump());

        gen.slowPathJumpList().link(&m_jit);
        silentSpillAllRegisters(resultRegs);

        if (leftOperand.isConst()) {
            leftRegs = resultRegs;
            m_jit.moveValue(leftChild->asJSValue(), leftRegs);
        }
        if (rightOperand.isConst()) {
            rightRegs = resultRegs;
            m_jit.moveValue(rightChild->asJSValue(), rightRegs);
        }

        callOperation(operationValueDiv, resultRegs, leftRegs, rightRegs);

        silentFillAllRegisters(resultRegs);
        m_jit.exceptionCheck();

        gen.endJumpList().link(&m_jit);
        jsValueResult(resultRegs, node);
        return;
    }

    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

void SpeculativeJIT::compileArithFRound(Node* node)
{
    if (node->child1().useKind() == DoubleRepUse) {
        SpeculateDoubleOperand op1(this, node->child1());
        FPRTemporary result(this, op1);
        m_jit.convertDoubleToFloat(op1.fpr(), result.fpr());
        m_jit.convertFloatToDouble(result.fpr(), result.fpr());
        doubleResult(result.fpr(), node);
        return;
    }

    JSValueOperand op1(this, node->child1());
    JSValueRegs op1Regs = op1.jsValueRegs();
    flushRegisters();
    FPRResult result(this);
    callOperation(operationArithFRound, result.fpr(), op1Regs);
    m_jit.exceptionCheck();
    doubleResult(result.fpr(), node);
}

void SpeculativeJIT::compileArithMod(Node* node)
{
    switch (node->binaryUseKind()) {
    case Int32Use: {
        // In the fast path, the dividend value could be the final result
        // (in case of |dividend| < |divisor|), so we speculate it as strict int32.
        SpeculateStrictInt32Operand op1(this, node->child1());
        
        if (node->child2()->isInt32Constant()) {
            int32_t divisor = node->child2()->asInt32();
            if (divisor > 1 && hasOneBitSet(divisor)) {
                unsigned logarithm = WTF::fastLog2(static_cast<uint32_t>(divisor));
                GPRReg dividendGPR = op1.gpr();
                GPRTemporary result(this);
                GPRReg resultGPR = result.gpr();

                // This is what LLVM generates. It's pretty crazy. Here's my
                // attempt at understanding it.
                
                // First, compute either divisor - 1, or 0, depending on whether
                // the dividend is negative:
                //
                // If dividend < 0:  resultGPR = divisor - 1
                // If dividend >= 0: resultGPR = 0
                m_jit.move(dividendGPR, resultGPR);
                m_jit.rshift32(TrustedImm32(31), resultGPR);
                m_jit.urshift32(TrustedImm32(32 - logarithm), resultGPR);
                
                // Add in the dividend, so that:
                //
                // If dividend < 0:  resultGPR = dividend + divisor - 1
                // If dividend >= 0: resultGPR = dividend
                m_jit.add32(dividendGPR, resultGPR);
                
                // Mask so as to only get the *high* bits. This rounds down
                // (towards negative infinity) resultGPR to the nearest multiple
                // of divisor, so that:
                //
                // If dividend < 0:  resultGPR = floor((dividend + divisor - 1) / divisor)
                // If dividend >= 0: resultGPR = floor(dividend / divisor)
                //
                // Note that this can be simplified to:
                //
                // If dividend < 0:  resultGPR = ceil(dividend / divisor)
                // If dividend >= 0: resultGPR = floor(dividend / divisor)
                //
                // Note that if the dividend is negative, resultGPR will also be negative.
                // Regardless of the sign of dividend, resultGPR will be rounded towards
                // zero, because of how things are conditionalized.
                m_jit.and32(TrustedImm32(-divisor), resultGPR);
                
                // Subtract resultGPR from dividendGPR, which yields the remainder:
                //
                // resultGPR = dividendGPR - resultGPR
                m_jit.neg32(resultGPR);
                m_jit.add32(dividendGPR, resultGPR);
                
                if (shouldCheckNegativeZero(node->arithMode())) {
                    // Check that we're not about to create negative zero.
                    JITCompiler::Jump numeratorPositive = m_jit.branch32(JITCompiler::GreaterThanOrEqual, dividendGPR, TrustedImm32(0));
                    speculationCheck(NegativeZero, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::Zero, resultGPR));
                    numeratorPositive.link(&m_jit);
                }

                int32Result(resultGPR, node);
                return;
            }
        }
        
#if CPU(X86) || CPU(X86_64)
        if (node->child2()->isInt32Constant()) {
            int32_t divisor = node->child2()->asInt32();
            if (divisor && divisor != -1) {
                GPRReg op1Gpr = op1.gpr();

                GPRTemporary eax(this, X86Registers::eax);
                GPRTemporary edx(this, X86Registers::edx);
                GPRTemporary scratch(this);
                GPRReg scratchGPR = scratch.gpr();

                GPRReg op1SaveGPR;
                if (op1Gpr == X86Registers::eax || op1Gpr == X86Registers::edx) {
                    op1SaveGPR = allocate();
                    ASSERT(op1Gpr != op1SaveGPR);
                    m_jit.move(op1Gpr, op1SaveGPR);
                } else
                    op1SaveGPR = op1Gpr;
                ASSERT(op1SaveGPR != X86Registers::eax);
                ASSERT(op1SaveGPR != X86Registers::edx);

                m_jit.move(op1Gpr, eax.gpr());
                m_jit.move(TrustedImm32(divisor), scratchGPR);
                m_jit.x86ConvertToDoubleWord32();
                m_jit.x86Div32(scratchGPR);
                if (shouldCheckNegativeZero(node->arithMode())) {
                    JITCompiler::Jump numeratorPositive = m_jit.branch32(JITCompiler::GreaterThanOrEqual, op1SaveGPR, TrustedImm32(0));
                    speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::Zero, edx.gpr()));
                    numeratorPositive.link(&m_jit);
                }
            
                if (op1SaveGPR != op1Gpr)
                    unlock(op1SaveGPR);

                int32Result(edx.gpr(), node);
                return;
            }
        }
#endif

        SpeculateInt32Operand op2(this, node->child2());
#if CPU(X86) || CPU(X86_64)
        GPRTemporary eax(this, X86Registers::eax);
        GPRTemporary edx(this, X86Registers::edx);
        GPRReg op1GPR = op1.gpr();
        GPRReg op2GPR = op2.gpr();
    
        GPRReg op2TempGPR;
        GPRReg temp;
        GPRReg op1SaveGPR;
    
        if (op2GPR == X86Registers::eax || op2GPR == X86Registers::edx) {
            op2TempGPR = allocate();
            temp = op2TempGPR;
        } else {
            op2TempGPR = InvalidGPRReg;
            if (op1GPR == X86Registers::eax)
                temp = X86Registers::edx;
            else
                temp = X86Registers::eax;
        }
    
        if (op1GPR == X86Registers::eax || op1GPR == X86Registers::edx) {
            op1SaveGPR = allocate();
            ASSERT(op1GPR != op1SaveGPR);
            m_jit.move(op1GPR, op1SaveGPR);
        } else
            op1SaveGPR = op1GPR;
    
        ASSERT(temp != op1GPR);
        ASSERT(temp != op2GPR);
        ASSERT(op1SaveGPR != X86Registers::eax);
        ASSERT(op1SaveGPR != X86Registers::edx);
    
        m_jit.add32(JITCompiler::TrustedImm32(1), op2GPR, temp);
    
        JITCompiler::Jump safeDenominator = m_jit.branch32(JITCompiler::Above, temp, JITCompiler::TrustedImm32(1));
    
        JITCompiler::JumpList done;
        
        // FIXME: -2^31 / -1 will actually yield negative zero, so we could have a
        // separate case for that. But it probably doesn't matter so much.
        if (shouldCheckOverflow(node->arithMode())) {
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::Zero, op2GPR));
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branch32(JITCompiler::Equal, op1GPR, TrustedImm32(-2147483647-1)));
        } else {
            // This is the case where we convert the result to an int after we're done, and we
            // already know that the denominator is either -1 or 0. So, if the denominator is
            // zero, then the result should be zero. If the denominator is not zero (i.e. it's
            // -1) and the numerator is -2^31 then the result should be 0. Otherwise we are
            // happy to fall through to a normal division, since we're just dividing something
            // by negative 1.
        
            JITCompiler::Jump notZero = m_jit.branchTest32(JITCompiler::NonZero, op2GPR);
            m_jit.move(TrustedImm32(0), edx.gpr());
            done.append(m_jit.jump());
        
            notZero.link(&m_jit);
            JITCompiler::Jump notNeg2ToThe31 =
                m_jit.branch32(JITCompiler::NotEqual, op1GPR, TrustedImm32(-2147483647-1));
            m_jit.move(TrustedImm32(0), edx.gpr());
            done.append(m_jit.jump());
        
            notNeg2ToThe31.link(&m_jit);
        }
        
        safeDenominator.link(&m_jit);
            
        if (op2TempGPR != InvalidGPRReg) {
            m_jit.move(op2GPR, op2TempGPR);
            op2GPR = op2TempGPR;
        }
            
        m_jit.move(op1GPR, eax.gpr());
        m_jit.x86ConvertToDoubleWord32();
        m_jit.x86Div32(op2GPR);
            
        if (op2TempGPR != InvalidGPRReg)
            unlock(op2TempGPR);

        // Check that we're not about to create negative zero.
        if (shouldCheckNegativeZero(node->arithMode())) {
            JITCompiler::Jump numeratorPositive = m_jit.branch32(JITCompiler::GreaterThanOrEqual, op1SaveGPR, TrustedImm32(0));
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::Zero, edx.gpr()));
            numeratorPositive.link(&m_jit);
        }
    
        if (op1SaveGPR != op1GPR)
            unlock(op1SaveGPR);
            
        done.link(&m_jit);
        int32Result(edx.gpr(), node);

#elif HAVE(ARM_IDIV_INSTRUCTIONS) || CPU(ARM64)
        GPRTemporary temp(this);
        GPRTemporary quotientThenRemainder(this);
        GPRTemporary multiplyAnswer(this);
        GPRReg dividendGPR = op1.gpr();
        GPRReg divisorGPR = op2.gpr();
        GPRReg quotientThenRemainderGPR = quotientThenRemainder.gpr();
        GPRReg multiplyAnswerGPR = multiplyAnswer.gpr();

        JITCompiler::JumpList done;
    
        if (shouldCheckOverflow(node->arithMode()))
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::Zero, divisorGPR));
        else {
            JITCompiler::Jump denominatorNotZero = m_jit.branchTest32(JITCompiler::NonZero, divisorGPR);
            m_jit.move(divisorGPR, quotientThenRemainderGPR);
            done.append(m_jit.jump());
            denominatorNotZero.link(&m_jit);
        }

        m_jit.assembler().sdiv<32>(quotientThenRemainderGPR, dividendGPR, divisorGPR);
        // FIXME: It seems like there are cases where we don't need this? What if we have
        // arithMode() == Arith::Unchecked?
        // https://bugs.webkit.org/show_bug.cgi?id=126444
        speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchMul32(JITCompiler::Overflow, quotientThenRemainderGPR, divisorGPR, multiplyAnswerGPR));
#if HAVE(ARM_IDIV_INSTRUCTIONS)
        m_jit.assembler().sub(quotientThenRemainderGPR, dividendGPR, multiplyAnswerGPR);
#else
        m_jit.assembler().sub<32>(quotientThenRemainderGPR, dividendGPR, multiplyAnswerGPR);
#endif

        // If the user cares about negative zero, then speculate that we're not about
        // to produce negative zero.
        if (shouldCheckNegativeZero(node->arithMode())) {
            // Check that we're not about to create negative zero.
            JITCompiler::Jump numeratorPositive = m_jit.branch32(JITCompiler::GreaterThanOrEqual, dividendGPR, TrustedImm32(0));
            speculationCheck(Overflow, JSValueRegs(), 0, m_jit.branchTest32(JITCompiler::Zero, quotientThenRemainderGPR));
            numeratorPositive.link(&m_jit);
        }

        done.link(&m_jit);

        int32Result(quotientThenRemainderGPR, node);
#else // not architecture that can do integer division
        RELEASE_ASSERT_NOT_REACHED();
#endif
        return;
    }
        
    case DoubleRepUse: {
        SpeculateDoubleOperand op1(this, node->child1());
        SpeculateDoubleOperand op2(this, node->child2());
        
        FPRReg op1FPR = op1.fpr();
        FPRReg op2FPR = op2.fpr();
        
        flushRegisters();
        
        FPRResult result(this);
        
        callOperation(jsMod, result.fpr(), op1FPR, op2FPR);
        
        doubleResult(result.fpr(), node);
        return;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return;
    }
}

void SpeculativeJIT::compileArithRounding(Node* node)
{
    if (node->child1().useKind() == DoubleRepUse) {
        SpeculateDoubleOperand value(this, node->child1());
        FPRReg valueFPR = value.fpr();

        auto setResult = [&] (FPRReg resultFPR) {
            if (producesInteger(node->arithRoundingMode())) {
                GPRTemporary roundedResultAsInt32(this);
                FPRTemporary scratch(this);
                FPRReg scratchFPR = scratch.fpr();
                GPRReg resultGPR = roundedResultAsInt32.gpr();
                JITCompiler::JumpList failureCases;
                m_jit.branchConvertDoubleToInt32(resultFPR, resultGPR, failureCases, scratchFPR, shouldCheckNegativeZero(node->arithRoundingMode()));
                speculationCheck(Overflow, JSValueRegs(), node, failureCases);

                int32Result(resultGPR, node);
            } else
                doubleResult(resultFPR, node);
        };

        if (m_jit.supportsFloatingPointRounding()) {
            switch (node->op()) {
            case ArithRound: {
                FPRTemporary result(this);
                FPRReg resultFPR = result.fpr();
                if (producesInteger(node->arithRoundingMode()) && !shouldCheckNegativeZero(node->arithRoundingMode())) {
                    static const double halfConstant = 0.5;
                    m_jit.loadDouble(TrustedImmPtr(&halfConstant), resultFPR);
                    m_jit.addDouble(valueFPR, resultFPR);
                    m_jit.floorDouble(resultFPR, resultFPR);
                } else {
                    m_jit.ceilDouble(valueFPR, resultFPR);
                    FPRTemporary realPart(this);
                    FPRReg realPartFPR = realPart.fpr();
                    m_jit.subDouble(resultFPR, valueFPR, realPartFPR);

                    FPRTemporary scratch(this);
                    FPRReg scratchFPR = scratch.fpr();
                    static const double halfConstant = 0.5;
                    m_jit.loadDouble(TrustedImmPtr(&halfConstant), scratchFPR);

                    JITCompiler::Jump shouldUseCeiled = m_jit.branchDouble(JITCompiler::DoubleLessThanOrEqual, realPartFPR, scratchFPR);
                    static const double oneConstant = -1.0;
                    m_jit.loadDouble(TrustedImmPtr(&oneConstant), scratchFPR);
                    m_jit.addDouble(scratchFPR, resultFPR);
                    shouldUseCeiled.link(&m_jit);
                }
                setResult(resultFPR);
                return;
            }

            case ArithFloor: {
                FPRTemporary rounded(this);
                FPRReg resultFPR = rounded.fpr();
                m_jit.floorDouble(valueFPR, resultFPR);
                setResult(resultFPR);
                return;
            }

            case ArithCeil: {
                FPRTemporary rounded(this);
                FPRReg resultFPR = rounded.fpr();
                m_jit.ceilDouble(valueFPR, resultFPR);
                setResult(resultFPR);
                return;
            }

            case ArithTrunc: {
                FPRTemporary rounded(this);
                FPRReg resultFPR = rounded.fpr();
                m_jit.roundTowardZeroDouble(valueFPR, resultFPR);
                setResult(resultFPR);
                return;
            }

            default:
                RELEASE_ASSERT_NOT_REACHED();
            }
        } else {
            flushRegisters();
            FPRResult roundedResultAsDouble(this);
            FPRReg resultFPR = roundedResultAsDouble.fpr();
            if (node->op() == ArithRound)
                callOperation(jsRound, resultFPR, valueFPR);
            else if (node->op() == ArithFloor)
                callOperation(floor, resultFPR, valueFPR);
            else if (node->op() == ArithCeil)
                callOperation(ceil, resultFPR, valueFPR);
            else {
                ASSERT(node->op() == ArithTrunc);
                callOperation(trunc, resultFPR, valueFPR);
            }
            setResult(resultFPR);
        }
        return;
    }

    DFG_ASSERT(m_jit.graph(), node, node->child1().useKind() == UntypedUse);

    JSValueOperand argument(this, node->child1());
    JSValueRegs argumentRegs = argument.jsValueRegs();
#if USE(JSVALUE64)
    GPRTemporary result(this);
    JSValueRegs resultRegs = JSValueRegs(result.gpr());
#else
    GPRTemporary resultTag(this);
    GPRTemporary resultPayload(this);
    JSValueRegs resultRegs = JSValueRegs(resultPayload.gpr(), resultTag.gpr());
#endif
    flushRegisters();
    J_JITOperation_EJ operation = nullptr;
    if (node->op() == ArithRound)
        operation = operationArithRound;
    else if (node->op() == ArithFloor)
        operation = operationArithFloor;
    else if (node->op() == ArithCeil)
        operation = operationArithCeil;
    else {
        ASSERT(node->op() == ArithTrunc);
        operation = operationArithTrunc;
    }
    callOperation(operation, resultRegs, argumentRegs);
    m_jit.exceptionCheck();
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileArithSin(Node* node)
{
    compileArithDoubleUnaryOp(node, sin, operationArithSin);
}

void SpeculativeJIT::compileArithSqrt(Node* node)
{
    if (node->child1().useKind() == DoubleRepUse) {
        SpeculateDoubleOperand op1(this, node->child1());
        FPRReg op1FPR = op1.fpr();

        if (!MacroAssembler::supportsFloatingPointSqrt() || !Options::useArchitectureSpecificOptimizations()) {
            flushRegisters();
            FPRResult result(this);
            callOperation(sqrt, result.fpr(), op1FPR);
            doubleResult(result.fpr(), node);
        } else {
            FPRTemporary result(this, op1);
            m_jit.sqrtDouble(op1.fpr(), result.fpr());
            doubleResult(result.fpr(), node);
        }
        return;
    }

    JSValueOperand op1(this, node->child1());
    JSValueRegs op1Regs = op1.jsValueRegs();
    flushRegisters();
    FPRResult result(this);
    callOperation(operationArithSqrt, result.fpr(), op1Regs);
    m_jit.exceptionCheck();
    doubleResult(result.fpr(), node);
}

// For small positive integers , it is worth doing a tiny inline loop to exponentiate the base.
// Every register is clobbered by this helper.
static MacroAssembler::Jump compileArithPowIntegerFastPath(JITCompiler& assembler, FPRReg xOperand, GPRReg yOperand, FPRReg result)
{
    MacroAssembler::JumpList skipFastPath;
    skipFastPath.append(assembler.branch32(MacroAssembler::Above, yOperand, MacroAssembler::TrustedImm32(maxExponentForIntegerMathPow)));

    static const double oneConstant = 1.0;
    assembler.loadDouble(MacroAssembler::TrustedImmPtr(&oneConstant), result);

    MacroAssembler::Label startLoop(assembler.label());
    MacroAssembler::Jump exponentIsEven = assembler.branchTest32(MacroAssembler::Zero, yOperand, MacroAssembler::TrustedImm32(1));
    assembler.mulDouble(xOperand, result);
    exponentIsEven.link(&assembler);
    assembler.mulDouble(xOperand, xOperand);
    assembler.rshift32(MacroAssembler::TrustedImm32(1), yOperand);
    assembler.branchTest32(MacroAssembler::NonZero, yOperand).linkTo(startLoop, &assembler);

    MacroAssembler::Jump skipSlowPath = assembler.jump();
    skipFastPath.link(&assembler);

    return skipSlowPath;
}

void SpeculativeJIT::compileArithPow(Node* node)
{
    if (node->child2().useKind() == Int32Use) {
        SpeculateDoubleOperand xOperand(this, node->child1());
        SpeculateInt32Operand yOperand(this, node->child2());
        FPRReg xOperandfpr = xOperand.fpr();
        GPRReg yOperandGpr = yOperand.gpr();
        FPRTemporary yOperandfpr(this);

        flushRegisters();

        FPRResult result(this);
        FPRReg resultFpr = result.fpr();

        FPRTemporary xOperandCopy(this);
        FPRReg xOperandCopyFpr = xOperandCopy.fpr();
        m_jit.moveDouble(xOperandfpr, xOperandCopyFpr);

        GPRTemporary counter(this);
        GPRReg counterGpr = counter.gpr();
        m_jit.move(yOperandGpr, counterGpr);

        MacroAssembler::Jump skipFallback = compileArithPowIntegerFastPath(m_jit, xOperandCopyFpr, counterGpr, resultFpr);
        m_jit.convertInt32ToDouble(yOperandGpr, yOperandfpr.fpr());
        callOperation(operationMathPow, resultFpr, xOperandfpr, yOperandfpr.fpr());

        skipFallback.link(&m_jit);
        doubleResult(resultFpr, node);
        return;
    }

    if (node->child2()->isDoubleConstant()) {
        double exponent = node->child2()->asNumber();
        static const double infinityConstant = std::numeric_limits<double>::infinity();
        static const double minusInfinityConstant = -std::numeric_limits<double>::infinity();
        if (exponent == 0.5) {
            SpeculateDoubleOperand xOperand(this, node->child1());
            FPRTemporary result(this);
            FPRReg xOperandFpr = xOperand.fpr();
            FPRReg resultFpr = result.fpr();

            m_jit.moveZeroToDouble(resultFpr);
            MacroAssembler::Jump xIsZeroOrNegativeZero = m_jit.branchDouble(MacroAssembler::DoubleEqual, xOperandFpr, resultFpr);

            m_jit.loadDouble(TrustedImmPtr(&minusInfinityConstant), resultFpr);
            MacroAssembler::Jump xIsMinusInfinity = m_jit.branchDouble(MacroAssembler::DoubleEqual, xOperandFpr, resultFpr);
            m_jit.sqrtDouble(xOperandFpr, resultFpr);
            MacroAssembler::Jump doneWithSqrt = m_jit.jump();

            xIsMinusInfinity.link(&m_jit);
            if (isX86())
                m_jit.loadDouble(TrustedImmPtr(&infinityConstant), resultFpr);
            else
                m_jit.absDouble(resultFpr, resultFpr);

            xIsZeroOrNegativeZero.link(&m_jit);
            doneWithSqrt.link(&m_jit);
            doubleResult(resultFpr, node);
            return;
        }
        if (exponent == -0.5) {
            SpeculateDoubleOperand xOperand(this, node->child1());
            FPRTemporary scratch(this);
            FPRTemporary result(this);
            FPRReg xOperandFpr = xOperand.fpr();
            FPRReg scratchFPR = scratch.fpr();
            FPRReg resultFpr = result.fpr();

            m_jit.moveZeroToDouble(resultFpr);
            MacroAssembler::Jump xIsZeroOrNegativeZero = m_jit.branchDouble(MacroAssembler::DoubleEqual, xOperandFpr, resultFpr);

            m_jit.loadDouble(TrustedImmPtr(&minusInfinityConstant), resultFpr);
            MacroAssembler::Jump xIsMinusInfinity = m_jit.branchDouble(MacroAssembler::DoubleEqual, xOperandFpr, resultFpr);

            static const double oneConstant = 1.;
            m_jit.loadDouble(TrustedImmPtr(&oneConstant), resultFpr);
            m_jit.sqrtDouble(xOperandFpr, scratchFPR);
            m_jit.divDouble(resultFpr, scratchFPR, resultFpr);
            MacroAssembler::Jump doneWithSqrt = m_jit.jump();

            xIsZeroOrNegativeZero.link(&m_jit);
            m_jit.loadDouble(TrustedImmPtr(&infinityConstant), resultFpr);
            MacroAssembler::Jump doneWithBaseZero = m_jit.jump();

            xIsMinusInfinity.link(&m_jit);
            m_jit.moveZeroToDouble(resultFpr);

            doneWithBaseZero.link(&m_jit);
            doneWithSqrt.link(&m_jit);
            doubleResult(resultFpr, node);
            return;
        }
    }

    SpeculateDoubleOperand xOperand(this, node->child1());
    SpeculateDoubleOperand yOperand(this, node->child2());
    FPRReg xOperandfpr = xOperand.fpr();
    FPRReg yOperandfpr = yOperand.fpr();

    flushRegisters();

    FPRResult result(this);
    FPRReg resultFpr = result.fpr();

    FPRTemporary xOperandCopy(this);
    FPRReg xOperandCopyFpr = xOperandCopy.fpr();

    FPRTemporary scratch(this);
    FPRReg scratchFpr = scratch.fpr();

    GPRTemporary yOperandInteger(this);
    GPRReg yOperandIntegerGpr = yOperandInteger.gpr();
    MacroAssembler::JumpList failedExponentConversionToInteger;
    m_jit.branchConvertDoubleToInt32(yOperandfpr, yOperandIntegerGpr, failedExponentConversionToInteger, scratchFpr, false);

    m_jit.moveDouble(xOperandfpr, xOperandCopyFpr);
    MacroAssembler::Jump skipFallback = compileArithPowIntegerFastPath(m_jit, xOperandCopyFpr, yOperandInteger.gpr(), resultFpr);
    failedExponentConversionToInteger.link(&m_jit);

    callOperation(operationMathPow, resultFpr, xOperandfpr, yOperandfpr);
    skipFallback.link(&m_jit);
    doubleResult(resultFpr, node);
}

void SpeculativeJIT::compileArithLog(Node* node)
{
    compileArithDoubleUnaryOp(node, log, operationArithLog);
}

// Returns true if the compare is fused with a subsequent branch.
bool SpeculativeJIT::compare(Node* node, MacroAssembler::RelationalCondition condition, MacroAssembler::DoubleCondition doubleCondition, S_JITOperation_EJJ operation)
{
    if (compilePeepHoleBranch(node, condition, doubleCondition, operation))
        return true;

    if (node->isBinaryUseKind(Int32Use)) {
        compileInt32Compare(node, condition);
        return false;
    }
    
#if USE(JSVALUE64)
    if (node->isBinaryUseKind(Int52RepUse)) {
        compileInt52Compare(node, condition);
        return false;
    }
#endif // USE(JSVALUE64)
    
    if (node->isBinaryUseKind(DoubleRepUse)) {
        compileDoubleCompare(node, doubleCondition);
        return false;
    }

    if (node->isBinaryUseKind(StringUse)) {
        if (node->op() == CompareEq)
            compileStringEquality(node);
        else
            compileStringCompare(node, condition);
        return false;
    }

    if (node->isBinaryUseKind(StringIdentUse)) {
        if (node->op() == CompareEq)
            compileStringIdentEquality(node);
        else
            compileStringIdentCompare(node, condition);
        return false;
    }

    if (node->op() == CompareEq) {
        if (node->isBinaryUseKind(BooleanUse)) {
            compileBooleanCompare(node, condition);
            return false;
        }

        if (node->isBinaryUseKind(SymbolUse)) {
            compileSymbolEquality(node);
            return false;
        }
        
        if (node->isBinaryUseKind(ObjectUse)) {
            compileObjectEquality(node);
            return false;
        }
        
        if (node->isBinaryUseKind(ObjectUse, ObjectOrOtherUse)) {
            compileObjectToObjectOrOtherEquality(node->child1(), node->child2());
            return false;
        }
        
        if (node->isBinaryUseKind(ObjectOrOtherUse, ObjectUse)) {
            compileObjectToObjectOrOtherEquality(node->child2(), node->child1());
            return false;
        }

        if (!needsTypeCheck(node->child1(), SpecOther)) {
            nonSpeculativeNonPeepholeCompareNullOrUndefined(node->child2());
            return false;
        }

        if (!needsTypeCheck(node->child2(), SpecOther)) {
            nonSpeculativeNonPeepholeCompareNullOrUndefined(node->child1());
            return false;
        }
    }

    nonSpeculativeNonPeepholeCompare(node, condition, operation);
    return false;
}

bool SpeculativeJIT::compileStrictEq(Node* node)
{
    // FIXME: Currently, we have op_jless, op_jgreater etc. But we don't have op_jeq, op_jstricteq etc.
    // `==` and `===` operations with branching will be compiled to op_{eq,stricteq} and op_{jfalse,jtrue}.
    // In DFG bytecodes, between op_eq and op_jfalse, we have MovHint to store the result of op_eq.
    // As a result, detectPeepHoleBranch() never detects peep hole for that case.
    // https://bugs.webkit.org/show_bug.cgi?id=149713

    if (node->isBinaryUseKind(BooleanUse)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleBooleanBranch(node, branchNode, MacroAssembler::Equal);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileBooleanCompare(node, MacroAssembler::Equal);
        return false;
    }

    if (node->isBinaryUseKind(Int32Use)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleInt32Branch(node, branchNode, MacroAssembler::Equal);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileInt32Compare(node, MacroAssembler::Equal);
        return false;
    }
    
#if USE(JSVALUE64)   
    if (node->isBinaryUseKind(Int52RepUse)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleInt52Branch(node, branchNode, MacroAssembler::Equal);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileInt52Compare(node, MacroAssembler::Equal);
        return false;
    }
#endif // USE(JSVALUE64)

    if (node->isBinaryUseKind(DoubleRepUse)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleDoubleBranch(node, branchNode, MacroAssembler::DoubleEqual);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileDoubleCompare(node, MacroAssembler::DoubleEqual);
        return false;
    }

    if (node->isBinaryUseKind(SymbolUse)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleSymbolEquality(node, branchNode);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileSymbolEquality(node);
        return false;
    }
    
    if (node->isBinaryUseKind(SymbolUse, UntypedUse)) {
        compileSymbolUntypedEquality(node, node->child1(), node->child2());
        return false;
    }
    
    if (node->isBinaryUseKind(UntypedUse, SymbolUse)) {
        compileSymbolUntypedEquality(node, node->child2(), node->child1());
        return false;
    }
    
    if (node->isBinaryUseKind(StringUse)) {
        compileStringEquality(node);
        return false;
    }
    
    if (node->isBinaryUseKind(StringIdentUse)) {
        compileStringIdentEquality(node);
        return false;
    }

    if (node->isBinaryUseKind(ObjectUse, UntypedUse)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleObjectStrictEquality(node->child1(), node->child2(), branchNode);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileObjectStrictEquality(node->child1(), node->child2());
        return false;
    }
    
    if (node->isBinaryUseKind(UntypedUse, ObjectUse)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleObjectStrictEquality(node->child2(), node->child1(), branchNode);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileObjectStrictEquality(node->child2(), node->child1());
        return false;
    }

    if (node->isBinaryUseKind(ObjectUse)) {
        unsigned branchIndexInBlock = detectPeepHoleBranch();
        if (branchIndexInBlock != UINT_MAX) {
            Node* branchNode = m_block->at(branchIndexInBlock);
            compilePeepHoleObjectEquality(node, branchNode);
            use(node->child1());
            use(node->child2());
            m_indexInBlock = branchIndexInBlock;
            m_currentNode = branchNode;
            return true;
        }
        compileObjectEquality(node);
        return false;
    }

    if (node->isBinaryUseKind(MiscUse, UntypedUse)
        || node->isBinaryUseKind(UntypedUse, MiscUse)) {
        compileMiscStrictEq(node);
        return false;
    }
    
    if (node->isBinaryUseKind(StringIdentUse, NotStringVarUse)) {
        compileStringIdentToNotStringVarEquality(node, node->child1(), node->child2());
        return false;
    }
    
    if (node->isBinaryUseKind(NotStringVarUse, StringIdentUse)) {
        compileStringIdentToNotStringVarEquality(node, node->child2(), node->child1());
        return false;
    }
    
    if (node->isBinaryUseKind(StringUse, UntypedUse)) {
        compileStringToUntypedEquality(node, node->child1(), node->child2());
        return false;
    }
    
    if (node->isBinaryUseKind(UntypedUse, StringUse)) {
        compileStringToUntypedEquality(node, node->child2(), node->child1());
        return false;
    }
    
    RELEASE_ASSERT(node->isBinaryUseKind(UntypedUse));
    return nonSpeculativeStrictEq(node);
}

void SpeculativeJIT::compileBooleanCompare(Node* node, MacroAssembler::RelationalCondition condition)
{
    SpeculateBooleanOperand op1(this, node->child1());
    SpeculateBooleanOperand op2(this, node->child2());
    GPRTemporary result(this);
    
    m_jit.compare32(condition, op1.gpr(), op2.gpr(), result.gpr());
    
    unblessedBooleanResult(result.gpr(), node);
}

void SpeculativeJIT::compileSymbolEquality(Node* node)
{
    SpeculateCellOperand left(this, node->child1());
    SpeculateCellOperand right(this, node->child2());
    GPRTemporary result(this, Reuse, left, right);

    GPRReg leftGPR = left.gpr();
    GPRReg rightGPR = right.gpr();
    GPRReg resultGPR = result.gpr();

    speculateSymbol(node->child1(), leftGPR);
    speculateSymbol(node->child2(), rightGPR);

    m_jit.comparePtr(JITCompiler::Equal, leftGPR, rightGPR, resultGPR);
    unblessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compilePeepHoleSymbolEquality(Node* node, Node* branchNode)
{
    SpeculateCellOperand left(this, node->child1());
    SpeculateCellOperand right(this, node->child2());

    GPRReg leftGPR = left.gpr();
    GPRReg rightGPR = right.gpr();

    speculateSymbol(node->child1(), leftGPR);
    speculateSymbol(node->child2(), rightGPR);

    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    if (taken == nextBlock()) {
        branchPtr(JITCompiler::NotEqual, leftGPR, rightGPR, notTaken);
        jump(taken);
    } else {
        branchPtr(JITCompiler::Equal, leftGPR, rightGPR, taken);
        jump(notTaken);
    }
}

void SpeculativeJIT::compileStringEquality(
    Node* node, GPRReg leftGPR, GPRReg rightGPR, GPRReg lengthGPR, GPRReg leftTempGPR,
    GPRReg rightTempGPR, GPRReg leftTemp2GPR, GPRReg rightTemp2GPR,
    JITCompiler::JumpList fastTrue, JITCompiler::JumpList fastFalse)
{
    JITCompiler::JumpList trueCase;
    JITCompiler::JumpList falseCase;
    JITCompiler::JumpList slowCase;
    
    trueCase.append(fastTrue);
    falseCase.append(fastFalse);

    m_jit.load32(MacroAssembler::Address(leftGPR, JSString::offsetOfLength()), lengthGPR);
    
    falseCase.append(m_jit.branch32(
        MacroAssembler::NotEqual,
        MacroAssembler::Address(rightGPR, JSString::offsetOfLength()),
        lengthGPR));
    
    trueCase.append(m_jit.branchTest32(MacroAssembler::Zero, lengthGPR));
    
    m_jit.loadPtr(MacroAssembler::Address(leftGPR, JSString::offsetOfValue()), leftTempGPR);
    m_jit.loadPtr(MacroAssembler::Address(rightGPR, JSString::offsetOfValue()), rightTempGPR);
    
    slowCase.append(m_jit.branchTestPtr(MacroAssembler::Zero, leftTempGPR));
    slowCase.append(m_jit.branchTestPtr(MacroAssembler::Zero, rightTempGPR));
    
    slowCase.append(m_jit.branchTest32(
        MacroAssembler::Zero,
        MacroAssembler::Address(leftTempGPR, StringImpl::flagsOffset()),
        TrustedImm32(StringImpl::flagIs8Bit())));
    slowCase.append(m_jit.branchTest32(
        MacroAssembler::Zero,
        MacroAssembler::Address(rightTempGPR, StringImpl::flagsOffset()),
        TrustedImm32(StringImpl::flagIs8Bit())));
    
    m_jit.loadPtr(MacroAssembler::Address(leftTempGPR, StringImpl::dataOffset()), leftTempGPR);
    m_jit.loadPtr(MacroAssembler::Address(rightTempGPR, StringImpl::dataOffset()), rightTempGPR);
    
    MacroAssembler::Label loop = m_jit.label();
    
    m_jit.sub32(TrustedImm32(1), lengthGPR);

    // This isn't going to generate the best code on x86. But that's OK, it's still better
    // than not inlining.
    m_jit.load8(MacroAssembler::BaseIndex(leftTempGPR, lengthGPR, MacroAssembler::TimesOne), leftTemp2GPR);
    m_jit.load8(MacroAssembler::BaseIndex(rightTempGPR, lengthGPR, MacroAssembler::TimesOne), rightTemp2GPR);
    falseCase.append(m_jit.branch32(MacroAssembler::NotEqual, leftTemp2GPR, rightTemp2GPR));
    
    m_jit.branchTest32(MacroAssembler::NonZero, lengthGPR).linkTo(loop, &m_jit);
    
    trueCase.link(&m_jit);
    moveTrueTo(leftTempGPR);
    
    JITCompiler::Jump done = m_jit.jump();

    falseCase.link(&m_jit);
    moveFalseTo(leftTempGPR);
    
    done.link(&m_jit);
    addSlowPathGenerator(
        slowPathCall(
            slowCase, this, operationCompareStringEq, leftTempGPR, leftGPR, rightGPR));
    
    blessedBooleanResult(leftTempGPR, node);
}

void SpeculativeJIT::compileStringEquality(Node* node)
{
    SpeculateCellOperand left(this, node->child1());
    SpeculateCellOperand right(this, node->child2());
    GPRTemporary length(this);
    GPRTemporary leftTemp(this);
    GPRTemporary rightTemp(this);
    GPRTemporary leftTemp2(this, Reuse, left);
    GPRTemporary rightTemp2(this, Reuse, right);
    
    GPRReg leftGPR = left.gpr();
    GPRReg rightGPR = right.gpr();
    GPRReg lengthGPR = length.gpr();
    GPRReg leftTempGPR = leftTemp.gpr();
    GPRReg rightTempGPR = rightTemp.gpr();
    GPRReg leftTemp2GPR = leftTemp2.gpr();
    GPRReg rightTemp2GPR = rightTemp2.gpr();
    
    speculateString(node->child1(), leftGPR);
    
    // It's safe to branch around the type check below, since proving that the values are
    // equal does indeed prove that the right value is a string.
    JITCompiler::Jump fastTrue = m_jit.branchPtr(MacroAssembler::Equal, leftGPR, rightGPR);
    
    speculateString(node->child2(), rightGPR);
    
    compileStringEquality(
        node, leftGPR, rightGPR, lengthGPR, leftTempGPR, rightTempGPR, leftTemp2GPR,
        rightTemp2GPR, fastTrue, JITCompiler::Jump());
}

void SpeculativeJIT::compileStringToUntypedEquality(Node* node, Edge stringEdge, Edge untypedEdge)
{
    SpeculateCellOperand left(this, stringEdge);
    JSValueOperand right(this, untypedEdge, ManualOperandSpeculation);
    GPRTemporary length(this);
    GPRTemporary leftTemp(this);
    GPRTemporary rightTemp(this);
    GPRTemporary leftTemp2(this, Reuse, left);
    GPRTemporary rightTemp2(this);
    
    GPRReg leftGPR = left.gpr();
    JSValueRegs rightRegs = right.jsValueRegs();
    GPRReg lengthGPR = length.gpr();
    GPRReg leftTempGPR = leftTemp.gpr();
    GPRReg rightTempGPR = rightTemp.gpr();
    GPRReg leftTemp2GPR = leftTemp2.gpr();
    GPRReg rightTemp2GPR = rightTemp2.gpr();
    
    speculateString(stringEdge, leftGPR);
    
    JITCompiler::JumpList fastTrue;
    JITCompiler::JumpList fastFalse;
    
    fastFalse.append(m_jit.branchIfNotCell(rightRegs));
    
    // It's safe to branch around the type check below, since proving that the values are
    // equal does indeed prove that the right value is a string.
    fastTrue.append(m_jit.branchPtr(
        MacroAssembler::Equal, leftGPR, rightRegs.payloadGPR()));
    
    fastFalse.append(m_jit.branchIfNotString(rightRegs.payloadGPR()));
    
    compileStringEquality(
        node, leftGPR, rightRegs.payloadGPR(), lengthGPR, leftTempGPR, rightTempGPR, leftTemp2GPR,
        rightTemp2GPR, fastTrue, fastFalse);
}

void SpeculativeJIT::compileStringIdentEquality(Node* node)
{
    SpeculateCellOperand left(this, node->child1());
    SpeculateCellOperand right(this, node->child2());
    GPRTemporary leftTemp(this);
    GPRTemporary rightTemp(this);
    
    GPRReg leftGPR = left.gpr();
    GPRReg rightGPR = right.gpr();
    GPRReg leftTempGPR = leftTemp.gpr();
    GPRReg rightTempGPR = rightTemp.gpr();

    speculateString(node->child1(), leftGPR);
    speculateString(node->child2(), rightGPR);
    
    speculateStringIdentAndLoadStorage(node->child1(), leftGPR, leftTempGPR);
    speculateStringIdentAndLoadStorage(node->child2(), rightGPR, rightTempGPR);
    
    m_jit.comparePtr(MacroAssembler::Equal, leftTempGPR, rightTempGPR, leftTempGPR);
    
    unblessedBooleanResult(leftTempGPR, node);
}

void SpeculativeJIT::compileStringIdentToNotStringVarEquality(
    Node* node, Edge stringEdge, Edge notStringVarEdge)
{
    SpeculateCellOperand left(this, stringEdge);
    JSValueOperand right(this, notStringVarEdge, ManualOperandSpeculation);
    GPRTemporary leftTemp(this);
    GPRTemporary rightTemp(this);
    GPRReg leftTempGPR = leftTemp.gpr();
    GPRReg rightTempGPR = rightTemp.gpr();
    GPRReg leftGPR = left.gpr();
    JSValueRegs rightRegs = right.jsValueRegs();
    
    speculateString(stringEdge, leftGPR);
    speculateStringIdentAndLoadStorage(stringEdge, leftGPR, leftTempGPR);

    moveFalseTo(rightTempGPR);
    JITCompiler::JumpList notString;
    notString.append(m_jit.branchIfNotCell(rightRegs));
    notString.append(m_jit.branchIfNotString(rightRegs.payloadGPR()));
    
    speculateStringIdentAndLoadStorage(notStringVarEdge, rightRegs.payloadGPR(), rightTempGPR);
    
    m_jit.comparePtr(MacroAssembler::Equal, leftTempGPR, rightTempGPR, rightTempGPR);
    notString.link(&m_jit);
    
    unblessedBooleanResult(rightTempGPR, node);
}

void SpeculativeJIT::compileStringCompare(Node* node, MacroAssembler::RelationalCondition condition)
{
    SpeculateCellOperand left(this, node->child1());
    SpeculateCellOperand right(this, node->child2());
    GPRReg leftGPR = left.gpr();
    GPRReg rightGPR = right.gpr();

    speculateString(node->child1(), leftGPR);
    speculateString(node->child2(), rightGPR);

    C_JITOperation_B_EJssJss compareFunction = nullptr;
    if (condition == MacroAssembler::LessThan)
        compareFunction = operationCompareStringLess;
    else if (condition == MacroAssembler::LessThanOrEqual)
        compareFunction = operationCompareStringLessEq;
    else if (condition == MacroAssembler::GreaterThan)
        compareFunction = operationCompareStringGreater;
    else if (condition == MacroAssembler::GreaterThanOrEqual)
        compareFunction = operationCompareStringGreaterEq;
    else
        RELEASE_ASSERT_NOT_REACHED();

    GPRFlushedCallResult result(this);
    GPRReg resultGPR = result.gpr();

    flushRegisters();
    callOperation(compareFunction, resultGPR, leftGPR, rightGPR);
    m_jit.exceptionCheck();

    unblessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compileStringIdentCompare(Node* node, MacroAssembler::RelationalCondition condition)
{
    SpeculateCellOperand left(this, node->child1());
    SpeculateCellOperand right(this, node->child2());
    GPRFlushedCallResult result(this);
    GPRTemporary leftTemp(this);
    GPRTemporary rightTemp(this);

    GPRReg leftGPR = left.gpr();
    GPRReg rightGPR = right.gpr();
    GPRReg resultGPR = result.gpr();
    GPRReg leftTempGPR = leftTemp.gpr();
    GPRReg rightTempGPR = rightTemp.gpr();

    speculateString(node->child1(), leftGPR);
    speculateString(node->child2(), rightGPR);

    C_JITOperation_TT compareFunction = nullptr;
    if (condition == MacroAssembler::LessThan)
        compareFunction = operationCompareStringImplLess;
    else if (condition == MacroAssembler::LessThanOrEqual)
        compareFunction = operationCompareStringImplLessEq;
    else if (condition == MacroAssembler::GreaterThan)
        compareFunction = operationCompareStringImplGreater;
    else if (condition == MacroAssembler::GreaterThanOrEqual)
        compareFunction = operationCompareStringImplGreaterEq;
    else
        RELEASE_ASSERT_NOT_REACHED();

    speculateStringIdentAndLoadStorage(node->child1(), leftGPR, leftTempGPR);
    speculateStringIdentAndLoadStorage(node->child2(), rightGPR, rightTempGPR);

    flushRegisters();
    callOperation(compareFunction, resultGPR, leftTempGPR, rightTempGPR);

    unblessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compileStringZeroLength(Node* node)
{
    SpeculateCellOperand str(this, node->child1());
    GPRReg strGPR = str.gpr();

    // Make sure that this is a string.
    speculateString(node->child1(), strGPR);

    GPRTemporary eq(this);
    GPRReg eqGPR = eq.gpr();

    // Fetch the length field from the string object.
    m_jit.test32(MacroAssembler::Zero, MacroAssembler::Address(strGPR, JSString::offsetOfLength()), MacroAssembler::TrustedImm32(-1), eqGPR);

    unblessedBooleanResult(eqGPR, node);
}

void SpeculativeJIT::compileLogicalNotStringOrOther(Node* node)
{
    JSValueOperand value(this, node->child1(), ManualOperandSpeculation);
    GPRTemporary temp(this);
    JSValueRegs valueRegs = value.jsValueRegs();
    GPRReg tempGPR = temp.gpr();

    JITCompiler::Jump notCell = m_jit.branchIfNotCell(valueRegs);
    GPRReg cellGPR = valueRegs.payloadGPR();
    DFG_TYPE_CHECK(
        valueRegs, node->child1(), (~SpecCell) | SpecString, m_jit.branchIfNotString(cellGPR));
    m_jit.test32(
        JITCompiler::Zero, JITCompiler::Address(cellGPR, JSString::offsetOfLength()),
        JITCompiler::TrustedImm32(-1), tempGPR);
    JITCompiler::Jump done = m_jit.jump();
    notCell.link(&m_jit);
    DFG_TYPE_CHECK(
        valueRegs, node->child1(), SpecCell | SpecOther, m_jit.branchIfNotOther(valueRegs, tempGPR));
    m_jit.move(TrustedImm32(1), tempGPR);
    done.link(&m_jit);

    unblessedBooleanResult(tempGPR, node);
}

void SpeculativeJIT::emitStringBranch(Edge nodeUse, BasicBlock* taken, BasicBlock* notTaken)
{
    SpeculateCellOperand str(this, nodeUse);
    speculateString(nodeUse, str.gpr());
    branchTest32(JITCompiler::NonZero, MacroAssembler::Address(str.gpr(), JSString::offsetOfLength()), taken);
    jump(notTaken);
    noResult(m_currentNode);
}

void SpeculativeJIT::emitStringOrOtherBranch(Edge nodeUse, BasicBlock* taken, BasicBlock* notTaken)
{
    JSValueOperand value(this, nodeUse, ManualOperandSpeculation);
    GPRTemporary temp(this);
    JSValueRegs valueRegs = value.jsValueRegs();
    GPRReg tempGPR = temp.gpr();
    
    JITCompiler::Jump notCell = m_jit.branchIfNotCell(valueRegs);
    GPRReg cellGPR = valueRegs.payloadGPR();
    DFG_TYPE_CHECK(valueRegs, nodeUse, (~SpecCell) | SpecString, m_jit.branchIfNotString(cellGPR));
    branchTest32(
        JITCompiler::Zero, JITCompiler::Address(cellGPR, JSString::offsetOfLength()),
        JITCompiler::TrustedImm32(-1), notTaken);
    jump(taken, ForceJump);
    notCell.link(&m_jit);
    DFG_TYPE_CHECK(
        valueRegs, nodeUse, SpecCell | SpecOther, m_jit.branchIfNotOther(valueRegs, tempGPR));
    jump(notTaken);
    noResult(m_currentNode);
}

void SpeculativeJIT::compileConstantStoragePointer(Node* node)
{
    GPRTemporary storage(this);
    GPRReg storageGPR = storage.gpr();
    m_jit.move(TrustedImmPtr(node->storagePointer()), storageGPR);
    storageResult(storageGPR, node);
}

void SpeculativeJIT::compileGetIndexedPropertyStorage(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    GPRReg baseReg = base.gpr();
    
    GPRTemporary storage(this);
    GPRReg storageReg = storage.gpr();
    
    switch (node->arrayMode().type()) {
    case Array::String:
        m_jit.loadPtr(MacroAssembler::Address(baseReg, JSString::offsetOfValue()), storageReg);
        
        addSlowPathGenerator(
            slowPathCall(
                m_jit.branchTest32(MacroAssembler::Zero, storageReg),
                this, operationResolveRope, storageReg, baseReg));

        m_jit.loadPtr(MacroAssembler::Address(storageReg, StringImpl::dataOffset()), storageReg);
        break;
        
    default:
        ASSERT(isTypedView(node->arrayMode().typedArrayType()));

        m_jit.loadPtr(JITCompiler::Address(baseReg, JSArrayBufferView::offsetOfVector()), storageReg);
        break;
    }
    
    storageResult(storageReg, node);
}

void SpeculativeJIT::compileGetTypedArrayByteOffset(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    GPRTemporary vector(this);
    GPRTemporary data(this);
    
    GPRReg baseGPR = base.gpr();
    GPRReg vectorGPR = vector.gpr();
    GPRReg dataGPR = data.gpr();
    
    JITCompiler::Jump emptyByteOffset = m_jit.branch32(
        MacroAssembler::NotEqual,
        MacroAssembler::Address(baseGPR, JSArrayBufferView::offsetOfMode()),
        TrustedImm32(WastefulTypedArray));

    m_jit.loadPtr(MacroAssembler::Address(baseGPR, JSObject::butterflyOffset()), dataGPR);
    m_jit.loadPtr(MacroAssembler::Address(baseGPR, JSArrayBufferView::offsetOfVector()), vectorGPR);
    m_jit.loadPtr(MacroAssembler::Address(dataGPR, Butterfly::offsetOfArrayBuffer()), dataGPR);
    m_jit.loadPtr(MacroAssembler::Address(dataGPR, ArrayBuffer::offsetOfData()), dataGPR);
    m_jit.subPtr(dataGPR, vectorGPR);
    
    JITCompiler::Jump done = m_jit.jump();
    
    emptyByteOffset.link(&m_jit);
    m_jit.move(TrustedImmPtr(0), vectorGPR);
    
    done.link(&m_jit);

    int32Result(vectorGPR, node);
}

void SpeculativeJIT::compileGetByValOnDirectArguments(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    SpeculateStrictInt32Operand property(this, node->child2());
    GPRTemporary result(this);
#if USE(JSVALUE32_64)
    GPRTemporary resultTag(this);
#endif
    
    GPRReg baseReg = base.gpr();
    GPRReg propertyReg = property.gpr();
    GPRReg resultReg = result.gpr();
#if USE(JSVALUE32_64)
    GPRReg resultTagReg = resultTag.gpr();
    JSValueRegs resultRegs = JSValueRegs(resultTagReg, resultReg);
#else
    JSValueRegs resultRegs = JSValueRegs(resultReg);
#endif
    
    if (!m_compileOkay)
        return;
    
    ASSERT(ArrayMode(Array::DirectArguments).alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1())));
    
    speculationCheck(
        ExoticObjectMode, JSValueSource(), 0,
        m_jit.branchTestPtr(
            MacroAssembler::NonZero,
            MacroAssembler::Address(baseReg, DirectArguments::offsetOfMappedArguments())));
    speculationCheck(
        ExoticObjectMode, JSValueSource(), 0,
        m_jit.branch32(
            MacroAssembler::AboveOrEqual, propertyReg,
            MacroAssembler::Address(baseReg, DirectArguments::offsetOfLength())));
    
    m_jit.loadValue(
        MacroAssembler::BaseIndex(
            baseReg, propertyReg, MacroAssembler::TimesEight, DirectArguments::storageOffset()),
        resultRegs);
    
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileGetByValOnScopedArguments(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    SpeculateStrictInt32Operand property(this, node->child2());
    GPRTemporary result(this);
#if USE(JSVALUE32_64)
    GPRTemporary resultTag(this);
#endif
    GPRTemporary scratch(this);
    GPRTemporary scratch2(this);
    
    GPRReg baseReg = base.gpr();
    GPRReg propertyReg = property.gpr();
    GPRReg resultReg = result.gpr();
#if USE(JSVALUE32_64)
    GPRReg resultTagReg = resultTag.gpr();
    JSValueRegs resultRegs = JSValueRegs(resultTagReg, resultReg);
#else
    JSValueRegs resultRegs = JSValueRegs(resultReg);
#endif
    GPRReg scratchReg = scratch.gpr();
    GPRReg scratch2Reg = scratch2.gpr();
    
    if (!m_compileOkay)
        return;
    
    ASSERT(ArrayMode(Array::ScopedArguments).alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1())));
    
    speculationCheck(
        ExoticObjectMode, JSValueSource(), nullptr,
        m_jit.branch32(
            MacroAssembler::AboveOrEqual, propertyReg,
            MacroAssembler::Address(baseReg, ScopedArguments::offsetOfTotalLength())));
    
    m_jit.loadPtr(MacroAssembler::Address(baseReg, ScopedArguments::offsetOfTable()), scratchReg);
    m_jit.load32(
        MacroAssembler::Address(scratchReg, ScopedArgumentsTable::offsetOfLength()), scratch2Reg);
    
    MacroAssembler::Jump overflowArgument = m_jit.branch32(
        MacroAssembler::AboveOrEqual, propertyReg, scratch2Reg);
    
    m_jit.loadPtr(MacroAssembler::Address(baseReg, ScopedArguments::offsetOfScope()), scratch2Reg);

    m_jit.loadPtr(
        MacroAssembler::Address(scratchReg, ScopedArgumentsTable::offsetOfArguments()),
        scratchReg);
    m_jit.load32(
        MacroAssembler::BaseIndex(scratchReg, propertyReg, MacroAssembler::TimesFour),
        scratchReg);
    
    speculationCheck(
        ExoticObjectMode, JSValueSource(), nullptr,
        m_jit.branch32(
            MacroAssembler::Equal, scratchReg, TrustedImm32(ScopeOffset::invalidOffset)));
    
    m_jit.loadValue(
        MacroAssembler::BaseIndex(
            scratch2Reg, propertyReg, MacroAssembler::TimesEight,
            JSEnvironmentRecord::offsetOfVariables()),
        resultRegs);
    
    MacroAssembler::Jump done = m_jit.jump();
    overflowArgument.link(&m_jit);
    
    m_jit.sub32(propertyReg, scratch2Reg);
    m_jit.neg32(scratch2Reg);
    
    m_jit.loadValue(
        MacroAssembler::BaseIndex(
            baseReg, scratch2Reg, MacroAssembler::TimesEight,
            ScopedArguments::overflowStorageOffset()),
        resultRegs);
    speculationCheck(ExoticObjectMode, JSValueSource(), nullptr, m_jit.branchIfEmpty(resultRegs));
    
    done.link(&m_jit);
    
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileGetScope(Node* node)
{
    SpeculateCellOperand function(this, node->child1());
    GPRTemporary result(this, Reuse, function);
    m_jit.loadPtr(JITCompiler::Address(function.gpr(), JSFunction::offsetOfScopeChain()), result.gpr());
    cellResult(result.gpr(), node);
}
    
void SpeculativeJIT::compileSkipScope(Node* node)
{
    SpeculateCellOperand scope(this, node->child1());
    GPRTemporary result(this, Reuse, scope);
    m_jit.loadPtr(JITCompiler::Address(scope.gpr(), JSScope::offsetOfNext()), result.gpr());
    cellResult(result.gpr(), node);
}

void SpeculativeJIT::compileGetGlobalObject(Node* node)
{
    SpeculateCellOperand object(this, node->child1());
    GPRTemporary result(this);
    GPRTemporary scratch(this);
    m_jit.emitLoadStructure(object.gpr(), result.gpr(), scratch.gpr());
    m_jit.loadPtr(JITCompiler::Address(result.gpr(), Structure::globalObjectOffset()), result.gpr());
    cellResult(result.gpr(), node);
}

void SpeculativeJIT::compileGetArrayLength(Node* node)
{
    switch (node->arrayMode().type()) {
    case Array::Undecided:
    case Array::Int32:
    case Array::Double:
    case Array::Contiguous: {
        StorageOperand storage(this, node->child2());
        GPRTemporary result(this, Reuse, storage);
        GPRReg storageReg = storage.gpr();
        GPRReg resultReg = result.gpr();
        m_jit.load32(MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength()), resultReg);
            
        int32Result(resultReg, node);
        break;
    }
    case Array::ArrayStorage:
    case Array::SlowPutArrayStorage: {
        StorageOperand storage(this, node->child2());
        GPRTemporary result(this, Reuse, storage);
        GPRReg storageReg = storage.gpr();
        GPRReg resultReg = result.gpr();
        m_jit.load32(MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength()), resultReg);
            
        speculationCheck(Uncountable, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::LessThan, resultReg, MacroAssembler::TrustedImm32(0)));
            
        int32Result(resultReg, node);
        break;
    }
    case Array::String: {
        SpeculateCellOperand base(this, node->child1());
        GPRTemporary result(this, Reuse, base);
        GPRReg baseGPR = base.gpr();
        GPRReg resultGPR = result.gpr();
        m_jit.load32(MacroAssembler::Address(baseGPR, JSString::offsetOfLength()), resultGPR);
        int32Result(resultGPR, node);
        break;
    }
    case Array::DirectArguments: {
        SpeculateCellOperand base(this, node->child1());
        GPRTemporary result(this, Reuse, base);
        
        GPRReg baseReg = base.gpr();
        GPRReg resultReg = result.gpr();
        
        if (!m_compileOkay)
            return;
        
        ASSERT(ArrayMode(Array::DirectArguments).alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1())));
        
        speculationCheck(
            ExoticObjectMode, JSValueSource(), 0,
            m_jit.branchTestPtr(
                MacroAssembler::NonZero,
                MacroAssembler::Address(baseReg, DirectArguments::offsetOfMappedArguments())));
        
        m_jit.load32(
            MacroAssembler::Address(baseReg, DirectArguments::offsetOfLength()), resultReg);
        
        int32Result(resultReg, node);
        break;
    }
    case Array::ScopedArguments: {
        SpeculateCellOperand base(this, node->child1());
        GPRTemporary result(this, Reuse, base);
        
        GPRReg baseReg = base.gpr();
        GPRReg resultReg = result.gpr();
        
        if (!m_compileOkay)
            return;
        
        ASSERT(ArrayMode(Array::ScopedArguments).alreadyChecked(m_jit.graph(), node, m_state.forNode(node->child1())));
        
        speculationCheck(
            ExoticObjectMode, JSValueSource(), 0,
            m_jit.branchTest8(
                MacroAssembler::NonZero,
                MacroAssembler::Address(baseReg, ScopedArguments::offsetOfOverrodeThings())));
        
        m_jit.load32(
            MacroAssembler::Address(baseReg, ScopedArguments::offsetOfTotalLength()), resultReg);
        
        int32Result(resultReg, node);
        break;
    }
    default: {
        ASSERT(node->arrayMode().isSomeTypedArrayView());
        SpeculateCellOperand base(this, node->child1());
        GPRTemporary result(this, Reuse, base);
        GPRReg baseGPR = base.gpr();
        GPRReg resultGPR = result.gpr();
        m_jit.load32(MacroAssembler::Address(baseGPR, JSArrayBufferView::offsetOfLength()), resultGPR);
        int32Result(resultGPR, node);
        break;
    } }
}

void SpeculativeJIT::compileCheckStringIdent(Node* node)
{
    SpeculateCellOperand string(this, node->child1());
    GPRTemporary storage(this);

    GPRReg stringGPR = string.gpr();
    GPRReg storageGPR = storage.gpr();

    speculateString(node->child1(), stringGPR);
    speculateStringIdentAndLoadStorage(node->child1(), stringGPR, storageGPR);

    UniquedStringImpl* uid = node->uidOperand();
    speculationCheck(
        BadIdent, JSValueSource(), nullptr,
        m_jit.branchPtr(JITCompiler::NotEqual, storageGPR, TrustedImmPtr(uid)));
    noResult(node);
}

template <typename ClassType> void SpeculativeJIT::compileNewFunctionCommon(GPRReg resultGPR, RegisteredStructure structure, GPRReg scratch1GPR, GPRReg scratch2GPR, GPRReg scopeGPR, MacroAssembler::JumpList& slowPath, size_t size, FunctionExecutable* executable, ptrdiff_t offsetOfScopeChain, ptrdiff_t offsetOfExecutable, ptrdiff_t offsetOfRareData)
{
    emitAllocateJSObjectWithKnownSize<ClassType>(resultGPR, TrustedImmPtr(structure), TrustedImmPtr(0), scratch1GPR, scratch2GPR, slowPath, size);
    
    m_jit.storePtr(scopeGPR, JITCompiler::Address(resultGPR, offsetOfScopeChain));
    m_jit.storePtr(TrustedImmPtr::weakPointer(m_jit.graph(), executable), JITCompiler::Address(resultGPR, offsetOfExecutable));
    m_jit.storePtr(TrustedImmPtr(0), JITCompiler::Address(resultGPR, offsetOfRareData));
    
    m_jit.mutatorFence();
}

void SpeculativeJIT::compileNewFunction(Node* node)
{
    NodeType nodeType = node->op();
    ASSERT(nodeType == NewFunction || nodeType == NewGeneratorFunction || nodeType == NewAsyncFunction);
    
    SpeculateCellOperand scope(this, node->child1());
    GPRReg scopeGPR = scope.gpr();

    FunctionExecutable* executable = node->castOperand<FunctionExecutable*>();

    if (executable->singletonFunction()->isStillValid()) {
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();
        
        flushRegisters();

        if (nodeType == NewGeneratorFunction)
            callOperation(operationNewGeneratorFunction, resultGPR, scopeGPR, executable);
        else if (nodeType == NewAsyncFunction)
            callOperation(operationNewAsyncFunction, resultGPR, scopeGPR, executable);
        else
            callOperation(operationNewFunction, resultGPR, scopeGPR, executable);
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
        return;
    }

    RegisteredStructure structure = m_jit.graph().registerStructure(
        nodeType == NewGeneratorFunction ? m_jit.graph().globalObjectFor(node->origin.semantic)->generatorFunctionStructure() :
        nodeType == NewAsyncFunction ? m_jit.graph().globalObjectFor(node->origin.semantic)->asyncFunctionStructure() :
        m_jit.graph().globalObjectFor(node->origin.semantic)->functionStructure());
    
    GPRTemporary result(this);
    GPRTemporary scratch1(this);
    GPRTemporary scratch2(this);
    
    GPRReg resultGPR = result.gpr();
    GPRReg scratch1GPR = scratch1.gpr();
    GPRReg scratch2GPR = scratch2.gpr();
    
    JITCompiler::JumpList slowPath;
    
    if (nodeType == NewFunction) {
        compileNewFunctionCommon<JSFunction>(resultGPR, structure, scratch1GPR, scratch2GPR, scopeGPR, slowPath, JSFunction::allocationSize(0), executable, JSFunction::offsetOfScopeChain(), JSFunction::offsetOfExecutable(), JSFunction::offsetOfRareData());
            
        addSlowPathGenerator(slowPathCall(slowPath, this, operationNewFunctionWithInvalidatedReallocationWatchpoint, resultGPR, scopeGPR, executable));
    }

    if (nodeType == NewGeneratorFunction) {
        compileNewFunctionCommon<JSGeneratorFunction>(resultGPR, structure, scratch1GPR, scratch2GPR, scopeGPR, slowPath, JSGeneratorFunction::allocationSize(0), executable, JSGeneratorFunction::offsetOfScopeChain(), JSGeneratorFunction::offsetOfExecutable(), JSGeneratorFunction::offsetOfRareData());

        addSlowPathGenerator(slowPathCall(slowPath, this, operationNewGeneratorFunctionWithInvalidatedReallocationWatchpoint, resultGPR, scopeGPR, executable));
    }

    if (nodeType == NewAsyncFunction) {
        compileNewFunctionCommon<JSAsyncFunction>(resultGPR, structure, scratch1GPR, scratch2GPR, scopeGPR, slowPath, JSAsyncFunction::allocationSize(0), executable, JSAsyncFunction::offsetOfScopeChain(), JSAsyncFunction::offsetOfExecutable(), JSAsyncFunction::offsetOfRareData());

        addSlowPathGenerator(slowPathCall(slowPath, this, operationNewAsyncFunctionWithInvalidatedReallocationWatchpoint, resultGPR, scopeGPR, executable));
    }

    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileSetFunctionName(Node* node)
{
    SpeculateCellOperand func(this, node->child1());
    GPRReg funcGPR = func.gpr();
    JSValueOperand nameValue(this, node->child2());
    JSValueRegs nameValueRegs = nameValue.jsValueRegs();

    flushRegisters();
    callOperation(operationSetFunctionName, funcGPR, nameValueRegs);
    m_jit.exceptionCheck();

    noResult(node);
}

void SpeculativeJIT::compileForwardVarargs(Node* node)
{
    LoadVarargsData* data = node->loadVarargsData();
    InlineCallFrame* inlineCallFrame;
    if (node->child1())
        inlineCallFrame = node->child1()->origin.semantic.inlineCallFrame;
    else
        inlineCallFrame = node->origin.semantic.inlineCallFrame;

    GPRTemporary length(this);
    JSValueRegsTemporary temp(this);
    GPRReg lengthGPR = length.gpr();
    JSValueRegs tempRegs = temp.regs();
        
    emitGetLength(inlineCallFrame, lengthGPR, /* includeThis = */ true);
    if (data->offset)
        m_jit.sub32(TrustedImm32(data->offset), lengthGPR);
        
    speculationCheck(
        VarargsOverflow, JSValueSource(), Edge(), m_jit.branch32(
            MacroAssembler::Above,
            lengthGPR, TrustedImm32(data->limit)));
        
    m_jit.store32(lengthGPR, JITCompiler::payloadFor(data->machineCount));
        
    VirtualRegister sourceStart = JITCompiler::argumentsStart(inlineCallFrame) + data->offset;
    VirtualRegister targetStart = data->machineStart;

    m_jit.sub32(TrustedImm32(1), lengthGPR);
        
    // First have a loop that fills in the undefined slots in case of an arity check failure.
    m_jit.move(TrustedImm32(data->mandatoryMinimum), tempRegs.payloadGPR());
    JITCompiler::Jump done = m_jit.branch32(JITCompiler::BelowOrEqual, tempRegs.payloadGPR(), lengthGPR);
        
    JITCompiler::Label loop = m_jit.label();
    m_jit.sub32(TrustedImm32(1), tempRegs.payloadGPR());
    m_jit.storeTrustedValue(
        jsUndefined(),
        JITCompiler::BaseIndex(
            GPRInfo::callFrameRegister, tempRegs.payloadGPR(), JITCompiler::TimesEight,
            targetStart.offset() * sizeof(EncodedJSValue)));
    m_jit.branch32(JITCompiler::Above, tempRegs.payloadGPR(), lengthGPR).linkTo(loop, &m_jit);
    done.link(&m_jit);
        
    // And then fill in the actual argument values.
    done = m_jit.branchTest32(JITCompiler::Zero, lengthGPR);
        
    loop = m_jit.label();
    m_jit.sub32(TrustedImm32(1), lengthGPR);
    m_jit.loadValue(
        JITCompiler::BaseIndex(
            GPRInfo::callFrameRegister, lengthGPR, JITCompiler::TimesEight,
            sourceStart.offset() * sizeof(EncodedJSValue)),
        tempRegs);
    m_jit.storeValue(
        tempRegs,
        JITCompiler::BaseIndex(
            GPRInfo::callFrameRegister, lengthGPR, JITCompiler::TimesEight,
            targetStart.offset() * sizeof(EncodedJSValue)));
    m_jit.branchTest32(JITCompiler::NonZero, lengthGPR).linkTo(loop, &m_jit);
        
    done.link(&m_jit);
        
    noResult(node);
}

void SpeculativeJIT::compileCreateActivation(Node* node)
{
    SymbolTable* table = node->castOperand<SymbolTable*>();
    RegisteredStructure structure = m_jit.graph().registerStructure(m_jit.graph().globalObjectFor(
        node->origin.semantic)->activationStructure());
        
    SpeculateCellOperand scope(this, node->child1());
    GPRReg scopeGPR = scope.gpr();
    JSValue initializationValue = node->initializationValueForActivation();
    ASSERT(initializationValue == jsUndefined() || initializationValue == jsTDZValue());
    
    if (table->singletonScope()->isStillValid()) {
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();
        
        flushRegisters();
        
#if USE(JSVALUE64)
        callOperation(operationCreateActivationDirect,
            resultGPR, structure, scopeGPR, table, TrustedImm64(JSValue::encode(initializationValue)));
#else
        callOperation(operationCreateActivationDirect,
            resultGPR, structure, scopeGPR, table, TrustedImm32(initializationValue.tag()), TrustedImm32(initializationValue.payload()));
#endif
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
        return;
    }
    
    GPRTemporary result(this);
    GPRTemporary scratch1(this);
    GPRTemporary scratch2(this);
    GPRReg resultGPR = result.gpr();
    GPRReg scratch1GPR = scratch1.gpr();
    GPRReg scratch2GPR = scratch2.gpr();
        
    JITCompiler::JumpList slowPath;
    emitAllocateJSObjectWithKnownSize<JSLexicalEnvironment>(
        resultGPR, TrustedImmPtr(structure), TrustedImmPtr(0), scratch1GPR, scratch2GPR,
        slowPath, JSLexicalEnvironment::allocationSize(table));
        
    // Don't need a memory barriers since we just fast-created the activation, so the
    // activation must be young.
    m_jit.storePtr(scopeGPR, JITCompiler::Address(resultGPR, JSScope::offsetOfNext()));
    m_jit.storePtr(
        TrustedImmPtr(node->cellOperand()),
        JITCompiler::Address(resultGPR, JSLexicalEnvironment::offsetOfSymbolTable()));
        
    // Must initialize all members to undefined or the TDZ empty value.
    for (unsigned i = 0; i < table->scopeSize(); ++i) {
        m_jit.storeTrustedValue(
            initializationValue,
            JITCompiler::Address(
                resultGPR, JSLexicalEnvironment::offsetOfVariable(ScopeOffset(i))));
    }
    
    m_jit.mutatorFence();

#if USE(JSVALUE64)
    addSlowPathGenerator(
        slowPathCall(
            slowPath, this, operationCreateActivationDirect, resultGPR, structure, scopeGPR, table, TrustedImm64(JSValue::encode(initializationValue))));
#else
    addSlowPathGenerator(
        slowPathCall(
            slowPath, this, operationCreateActivationDirect, resultGPR, structure, scopeGPR, table, TrustedImm32(initializationValue.tag()), TrustedImm32(initializationValue.payload())));
#endif

    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileCreateDirectArguments(Node* node)
{
    // FIXME: A more effective way of dealing with the argument count and callee is to have
    // them be explicit arguments to this node.
    // https://bugs.webkit.org/show_bug.cgi?id=142207
    
    GPRTemporary result(this);
    GPRTemporary scratch1(this);
    GPRTemporary scratch2(this);
    GPRTemporary length;
    GPRReg resultGPR = result.gpr();
    GPRReg scratch1GPR = scratch1.gpr();
    GPRReg scratch2GPR = scratch2.gpr();
    GPRReg lengthGPR = InvalidGPRReg;
    JSValueRegs valueRegs = JSValueRegs::withTwoAvailableRegs(scratch1GPR, scratch2GPR);
        
    unsigned minCapacity = m_jit.graph().baselineCodeBlockFor(node->origin.semantic)->numParameters() - 1;
        
    unsigned knownLength;
    bool lengthIsKnown; // if false, lengthGPR will have the length.
    if (node->origin.semantic.inlineCallFrame
        && !node->origin.semantic.inlineCallFrame->isVarargs()) {
        knownLength = node->origin.semantic.inlineCallFrame->arguments.size() - 1;
        lengthIsKnown = true;
    } else {
        knownLength = UINT_MAX;
        lengthIsKnown = false;
            
        GPRTemporary realLength(this);
        length.adopt(realLength);
        lengthGPR = length.gpr();

        VirtualRegister argumentCountRegister = m_jit.argumentCount(node->origin.semantic);
        m_jit.load32(JITCompiler::payloadFor(argumentCountRegister), lengthGPR);
        m_jit.sub32(TrustedImm32(1), lengthGPR);
    }
        
    RegisteredStructure structure =
        m_jit.graph().registerStructure(m_jit.graph().globalObjectFor(node->origin.semantic)->directArgumentsStructure());
        
    // Use a different strategy for allocating the object depending on whether we know its
    // size statically.
    JITCompiler::JumpList slowPath;
    if (lengthIsKnown) {
        emitAllocateJSObjectWithKnownSize<DirectArguments>(
            resultGPR, TrustedImmPtr(structure), TrustedImmPtr(0), scratch1GPR, scratch2GPR,
            slowPath, DirectArguments::allocationSize(std::max(knownLength, minCapacity)));
            
        m_jit.store32(
            TrustedImm32(knownLength),
            JITCompiler::Address(resultGPR, DirectArguments::offsetOfLength()));
    } else {
        JITCompiler::Jump tooFewArguments;
        if (minCapacity) {
            tooFewArguments =
                m_jit.branch32(JITCompiler::Below, lengthGPR, TrustedImm32(minCapacity));
        }
        m_jit.lshift32(lengthGPR, TrustedImm32(3), scratch1GPR);
        m_jit.add32(TrustedImm32(DirectArguments::storageOffset()), scratch1GPR);
        if (minCapacity) {
            JITCompiler::Jump done = m_jit.jump();
            tooFewArguments.link(&m_jit);
            m_jit.move(TrustedImm32(DirectArguments::allocationSize(minCapacity)), scratch1GPR);
            done.link(&m_jit);
        }
        
        emitAllocateVariableSizedJSObject<DirectArguments>(
            resultGPR, TrustedImmPtr(structure), scratch1GPR, scratch1GPR, scratch2GPR,
            slowPath);
            
        m_jit.store32(
            lengthGPR, JITCompiler::Address(resultGPR, DirectArguments::offsetOfLength()));
    }
        
    m_jit.store32(
        TrustedImm32(minCapacity),
        JITCompiler::Address(resultGPR, DirectArguments::offsetOfMinCapacity()));
        
    m_jit.storePtr(
        TrustedImmPtr(0), JITCompiler::Address(resultGPR, DirectArguments::offsetOfMappedArguments()));

    m_jit.storePtr(
        TrustedImmPtr(0), JITCompiler::Address(resultGPR, DirectArguments::offsetOfModifiedArgumentsDescriptor()));
    
    if (lengthIsKnown) {
        addSlowPathGenerator(
            slowPathCall(
                slowPath, this, operationCreateDirectArguments, resultGPR, structure,
                knownLength, minCapacity));
    } else {
        auto generator = std::make_unique<CallCreateDirectArgumentsSlowPathGenerator>(
            slowPath, this, resultGPR, structure, lengthGPR, minCapacity);
        addSlowPathGenerator(WTFMove(generator));
    }
        
    if (node->origin.semantic.inlineCallFrame) {
        if (node->origin.semantic.inlineCallFrame->isClosureCall) {
            m_jit.loadPtr(
                JITCompiler::addressFor(
                    node->origin.semantic.inlineCallFrame->calleeRecovery.virtualRegister()),
                scratch1GPR);
        } else {
            m_jit.move(
                TrustedImmPtr::weakPointer(
                    m_jit.graph(), node->origin.semantic.inlineCallFrame->calleeRecovery.constant().asCell()),
                scratch1GPR);
        }
    } else
        m_jit.loadPtr(JITCompiler::addressFor(CallFrameSlot::callee), scratch1GPR);

    // Don't need a memory barriers since we just fast-created the activation, so the
    // activation must be young.
    m_jit.storePtr(
        scratch1GPR, JITCompiler::Address(resultGPR, DirectArguments::offsetOfCallee()));
        
    VirtualRegister start = m_jit.argumentsStart(node->origin.semantic);
    if (lengthIsKnown) {
        for (unsigned i = 0; i < std::max(knownLength, minCapacity); ++i) {
            m_jit.loadValue(JITCompiler::addressFor(start + i), valueRegs);
            m_jit.storeValue(
                valueRegs, JITCompiler::Address(resultGPR, DirectArguments::offsetOfSlot(i)));
        }
    } else {
        JITCompiler::Jump done;
        if (minCapacity) {
            JITCompiler::Jump startLoop = m_jit.branch32(
                JITCompiler::AboveOrEqual, lengthGPR, TrustedImm32(minCapacity));
            m_jit.move(TrustedImm32(minCapacity), lengthGPR);
            startLoop.link(&m_jit);
        } else
            done = m_jit.branchTest32(MacroAssembler::Zero, lengthGPR);
        JITCompiler::Label loop = m_jit.label();
        m_jit.sub32(TrustedImm32(1), lengthGPR);
        m_jit.loadValue(
            JITCompiler::BaseIndex(
                GPRInfo::callFrameRegister, lengthGPR, JITCompiler::TimesEight,
                start.offset() * static_cast<int>(sizeof(Register))),
            valueRegs);
        m_jit.storeValue(
            valueRegs,
            JITCompiler::BaseIndex(
                resultGPR, lengthGPR, JITCompiler::TimesEight,
                DirectArguments::storageOffset()));
        m_jit.branchTest32(MacroAssembler::NonZero, lengthGPR).linkTo(loop, &m_jit);
        if (done.isSet())
            done.link(&m_jit);
    }
        
    m_jit.mutatorFence();
        
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileGetFromArguments(Node* node)
{
    SpeculateCellOperand arguments(this, node->child1());
    JSValueRegsTemporary result(this);
    
    GPRReg argumentsGPR = arguments.gpr();
    JSValueRegs resultRegs = result.regs();
    
    m_jit.loadValue(JITCompiler::Address(argumentsGPR, DirectArguments::offsetOfSlot(node->capturedArgumentsOffset().offset())), resultRegs);
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compilePutToArguments(Node* node)
{
    SpeculateCellOperand arguments(this, node->child1());
    JSValueOperand value(this, node->child2());
    
    GPRReg argumentsGPR = arguments.gpr();
    JSValueRegs valueRegs = value.jsValueRegs();
    
    m_jit.storeValue(valueRegs, JITCompiler::Address(argumentsGPR, DirectArguments::offsetOfSlot(node->capturedArgumentsOffset().offset())));
    noResult(node);
}

void SpeculativeJIT::compileGetArgument(Node* node)
{
    GPRTemporary argumentCount(this);
    JSValueRegsTemporary result(this);
    GPRReg argumentCountGPR = argumentCount.gpr();
    JSValueRegs resultRegs = result.regs();
    m_jit.load32(CCallHelpers::payloadFor(m_jit.argumentCount(node->origin.semantic)), argumentCountGPR);
    auto argumentOutOfBounds = m_jit.branch32(CCallHelpers::LessThanOrEqual, argumentCountGPR, CCallHelpers::TrustedImm32(node->argumentIndex()));
    m_jit.loadValue(CCallHelpers::addressFor(CCallHelpers::argumentsStart(node->origin.semantic) + node->argumentIndex() - 1), resultRegs);
    auto done = m_jit.jump();

    argumentOutOfBounds.link(&m_jit);
    m_jit.moveValue(jsUndefined(), resultRegs);

    done.link(&m_jit);
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileCreateScopedArguments(Node* node)
{
    SpeculateCellOperand scope(this, node->child1());
    GPRReg scopeGPR = scope.gpr();
    
    GPRFlushedCallResult result(this);
    GPRReg resultGPR = result.gpr();
    flushRegisters();
    
    // We set up the arguments ourselves, because we have the whole register file and we can
    // set them up directly into the argument registers. This also means that we don't have to
    // invent a four-argument-register shuffle.
    
    // Arguments: 0:exec, 1:structure, 2:start, 3:length, 4:callee, 5:scope
    
    // Do the scopeGPR first, since it might alias an argument register.
    m_jit.setupArgument(5, [&] (GPRReg destGPR) { m_jit.move(scopeGPR, destGPR); });
    
    // These other things could be done in any order.
    m_jit.setupArgument(4, [&] (GPRReg destGPR) { emitGetCallee(node->origin.semantic, destGPR); });
    m_jit.setupArgument(3, [&] (GPRReg destGPR) { emitGetLength(node->origin.semantic, destGPR); });
    m_jit.setupArgument(2, [&] (GPRReg destGPR) { emitGetArgumentStart(node->origin.semantic, destGPR); });
    m_jit.setupArgument(
        1, [&] (GPRReg destGPR) {
            m_jit.move(
                TrustedImmPtr::weakPointer(m_jit.graph(), m_jit.globalObjectFor(node->origin.semantic)->scopedArgumentsStructure()),
                destGPR);
        });
    m_jit.setupArgument(0, [&] (GPRReg destGPR) { m_jit.move(GPRInfo::callFrameRegister, destGPR); });
    
    appendCallSetResult(operationCreateScopedArguments, resultGPR);
    m_jit.exceptionCheck();
    
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileCreateClonedArguments(Node* node)
{
    GPRFlushedCallResult result(this);
    GPRReg resultGPR = result.gpr();
    flushRegisters();
    
    // We set up the arguments ourselves, because we have the whole register file and we can
    // set them up directly into the argument registers.
    
    // Arguments: 0:exec, 1:structure, 2:start, 3:length, 4:callee
    m_jit.setupArgument(4, [&] (GPRReg destGPR) { emitGetCallee(node->origin.semantic, destGPR); });
    m_jit.setupArgument(3, [&] (GPRReg destGPR) { emitGetLength(node->origin.semantic, destGPR); });
    m_jit.setupArgument(2, [&] (GPRReg destGPR) { emitGetArgumentStart(node->origin.semantic, destGPR); });
    m_jit.setupArgument(
        1, [&] (GPRReg destGPR) {
            m_jit.move(
                TrustedImmPtr::weakPointer(
                    m_jit.graph(), m_jit.globalObjectFor(node->origin.semantic)->clonedArgumentsStructure()),
                destGPR);
        });
    m_jit.setupArgument(0, [&] (GPRReg destGPR) { m_jit.move(GPRInfo::callFrameRegister, destGPR); });
    
    appendCallSetResult(operationCreateClonedArguments, resultGPR);
    m_jit.exceptionCheck();
    
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileCreateRest(Node* node)
{
    ASSERT(node->op() == CreateRest);

#if !CPU(X86)
    if (m_jit.graph().isWatchingHavingABadTimeWatchpoint(node)) {
        SpeculateStrictInt32Operand arrayLength(this, node->child1());
        GPRTemporary arrayResult(this);

        GPRReg arrayLengthGPR = arrayLength.gpr();
        GPRReg arrayResultGPR = arrayResult.gpr();

        bool shouldAllowForArrayStorageStructureForLargeArrays = false;
        ASSERT(m_jit.graph().globalObjectFor(node->origin.semantic)->restParameterStructure()->indexingType() == ArrayWithContiguous);
        compileAllocateNewArrayWithSize(m_jit.graph().globalObjectFor(node->origin.semantic), arrayResultGPR, arrayLengthGPR, ArrayWithContiguous, shouldAllowForArrayStorageStructureForLargeArrays);

        GPRTemporary argumentsStart(this);
        GPRReg argumentsStartGPR = argumentsStart.gpr();

        emitGetArgumentStart(node->origin.semantic, argumentsStartGPR);

        GPRTemporary butterfly(this);
        GPRTemporary currentLength(this);
#if USE(JSVALUE64)
        GPRTemporary value(this);
        JSValueRegs valueRegs = JSValueRegs(value.gpr());
#else
        GPRTemporary valueTag(this);
        GPRTemporary valuePayload(this);
        JSValueRegs valueRegs = JSValueRegs(valueTag.gpr(), valuePayload.gpr());
#endif

        GPRReg currentLengthGPR = currentLength.gpr();
        GPRReg butterflyGPR = butterfly.gpr();

        m_jit.loadPtr(MacroAssembler::Address(arrayResultGPR, JSObject::butterflyOffset()), butterflyGPR);

        CCallHelpers::Jump skipLoop = m_jit.branch32(MacroAssembler::Equal, arrayLengthGPR, TrustedImm32(0));
        m_jit.zeroExtend32ToPtr(arrayLengthGPR, currentLengthGPR);
        m_jit.addPtr(Imm32(sizeof(Register) * node->numberOfArgumentsToSkip()), argumentsStartGPR);

        auto loop = m_jit.label();
        m_jit.sub32(TrustedImm32(1), currentLengthGPR);
        m_jit.loadValue(JITCompiler::BaseIndex(argumentsStartGPR, currentLengthGPR, MacroAssembler::TimesEight), valueRegs);
        m_jit.storeValue(valueRegs, MacroAssembler::BaseIndex(butterflyGPR, currentLengthGPR, MacroAssembler::TimesEight));
        m_jit.branch32(MacroAssembler::NotEqual, currentLengthGPR, TrustedImm32(0)).linkTo(loop, &m_jit);

        skipLoop.link(&m_jit);
        cellResult(arrayResultGPR, node);
        return;
    }
#endif // !CPU(X86)

    SpeculateStrictInt32Operand arrayLength(this, node->child1());
    GPRTemporary argumentsStart(this);
    GPRTemporary numberOfArgumentsToSkip(this);

    GPRReg arrayLengthGPR = arrayLength.gpr();
    GPRReg argumentsStartGPR = argumentsStart.gpr();

    emitGetArgumentStart(node->origin.semantic, argumentsStartGPR);

    flushRegisters();

    GPRFlushedCallResult result(this);
    GPRReg resultGPR = result.gpr();
    callOperation(operationCreateRest, resultGPR, argumentsStartGPR, Imm32(node->numberOfArgumentsToSkip()), arrayLengthGPR);
    m_jit.exceptionCheck();

    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileSpread(Node* node)
{
    ASSERT(node->op() == Spread);

    SpeculateCellOperand operand(this, node->child1());
    GPRReg argument = operand.gpr();

    if (node->child1().useKind() == ArrayUse) {
        // Note: we only speculate on ArrayUse when we've set up the necessary watchpoints
        // to prove that the iteration protocol is non-observable.
        speculateArray(node->child1(), argument);

#if USE(JSVALUE64)
        GPRTemporary result(this);
        GPRTemporary scratch1(this);
        GPRTemporary scratch2(this);
        GPRTemporary length(this);
        FPRTemporary doubleRegister(this);

        GPRReg resultGPR = result.gpr();
        GPRReg scratch1GPR = scratch1.gpr();
        GPRReg scratch2GPR = scratch2.gpr();
        GPRReg lengthGPR = length.gpr();
        FPRReg doubleFPR = doubleRegister.fpr();

        MacroAssembler::JumpList slowPath;

        m_jit.load8(MacroAssembler::Address(argument, JSCell::indexingTypeAndMiscOffset()), scratch1GPR);
        m_jit.and32(TrustedImm32(IndexingShapeMask), scratch1GPR);
        m_jit.sub32(TrustedImm32(Int32Shape), scratch1GPR);

        slowPath.append(m_jit.branch32(MacroAssembler::Above, scratch1GPR, TrustedImm32(ContiguousShape - Int32Shape)));

        m_jit.loadPtr(MacroAssembler::Address(argument, JSObject::butterflyOffset()), lengthGPR);
        m_jit.load32(MacroAssembler::Address(lengthGPR, Butterfly::offsetOfPublicLength()), lengthGPR);
        static_assert(sizeof(JSValue) == 8 && 1 << 3 == 8, "This is strongly assumed in the code below.");
        m_jit.move(lengthGPR, scratch1GPR);
        m_jit.lshift32(TrustedImm32(3), scratch1GPR);
        m_jit.add32(TrustedImm32(JSFixedArray::offsetOfData()), scratch1GPR);

        m_jit.emitAllocateVariableSizedCell<JSFixedArray>(resultGPR, TrustedImmPtr(m_jit.graph().registerStructure(m_jit.graph().m_vm.fixedArrayStructure.get())), scratch1GPR, scratch1GPR, scratch2GPR, slowPath);
        m_jit.store32(lengthGPR, MacroAssembler::Address(resultGPR, JSFixedArray::offsetOfSize()));

        m_jit.loadPtr(MacroAssembler::Address(argument, JSObject::butterflyOffset()), scratch1GPR);

        MacroAssembler::JumpList done;

        m_jit.load8(MacroAssembler::Address(argument, JSCell::indexingTypeAndMiscOffset()), scratch2GPR);
        m_jit.and32(TrustedImm32(IndexingShapeMask), scratch2GPR);
        auto isDoubleArray = m_jit.branch32(MacroAssembler::Equal, scratch2GPR, TrustedImm32(DoubleShape));

        {
            done.append(m_jit.branchTest32(MacroAssembler::Zero, lengthGPR));
            auto loopStart = m_jit.label();
            m_jit.sub32(TrustedImm32(1), lengthGPR);
            m_jit.load64(MacroAssembler::BaseIndex(scratch1GPR, lengthGPR, MacroAssembler::TimesEight), scratch2GPR);
            auto notEmpty = m_jit.branchTest64(MacroAssembler::NonZero, scratch2GPR);
            m_jit.move(TrustedImm64(JSValue::encode(jsUndefined())), scratch2GPR);
            notEmpty.link(&m_jit);
            m_jit.store64(scratch2GPR, MacroAssembler::BaseIndex(resultGPR, lengthGPR, MacroAssembler::TimesEight, JSFixedArray::offsetOfData()));
            m_jit.branchTest32(MacroAssembler::NonZero, lengthGPR).linkTo(loopStart, &m_jit);
            done.append(m_jit.jump());
        }

        isDoubleArray.link(&m_jit);
        {

            done.append(m_jit.branchTest32(MacroAssembler::Zero, lengthGPR));
            auto loopStart = m_jit.label();
            m_jit.sub32(TrustedImm32(1), lengthGPR);
            m_jit.loadDouble(MacroAssembler::BaseIndex(scratch1GPR, lengthGPR, MacroAssembler::TimesEight), doubleFPR);
            auto notEmpty = m_jit.branchDouble(JITCompiler::DoubleEqual, doubleFPR, doubleFPR);
            m_jit.move(TrustedImm64(JSValue::encode(jsUndefined())), scratch2GPR);
            auto doStore = m_jit.jump();
            notEmpty.link(&m_jit);
            m_jit.boxDouble(doubleFPR, scratch2GPR);
            doStore.link(&m_jit);
            m_jit.store64(scratch2GPR, MacroAssembler::BaseIndex(resultGPR, lengthGPR, MacroAssembler::TimesEight, JSFixedArray::offsetOfData()));
            m_jit.branchTest32(MacroAssembler::NonZero, lengthGPR).linkTo(loopStart, &m_jit);
            done.append(m_jit.jump());
        }
        
        m_jit.mutatorFence();

        slowPath.link(&m_jit);
        addSlowPathGenerator(slowPathCall(m_jit.jump(), this, operationSpreadFastArray, resultGPR, argument));

        done.link(&m_jit);
        cellResult(resultGPR, node);
#else
        flushRegisters();

        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();
        callOperation(operationSpreadFastArray, resultGPR, argument);
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
#endif // USE(JSVALUE64)
    } else {
        flushRegisters();

        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();
        callOperation(operationSpreadGeneric, resultGPR, argument);
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
    }
}

void SpeculativeJIT::compileNewArrayWithSpread(Node* node)
{
    ASSERT(node->op() == NewArrayWithSpread);

#if USE(JSVALUE64)
    if (m_jit.graph().isWatchingHavingABadTimeWatchpoint(node)) {
        GPRTemporary result(this);
        GPRReg resultGPR = result.gpr();

        BitVector* bitVector = node->bitVector();
        {
            unsigned startLength = 0;
            for (unsigned i = 0; i < node->numChildren(); ++i) {
                if (!bitVector->get(i))
                    ++startLength;
            }

            GPRTemporary length(this);
            GPRReg lengthGPR = length.gpr();
            m_jit.move(TrustedImm32(startLength), lengthGPR);

            for (unsigned i = 0; i < node->numChildren(); ++i) {
                if (bitVector->get(i)) {
                    Edge use = m_jit.graph().varArgChild(node, i);
                    SpeculateCellOperand fixedArray(this, use);
                    GPRReg fixedArrayGPR = fixedArray.gpr();
                    m_jit.add32(MacroAssembler::Address(fixedArrayGPR, JSFixedArray::offsetOfSize()), lengthGPR);
                }
            }


            bool shouldAllowForArrayStorageStructureForLargeArrays = false;
            ASSERT(m_jit.graph().globalObjectFor(node->origin.semantic)->restParameterStructure()->indexingType() == ArrayWithContiguous);
            compileAllocateNewArrayWithSize(m_jit.graph().globalObjectFor(node->origin.semantic), resultGPR, lengthGPR, ArrayWithContiguous, shouldAllowForArrayStorageStructureForLargeArrays);
        }

        GPRTemporary index(this);
        GPRReg indexGPR = index.gpr();

        GPRTemporary storage(this);
        GPRReg storageGPR = storage.gpr();

        m_jit.move(TrustedImm32(0), indexGPR);
        m_jit.loadPtr(MacroAssembler::Address(resultGPR, JSObject::butterflyOffset()), storageGPR);

        for (unsigned i = 0; i < node->numChildren(); ++i) {
            Edge use = m_jit.graph().varArgChild(node, i);
            if (bitVector->get(i)) {
                SpeculateCellOperand fixedArray(this, use);
                GPRReg fixedArrayGPR = fixedArray.gpr();

                GPRTemporary fixedIndex(this);
                GPRReg fixedIndexGPR = fixedIndex.gpr();

                GPRTemporary item(this);
                GPRReg itemGPR = item.gpr();

                GPRTemporary fixedLength(this);
                GPRReg fixedLengthGPR = fixedLength.gpr();

                m_jit.load32(MacroAssembler::Address(fixedArrayGPR, JSFixedArray::offsetOfSize()), fixedLengthGPR);
                m_jit.move(TrustedImm32(0), fixedIndexGPR);
                auto done = m_jit.branchPtr(MacroAssembler::AboveOrEqual, fixedIndexGPR, fixedLengthGPR);
                auto loopStart = m_jit.label();
                m_jit.load64(
                    MacroAssembler::BaseIndex(fixedArrayGPR, fixedIndexGPR, MacroAssembler::TimesEight, JSFixedArray::offsetOfData()),
                    itemGPR);

                m_jit.store64(itemGPR, MacroAssembler::BaseIndex(storageGPR, indexGPR, MacroAssembler::TimesEight));
                m_jit.addPtr(TrustedImm32(1), fixedIndexGPR);
                m_jit.addPtr(TrustedImm32(1), indexGPR);
                m_jit.branchPtr(MacroAssembler::Below, fixedIndexGPR, fixedLengthGPR).linkTo(loopStart, &m_jit);

                done.link(&m_jit);
            } else {
                JSValueOperand item(this, use);
                GPRReg itemGPR = item.gpr();
                m_jit.store64(itemGPR, MacroAssembler::BaseIndex(storageGPR, indexGPR, MacroAssembler::TimesEight));
                m_jit.addPtr(TrustedImm32(1), indexGPR);
            }
        }

        cellResult(resultGPR, node);
        return;
    }
#endif // USE(JSVALUE64)

    ASSERT(node->numChildren());
    size_t scratchSize = sizeof(EncodedJSValue) * node->numChildren();
    ScratchBuffer* scratchBuffer = m_jit.vm()->scratchBufferForSize(scratchSize);
    EncodedJSValue* buffer = static_cast<EncodedJSValue*>(scratchBuffer->dataBuffer());

    BitVector* bitVector = node->bitVector();
    for (unsigned i = 0; i < node->numChildren(); ++i) {
        Edge use = m_jit.graph().m_varArgChildren[node->firstChild() + i];
        if (bitVector->get(i)) {
            SpeculateCellOperand fixedArray(this, use);
            GPRReg arrayGPR = fixedArray.gpr();
#if USE(JSVALUE64)
            m_jit.store64(arrayGPR, &buffer[i]);
#else
            char* pointer = static_cast<char*>(static_cast<void*>(&buffer[i]));
            m_jit.store32(arrayGPR, pointer + PayloadOffset);
            m_jit.store32(TrustedImm32(JSValue::CellTag), pointer + TagOffset);
#endif
        } else {
            JSValueOperand input(this, use);
            JSValueRegs inputRegs = input.jsValueRegs();
            m_jit.storeValue(inputRegs, &buffer[i]);
        }
    }

    {
        GPRTemporary scratch(this);
        m_jit.move(TrustedImmPtr(scratchBuffer->activeLengthPtr()), scratch.gpr());
        m_jit.storePtr(TrustedImmPtr(scratchSize), MacroAssembler::Address(scratch.gpr()));
    }

    flushRegisters();

    GPRFlushedCallResult result(this);
    GPRReg resultGPR = result.gpr();

    callOperation(operationNewArrayWithSpreadSlow, resultGPR, buffer, node->numChildren());
    m_jit.exceptionCheck();
    {
        GPRTemporary scratch(this);
        m_jit.move(TrustedImmPtr(scratchBuffer->activeLengthPtr()), scratch.gpr());
        m_jit.storePtr(TrustedImmPtr(0), MacroAssembler::Address(scratch.gpr()));
    }

    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileGetRestLength(Node* node)
{
    ASSERT(node->op() == GetRestLength);

    GPRTemporary result(this);
    GPRReg resultGPR = result.gpr();

    emitGetLength(node->origin.semantic, resultGPR);
    CCallHelpers::Jump hasNonZeroLength = m_jit.branch32(MacroAssembler::Above, resultGPR, Imm32(node->numberOfArgumentsToSkip()));
    m_jit.move(TrustedImm32(0), resultGPR);
    CCallHelpers::Jump done = m_jit.jump();
    hasNonZeroLength.link(&m_jit);
    if (node->numberOfArgumentsToSkip())
        m_jit.sub32(TrustedImm32(node->numberOfArgumentsToSkip()), resultGPR);
    done.link(&m_jit);
    int32Result(resultGPR, node);
}

void SpeculativeJIT::compileArraySlice(Node* node)
{
    ASSERT(node->op() == ArraySlice);

    JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);

    GPRTemporary temp(this);
    StorageOperand storage(this, node->numChildren() == 3 ? m_jit.graph().varArgChild(node, 2) : m_jit.graph().varArgChild(node, 3));
    GPRTemporary result(this);
    
    GPRReg storageGPR = storage.gpr();
    GPRReg resultGPR = result.gpr();
    GPRReg tempGPR = temp.gpr();

    auto populateIndex = [&] (unsigned childIndex, GPRReg length, GPRReg result) {
        SpeculateInt32Operand index(this, m_jit.graph().varArgChild(node, childIndex));
        GPRReg indexGPR = index.gpr();
        MacroAssembler::JumpList done;
        auto isPositive = m_jit.branch32(MacroAssembler::GreaterThanOrEqual, indexGPR, TrustedImm32(0));
        m_jit.move(length, result);
        done.append(m_jit.branchAdd32(MacroAssembler::PositiveOrZero, indexGPR, result));
        m_jit.move(TrustedImm32(0), result);
        done.append(m_jit.jump());

        isPositive.link(&m_jit);
        m_jit.move(indexGPR, result);
        done.append(m_jit.branch32(MacroAssembler::BelowOrEqual, result, length));
        m_jit.move(length, result);

        done.link(&m_jit);
    };

    {
        GPRTemporary tempLength(this);
        GPRReg lengthGPR = tempLength.gpr();
        m_jit.load32(MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()), lengthGPR);

        if (node->numChildren() == 4)
            populateIndex(2, lengthGPR, tempGPR);
        else
            m_jit.move(lengthGPR, tempGPR);

        GPRTemporary tempStartIndex(this);
        GPRReg startGPR = tempStartIndex.gpr();
        populateIndex(1, lengthGPR, startGPR);

        auto tooBig = m_jit.branch32(MacroAssembler::Above, startGPR, tempGPR);
        m_jit.sub32(startGPR, tempGPR); // the size of the array we'll make.
        auto done = m_jit.jump();

        tooBig.link(&m_jit);
        m_jit.move(TrustedImm32(0), tempGPR);
        done.link(&m_jit);
    }


    GPRTemporary temp3(this);
    GPRReg tempValue = temp3.gpr();
    {
        SpeculateCellOperand cell(this, m_jit.graph().varArgChild(node, 0));
        m_jit.load8(MacroAssembler::Address(cell.gpr(), JSCell::indexingTypeAndMiscOffset()), tempValue);
        m_jit.and32(TrustedImm32(AllArrayTypesAndHistory), tempValue);
    }

    {
#if USE(JSVALUE64)
        GPRTemporary emptyValue(this);
        JSValueRegs emptyValueRegs = JSValueRegs(emptyValue.gpr());
#else
        GPRTemporary emptyValuePayload(this);
        GPRTemporary emptyValueTag(this);
        JSValueRegs emptyValueRegs(emptyValueTag.gpr(), emptyValuePayload.gpr());
#endif

        GPRTemporary storage(this);
        GPRReg storageResultGPR = storage.gpr();

        GPRReg sizeGPR = tempGPR; 

        CCallHelpers::JumpList done;

        auto emitMoveEmptyValue = [&] (JSValue v) {
#if USE(JSVALUE64)
            m_jit.move(TrustedImm64(JSValue::encode(v)), emptyValueRegs.gpr());
#else
            m_jit.move(TrustedImm32(v.tag()), emptyValueRegs.tagGPR());
            m_jit.move(TrustedImm32(v.payload()), emptyValueRegs.payloadGPR());
#endif
        };

        auto isContiguous = m_jit.branch32(MacroAssembler::Equal, tempValue, TrustedImm32(ArrayWithContiguous));
        auto isInt32 = m_jit.branch32(MacroAssembler::Equal, tempValue, TrustedImm32(ArrayWithInt32));
        // When we emit an ArraySlice, we dominate the use of the array by a CheckStructure
        // to ensure the incoming array is one to be one of the original array structures
        // with one of the following indexing shapes: Int32, Contiguous, Double. Therefore,
        // we're a double array here.
        m_jit.move(TrustedImmPtr(m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithDouble))), tempValue);
        emitMoveEmptyValue(jsNaN());
        done.append(m_jit.jump());

        isContiguous.link(&m_jit);
        m_jit.move(TrustedImmPtr(m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithContiguous))), tempValue);
        emitMoveEmptyValue(JSValue());
        done.append(m_jit.jump());

        isInt32.link(&m_jit);
        m_jit.move(TrustedImmPtr(m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithInt32))), tempValue);
        emitMoveEmptyValue(JSValue());

        done.link(&m_jit);

        {
            GPRTemporary scratch(this);
            GPRTemporary scratch2(this);
            GPRReg scratchGPR = scratch.gpr();
            GPRReg scratch2GPR = scratch2.gpr();

            MacroAssembler::JumpList slowCases;
            m_jit.move(TrustedImmPtr(0), storageResultGPR);

            emitAllocateButterfly(storageResultGPR, sizeGPR, scratchGPR, scratch2GPR, resultGPR, slowCases);
            emitInitializeButterfly(storageResultGPR, sizeGPR, emptyValueRegs, scratchGPR);
            emitAllocateJSObject<JSArray>(resultGPR, tempValue, storageResultGPR, scratchGPR, scratch2GPR, slowCases);
            m_jit.mutatorFence();

            addSlowPathGenerator(std::make_unique<CallArrayAllocatorWithVariableStructureVariableSizeSlowPathGenerator>(
                slowCases, this, operationNewArrayWithSize, resultGPR, tempValue, sizeGPR, storageResultGPR, scratchGPR));
        }
    }

    GPRTemporary temp4(this);
    GPRReg loadIndex = temp4.gpr();

    m_jit.load32(MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()), tempValue);
    if (node->numChildren() == 4)
        populateIndex(2, tempValue, tempGPR);
    else
        m_jit.move(tempValue, tempGPR);
    populateIndex(1, tempValue, loadIndex);

    GPRTemporary temp5(this);
    GPRReg storeIndex = temp5.gpr();
    m_jit.move(TrustedImmPtr(0), storeIndex);

    GPRTemporary temp2(this);
    GPRReg resultButterfly = temp2.gpr();

    m_jit.loadPtr(MacroAssembler::Address(resultGPR, JSObject::butterflyOffset()), resultButterfly);
    m_jit.zeroExtend32ToPtr(tempGPR, tempGPR);
    m_jit.zeroExtend32ToPtr(loadIndex, loadIndex);
    auto done = m_jit.branchPtr(MacroAssembler::AboveOrEqual, loadIndex, tempGPR);

    auto loop = m_jit.label();
#if USE(JSVALUE64)
    m_jit.load64(
        MacroAssembler::BaseIndex(storageGPR, loadIndex, MacroAssembler::TimesEight), tempValue);
    m_jit.store64(
        tempValue, MacroAssembler::BaseIndex(resultButterfly, storeIndex, MacroAssembler::TimesEight));
#else
    m_jit.load32(
        MacroAssembler::BaseIndex(storageGPR, loadIndex, MacroAssembler::TimesEight, PayloadOffset), tempValue);
    m_jit.store32(
        tempValue, MacroAssembler::BaseIndex(resultButterfly, storeIndex, MacroAssembler::TimesEight, PayloadOffset));
    m_jit.load32(
        MacroAssembler::BaseIndex(storageGPR, loadIndex, MacroAssembler::TimesEight, TagOffset), tempValue);
    m_jit.store32(
        tempValue, MacroAssembler::BaseIndex(resultButterfly, storeIndex, MacroAssembler::TimesEight, TagOffset));
#endif // USE(JSVALUE64)
    m_jit.addPtr(TrustedImm32(1), loadIndex);
    m_jit.addPtr(TrustedImm32(1), storeIndex);
    m_jit.branchPtr(MacroAssembler::Below, loadIndex, tempGPR).linkTo(loop, &m_jit);

    done.link(&m_jit);
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileNotifyWrite(Node* node)
{
    WatchpointSet* set = node->watchpointSet();
    
    JITCompiler::Jump slowCase = m_jit.branch8(
        JITCompiler::NotEqual,
        JITCompiler::AbsoluteAddress(set->addressOfState()),
        TrustedImm32(IsInvalidated));
    
    addSlowPathGenerator(
        slowPathCall(slowCase, this, operationNotifyWrite, NeedToSpill, ExceptionCheckRequirement::CheckNotNeeded, NoResult, set));
    
    noResult(node);
}

void SpeculativeJIT::compileIsObjectOrNull(Node* node)
{
    JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
    
    JSValueOperand value(this, node->child1());
    JSValueRegs valueRegs = value.jsValueRegs();
    
    GPRTemporary result(this);
    GPRReg resultGPR = result.gpr();
    
    JITCompiler::Jump isCell = m_jit.branchIfCell(valueRegs);
    
    JITCompiler::Jump isNull = m_jit.branchIfEqual(valueRegs, jsNull());
    JITCompiler::Jump isNonNullNonCell = m_jit.jump();
    
    isCell.link(&m_jit);
    JITCompiler::Jump isFunction = m_jit.branchIfFunction(valueRegs.payloadGPR());
    JITCompiler::Jump notObject = m_jit.branchIfNotObject(valueRegs.payloadGPR());
    
    JITCompiler::Jump slowPath = m_jit.branchTest8(
        JITCompiler::NonZero,
        JITCompiler::Address(valueRegs.payloadGPR(), JSCell::typeInfoFlagsOffset()),
        TrustedImm32(MasqueradesAsUndefined | TypeOfShouldCallGetCallData));
    
    isNull.link(&m_jit);
    m_jit.move(TrustedImm32(1), resultGPR);
    JITCompiler::Jump done = m_jit.jump();
    
    isNonNullNonCell.link(&m_jit);
    isFunction.link(&m_jit);
    notObject.link(&m_jit);
    m_jit.move(TrustedImm32(0), resultGPR);
    
    addSlowPathGenerator(
        slowPathCall(
            slowPath, this, operationObjectIsObject, resultGPR, globalObject,
            valueRegs.payloadGPR()));
    
    done.link(&m_jit);
    
    unblessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compileIsFunction(Node* node)
{
    JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
    
    JSValueOperand value(this, node->child1());
    JSValueRegs valueRegs = value.jsValueRegs();
    
    GPRTemporary result(this);
    GPRReg resultGPR = result.gpr();
    
    JITCompiler::Jump notCell = m_jit.branchIfNotCell(valueRegs);
    JITCompiler::Jump isFunction = m_jit.branchIfFunction(valueRegs.payloadGPR());
    JITCompiler::Jump notObject = m_jit.branchIfNotObject(valueRegs.payloadGPR());
    
    JITCompiler::Jump slowPath = m_jit.branchTest8(
        JITCompiler::NonZero,
        JITCompiler::Address(valueRegs.payloadGPR(), JSCell::typeInfoFlagsOffset()),
        TrustedImm32(MasqueradesAsUndefined | TypeOfShouldCallGetCallData));
    
    notCell.link(&m_jit);
    notObject.link(&m_jit);
    m_jit.move(TrustedImm32(0), resultGPR);
    JITCompiler::Jump done = m_jit.jump();
    
    isFunction.link(&m_jit);
    m_jit.move(TrustedImm32(1), resultGPR);
    
    addSlowPathGenerator(
        slowPathCall(
            slowPath, this, operationObjectIsFunction, resultGPR, globalObject,
            valueRegs.payloadGPR()));
    
    done.link(&m_jit);
    
    unblessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compileTypeOf(Node* node)
{
    JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
    
    JSValueOperand value(this, node->child1());
    JSValueRegs valueRegs = value.jsValueRegs();
    
    GPRTemporary result(this);
    GPRReg resultGPR = result.gpr();
    
    JITCompiler::JumpList done;
    JITCompiler::Jump slowPath;
    m_jit.emitTypeOf(
        valueRegs, resultGPR,
        [&] (TypeofType type, bool fallsThrough) {
            m_jit.move(TrustedImmPtr::weakPointer(m_jit.graph(), m_jit.vm()->smallStrings.typeString(type)), resultGPR);
            if (!fallsThrough)
                done.append(m_jit.jump());
        },
        [&] (JITCompiler::Jump theSlowPath) {
            slowPath = theSlowPath;
        });
    done.link(&m_jit);

    addSlowPathGenerator(
        slowPathCall(
            slowPath, this, operationTypeOfObject, resultGPR, globalObject,
            valueRegs.payloadGPR()));
    
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileCheckStructure(Node* node, GPRReg cellGPR, GPRReg tempGPR)
{
    ASSERT(node->structureSet().size());
    
    if (node->structureSet().size() == 1) {
        speculationCheck(
            BadCache, JSValueSource::unboxedCell(cellGPR), 0,
            m_jit.branchWeakStructure(
                JITCompiler::NotEqual,
                JITCompiler::Address(cellGPR, JSCell::structureIDOffset()),
                node->structureSet()[0]));
    } else {
        std::unique_ptr<GPRTemporary> structure;
        GPRReg structureGPR;
        
        if (tempGPR == InvalidGPRReg) {
            structure = std::make_unique<GPRTemporary>(this);
            structureGPR = structure->gpr();
        } else
            structureGPR = tempGPR;
        
        m_jit.load32(JITCompiler::Address(cellGPR, JSCell::structureIDOffset()), structureGPR);
        
        JITCompiler::JumpList done;
        
        for (size_t i = 0; i < node->structureSet().size() - 1; ++i) {
            done.append(
                m_jit.branchWeakStructure(JITCompiler::Equal, structureGPR, node->structureSet()[i]));
        }
        
        speculationCheck(
            BadCache, JSValueSource::unboxedCell(cellGPR), 0,
            m_jit.branchWeakStructure(
                JITCompiler::NotEqual, structureGPR, node->structureSet().last()));
        
        done.link(&m_jit);
    }
}

void SpeculativeJIT::compileCheckStructure(Node* node)
{
    switch (node->child1().useKind()) {
    case CellUse:
    case KnownCellUse: {
        SpeculateCellOperand cell(this, node->child1());
        compileCheckStructure(node, cell.gpr(), InvalidGPRReg);
        noResult(node);
        return;
    }

    case CellOrOtherUse: {
        JSValueOperand value(this, node->child1(), ManualOperandSpeculation);
        GPRTemporary temp(this);

        JSValueRegs valueRegs = value.jsValueRegs();
        GPRReg tempGPR = temp.gpr();

        JITCompiler::Jump cell = m_jit.branchIfCell(valueRegs);
        DFG_TYPE_CHECK(
            valueRegs, node->child1(), SpecCell | SpecOther,
            m_jit.branchIfNotOther(valueRegs, tempGPR));
        JITCompiler::Jump done = m_jit.jump();
        cell.link(&m_jit);
        compileCheckStructure(node, valueRegs.payloadGPR(), tempGPR);
        done.link(&m_jit);
        noResult(node);
        return;
    }

    default:
        DFG_CRASH(m_jit.graph(), node, "Bad use kind");
        return;
    }
}

void SpeculativeJIT::compileAllocatePropertyStorage(Node* node)
{
    ASSERT(!node->transition()->previous->outOfLineCapacity());
    ASSERT(initialOutOfLineCapacity == node->transition()->next->outOfLineCapacity());
    
    size_t size = initialOutOfLineCapacity * sizeof(JSValue);

    MarkedAllocator* allocator = m_jit.vm()->auxiliarySpace.allocatorFor(size);

    if (!allocator || node->transition()->previous->couldHaveIndexingHeader()) {
        SpeculateCellOperand base(this, node->child1());
        
        GPRReg baseGPR = base.gpr();
        
        flushRegisters();

        GPRFlushedCallResult result(this);
        callOperation(operationAllocateComplexPropertyStorageWithInitialCapacity, result.gpr(), baseGPR);
        m_jit.exceptionCheck();
        
        storageResult(result.gpr(), node);
        return;
    }
    
    GPRTemporary scratch1(this);
    GPRTemporary scratch2(this);
    GPRTemporary scratch3(this);
        
    GPRReg scratchGPR1 = scratch1.gpr();
    GPRReg scratchGPR2 = scratch2.gpr();
    GPRReg scratchGPR3 = scratch3.gpr();
        
    m_jit.move(TrustedImmPtr(allocator), scratchGPR2);
    JITCompiler::JumpList slowPath;
    m_jit.emitAllocate(scratchGPR1, allocator, scratchGPR2, scratchGPR3, slowPath);
    m_jit.addPtr(JITCompiler::TrustedImm32(size + sizeof(IndexingHeader)), scratchGPR1);
    
    for (ptrdiff_t offset = 0; offset < static_cast<ptrdiff_t>(size); offset += sizeof(void*))
        m_jit.storePtr(TrustedImmPtr(0), JITCompiler::Address(scratchGPR1, -(offset + sizeof(JSValue) + sizeof(void*))));
        
    addSlowPathGenerator(
        slowPathCall(slowPath, this, operationAllocateSimplePropertyStorageWithInitialCapacity, scratchGPR1));

    storageResult(scratchGPR1, node);
}

void SpeculativeJIT::compileReallocatePropertyStorage(Node* node)
{
    size_t oldSize = node->transition()->previous->outOfLineCapacity() * sizeof(JSValue);
    size_t newSize = oldSize * outOfLineGrowthFactor;
    ASSERT(newSize == node->transition()->next->outOfLineCapacity() * sizeof(JSValue));
    
    MarkedAllocator* allocator = m_jit.vm()->auxiliarySpace.allocatorFor(newSize);

    if (!allocator || node->transition()->previous->couldHaveIndexingHeader()) {
        SpeculateCellOperand base(this, node->child1());
        
        GPRReg baseGPR = base.gpr();
        
        flushRegisters();

        GPRFlushedCallResult result(this);
        callOperation(operationAllocateComplexPropertyStorage, result.gpr(), baseGPR, newSize / sizeof(JSValue));
        m_jit.exceptionCheck();

        storageResult(result.gpr(), node);
        return;
    }
    
    StorageOperand oldStorage(this, node->child2());
    GPRTemporary scratch1(this);
    GPRTemporary scratch2(this);
    GPRTemporary scratch3(this);
        
    GPRReg oldStorageGPR = oldStorage.gpr();
    GPRReg scratchGPR1 = scratch1.gpr();
    GPRReg scratchGPR2 = scratch2.gpr();
    GPRReg scratchGPR3 = scratch3.gpr();
    
    JITCompiler::JumpList slowPath;
    m_jit.move(TrustedImmPtr(allocator), scratchGPR2);
    m_jit.emitAllocate(scratchGPR1, allocator, scratchGPR2, scratchGPR3, slowPath);
    
    m_jit.addPtr(JITCompiler::TrustedImm32(newSize + sizeof(IndexingHeader)), scratchGPR1);
        
    for (ptrdiff_t offset = oldSize; offset < static_cast<ptrdiff_t>(newSize); offset += sizeof(void*))
        m_jit.storePtr(TrustedImmPtr(0), JITCompiler::Address(scratchGPR1, -(offset + sizeof(JSValue) + sizeof(void*))));

    addSlowPathGenerator(
        slowPathCall(slowPath, this, operationAllocateSimplePropertyStorage, scratchGPR1, newSize / sizeof(JSValue)));

    // We have scratchGPR1 = new storage, scratchGPR2 = scratch
    for (ptrdiff_t offset = 0; offset < static_cast<ptrdiff_t>(oldSize); offset += sizeof(void*)) {
        m_jit.loadPtr(JITCompiler::Address(oldStorageGPR, -(offset + sizeof(JSValue) + sizeof(void*))), scratchGPR2);
        m_jit.storePtr(scratchGPR2, JITCompiler::Address(scratchGPR1, -(offset + sizeof(JSValue) + sizeof(void*))));
    }
        
    storageResult(scratchGPR1, node);
}

void SpeculativeJIT::compileNukeStructureAndSetButterfly(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    StorageOperand storage(this, node->child2());
    
    GPRReg baseGPR = base.gpr();
    GPRReg storageGPR = storage.gpr();
    
    m_jit.nukeStructureAndStoreButterfly(storageGPR, baseGPR);
    
    noResult(node);
}

void SpeculativeJIT::compileGetButterfly(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    GPRTemporary result(this, Reuse, base);
    
    GPRReg baseGPR = base.gpr();
    GPRReg resultGPR = result.gpr();
    
    m_jit.loadPtr(JITCompiler::Address(baseGPR, JSObject::butterflyOffset()), resultGPR);

    storageResult(resultGPR, node);
}

static void allocateTemporaryRegistersForPatchpoint(SpeculativeJIT* jit, Vector<GPRTemporary>& gpHolders, Vector<FPRTemporary>& fpHolders, Vector<GPRReg>& gpScratch, Vector<FPRReg>& fpScratch, DOMJIT::Patchpoint& patchpoint)
{
    for (unsigned i = 0; i < patchpoint.numGPScratchRegisters; ++i) {
        GPRTemporary temporary(jit);
        gpScratch.append(temporary.gpr());
        gpHolders.append(WTFMove(temporary));
    }

    for (unsigned i = 0; i < patchpoint.numFPScratchRegisters; ++i) {
        FPRTemporary temporary(jit);
        fpScratch.append(temporary.fpr());
        fpHolders.append(WTFMove(temporary));
    }
}

void SpeculativeJIT::compileCallDOM(Node* node)
{
    const DOMJIT::Signature* signature = node->signature();

    // FIXME: We should have a way to call functions with the vector of registers.
    // https://bugs.webkit.org/show_bug.cgi?id=163099
    Vector<Variant<SpeculateCellOperand, SpeculateInt32Operand, SpeculateBooleanOperand>, JSC_DOMJIT_SIGNATURE_MAX_ARGUMENTS_INCLUDING_THIS> operands;
    Vector<GPRReg, JSC_DOMJIT_SIGNATURE_MAX_ARGUMENTS_INCLUDING_THIS> regs;

    auto appendCell = [&](Edge& edge) {
        SpeculateCellOperand operand(this, edge);
        regs.append(operand.gpr());
        operands.append(WTFMove(operand));
    };

    auto appendString = [&](Edge& edge) {
        SpeculateCellOperand operand(this, edge);
        GPRReg gpr = operand.gpr();
        regs.append(gpr);
        speculateString(edge, gpr);
        operands.append(WTFMove(operand));
    };

    auto appendInt32 = [&](Edge& edge) {
        SpeculateInt32Operand operand(this, edge);
        regs.append(operand.gpr());
        operands.append(WTFMove(operand));
    };

    auto appendBoolean = [&](Edge& edge) {
        SpeculateBooleanOperand operand(this, edge);
        regs.append(operand.gpr());
        operands.append(WTFMove(operand));
    };

    unsigned index = 0;
    m_jit.graph().doToChildren(node, [&](Edge edge) {
        if (!index)
            appendCell(edge);
        else {
            switch (signature->arguments[index - 1]) {
            case SpecString:
                appendString(edge);
                break;
            case SpecInt32Only:
                appendInt32(edge);
                break;
            case SpecBoolean:
                appendBoolean(edge);
                break;
            default:
                RELEASE_ASSERT_NOT_REACHED();
                break;
            }
        }
        ++index;
    });

    JSValueRegsTemporary result(this);
    JSValueRegs resultRegs = result.regs();

    flushRegisters();
    unsigned argumentCountIncludingThis = signature->argumentCount + 1;
    switch (argumentCountIncludingThis) {
    case 1:
        callOperation(reinterpret_cast<J_JITOperation_EP>(signature->unsafeFunction), extractResult(resultRegs), regs[0]);
        break;
    case 2:
        callOperation(reinterpret_cast<J_JITOperation_EPP>(signature->unsafeFunction), extractResult(resultRegs), regs[0], regs[1]);
        break;
    case 3:
        callOperation(reinterpret_cast<J_JITOperation_EPPP>(signature->unsafeFunction), extractResult(resultRegs), regs[0], regs[1], regs[2]);
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }

    m_jit.exceptionCheck();
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileCallDOMGetter(Node* node)
{
    DOMJIT::CallDOMGetterPatchpoint* patchpoint = node->callDOMGetterData()->patchpoint;

    Vector<GPRReg> gpScratch;
    Vector<FPRReg> fpScratch;
    Vector<DOMJIT::Value> regs;

    JSValueRegsTemporary result(this);
    regs.append(result.regs());

    Edge& baseEdge = node->child1();
    SpeculateCellOperand base(this, baseEdge);
    regs.append(DOMJIT::Value(base.gpr(), m_state.forNode(baseEdge).value()));

    std::optional<SpeculateCellOperand> globalObject;
    if (patchpoint->requireGlobalObject) {
        Edge& globalObjectEdge = node->child2();
        globalObject.emplace(this, globalObjectEdge);
        regs.append(DOMJIT::Value(globalObject->gpr(), m_state.forNode(globalObjectEdge).value()));
    }

    Vector<GPRTemporary> gpTempraries;
    Vector<FPRTemporary> fpTempraries;
    allocateTemporaryRegistersForPatchpoint(this, gpTempraries, fpTempraries, gpScratch, fpScratch, *patchpoint);
    DOMJITPatchpointParams params(this, WTFMove(regs), WTFMove(gpScratch), WTFMove(fpScratch));
    patchpoint->generator()->run(m_jit, params);
    jsValueResult(result.regs(), node);
}

void SpeculativeJIT::compileCheckDOM(Node* node)
{
    // FIXME: We can add the fallback implementation that inlines jsDynamicCast things here.
    DOMJIT::Patchpoint* patchpoint = node->checkDOMPatchpoint();

    Vector<GPRReg> gpScratch;
    Vector<FPRReg> fpScratch;
    Vector<DOMJIT::Value> regs;

    SpeculateCellOperand base(this, node->child1());
    GPRReg baseGPR = base.gpr();
    regs.append(DOMJIT::Value(baseGPR, m_state.forNode(node->child1()).value()));

    Vector<GPRTemporary> gpTempraries;
    Vector<FPRTemporary> fpTempraries;
    allocateTemporaryRegistersForPatchpoint(this, gpTempraries, fpTempraries, gpScratch, fpScratch, *patchpoint);

    DOMJITPatchpointParams params(this, WTFMove(regs), WTFMove(gpScratch), WTFMove(fpScratch));
    CCallHelpers::JumpList failureCases = patchpoint->generator()->run(m_jit, params);
    speculationCheck(BadType, JSValueSource::unboxedCell(baseGPR), node->child1(), failureCases);
    noResult(node);
}

GPRReg SpeculativeJIT::temporaryRegisterForPutByVal(GPRTemporary& temporary, ArrayMode arrayMode)
{
    if (!putByValWillNeedExtraRegister(arrayMode))
        return InvalidGPRReg;
    
    GPRTemporary realTemporary(this);
    temporary.adopt(realTemporary);
    return temporary.gpr();
}

void SpeculativeJIT::compileToStringOrCallStringConstructorOnCell(Node* node)
{
    if (node->child1().useKind() == NotCellUse) {
        JSValueOperand op1(this, node->child1(), ManualOperandSpeculation);
        JSValueRegs op1Regs = op1.jsValueRegs();

        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();

        speculateNotCell(node->child1(), op1Regs);

        flushRegisters();

        if (node->op() == ToString)
            callOperation(operationToString, resultGPR, op1Regs);
        else {
            ASSERT(node->op() == CallStringConstructor);
            callOperation(operationCallStringConstructor, resultGPR, op1Regs);
        }
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
        return;
    }

    if (node->child1().useKind() == UntypedUse) {
        JSValueOperand op1(this, node->child1());
        JSValueRegs op1Regs = op1.jsValueRegs();
        GPRReg op1PayloadGPR = op1Regs.payloadGPR();

        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();

        flushRegisters();

        JITCompiler::Jump done;
        if (node->child1()->prediction() & SpecString) {
            JITCompiler::Jump slowPath1 = m_jit.branchIfNotCell(op1.jsValueRegs());
            JITCompiler::Jump slowPath2 = m_jit.branchIfNotString(op1PayloadGPR);
            m_jit.move(op1PayloadGPR, resultGPR);
            done = m_jit.jump();
            slowPath1.link(&m_jit);
            slowPath2.link(&m_jit);
        }
        if (node->op() == ToString)
            callOperation(operationToString, resultGPR, op1Regs);
        else {
            ASSERT(node->op() == CallStringConstructor);
            callOperation(operationCallStringConstructor, resultGPR, op1Regs);
        }
        m_jit.exceptionCheck();
        if (done.isSet())
            done.link(&m_jit);
        cellResult(resultGPR, node);
        return;
    }


    SpeculateCellOperand op1(this, node->child1());
    GPRReg op1GPR = op1.gpr();
    
    switch (node->child1().useKind()) {
    case StringObjectUse: {
        GPRTemporary result(this);
        GPRReg resultGPR = result.gpr();
        
        speculateStringObject(node->child1(), op1GPR);
        m_interpreter.filter(node->child1(), SpecStringObject);

        m_jit.loadPtr(JITCompiler::Address(op1GPR, JSWrapperObject::internalValueCellOffset()), resultGPR);
        cellResult(resultGPR, node);
        break;
    }
        
    case StringOrStringObjectUse: {
        GPRTemporary result(this);
        GPRReg resultGPR = result.gpr();

        m_jit.load32(JITCompiler::Address(op1GPR, JSCell::structureIDOffset()), resultGPR);
        JITCompiler::Jump isString = m_jit.branchWeakStructure(
            JITCompiler::Equal,
            resultGPR,
            m_jit.graph().registerStructure(m_jit.vm()->stringStructure.get()));

        speculateStringObjectForStructure(node->child1(), resultGPR);
        
        m_jit.loadPtr(JITCompiler::Address(op1GPR, JSWrapperObject::internalValueCellOffset()), resultGPR);
        
        JITCompiler::Jump done = m_jit.jump();
        isString.link(&m_jit);
        m_jit.move(op1GPR, resultGPR);
        done.link(&m_jit);
        
        m_interpreter.filter(node->child1(), SpecString | SpecStringObject);
        
        cellResult(resultGPR, node);
        break;
    }
        
    case CellUse: {
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();
        
        // We flush registers instead of silent spill/fill because in this mode we
        // believe that most likely the input is not a string, and we need to take
        // slow path.
        flushRegisters();
        JITCompiler::Jump done;
        if (node->child1()->prediction() & SpecString) {
            JITCompiler::Jump needCall = m_jit.branchIfNotString(op1GPR);
            m_jit.move(op1GPR, resultGPR);
            done = m_jit.jump();
            needCall.link(&m_jit);
        }
        if (node->op() == ToString)
            callOperation(operationToStringOnCell, resultGPR, op1GPR);
        else {
            ASSERT(node->op() == CallStringConstructor);
            callOperation(operationCallStringConstructorOnCell, resultGPR, op1GPR);
        }
        m_jit.exceptionCheck();
        if (done.isSet())
            done.link(&m_jit);
        cellResult(resultGPR, node);
        break;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
}

void SpeculativeJIT::compileNewStringObject(Node* node)
{
    SpeculateCellOperand operand(this, node->child1());
    
    GPRTemporary result(this);
    GPRTemporary scratch1(this);
    GPRTemporary scratch2(this);

    GPRReg operandGPR = operand.gpr();
    GPRReg resultGPR = result.gpr();
    GPRReg scratch1GPR = scratch1.gpr();
    GPRReg scratch2GPR = scratch2.gpr();
    
    JITCompiler::JumpList slowPath;
    
    emitAllocateJSObject<StringObject>(
        resultGPR, TrustedImmPtr(node->structure()), TrustedImmPtr(0), scratch1GPR, scratch2GPR,
        slowPath);
    
    m_jit.storePtr(
        TrustedImmPtr(StringObject::info()),
        JITCompiler::Address(resultGPR, JSDestructibleObject::classInfoOffset()));
#if USE(JSVALUE64)
    m_jit.store64(
        operandGPR, JITCompiler::Address(resultGPR, JSWrapperObject::internalValueOffset()));
#else
    m_jit.store32(
        TrustedImm32(JSValue::CellTag),
        JITCompiler::Address(resultGPR, JSWrapperObject::internalValueOffset() + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
    m_jit.store32(
        operandGPR,
        JITCompiler::Address(resultGPR, JSWrapperObject::internalValueOffset() + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
#endif
    
    m_jit.mutatorFence();
    
    addSlowPathGenerator(slowPathCall(
        slowPath, this, operationNewStringObject, resultGPR, operandGPR, node->structure()));
    
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileNewTypedArray(Node* node)
{
    JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
    TypedArrayType type = node->typedArrayType();
    RegisteredStructure structure = m_jit.graph().registerStructure(globalObject->typedArrayStructureConcurrently(type));
    RELEASE_ASSERT(structure.get());
    
    SpeculateInt32Operand size(this, node->child1());
    GPRReg sizeGPR = size.gpr();
    
    GPRTemporary result(this);
    GPRTemporary storage(this);
    GPRTemporary scratch(this);
    GPRTemporary scratch2(this);
    GPRReg resultGPR = result.gpr();
    GPRReg storageGPR = storage.gpr();
    GPRReg scratchGPR = scratch.gpr();
    GPRReg scratchGPR2 = scratch2.gpr();
    
    JITCompiler::JumpList slowCases;
    
    m_jit.move(TrustedImmPtr(0), storageGPR);

    slowCases.append(m_jit.branch32(
        MacroAssembler::Above, sizeGPR, TrustedImm32(JSArrayBufferView::fastSizeLimit)));
    slowCases.append(m_jit.branchTest32(MacroAssembler::Zero, sizeGPR));
    
    m_jit.move(sizeGPR, scratchGPR);
    m_jit.lshift32(TrustedImm32(logElementSize(type)), scratchGPR);
    if (elementSize(type) < 8) {
        m_jit.add32(TrustedImm32(7), scratchGPR);
        m_jit.and32(TrustedImm32(~7), scratchGPR);
    }
    m_jit.emitAllocateVariableSized(
        storageGPR, m_jit.vm()->auxiliarySpace, scratchGPR, scratchGPR,
        scratchGPR2, slowCases);
    
    MacroAssembler::Jump done = m_jit.branchTest32(MacroAssembler::Zero, sizeGPR);
    m_jit.move(sizeGPR, scratchGPR);
    if (elementSize(type) != 4) {
        if (elementSize(type) > 4)
            m_jit.lshift32(TrustedImm32(logElementSize(type) - 2), scratchGPR);
        else {
            if (elementSize(type) > 1)
                m_jit.lshift32(TrustedImm32(logElementSize(type)), scratchGPR);
            m_jit.add32(TrustedImm32(3), scratchGPR);
            m_jit.urshift32(TrustedImm32(2), scratchGPR);
        }
    }
    MacroAssembler::Label loop = m_jit.label();
    m_jit.sub32(TrustedImm32(1), scratchGPR);
    m_jit.store32(
        TrustedImm32(0),
        MacroAssembler::BaseIndex(storageGPR, scratchGPR, MacroAssembler::TimesFour));
    m_jit.branchTest32(MacroAssembler::NonZero, scratchGPR).linkTo(loop, &m_jit);
    done.link(&m_jit);
    
    emitAllocateJSObject<JSArrayBufferView>(
        resultGPR, TrustedImmPtr(structure), TrustedImmPtr(0), scratchGPR, scratchGPR2,
        slowCases);
    
    m_jit.storePtr(
        storageGPR,
        MacroAssembler::Address(resultGPR, JSArrayBufferView::offsetOfVector()));
    m_jit.store32(
        sizeGPR,
        MacroAssembler::Address(resultGPR, JSArrayBufferView::offsetOfLength()));
    m_jit.store32(
        TrustedImm32(FastTypedArray),
        MacroAssembler::Address(resultGPR, JSArrayBufferView::offsetOfMode()));
    
    m_jit.mutatorFence();
    
    addSlowPathGenerator(slowPathCall(
        slowCases, this, operationNewTypedArrayWithSizeForType(type),
        resultGPR, structure, sizeGPR, storageGPR));
    
    cellResult(resultGPR, node);
}

void SpeculativeJIT::speculateCellTypeWithoutTypeFiltering(
    Edge edge, GPRReg cellGPR, JSType jsType)
{
    speculationCheck(
        BadType, JSValueSource::unboxedCell(cellGPR), edge,
        m_jit.branch8(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(cellGPR, JSCell::typeInfoTypeOffset()),
            MacroAssembler::TrustedImm32(jsType)));
}

void SpeculativeJIT::speculateCellType(
    Edge edge, GPRReg cellGPR, SpeculatedType specType, JSType jsType)
{
    DFG_TYPE_CHECK(
        JSValueSource::unboxedCell(cellGPR), edge, specType,
        m_jit.branch8(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(cellGPR, JSCell::typeInfoTypeOffset()),
            TrustedImm32(jsType)));
}

void SpeculativeJIT::speculateInt32(Edge edge)
{
    if (!needsTypeCheck(edge, SpecInt32Only))
        return;
    
    (SpeculateInt32Operand(this, edge)).gpr();
}

void SpeculativeJIT::speculateNumber(Edge edge)
{
    if (!needsTypeCheck(edge, SpecBytecodeNumber))
        return;
    
    JSValueOperand value(this, edge, ManualOperandSpeculation);
#if USE(JSVALUE64)
    GPRReg gpr = value.gpr();
    typeCheck(
        JSValueRegs(gpr), edge, SpecBytecodeNumber,
        m_jit.branchTest64(MacroAssembler::Zero, gpr, GPRInfo::tagTypeNumberRegister));
#else
    GPRReg tagGPR = value.tagGPR();
    DFG_TYPE_CHECK(
        value.jsValueRegs(), edge, ~SpecInt32Only,
        m_jit.branch32(MacroAssembler::Equal, tagGPR, TrustedImm32(JSValue::Int32Tag)));
    DFG_TYPE_CHECK(
        value.jsValueRegs(), edge, SpecBytecodeNumber,
        m_jit.branch32(MacroAssembler::AboveOrEqual, tagGPR, TrustedImm32(JSValue::LowestTag)));
#endif
}

void SpeculativeJIT::speculateRealNumber(Edge edge)
{
    if (!needsTypeCheck(edge, SpecBytecodeRealNumber))
        return;
    
    JSValueOperand op1(this, edge, ManualOperandSpeculation);
    FPRTemporary result(this);
    
    JSValueRegs op1Regs = op1.jsValueRegs();
    FPRReg resultFPR = result.fpr();
    
#if USE(JSVALUE64)
    GPRTemporary temp(this);
    GPRReg tempGPR = temp.gpr();
    m_jit.unboxDoubleWithoutAssertions(op1Regs.gpr(), tempGPR, resultFPR);
#else
    FPRTemporary temp(this);
    FPRReg tempFPR = temp.fpr();
    unboxDouble(op1Regs.tagGPR(), op1Regs.payloadGPR(), resultFPR, tempFPR);
#endif
    
    JITCompiler::Jump done = m_jit.branchDouble(
        JITCompiler::DoubleEqual, resultFPR, resultFPR);

    typeCheck(op1Regs, edge, SpecBytecodeRealNumber, m_jit.branchIfNotInt32(op1Regs));
    
    done.link(&m_jit);
}

void SpeculativeJIT::speculateDoubleRepReal(Edge edge)
{
    if (!needsTypeCheck(edge, SpecDoubleReal))
        return;
    
    SpeculateDoubleOperand operand(this, edge);
    FPRReg fpr = operand.fpr();
    typeCheck(
        JSValueRegs(), edge, SpecDoubleReal,
        m_jit.branchDouble(
            MacroAssembler::DoubleNotEqualOrUnordered, fpr, fpr));
}

void SpeculativeJIT::speculateBoolean(Edge edge)
{
    if (!needsTypeCheck(edge, SpecBoolean))
        return;
    
    (SpeculateBooleanOperand(this, edge)).gpr();
}

void SpeculativeJIT::speculateCell(Edge edge)
{
    if (!needsTypeCheck(edge, SpecCell))
        return;
    
    (SpeculateCellOperand(this, edge)).gpr();
}

void SpeculativeJIT::speculateCellOrOther(Edge edge)
{
    if (!needsTypeCheck(edge, SpecCell | SpecOther))
        return;
    
    JSValueOperand operand(this, edge, ManualOperandSpeculation);
    GPRTemporary temp(this);
    GPRReg tempGPR = temp.gpr();

    MacroAssembler::Jump ok = m_jit.branchIfCell(operand.jsValueRegs());
    DFG_TYPE_CHECK(
        operand.jsValueRegs(), edge, SpecCell | SpecOther,
        m_jit.branchIfNotOther(operand.jsValueRegs(), tempGPR));
    ok.link(&m_jit);
}

void SpeculativeJIT::speculateObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecObject))
        return;
    
    SpeculateCellOperand operand(this, edge);
    GPRReg gpr = operand.gpr();
    DFG_TYPE_CHECK(
        JSValueSource::unboxedCell(gpr), edge, SpecObject, m_jit.branchIfNotObject(gpr));
}

void SpeculativeJIT::speculateFunction(Edge edge)
{
    if (!needsTypeCheck(edge, SpecFunction))
        return;
    
    SpeculateCellOperand operand(this, edge);
    speculateCellType(edge, operand.gpr(), SpecFunction, JSFunctionType);
}

void SpeculativeJIT::speculateFinalObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecFinalObject))
        return;
    
    SpeculateCellOperand operand(this, edge);
    speculateCellType(edge, operand.gpr(), SpecFinalObject, FinalObjectType);
}

void SpeculativeJIT::speculateRegExpObject(Edge edge, GPRReg cell)
{
    speculateCellType(edge, cell, SpecRegExpObject, RegExpObjectType);
}

void SpeculativeJIT::speculateRegExpObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecRegExpObject))
        return;
    
    SpeculateCellOperand operand(this, edge);
    speculateRegExpObject(edge, operand.gpr());
}

void SpeculativeJIT::speculateArray(Edge edge, GPRReg cell)
{
    speculateCellType(edge, cell, SpecArray, ArrayType);
}

void SpeculativeJIT::speculateArray(Edge edge)
{
    if (!needsTypeCheck(edge, SpecArray))
        return;

    SpeculateCellOperand operand(this, edge);
    speculateArray(edge, operand.gpr());
}

void SpeculativeJIT::speculateProxyObject(Edge edge, GPRReg cell)
{
    speculateCellType(edge, cell, SpecProxyObject, ProxyObjectType);
}

void SpeculativeJIT::speculateProxyObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecProxyObject))
        return;

    SpeculateCellOperand operand(this, edge);
    speculateProxyObject(edge, operand.gpr());
}

void SpeculativeJIT::speculateDerivedArray(Edge edge, GPRReg cell)
{
    speculateCellType(edge, cell, SpecDerivedArray, DerivedArrayType);
}

void SpeculativeJIT::speculateDerivedArray(Edge edge)
{
    if (!needsTypeCheck(edge, SpecDerivedArray))
        return;

    SpeculateCellOperand operand(this, edge);
    speculateDerivedArray(edge, operand.gpr());
}

void SpeculativeJIT::speculateMapObject(Edge edge, GPRReg cell)
{
    speculateCellType(edge, cell, SpecMapObject, JSMapType);
}

void SpeculativeJIT::speculateMapObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecMapObject))
        return;

    SpeculateCellOperand operand(this, edge);
    speculateMapObject(edge, operand.gpr());
}

void SpeculativeJIT::speculateSetObject(Edge edge, GPRReg cell)
{
    speculateCellType(edge, cell, SpecSetObject, JSSetType);
}

void SpeculativeJIT::speculateSetObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecSetObject))
        return;

    SpeculateCellOperand operand(this, edge);
    speculateSetObject(edge, operand.gpr());
}

void SpeculativeJIT::speculateObjectOrOther(Edge edge)
{
    if (!needsTypeCheck(edge, SpecObject | SpecOther))
        return;
    
    JSValueOperand operand(this, edge, ManualOperandSpeculation);
    GPRTemporary temp(this);
    GPRReg tempGPR = temp.gpr();
    MacroAssembler::Jump notCell = m_jit.branchIfNotCell(operand.jsValueRegs());
    GPRReg gpr = operand.jsValueRegs().payloadGPR();
    DFG_TYPE_CHECK(
        operand.jsValueRegs(), edge, (~SpecCell) | SpecObject, m_jit.branchIfNotObject(gpr));
    MacroAssembler::Jump done = m_jit.jump();
    notCell.link(&m_jit);
    DFG_TYPE_CHECK(
        operand.jsValueRegs(), edge, SpecCell | SpecOther,
        m_jit.branchIfNotOther(operand.jsValueRegs(), tempGPR));
    done.link(&m_jit);
}

void SpeculativeJIT::speculateString(Edge edge, GPRReg cell)
{
    DFG_TYPE_CHECK(
        JSValueSource::unboxedCell(cell), edge, SpecString | ~SpecCell, m_jit.branchIfNotString(cell));
}

void SpeculativeJIT::speculateStringOrOther(Edge edge, JSValueRegs regs, GPRReg scratch)
{
    JITCompiler::Jump notCell = m_jit.branchIfNotCell(regs);
    GPRReg cell = regs.payloadGPR();
    DFG_TYPE_CHECK(regs, edge, (~SpecCell) | SpecString, m_jit.branchIfNotString(cell));
    JITCompiler::Jump done = m_jit.jump();
    notCell.link(&m_jit);
    DFG_TYPE_CHECK(regs, edge, SpecCell | SpecOther, m_jit.branchIfNotOther(regs, scratch));
    done.link(&m_jit);
}

void SpeculativeJIT::speculateStringOrOther(Edge edge)
{
    if (!needsTypeCheck(edge, SpecString | SpecOther))
        return;

    JSValueOperand operand(this, edge, ManualOperandSpeculation);
    GPRTemporary temp(this);
    JSValueRegs regs = operand.jsValueRegs();
    GPRReg tempGPR = temp.gpr();
    speculateStringOrOther(edge, regs, tempGPR);
}

void SpeculativeJIT::speculateStringIdentAndLoadStorage(Edge edge, GPRReg string, GPRReg storage)
{
    m_jit.loadPtr(MacroAssembler::Address(string, JSString::offsetOfValue()), storage);
    
    if (!needsTypeCheck(edge, SpecStringIdent | ~SpecString))
        return;

    speculationCheck(
        BadType, JSValueSource::unboxedCell(string), edge,
        m_jit.branchTestPtr(MacroAssembler::Zero, storage));
    speculationCheck(
        BadType, JSValueSource::unboxedCell(string), edge, m_jit.branchTest32(
            MacroAssembler::Zero,
            MacroAssembler::Address(storage, StringImpl::flagsOffset()),
            MacroAssembler::TrustedImm32(StringImpl::flagIsAtomic())));
    
    m_interpreter.filter(edge, SpecStringIdent | ~SpecString);
}

void SpeculativeJIT::speculateStringIdent(Edge edge, GPRReg string)
{
    if (!needsTypeCheck(edge, SpecStringIdent))
        return;

    GPRTemporary temp(this);
    speculateStringIdentAndLoadStorage(edge, string, temp.gpr());
}

void SpeculativeJIT::speculateStringIdent(Edge edge)
{
    if (!needsTypeCheck(edge, SpecStringIdent))
        return;
    
    SpeculateCellOperand operand(this, edge);
    GPRReg gpr = operand.gpr();
    speculateString(edge, gpr);
    speculateStringIdent(edge, gpr);
}

void SpeculativeJIT::speculateString(Edge edge)
{
    if (!needsTypeCheck(edge, SpecString))
        return;
    
    SpeculateCellOperand operand(this, edge);
    speculateString(edge, operand.gpr());
}

void SpeculativeJIT::speculateStringObject(Edge edge, GPRReg gpr)
{
    speculateStringObjectForStructure(edge, JITCompiler::Address(gpr, JSCell::structureIDOffset()));
}

void SpeculativeJIT::speculateStringObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecStringObject))
        return;
    
    SpeculateCellOperand operand(this, edge);
    GPRReg gpr = operand.gpr();
    if (!needsTypeCheck(edge, SpecStringObject))
        return;
    
    speculateStringObject(edge, gpr);
    m_interpreter.filter(edge, SpecStringObject);
}

void SpeculativeJIT::speculateStringOrStringObject(Edge edge)
{
    if (!needsTypeCheck(edge, SpecString | SpecStringObject))
        return;
    
    SpeculateCellOperand operand(this, edge);
    GPRReg gpr = operand.gpr();
    if (!needsTypeCheck(edge, SpecString | SpecStringObject))
        return;

    GPRTemporary structureID(this);
    GPRReg structureIDGPR = structureID.gpr();

    m_jit.load32(JITCompiler::Address(gpr, JSCell::structureIDOffset()), structureIDGPR); 
    JITCompiler::Jump isString = m_jit.branchWeakStructure(
        JITCompiler::Equal,
        structureIDGPR, 
        m_jit.graph().registerStructure(m_jit.vm()->stringStructure.get()));
    
    speculateStringObjectForStructure(edge, structureIDGPR);
    
    isString.link(&m_jit);
    
    m_interpreter.filter(edge, SpecString | SpecStringObject);
}

void SpeculativeJIT::speculateNotStringVar(Edge edge)
{
    JSValueOperand operand(this, edge, ManualOperandSpeculation);
    GPRTemporary temp(this);
    GPRReg tempGPR = temp.gpr();
    
    JITCompiler::Jump notCell = m_jit.branchIfNotCell(operand.jsValueRegs());
    GPRReg cell = operand.jsValueRegs().payloadGPR();
    
    JITCompiler::Jump notString = m_jit.branchIfNotString(cell);
    
    speculateStringIdentAndLoadStorage(edge, cell, tempGPR);
    
    notString.link(&m_jit);
    notCell.link(&m_jit);
}

void SpeculativeJIT::speculateSymbol(Edge edge, GPRReg cell)
{
    DFG_TYPE_CHECK(JSValueSource::unboxedCell(cell), edge, SpecSymbol, m_jit.branchIfNotSymbol(cell));
}

void SpeculativeJIT::speculateSymbol(Edge edge)
{
    if (!needsTypeCheck(edge, SpecSymbol))
        return;

    SpeculateCellOperand operand(this, edge);
    speculateSymbol(edge, operand.gpr());
}

void SpeculativeJIT::speculateNotCell(Edge edge, JSValueRegs regs)
{
    DFG_TYPE_CHECK(regs, edge, ~SpecCell, m_jit.branchIfCell(regs));
}

void SpeculativeJIT::speculateNotCell(Edge edge)
{
    if (!needsTypeCheck(edge, ~SpecCell))
        return;
    
    JSValueOperand operand(this, edge, ManualOperandSpeculation); 
    speculateNotCell(edge, operand.jsValueRegs());
}

void SpeculativeJIT::speculateOther(Edge edge)
{
    if (!needsTypeCheck(edge, SpecOther))
        return;
    
    JSValueOperand operand(this, edge, ManualOperandSpeculation);
    GPRTemporary temp(this);
    GPRReg tempGPR = temp.gpr();
    typeCheck(
        operand.jsValueRegs(), edge, SpecOther,
        m_jit.branchIfNotOther(operand.jsValueRegs(), tempGPR));
}

void SpeculativeJIT::speculateMisc(Edge edge, JSValueRegs regs)
{
#if USE(JSVALUE64)
    DFG_TYPE_CHECK(
        regs, edge, SpecMisc,
        m_jit.branch64(MacroAssembler::Above, regs.gpr(), MacroAssembler::TrustedImm64(TagBitTypeOther | TagBitBool | TagBitUndefined)));
#else
    DFG_TYPE_CHECK(
        regs, edge, ~SpecInt32Only,
        m_jit.branch32(MacroAssembler::Equal, regs.tagGPR(), MacroAssembler::TrustedImm32(JSValue::Int32Tag)));
    DFG_TYPE_CHECK(
        regs, edge, SpecMisc,
        m_jit.branch32(MacroAssembler::Below, regs.tagGPR(), MacroAssembler::TrustedImm32(JSValue::UndefinedTag)));
#endif
}

void SpeculativeJIT::speculateMisc(Edge edge)
{
    if (!needsTypeCheck(edge, SpecMisc))
        return;
    
    JSValueOperand operand(this, edge, ManualOperandSpeculation);
    speculateMisc(edge, operand.jsValueRegs());
}

void SpeculativeJIT::speculate(Node*, Edge edge)
{
    switch (edge.useKind()) {
    case UntypedUse:
        break;
    case KnownInt32Use:
        ASSERT(!needsTypeCheck(edge, SpecInt32Only));
        break;
    case DoubleRepUse:
        ASSERT(!needsTypeCheck(edge, SpecFullDouble));
        break;
    case Int52RepUse:
        ASSERT(!needsTypeCheck(edge, SpecAnyInt));
        break;
    case KnownCellUse:
        ASSERT(!needsTypeCheck(edge, SpecCell));
        break;
    case KnownStringUse:
        ASSERT(!needsTypeCheck(edge, SpecString));
        break;
    case KnownPrimitiveUse:
        ASSERT(!needsTypeCheck(edge, SpecHeapTop & ~SpecObject));
        break;
    case Int32Use:
        speculateInt32(edge);
        break;
    case NumberUse:
        speculateNumber(edge);
        break;
    case RealNumberUse:
        speculateRealNumber(edge);
        break;
    case DoubleRepRealUse:
        speculateDoubleRepReal(edge);
        break;
#if USE(JSVALUE64)
    case AnyIntUse:
        speculateAnyInt(edge);
        break;
    case DoubleRepAnyIntUse:
        speculateDoubleRepAnyInt(edge);
        break;
#endif
    case BooleanUse:
        speculateBoolean(edge);
        break;
    case KnownBooleanUse:
        ASSERT(!needsTypeCheck(edge, SpecBoolean));
        break;
    case CellUse:
        speculateCell(edge);
        break;
    case CellOrOtherUse:
        speculateCellOrOther(edge);
        break;
    case ObjectUse:
        speculateObject(edge);
        break;
    case FunctionUse:
        speculateFunction(edge);
        break;
    case ArrayUse:
        speculateArray(edge);
        break;
    case FinalObjectUse:
        speculateFinalObject(edge);
        break;
    case RegExpObjectUse:
        speculateRegExpObject(edge);
        break;
    case ProxyObjectUse:
        speculateProxyObject(edge);
        break;
    case DerivedArrayUse:
        speculateDerivedArray(edge);
        break;
    case MapObjectUse:
        speculateMapObject(edge);
        break;
    case SetObjectUse:
        speculateSetObject(edge);
        break;
    case ObjectOrOtherUse:
        speculateObjectOrOther(edge);
        break;
    case StringIdentUse:
        speculateStringIdent(edge);
        break;
    case StringUse:
        speculateString(edge);
        break;
    case StringOrOtherUse:
        speculateStringOrOther(edge);
        break;
    case SymbolUse:
        speculateSymbol(edge);
        break;
    case StringObjectUse:
        speculateStringObject(edge);
        break;
    case StringOrStringObjectUse:
        speculateStringOrStringObject(edge);
        break;
    case NotStringVarUse:
        speculateNotStringVar(edge);
        break;
    case NotCellUse:
        speculateNotCell(edge);
        break;
    case OtherUse:
        speculateOther(edge);
        break;
    case MiscUse:
        speculateMisc(edge);
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

void SpeculativeJIT::emitSwitchIntJump(
    SwitchData* data, GPRReg value, GPRReg scratch)
{
    SimpleJumpTable& table = m_jit.codeBlock()->switchJumpTable(data->switchTableIndex);
    table.ensureCTITable();
    m_jit.sub32(Imm32(table.min), value);
    addBranch(
        m_jit.branch32(JITCompiler::AboveOrEqual, value, Imm32(table.ctiOffsets.size())),
        data->fallThrough.block);
    m_jit.move(TrustedImmPtr(table.ctiOffsets.begin()), scratch);
    m_jit.loadPtr(JITCompiler::BaseIndex(scratch, value, JITCompiler::timesPtr()), scratch);
    m_jit.jump(scratch);
    data->didUseJumpTable = true;
}

void SpeculativeJIT::emitSwitchImm(Node* node, SwitchData* data)
{
    switch (node->child1().useKind()) {
    case Int32Use: {
        SpeculateInt32Operand value(this, node->child1());
        GPRTemporary temp(this);
        emitSwitchIntJump(data, value.gpr(), temp.gpr());
        noResult(node);
        break;
    }
        
    case UntypedUse: {
        JSValueOperand value(this, node->child1());
        GPRTemporary temp(this);
        JSValueRegs valueRegs = value.jsValueRegs();
        GPRReg scratch = temp.gpr();
        
        value.use();
        
#if USE(JSVALUE64)
        JITCompiler::Jump notInt = m_jit.branch64(
            JITCompiler::Below, valueRegs.gpr(), GPRInfo::tagTypeNumberRegister);
        emitSwitchIntJump(data, valueRegs.gpr(), scratch);
        notInt.link(&m_jit);
        addBranch(
            m_jit.branchTest64(
                JITCompiler::Zero, valueRegs.gpr(), GPRInfo::tagTypeNumberRegister),
            data->fallThrough.block);
        silentSpillAllRegisters(scratch);
        callOperation(operationFindSwitchImmTargetForDouble, scratch, valueRegs.gpr(), data->switchTableIndex);
        silentFillAllRegisters(scratch);
        m_jit.jump(scratch);
#else
        JITCompiler::Jump notInt = m_jit.branch32(
            JITCompiler::NotEqual, valueRegs.tagGPR(), TrustedImm32(JSValue::Int32Tag));
        emitSwitchIntJump(data, valueRegs.payloadGPR(), scratch);
        notInt.link(&m_jit);
        addBranch(
            m_jit.branch32(
                JITCompiler::AboveOrEqual, valueRegs.tagGPR(),
                TrustedImm32(JSValue::LowestTag)),
            data->fallThrough.block);
        silentSpillAllRegisters(scratch);
        callOperation(operationFindSwitchImmTargetForDouble, scratch, valueRegs, data->switchTableIndex);
        silentFillAllRegisters(scratch);

        m_jit.jump(scratch);
#endif
        noResult(node, UseChildrenCalledExplicitly);
        break;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

void SpeculativeJIT::emitSwitchCharStringJump(
    SwitchData* data, GPRReg value, GPRReg scratch)
{
    addBranch(
        m_jit.branch32(
            MacroAssembler::NotEqual,
            MacroAssembler::Address(value, JSString::offsetOfLength()),
            TrustedImm32(1)),
        data->fallThrough.block);
    
    m_jit.loadPtr(MacroAssembler::Address(value, JSString::offsetOfValue()), scratch);
    
    addSlowPathGenerator(
        slowPathCall(
            m_jit.branchTestPtr(MacroAssembler::Zero, scratch),
            this, operationResolveRope, scratch, value));
    
    m_jit.loadPtr(MacroAssembler::Address(scratch, StringImpl::dataOffset()), value);
    
    JITCompiler::Jump is8Bit = m_jit.branchTest32(
        MacroAssembler::NonZero,
        MacroAssembler::Address(scratch, StringImpl::flagsOffset()),
        TrustedImm32(StringImpl::flagIs8Bit()));
    
    m_jit.load16(MacroAssembler::Address(value), scratch);
    
    JITCompiler::Jump ready = m_jit.jump();
    
    is8Bit.link(&m_jit);
    m_jit.load8(MacroAssembler::Address(value), scratch);
    
    ready.link(&m_jit);
    emitSwitchIntJump(data, scratch, value);
}

void SpeculativeJIT::emitSwitchChar(Node* node, SwitchData* data)
{
    switch (node->child1().useKind()) {
    case StringUse: {
        SpeculateCellOperand op1(this, node->child1());
        GPRTemporary temp(this);
        
        GPRReg op1GPR = op1.gpr();
        GPRReg tempGPR = temp.gpr();
        
        op1.use();

        speculateString(node->child1(), op1GPR);
        emitSwitchCharStringJump(data, op1GPR, tempGPR);
        noResult(node, UseChildrenCalledExplicitly);
        break;
    }
        
    case UntypedUse: {
        JSValueOperand op1(this, node->child1());
        GPRTemporary temp(this);
        
        JSValueRegs op1Regs = op1.jsValueRegs();
        GPRReg tempGPR = temp.gpr();
        
        op1.use();
        
        addBranch(m_jit.branchIfNotCell(op1Regs), data->fallThrough.block);
        
        addBranch(m_jit.branchIfNotString(op1Regs.payloadGPR()), data->fallThrough.block);
        
        emitSwitchCharStringJump(data, op1Regs.payloadGPR(), tempGPR);
        noResult(node, UseChildrenCalledExplicitly);
        break;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

namespace {

struct CharacterCase {
    bool operator<(const CharacterCase& other) const
    {
        return character < other.character;
    }
    
    LChar character;
    unsigned begin;
    unsigned end;
};

} // anonymous namespace

void SpeculativeJIT::emitBinarySwitchStringRecurse(
    SwitchData* data, const Vector<SpeculativeJIT::StringSwitchCase>& cases,
    unsigned numChecked, unsigned begin, unsigned end, GPRReg buffer, GPRReg length,
    GPRReg temp, unsigned alreadyCheckedLength, bool checkedExactLength)
{
    static const bool verbose = false;
    
    if (verbose) {
        dataLog("We're down to the following cases, alreadyCheckedLength = ", alreadyCheckedLength, ":\n");
        for (unsigned i = begin; i < end; ++i) {
            dataLog("    ", cases[i].string, "\n");
        }
    }
    
    if (begin == end) {
        jump(data->fallThrough.block, ForceJump);
        return;
    }
    
    unsigned minLength = cases[begin].string->length();
    unsigned commonChars = minLength;
    bool allLengthsEqual = true;
    for (unsigned i = begin + 1; i < end; ++i) {
        unsigned myCommonChars = numChecked;
        for (unsigned j = numChecked;
            j < std::min(cases[begin].string->length(), cases[i].string->length());
            ++j) {
            if (cases[begin].string->at(j) != cases[i].string->at(j)) {
                if (verbose)
                    dataLog("string(", cases[i].string, ")[", j, "] != string(", cases[begin].string, ")[", j, "]\n");
                break;
            }
            myCommonChars++;
        }
        commonChars = std::min(commonChars, myCommonChars);
        if (minLength != cases[i].string->length())
            allLengthsEqual = false;
        minLength = std::min(minLength, cases[i].string->length());
    }
    
    if (checkedExactLength) {
        RELEASE_ASSERT(alreadyCheckedLength == minLength);
        RELEASE_ASSERT(allLengthsEqual);
    }
    
    RELEASE_ASSERT(minLength >= commonChars);
    
    if (verbose)
        dataLog("length = ", minLength, ", commonChars = ", commonChars, ", allLengthsEqual = ", allLengthsEqual, "\n");
    
    if (!allLengthsEqual && alreadyCheckedLength < minLength)
        branch32(MacroAssembler::Below, length, Imm32(minLength), data->fallThrough.block);
    if (allLengthsEqual && (alreadyCheckedLength < minLength || !checkedExactLength))
        branch32(MacroAssembler::NotEqual, length, Imm32(minLength), data->fallThrough.block);
    
    for (unsigned i = numChecked; i < commonChars; ++i) {
        branch8(
            MacroAssembler::NotEqual, MacroAssembler::Address(buffer, i),
            TrustedImm32(cases[begin].string->at(i)), data->fallThrough.block);
    }
    
    if (minLength == commonChars) {
        // This is the case where one of the cases is a prefix of all of the other cases.
        // We've already checked that the input string is a prefix of all of the cases,
        // so we just check length to jump to that case.
        
        if (!ASSERT_DISABLED) {
            ASSERT(cases[begin].string->length() == commonChars);
            for (unsigned i = begin + 1; i < end; ++i)
                ASSERT(cases[i].string->length() > commonChars);
        }
        
        if (allLengthsEqual) {
            RELEASE_ASSERT(end == begin + 1);
            jump(cases[begin].target, ForceJump);
            return;
        }
        
        branch32(MacroAssembler::Equal, length, Imm32(commonChars), cases[begin].target);
        
        // We've checked if the length is >= minLength, and then we checked if the
        // length is == commonChars. We get to this point if it is >= minLength but not
        // == commonChars. Hence we know that it now must be > minLength, i.e., that
        // it's >= minLength + 1.
        emitBinarySwitchStringRecurse(
            data, cases, commonChars, begin + 1, end, buffer, length, temp, minLength + 1, false);
        return;
    }
    
    // At this point we know that the string is longer than commonChars, and we've only
    // verified commonChars. Use a binary switch on the next unchecked character, i.e.
    // string[commonChars].
    
    RELEASE_ASSERT(end >= begin + 2);
    
    m_jit.load8(MacroAssembler::Address(buffer, commonChars), temp);
    
    Vector<CharacterCase> characterCases;
    CharacterCase currentCase;
    currentCase.character = cases[begin].string->at(commonChars);
    currentCase.begin = begin;
    currentCase.end = begin + 1;
    for (unsigned i = begin + 1; i < end; ++i) {
        if (cases[i].string->at(commonChars) != currentCase.character) {
            if (verbose)
                dataLog("string(", cases[i].string, ")[", commonChars, "] != string(", cases[begin].string, ")[", commonChars, "]\n");
            currentCase.end = i;
            characterCases.append(currentCase);
            currentCase.character = cases[i].string->at(commonChars);
            currentCase.begin = i;
            currentCase.end = i + 1;
        } else
            currentCase.end = i + 1;
    }
    characterCases.append(currentCase);
    
    Vector<int64_t> characterCaseValues;
    for (unsigned i = 0; i < characterCases.size(); ++i)
        characterCaseValues.append(characterCases[i].character);
    
    BinarySwitch binarySwitch(temp, characterCaseValues, BinarySwitch::Int32);
    while (binarySwitch.advance(m_jit)) {
        const CharacterCase& myCase = characterCases[binarySwitch.caseIndex()];
        emitBinarySwitchStringRecurse(
            data, cases, commonChars + 1, myCase.begin, myCase.end, buffer, length,
            temp, minLength, allLengthsEqual);
    }
    
    addBranch(binarySwitch.fallThrough(), data->fallThrough.block);
}

void SpeculativeJIT::emitSwitchStringOnString(SwitchData* data, GPRReg string)
{
    data->didUseJumpTable = true;
    
    bool canDoBinarySwitch = true;
    unsigned totalLength = 0;
    
    for (unsigned i = data->cases.size(); i--;) {
        StringImpl* string = data->cases[i].value.stringImpl();
        if (!string->is8Bit()) {
            canDoBinarySwitch = false;
            break;
        }
        if (string->length() > Options::maximumBinaryStringSwitchCaseLength()) {
            canDoBinarySwitch = false;
            break;
        }
        totalLength += string->length();
    }
    
    if (!canDoBinarySwitch || totalLength > Options::maximumBinaryStringSwitchTotalLength()) {
        flushRegisters();
        callOperation(
            operationSwitchString, string, data->switchTableIndex, string);
        m_jit.exceptionCheck();
        m_jit.jump(string);
        return;
    }
    
    GPRTemporary length(this);
    GPRTemporary temp(this);
    
    GPRReg lengthGPR = length.gpr();
    GPRReg tempGPR = temp.gpr();
    
    m_jit.load32(MacroAssembler::Address(string, JSString::offsetOfLength()), lengthGPR);
    m_jit.loadPtr(MacroAssembler::Address(string, JSString::offsetOfValue()), tempGPR);
    
    MacroAssembler::JumpList slowCases;
    slowCases.append(m_jit.branchTestPtr(MacroAssembler::Zero, tempGPR));
    slowCases.append(m_jit.branchTest32(
        MacroAssembler::Zero,
        MacroAssembler::Address(tempGPR, StringImpl::flagsOffset()),
        TrustedImm32(StringImpl::flagIs8Bit())));
    
    m_jit.loadPtr(MacroAssembler::Address(tempGPR, StringImpl::dataOffset()), string);
    
    Vector<StringSwitchCase> cases;
    for (unsigned i = 0; i < data->cases.size(); ++i) {
        cases.append(
            StringSwitchCase(data->cases[i].value.stringImpl(), data->cases[i].target.block));
    }
    
    std::sort(cases.begin(), cases.end());
    
    emitBinarySwitchStringRecurse(
        data, cases, 0, 0, cases.size(), string, lengthGPR, tempGPR, 0, false);
    
    slowCases.link(&m_jit);
    silentSpillAllRegisters(string);
    callOperation(operationSwitchString, string, data->switchTableIndex, string);
    silentFillAllRegisters(string);
    m_jit.exceptionCheck();
    m_jit.jump(string);
}

void SpeculativeJIT::emitSwitchString(Node* node, SwitchData* data)
{
    switch (node->child1().useKind()) {
    case StringIdentUse: {
        SpeculateCellOperand op1(this, node->child1());
        GPRTemporary temp(this);
        
        GPRReg op1GPR = op1.gpr();
        GPRReg tempGPR = temp.gpr();
        
        speculateString(node->child1(), op1GPR);
        speculateStringIdentAndLoadStorage(node->child1(), op1GPR, tempGPR);
        
        Vector<int64_t> identifierCaseValues;
        for (unsigned i = 0; i < data->cases.size(); ++i) {
            identifierCaseValues.append(
                static_cast<int64_t>(bitwise_cast<intptr_t>(data->cases[i].value.stringImpl())));
        }
        
        BinarySwitch binarySwitch(tempGPR, identifierCaseValues, BinarySwitch::IntPtr);
        while (binarySwitch.advance(m_jit))
            jump(data->cases[binarySwitch.caseIndex()].target.block, ForceJump);
        addBranch(binarySwitch.fallThrough(), data->fallThrough.block);
        
        noResult(node);
        break;
    }
        
    case StringUse: {
        SpeculateCellOperand op1(this, node->child1());
        
        GPRReg op1GPR = op1.gpr();
        
        op1.use();

        speculateString(node->child1(), op1GPR);
        emitSwitchStringOnString(data, op1GPR);
        noResult(node, UseChildrenCalledExplicitly);
        break;
    }
        
    case UntypedUse: {
        JSValueOperand op1(this, node->child1());
        
        JSValueRegs op1Regs = op1.jsValueRegs();
        
        op1.use();
        
        addBranch(m_jit.branchIfNotCell(op1Regs), data->fallThrough.block);
        
        addBranch(m_jit.branchIfNotString(op1Regs.payloadGPR()), data->fallThrough.block);
        
        emitSwitchStringOnString(data, op1Regs.payloadGPR());
        noResult(node, UseChildrenCalledExplicitly);
        break;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

void SpeculativeJIT::emitSwitch(Node* node)
{
    SwitchData* data = node->switchData();
    switch (data->kind) {
    case SwitchImm: {
        emitSwitchImm(node, data);
        return;
    }
    case SwitchChar: {
        emitSwitchChar(node, data);
        return;
    }
    case SwitchString: {
        emitSwitchString(node, data);
        return;
    }
    case SwitchCell: {
        DFG_CRASH(m_jit.graph(), node, "Bad switch kind");
        return;
    } }
    RELEASE_ASSERT_NOT_REACHED();
}

void SpeculativeJIT::addBranch(const MacroAssembler::JumpList& jump, BasicBlock* destination)
{
    for (unsigned i = jump.jumps().size(); i--;)
        addBranch(jump.jumps()[i], destination);
}

void SpeculativeJIT::linkBranches()
{
    for (auto& branch : m_branches)
        branch.jump.linkTo(m_jit.blockHeads()[branch.destination->index], &m_jit);
}

void SpeculativeJIT::compileStoreBarrier(Node* node)
{
    ASSERT(node->op() == StoreBarrier || node->op() == FencedStoreBarrier);
    
    bool isFenced = node->op() == FencedStoreBarrier;
    
    SpeculateCellOperand base(this, node->child1());
    GPRTemporary scratch1(this);
    
    GPRReg baseGPR = base.gpr();
    GPRReg scratch1GPR = scratch1.gpr();
    
    JITCompiler::JumpList ok;
    
    if (isFenced) {
        ok.append(m_jit.barrierBranch(baseGPR, scratch1GPR));
        
        JITCompiler::Jump noFence = m_jit.jumpIfMutatorFenceNotNeeded();
        m_jit.memoryFence();
        ok.append(m_jit.barrierBranchWithoutFence(baseGPR));
        noFence.link(&m_jit);
    } else
        ok.append(m_jit.barrierBranchWithoutFence(baseGPR));

    silentSpillAllRegisters(InvalidGPRReg);
    callOperation(operationWriteBarrierSlowPath, baseGPR);
    silentFillAllRegisters(InvalidGPRReg);

    ok.link(&m_jit);

    noResult(node);
}

void SpeculativeJIT::compilePutAccessorById(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    SpeculateCellOperand accessor(this, node->child2());

    GPRReg baseGPR = base.gpr();
    GPRReg accessorGPR = accessor.gpr();

    flushRegisters();
    callOperation(node->op() == PutGetterById ? operationPutGetterById : operationPutSetterById, NoResult, baseGPR, identifierUID(node->identifierNumber()), node->accessorAttributes(), accessorGPR);
    m_jit.exceptionCheck();

    noResult(node);
}

void SpeculativeJIT::compilePutGetterSetterById(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    JSValueOperand getter(this, node->child2());
    JSValueOperand setter(this, node->child3());

#if USE(JSVALUE64)
    GPRReg baseGPR = base.gpr();
    GPRReg getterGPR = getter.gpr();
    GPRReg setterGPR = setter.gpr();

    flushRegisters();
    callOperation(operationPutGetterSetter, NoResult, baseGPR, identifierUID(node->identifierNumber()), node->accessorAttributes(), getterGPR, setterGPR);
#else
    // These JSValues may be JSUndefined OR JSFunction*.
    // At that time,
    // 1. If the JSValue is JSUndefined, its payload becomes nullptr.
    // 2. If the JSValue is JSFunction*, its payload becomes JSFunction*.
    // So extract payload and pass it to operationPutGetterSetter. This hack is used as the same way in baseline JIT.
    GPRReg baseGPR = base.gpr();
    JSValueRegs getterRegs = getter.jsValueRegs();
    JSValueRegs setterRegs = setter.jsValueRegs();

    flushRegisters();
    callOperation(operationPutGetterSetter, NoResult, baseGPR, identifierUID(node->identifierNumber()), node->accessorAttributes(), getterRegs.payloadGPR(), setterRegs.payloadGPR());
#endif
    m_jit.exceptionCheck();

    noResult(node);
}

void SpeculativeJIT::compileResolveScope(Node* node)
{
    SpeculateCellOperand scope(this, node->child1());
    GPRReg scopeGPR = scope.gpr();
    GPRFlushedCallResult result(this);
    GPRReg resultGPR = result.gpr();
    flushRegisters();
    callOperation(operationResolveScope, resultGPR, scopeGPR, identifierUID(node->identifierNumber()));
    m_jit.exceptionCheck();
    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileGetDynamicVar(Node* node)
{
    SpeculateCellOperand scope(this, node->child1());
    GPRReg scopeGPR = scope.gpr();
#if USE(JSVALUE64)
    flushRegisters();
    GPRFlushedCallResult result(this);
    callOperation(operationGetDynamicVar, result.gpr(), scopeGPR, identifierUID(node->identifierNumber()), node->getPutInfo());
    m_jit.exceptionCheck();
    jsValueResult(result.gpr(), node);
#else
    flushRegisters();
    GPRFlushedCallResult2 resultTag(this);
    GPRFlushedCallResult resultPayload(this);
    callOperation(operationGetDynamicVar, JSValueRegs(resultTag.gpr(), resultPayload.gpr()), scopeGPR, identifierUID(node->identifierNumber()), node->getPutInfo());
    m_jit.exceptionCheck();
    jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
#endif
}

void SpeculativeJIT::compilePutDynamicVar(Node* node)
{
    SpeculateCellOperand scope(this, node->child1());
    GPRReg scopeGPR = scope.gpr();
    JSValueOperand value(this, node->child2());

#if USE(JSVALUE64)
    GPRReg valueGPR = value.gpr();
    flushRegisters();
    callOperation(operationPutDynamicVar, NoResult, scopeGPR, valueGPR, identifierUID(node->identifierNumber()), node->getPutInfo());
#else
    GPRReg tag = value.tagGPR();
    GPRReg payload = value.payloadGPR();
    flushRegisters();
    callOperation(operationPutDynamicVar, NoResult, scopeGPR, JSValueRegs(tag, payload), identifierUID(node->identifierNumber()), node->getPutInfo());
#endif
    m_jit.exceptionCheck();
    noResult(node);
}

void SpeculativeJIT::compilePutAccessorByVal(Node* node)
{
    SpeculateCellOperand base(this, node->child1());
    JSValueOperand subscript(this, node->child2());
    SpeculateCellOperand accessor(this, node->child3());

    auto operation = node->op() == PutGetterByVal ? operationPutGetterByVal : operationPutSetterByVal;
#if USE(JSVALUE64)
    GPRReg baseGPR = base.gpr();
    GPRReg subscriptGPR = subscript.gpr();
    GPRReg accessorGPR = accessor.gpr();

    flushRegisters();
    callOperation(operation, NoResult, baseGPR, subscriptGPR, node->accessorAttributes(), accessorGPR);
#else
    GPRReg baseGPR = base.gpr();
    JSValueRegs subscriptRegs = subscript.jsValueRegs();
    GPRReg accessorGPR = accessor.gpr();

    flushRegisters();
    callOperation(operation, NoResult, baseGPR, subscriptRegs, node->accessorAttributes(), accessorGPR);
#endif
    m_jit.exceptionCheck();

    noResult(node);
}

void SpeculativeJIT::compileGetRegExpObjectLastIndex(Node* node)
{
    SpeculateCellOperand regExp(this, node->child1());
    JSValueRegsTemporary result(this);
    GPRReg regExpGPR = regExp.gpr();
    JSValueRegs resultRegs = result.regs();
    speculateRegExpObject(node->child1(), regExpGPR);
    m_jit.loadValue(JITCompiler::Address(regExpGPR, RegExpObject::offsetOfLastIndex()), resultRegs);
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileSetRegExpObjectLastIndex(Node* node)
{
    SpeculateCellOperand regExp(this, node->child1());
    JSValueOperand value(this, node->child2());
    GPRReg regExpGPR = regExp.gpr();
    JSValueRegs valueRegs = value.jsValueRegs();
    speculateRegExpObject(node->child1(), regExpGPR);
    speculationCheck(
        ExoticObjectMode, JSValueRegs(), nullptr,
        m_jit.branchTest8(
            JITCompiler::Zero,
            JITCompiler::Address(regExpGPR, RegExpObject::offsetOfLastIndexIsWritable())));
    m_jit.storeValue(valueRegs, JITCompiler::Address(regExpGPR, RegExpObject::offsetOfLastIndex()));
    noResult(node);
}

void SpeculativeJIT::compileLazyJSConstant(Node* node)
{
    JSValueRegsTemporary result(this);
    JSValueRegs resultRegs = result.regs();
    node->lazyJSValue().emit(m_jit, resultRegs);
    jsValueResult(resultRegs, node);
}

void SpeculativeJIT::compileMaterializeNewObject(Node* node)
{
    RegisteredStructure structure = node->structureSet().at(0);
    ASSERT(m_jit.graph().varArgChild(node, 0)->dynamicCastConstant<Structure*>(*m_jit.vm()) == structure.get());

    ObjectMaterializationData& data = node->objectMaterializationData();
        
    IndexingType indexingType = structure->indexingType();
    bool hasIndexingHeader = hasIndexedProperties(indexingType);
    int32_t publicLength = 0;
    int32_t vectorLength = 0;

    if (hasIndexingHeader) {
        for (unsigned i = data.m_properties.size(); i--;) {
            Edge edge = m_jit.graph().varArgChild(node, 1 + i);
            switch (data.m_properties[i].kind()) {
            case PublicLengthPLoc:
                publicLength = edge->asInt32();
                break;
            case VectorLengthPLoc:
                vectorLength = edge->asInt32();
                break;
            default:
                break;
            }
        }
    }

    GPRTemporary result(this);
    GPRTemporary storage(this);
    GPRReg resultGPR = result.gpr();
    GPRReg storageGPR = storage.gpr();
    
    emitAllocateRawObject(resultGPR, structure, storageGPR, 0, vectorLength);
    
    m_jit.store32(
        JITCompiler::TrustedImm32(publicLength),
        JITCompiler::Address(storageGPR, Butterfly::offsetOfPublicLength()));

    for (unsigned i = data.m_properties.size(); i--;) {
        Edge edge = m_jit.graph().varArgChild(node, 1 + i);
        PromotedLocationDescriptor descriptor = data.m_properties[i];
        switch (descriptor.kind()) {
        case IndexedPropertyPLoc: {
            JSValueOperand value(this, edge);
            m_jit.storeValue(
                value.jsValueRegs(),
                JITCompiler::Address(storageGPR, sizeof(EncodedJSValue) * descriptor.info()));
            break;
        }

        case NamedPropertyPLoc: {
            StringImpl* uid = m_jit.graph().identifiers()[descriptor.info()];
            for (PropertyMapEntry entry : structure->getPropertiesConcurrently()) {
                if (uid != entry.key)
                    continue;

                JSValueOperand value(this, edge);
                GPRReg baseGPR = isInlineOffset(entry.offset) ? resultGPR : storageGPR;
                m_jit.storeValue(
                    value.jsValueRegs(),
                    JITCompiler::Address(baseGPR, offsetRelativeToBase(entry.offset)));
            }
            break;
        }
            
        default:
            break;
        }
    }

    cellResult(resultGPR, node);
}

void SpeculativeJIT::compileRecordRegExpCachedResult(Node* node)
{
    Edge constructorEdge = m_jit.graph().varArgChild(node, 0);
    Edge regExpEdge = m_jit.graph().varArgChild(node, 1);
    Edge stringEdge = m_jit.graph().varArgChild(node, 2);
    Edge startEdge = m_jit.graph().varArgChild(node, 3);
    Edge endEdge = m_jit.graph().varArgChild(node, 4);

    SpeculateCellOperand constructor(this, constructorEdge);
    SpeculateCellOperand regExp(this, regExpEdge);
    SpeculateCellOperand string(this, stringEdge);
    SpeculateInt32Operand start(this, startEdge);
    SpeculateInt32Operand end(this, endEdge);

    GPRReg constructorGPR = constructor.gpr();
    GPRReg regExpGPR = regExp.gpr();
    GPRReg stringGPR = string.gpr();
    GPRReg startGPR = start.gpr();
    GPRReg endGPR = end.gpr();

    ptrdiff_t offset = RegExpConstructor::offsetOfCachedResult();

    m_jit.storePtr(
        regExpGPR,
        JITCompiler::Address(constructorGPR, offset + RegExpCachedResult::offsetOfLastRegExp()));
    m_jit.storePtr(
        stringGPR,
        JITCompiler::Address(constructorGPR, offset + RegExpCachedResult::offsetOfLastInput()));
    m_jit.store32(
        startGPR,
        JITCompiler::Address(
            constructorGPR,
            offset + RegExpCachedResult::offsetOfResult() + OBJECT_OFFSETOF(MatchResult, start)));
    m_jit.store32(
        endGPR,
        JITCompiler::Address(
            constructorGPR,
            offset + RegExpCachedResult::offsetOfResult() + OBJECT_OFFSETOF(MatchResult, end)));
    m_jit.store8(
        TrustedImm32(0),
        JITCompiler::Address(constructorGPR, offset + RegExpCachedResult::offsetOfReified()));

    noResult(node);
}

void SpeculativeJIT::compileCompareEqPtr(Node* node)
{
    JSValueOperand operand(this, node->child1());
    GPRTemporary result(this);
    JSValueRegs regs = operand.jsValueRegs();
    GPRReg resultGPR = result.gpr();
    m_jit.boxBooleanPayload(false, resultGPR);
    JITCompiler::JumpList notEqual = m_jit.branchIfNotEqual(regs, node->cellOperand()->value());
    m_jit.boxBooleanPayload(true, resultGPR);
    notEqual.link(&m_jit);
    blessedBooleanResult(resultGPR, node);
}

void SpeculativeJIT::compileDefineDataProperty(Node* node)
{
#if USE(JSVALUE64)
    static_assert(GPRInfo::numberOfRegisters >= 5, "We are assuming we have enough registers to make this call without incrementally setting up the arguments.");
#else
    static_assert(GPRInfo::numberOfRegisters >= 6, "We are assuming we have enough registers to make this call without incrementally setting up the arguments.");
#endif

    SpeculateCellOperand base(this, m_jit.graph().varArgChild(node, 0));
    GPRReg baseGPR = base.gpr();

    JSValueOperand value(this, m_jit.graph().varArgChild(node, 2));
    JSValueRegs valueRegs = value.jsValueRegs();

    SpeculateInt32Operand attributes(this, m_jit.graph().varArgChild(node, 3));
    GPRReg attributesGPR = attributes.gpr();

    Edge& propertyEdge = m_jit.graph().varArgChild(node, 1);
    switch (propertyEdge.useKind()) {
    case StringUse: {
        SpeculateCellOperand property(this, propertyEdge);
        GPRReg propertyGPR = property.gpr();
        speculateString(propertyEdge, propertyGPR);

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineDataPropertyString, NoResult, baseGPR, propertyGPR, valueRegs, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    case StringIdentUse: {
        SpeculateCellOperand property(this, propertyEdge);
        GPRTemporary ident(this);

        GPRReg propertyGPR = property.gpr();
        GPRReg identGPR = ident.gpr();

        speculateString(propertyEdge, propertyGPR);
        speculateStringIdentAndLoadStorage(propertyEdge, propertyGPR, identGPR);

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineDataPropertyStringIdent, NoResult, baseGPR, identGPR, valueRegs, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    case SymbolUse: {
        SpeculateCellOperand property(this, propertyEdge);
        GPRReg propertyGPR = property.gpr();
        speculateSymbol(propertyEdge, propertyGPR);

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineDataPropertySymbol, NoResult, baseGPR, propertyGPR, valueRegs, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    case UntypedUse: {
        JSValueOperand property(this, propertyEdge);
        JSValueRegs propertyRegs = property.jsValueRegs();

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineDataProperty, NoResult, baseGPR, propertyRegs, valueRegs, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }

    noResult(node, UseChildrenCalledExplicitly);
}

void SpeculativeJIT::compileDefineAccessorProperty(Node* node)
{
#if USE(JSVALUE64)
    static_assert(GPRInfo::numberOfRegisters >= 5, "We are assuming we have enough registers to make this call without incrementally setting up the arguments.");
#else
    static_assert(GPRInfo::numberOfRegisters >= 6, "We are assuming we have enough registers to make this call without incrementally setting up the arguments.");
#endif

    SpeculateCellOperand base(this, m_jit.graph().varArgChild(node, 0));
    GPRReg baseGPR = base.gpr();

    SpeculateCellOperand getter(this, m_jit.graph().varArgChild(node, 2));
    GPRReg getterGPR = getter.gpr();

    SpeculateCellOperand setter(this, m_jit.graph().varArgChild(node, 3));
    GPRReg setterGPR = setter.gpr();

    SpeculateInt32Operand attributes(this, m_jit.graph().varArgChild(node, 4));
    GPRReg attributesGPR = attributes.gpr();

    Edge& propertyEdge = m_jit.graph().varArgChild(node, 1);
    switch (propertyEdge.useKind()) {
    case StringUse: {
        SpeculateCellOperand property(this, propertyEdge);
        GPRReg propertyGPR = property.gpr();
        speculateString(propertyEdge, propertyGPR);

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineAccessorPropertyString, NoResult, baseGPR, propertyGPR, getterGPR, setterGPR, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    case StringIdentUse: {
        SpeculateCellOperand property(this, propertyEdge);
        GPRTemporary ident(this);

        GPRReg propertyGPR = property.gpr();
        GPRReg identGPR = ident.gpr();

        speculateString(propertyEdge, propertyGPR);
        speculateStringIdentAndLoadStorage(propertyEdge, propertyGPR, identGPR);

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineAccessorPropertyStringIdent, NoResult, baseGPR, identGPR, getterGPR, setterGPR, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    case SymbolUse: {
        SpeculateCellOperand property(this, propertyEdge);
        GPRReg propertyGPR = property.gpr();
        speculateSymbol(propertyEdge, propertyGPR);

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineAccessorPropertySymbol, NoResult, baseGPR, propertyGPR, getterGPR, setterGPR, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    case UntypedUse: {
        JSValueOperand property(this, propertyEdge);
        JSValueRegs propertyRegs = property.jsValueRegs();

        useChildren(node);

        flushRegisters();
        callOperation(operationDefineAccessorProperty, NoResult, baseGPR, propertyRegs, getterGPR, setterGPR, attributesGPR);
        m_jit.exceptionCheck();
        break;
    }
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }

    noResult(node, UseChildrenCalledExplicitly);
}

void SpeculativeJIT::emitAllocateButterfly(GPRReg storageResultGPR, GPRReg sizeGPR, GPRReg scratch1, GPRReg scratch2, GPRReg scratch3, MacroAssembler::JumpList& slowCases)
{
    RELEASE_ASSERT(RegisterSet(storageResultGPR, sizeGPR, scratch1, scratch2, scratch3).numberOfSetGPRs() == 5);
    ASSERT((1 << 3) == sizeof(JSValue));
    m_jit.zeroExtend32ToPtr(sizeGPR, scratch1);
    m_jit.lshift32(TrustedImm32(3), scratch1);
    m_jit.add32(TrustedImm32(sizeof(IndexingHeader)), scratch1, scratch2);
    m_jit.emitAllocateVariableSized(
        storageResultGPR, m_jit.vm()->auxiliarySpace, scratch2, scratch1, scratch3, slowCases);
    m_jit.addPtr(TrustedImm32(sizeof(IndexingHeader)), storageResultGPR);

    m_jit.store32(sizeGPR, MacroAssembler::Address(storageResultGPR, Butterfly::offsetOfPublicLength()));
    m_jit.store32(sizeGPR, MacroAssembler::Address(storageResultGPR, Butterfly::offsetOfVectorLength()));
}

} } // namespace JSC::DFG

#endif
