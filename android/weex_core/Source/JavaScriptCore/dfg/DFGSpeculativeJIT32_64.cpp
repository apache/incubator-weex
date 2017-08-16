/*
 * Copyright (C) 2011-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2011 Intel Corporation. All rights reserved.
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

#include "ArrayPrototype.h"
#include "CallFrameShuffler.h"
#include "DFGAbstractInterpreterInlines.h"
#include "DFGCallArrayAllocatorSlowPathGenerator.h"
#include "DFGOperations.h"
#include "DFGSlowPathGenerator.h"
#include "DirectArguments.h"
#include "GetterSetter.h"
#include "HasOwnPropertyCache.h"
#include "HashMapImpl.h"
#include "JSEnvironmentRecord.h"
#include "JSPropertyNameEnumerator.h"
#include "ObjectPrototype.h"
#include "JSCInlines.h"
#include "SetupVarargsFrame.h"
#include "TypeProfilerLog.h"
#include "Watchdog.h"

namespace JSC { namespace DFG {

#if USE(JSVALUE32_64)

bool SpeculativeJIT::fillJSValue(Edge edge, GPRReg& tagGPR, GPRReg& payloadGPR, FPRReg& fpr)
{
    // FIXME: For double we could fill with a FPR.
    UNUSED_PARAM(fpr);

    VirtualRegister virtualRegister = edge->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);

    switch (info.registerFormat()) {
    case DataFormatNone: {

        if (edge->hasConstant()) {
            tagGPR = allocate();
            payloadGPR = allocate();
            JSValue value = edge->asJSValue();
            m_jit.move(Imm32(value.tag()), tagGPR);
            m_jit.move(Imm32(value.payload()), payloadGPR);
            m_gprs.retain(tagGPR, virtualRegister, SpillOrderConstant);
            m_gprs.retain(payloadGPR, virtualRegister, SpillOrderConstant);
            info.fillJSValue(*m_stream, tagGPR, payloadGPR, DataFormatJS);
        } else {
            DataFormat spillFormat = info.spillFormat();
            ASSERT(spillFormat != DataFormatNone && spillFormat != DataFormatStorage);
            tagGPR = allocate();
            payloadGPR = allocate();
            switch (spillFormat) {
            case DataFormatInt32:
                m_jit.move(TrustedImm32(JSValue::Int32Tag), tagGPR);
                spillFormat = DataFormatJSInt32; // This will be used as the new register format.
                break;
            case DataFormatCell:
                m_jit.move(TrustedImm32(JSValue::CellTag), tagGPR);
                spillFormat = DataFormatJSCell; // This will be used as the new register format.
                break;
            case DataFormatBoolean:
                m_jit.move(TrustedImm32(JSValue::BooleanTag), tagGPR);
                spillFormat = DataFormatJSBoolean; // This will be used as the new register format.
                break;
            default:
                m_jit.load32(JITCompiler::tagFor(virtualRegister), tagGPR);
                break;
            }
            m_jit.load32(JITCompiler::payloadFor(virtualRegister), payloadGPR);
            m_gprs.retain(tagGPR, virtualRegister, SpillOrderSpilled);
            m_gprs.retain(payloadGPR, virtualRegister, SpillOrderSpilled);
            info.fillJSValue(*m_stream, tagGPR, payloadGPR, spillFormat == DataFormatJSDouble ? DataFormatJS : spillFormat);
        }

        return true;
    }

    case DataFormatInt32:
    case DataFormatCell:
    case DataFormatBoolean: {
        GPRReg gpr = info.gpr();
        // If the register has already been locked we need to take a copy.
        if (m_gprs.isLocked(gpr)) {
            payloadGPR = allocate();
            m_jit.move(gpr, payloadGPR);
        } else {
            payloadGPR = gpr;
            m_gprs.lock(gpr);
        }
        tagGPR = allocate();
        int32_t tag = JSValue::EmptyValueTag;
        DataFormat fillFormat = DataFormatJS;
        switch (info.registerFormat()) {
        case DataFormatInt32:
            tag = JSValue::Int32Tag;
            fillFormat = DataFormatJSInt32;
            break;
        case DataFormatCell:
            tag = JSValue::CellTag;
            fillFormat = DataFormatJSCell;
            break;
        case DataFormatBoolean:
            tag = JSValue::BooleanTag;
            fillFormat = DataFormatJSBoolean;
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        m_jit.move(TrustedImm32(tag), tagGPR);
        m_gprs.release(gpr);
        m_gprs.retain(tagGPR, virtualRegister, SpillOrderJS);
        m_gprs.retain(payloadGPR, virtualRegister, SpillOrderJS);
        info.fillJSValue(*m_stream, tagGPR, payloadGPR, fillFormat);
        return true;
    }

    case DataFormatJSDouble:
    case DataFormatJS:
    case DataFormatJSInt32:
    case DataFormatJSCell:
    case DataFormatJSBoolean: {
        tagGPR = info.tagGPR();
        payloadGPR = info.payloadGPR();
        m_gprs.lock(tagGPR);
        m_gprs.lock(payloadGPR);
        return true;
    }
        
    case DataFormatStorage:
    case DataFormatDouble:
        // this type currently never occurs
        RELEASE_ASSERT_NOT_REACHED();

    default:
        RELEASE_ASSERT_NOT_REACHED();
        return true;
    }
}

void SpeculativeJIT::cachedGetById(CodeOrigin origin, JSValueRegs base, JSValueRegs result, unsigned identifierNumber, JITCompiler::Jump slowPathTarget , SpillRegistersMode mode, AccessType type)
{
    cachedGetById(origin, base.tagGPR(), base.payloadGPR(), result.tagGPR(), result.payloadGPR(), identifierNumber, slowPathTarget, mode, type);
}

void SpeculativeJIT::cachedGetById(
    CodeOrigin codeOrigin, GPRReg baseTagGPROrNone, GPRReg basePayloadGPR, GPRReg resultTagGPR, GPRReg resultPayloadGPR,
    unsigned identifierNumber, JITCompiler::Jump slowPathTarget, SpillRegistersMode spillMode, AccessType type)
{
    // This is a hacky fix for when the register allocator decides to alias the base payload with the result tag. This only happens
    // in the case of GetByIdFlush, which has a relatively expensive register allocation story already so we probably don't need to
    // trip over one move instruction.
    if (basePayloadGPR == resultTagGPR) {
        RELEASE_ASSERT(basePayloadGPR != resultPayloadGPR);
        
        if (baseTagGPROrNone == resultPayloadGPR) {
            m_jit.swap(basePayloadGPR, baseTagGPROrNone);
            baseTagGPROrNone = resultTagGPR;
        } else
            m_jit.move(basePayloadGPR, resultPayloadGPR);
        basePayloadGPR = resultPayloadGPR;
    }
    
    RegisterSet usedRegisters = this->usedRegisters();
    if (spillMode == DontSpill) {
        // We've already flushed registers to the stack, we don't need to spill these.
        usedRegisters.set(JSValueRegs(baseTagGPROrNone, basePayloadGPR), false);
        usedRegisters.set(JSValueRegs(resultTagGPR, resultPayloadGPR), false);
    }
    
    CallSiteIndex callSite = m_jit.recordCallSiteAndGenerateExceptionHandlingOSRExitIfNeeded(codeOrigin, m_stream->size());
    JITGetByIdGenerator gen(
        m_jit.codeBlock(), codeOrigin, callSite, usedRegisters, identifierUID(identifierNumber),
        JSValueRegs(baseTagGPROrNone, basePayloadGPR), JSValueRegs(resultTagGPR, resultPayloadGPR), type);
    
    gen.generateFastPath(m_jit);
    
    JITCompiler::JumpList slowCases;
    if (slowPathTarget.isSet())
        slowCases.append(slowPathTarget);
    slowCases.append(gen.slowPathJump());

    J_JITOperation_ESsiJI getByIdFunction;
    if (type == AccessType::Get)
        getByIdFunction = operationGetByIdOptimize;
    else
        getByIdFunction = operationTryGetByIdOptimize;

    std::unique_ptr<SlowPathGenerator> slowPath;
    if (baseTagGPROrNone == InvalidGPRReg) {
        slowPath = slowPathCall(
            slowCases, this, getByIdFunction,
            JSValueRegs(resultTagGPR, resultPayloadGPR), gen.stubInfo(),
            static_cast<int32_t>(JSValue::CellTag), basePayloadGPR,
            identifierUID(identifierNumber));
    } else {
        slowPath = slowPathCall(
            slowCases, this, getByIdFunction,
            JSValueRegs(resultTagGPR, resultPayloadGPR), gen.stubInfo(), JSValueRegs(baseTagGPROrNone, basePayloadGPR), identifierUID(identifierNumber));
    }

    m_jit.addGetById(gen, slowPath.get());
    addSlowPathGenerator(WTFMove(slowPath));
}

void SpeculativeJIT::cachedGetByIdWithThis(
    CodeOrigin codeOrigin, GPRReg baseTagGPROrNone, GPRReg basePayloadGPR, GPRReg thisTagGPR, GPRReg thisPayloadGPR, GPRReg resultTagGPR, GPRReg resultPayloadGPR,
    unsigned identifierNumber, JITCompiler::JumpList slowPathTarget)
{
    RegisterSet usedRegisters = this->usedRegisters();
    
    CallSiteIndex callSite = m_jit.recordCallSiteAndGenerateExceptionHandlingOSRExitIfNeeded(codeOrigin, m_stream->size());
    JITGetByIdWithThisGenerator gen(
        m_jit.codeBlock(), codeOrigin, callSite, usedRegisters, identifierUID(identifierNumber),
        JSValueRegs(resultTagGPR, resultPayloadGPR), JSValueRegs(baseTagGPROrNone, basePayloadGPR), JSValueRegs(thisTagGPR, thisPayloadGPR), AccessType::GetWithThis);
    
    gen.generateFastPath(m_jit);
    
    JITCompiler::JumpList slowCases;
    if (!slowPathTarget.empty())
        slowCases.append(slowPathTarget);
    slowCases.append(gen.slowPathJump());

    std::unique_ptr<SlowPathGenerator> slowPath;
    if (baseTagGPROrNone == InvalidGPRReg && thisTagGPR == InvalidGPRReg) {
        slowPath = slowPathCall(
            slowCases, this, operationGetByIdWithThisOptimize,
            JSValueRegs(resultTagGPR, resultPayloadGPR), gen.stubInfo(),
            static_cast<int32_t>(JSValue::CellTag), basePayloadGPR,
            static_cast<int32_t>(JSValue::CellTag), thisPayloadGPR,
            identifierUID(identifierNumber));
    } else {
        ASSERT(baseTagGPROrNone != InvalidGPRReg);
        ASSERT(thisTagGPR != InvalidGPRReg);
        
        slowPath = slowPathCall(
            slowCases, this, operationGetByIdWithThisOptimize,
            JSValueRegs(resultTagGPR, resultPayloadGPR), gen.stubInfo(), JSValueRegs(baseTagGPROrNone, basePayloadGPR), JSValueRegs(thisTagGPR, thisPayloadGPR), identifierUID(identifierNumber));
    }

    m_jit.addGetByIdWithThis(gen, slowPath.get());
    addSlowPathGenerator(WTFMove(slowPath));
}

void SpeculativeJIT::cachedPutById(CodeOrigin codeOrigin, GPRReg basePayloadGPR, GPRReg valueTagGPR, GPRReg valuePayloadGPR, GPRReg scratchGPR, unsigned identifierNumber, PutKind putKind, JITCompiler::Jump slowPathTarget, SpillRegistersMode spillMode)
{
    RegisterSet usedRegisters = this->usedRegisters();
    if (spillMode == DontSpill) {
        // We've already flushed registers to the stack, we don't need to spill these.
        usedRegisters.set(basePayloadGPR, false);
        usedRegisters.set(JSValueRegs(valueTagGPR, valuePayloadGPR), false);
    }
    CallSiteIndex callSite = m_jit.recordCallSiteAndGenerateExceptionHandlingOSRExitIfNeeded(codeOrigin, m_stream->size());
    JITPutByIdGenerator gen(
        m_jit.codeBlock(), codeOrigin, callSite, usedRegisters,
        JSValueRegs::payloadOnly(basePayloadGPR), JSValueRegs(valueTagGPR, valuePayloadGPR),
        scratchGPR, m_jit.ecmaModeFor(codeOrigin), putKind);
    
    gen.generateFastPath(m_jit);
    
    JITCompiler::JumpList slowCases;
    if (slowPathTarget.isSet())
        slowCases.append(slowPathTarget);
    slowCases.append(gen.slowPathJump());

    auto slowPath = slowPathCall(
        slowCases, this, gen.slowPathFunction(), NoResult, gen.stubInfo(), JSValueRegs(valueTagGPR, valuePayloadGPR),
        basePayloadGPR, identifierUID(identifierNumber));

    m_jit.addPutById(gen, slowPath.get());
    addSlowPathGenerator(WTFMove(slowPath));
}

void SpeculativeJIT::nonSpeculativeNonPeepholeCompareNullOrUndefined(Edge operand)
{
    JSValueOperand arg(this, operand, ManualOperandSpeculation);
    GPRReg argTagGPR = arg.tagGPR();
    GPRReg argPayloadGPR = arg.payloadGPR();

    GPRTemporary resultPayload(this, Reuse, arg, PayloadWord);
    GPRReg resultPayloadGPR = resultPayload.gpr();

    JITCompiler::Jump notCell;
    JITCompiler::Jump notMasqueradesAsUndefined;   
    if (masqueradesAsUndefinedWatchpointIsStillValid()) {
        if (!isKnownCell(operand.node()))
            notCell = m_jit.branchIfNotCell(arg.jsValueRegs());
        
        m_jit.move(TrustedImm32(0), resultPayloadGPR);
        notMasqueradesAsUndefined = m_jit.jump();
    } else {
        GPRTemporary localGlobalObject(this);
        GPRTemporary remoteGlobalObject(this);

        if (!isKnownCell(operand.node()))
            notCell = m_jit.branchIfNotCell(arg.jsValueRegs());
        
        JITCompiler::Jump isMasqueradesAsUndefined = m_jit.branchTest8(
            JITCompiler::NonZero, 
            JITCompiler::Address(argPayloadGPR, JSCell::typeInfoFlagsOffset()), 
            JITCompiler::TrustedImm32(MasqueradesAsUndefined));
        
        m_jit.move(TrustedImm32(0), resultPayloadGPR);
        notMasqueradesAsUndefined = m_jit.jump();

        isMasqueradesAsUndefined.link(&m_jit);
        GPRReg localGlobalObjectGPR = localGlobalObject.gpr();
        GPRReg remoteGlobalObjectGPR = remoteGlobalObject.gpr();
        m_jit.move(TrustedImmPtr::weakPointer(m_jit.graph(), m_jit.graph().globalObjectFor(m_currentNode->origin.semantic)), localGlobalObjectGPR);
        m_jit.loadPtr(JITCompiler::Address(argPayloadGPR, JSCell::structureIDOffset()), resultPayloadGPR);
        m_jit.loadPtr(JITCompiler::Address(resultPayloadGPR, Structure::globalObjectOffset()), remoteGlobalObjectGPR);
        m_jit.compare32(JITCompiler::Equal, localGlobalObjectGPR, remoteGlobalObjectGPR, resultPayloadGPR);
    }
 
    if (!isKnownCell(operand.node())) {
        JITCompiler::Jump done = m_jit.jump();
        
        notCell.link(&m_jit);
        // null or undefined?
        COMPILE_ASSERT((JSValue::UndefinedTag | 1) == JSValue::NullTag, UndefinedTag_OR_1_EQUALS_NullTag);
        m_jit.or32(TrustedImm32(1), argTagGPR, resultPayloadGPR);
        m_jit.compare32(JITCompiler::Equal, resultPayloadGPR, TrustedImm32(JSValue::NullTag), resultPayloadGPR);

        done.link(&m_jit);
    }
    
    notMasqueradesAsUndefined.link(&m_jit);
 
    booleanResult(resultPayloadGPR, m_currentNode);
}

void SpeculativeJIT::nonSpeculativePeepholeBranchNullOrUndefined(Edge operand, Node* branchNode)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    bool invert = false;
    if (taken == nextBlock()) {
        invert = !invert;
        BasicBlock* tmp = taken;
        taken = notTaken;
        notTaken = tmp;
    }

    JSValueOperand arg(this, operand, ManualOperandSpeculation);
    GPRReg argTagGPR = arg.tagGPR();
    GPRReg argPayloadGPR = arg.payloadGPR();
    
    GPRTemporary result(this, Reuse, arg, TagWord);
    GPRReg resultGPR = result.gpr();

    JITCompiler::Jump notCell;

    if (masqueradesAsUndefinedWatchpointIsStillValid()) {
        if (!isKnownCell(operand.node()))
            notCell = m_jit.branchIfNotCell(arg.jsValueRegs());
        
        jump(invert ? taken : notTaken, ForceJump);
    } else {
        GPRTemporary localGlobalObject(this);
        GPRTemporary remoteGlobalObject(this);

        if (!isKnownCell(operand.node()))
            notCell = m_jit.branchIfNotCell(arg.jsValueRegs());
        
        branchTest8(JITCompiler::Zero, 
            JITCompiler::Address(argPayloadGPR, JSCell::typeInfoFlagsOffset()), 
            JITCompiler::TrustedImm32(MasqueradesAsUndefined), 
            invert ? taken : notTaken);
   
        GPRReg localGlobalObjectGPR = localGlobalObject.gpr();
        GPRReg remoteGlobalObjectGPR = remoteGlobalObject.gpr();
        m_jit.move(TrustedImmPtr::weakPointer(m_jit.graph(), m_jit.graph().globalObjectFor(m_currentNode->origin.semantic)), localGlobalObjectGPR);
        m_jit.loadPtr(JITCompiler::Address(argPayloadGPR, JSCell::structureIDOffset()), resultGPR);
        m_jit.loadPtr(JITCompiler::Address(resultGPR, Structure::globalObjectOffset()), remoteGlobalObjectGPR);
        branchPtr(JITCompiler::Equal, localGlobalObjectGPR, remoteGlobalObjectGPR, invert ? notTaken : taken);
    }
 
    if (!isKnownCell(operand.node())) {
        jump(notTaken, ForceJump);
        
        notCell.link(&m_jit);
        // null or undefined?
        COMPILE_ASSERT((JSValue::UndefinedTag | 1) == JSValue::NullTag, UndefinedTag_OR_1_EQUALS_NullTag);
        m_jit.or32(TrustedImm32(1), argTagGPR, resultGPR);
        branch32(invert ? JITCompiler::NotEqual : JITCompiler::Equal, resultGPR, JITCompiler::TrustedImm32(JSValue::NullTag), taken);
    }
    
    jump(notTaken);
}

void SpeculativeJIT::nonSpeculativePeepholeBranch(Node* node, Node* branchNode, MacroAssembler::RelationalCondition cond, S_JITOperation_EJJ helperFunction)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    JITCompiler::ResultCondition callResultCondition = JITCompiler::NonZero;

    // The branch instruction will branch to the taken block.
    // If taken is next, switch taken with notTaken & invert the branch condition so we can fall through.
    if (taken == nextBlock()) {
        cond = JITCompiler::invert(cond);
        callResultCondition = JITCompiler::Zero;
        BasicBlock* tmp = taken;
        taken = notTaken;
        notTaken = tmp;
    }

    JSValueOperand arg1(this, node->child1());
    JSValueOperand arg2(this, node->child2());
    JSValueRegs arg1Regs = arg1.jsValueRegs();
    JSValueRegs arg2Regs = arg2.jsValueRegs();
    GPRReg arg1TagGPR = arg1.tagGPR();
    GPRReg arg1PayloadGPR = arg1.payloadGPR();
    GPRReg arg2TagGPR = arg2.tagGPR();
    GPRReg arg2PayloadGPR = arg2.payloadGPR();
    
    JITCompiler::JumpList slowPath;
    
    if (isKnownNotInteger(node->child1().node()) || isKnownNotInteger(node->child2().node())) {
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();

        arg1.use();
        arg2.use();

        flushRegisters();
        callOperation(helperFunction, resultGPR, arg1Regs, arg2Regs);
        m_jit.exceptionCheck();

        branchTest32(callResultCondition, resultGPR, taken);
    } else {
        GPRTemporary result(this);
        GPRReg resultGPR = result.gpr();
    
        arg1.use();
        arg2.use();

        if (!isKnownInteger(node->child1().node()))
            slowPath.append(m_jit.branch32(MacroAssembler::NotEqual, arg1TagGPR, JITCompiler::TrustedImm32(JSValue::Int32Tag)));
        if (!isKnownInteger(node->child2().node()))
            slowPath.append(m_jit.branch32(MacroAssembler::NotEqual, arg2TagGPR, JITCompiler::TrustedImm32(JSValue::Int32Tag)));
    
        branch32(cond, arg1PayloadGPR, arg2PayloadGPR, taken);
    
        if (!isKnownInteger(node->child1().node()) || !isKnownInteger(node->child2().node())) {
            jump(notTaken, ForceJump);
    
            slowPath.link(&m_jit);
    
            silentSpillAllRegisters(resultGPR);
            callOperation(helperFunction, resultGPR, arg1Regs, arg2Regs);
            m_jit.exceptionCheck();
            silentFillAllRegisters(resultGPR);
        
            branchTest32(callResultCondition, resultGPR, taken);
        }
    }

    jump(notTaken);
    
    m_indexInBlock = m_block->size() - 1;
    m_currentNode = branchNode;
}

template<typename JumpType>
class CompareAndBoxBooleanSlowPathGenerator
    : public CallSlowPathGenerator<JumpType, S_JITOperation_EJJ, GPRReg> {
public:
    CompareAndBoxBooleanSlowPathGenerator(
        JumpType from, SpeculativeJIT* jit,
        S_JITOperation_EJJ function, GPRReg result, GPRReg arg1Tag, GPRReg arg1Payload,
        GPRReg arg2Tag, GPRReg arg2Payload)
        : CallSlowPathGenerator<JumpType, S_JITOperation_EJJ, GPRReg>(
            from, jit, function, NeedToSpill, ExceptionCheckRequirement::CheckNeeded, result)
        , m_arg1Tag(arg1Tag)
        , m_arg1Payload(arg1Payload)
        , m_arg2Tag(arg2Tag)
        , m_arg2Payload(arg2Payload)
    {
    }
    
protected:
    virtual void generateInternal(SpeculativeJIT* jit)
    {
        this->setUp(jit);
        this->recordCall(
            jit->callOperation(
                this->m_function, this->m_result, JSValueRegs(m_arg1Tag, m_arg1Payload), JSValueRegs(m_arg2Tag, m_arg2Payload)));
        jit->m_jit.and32(JITCompiler::TrustedImm32(1), this->m_result);
        this->tearDown(jit);
    }
   
private:
    GPRReg m_arg1Tag;
    GPRReg m_arg1Payload;
    GPRReg m_arg2Tag;
    GPRReg m_arg2Payload;
};

void SpeculativeJIT::nonSpeculativeNonPeepholeCompare(Node* node, MacroAssembler::RelationalCondition cond, S_JITOperation_EJJ helperFunction)
{
    JSValueOperand arg1(this, node->child1());
    JSValueOperand arg2(this, node->child2());
    GPRReg arg1TagGPR = arg1.tagGPR();
    GPRReg arg1PayloadGPR = arg1.payloadGPR();
    GPRReg arg2TagGPR = arg2.tagGPR();
    GPRReg arg2PayloadGPR = arg2.payloadGPR();
    
    JITCompiler::JumpList slowPath;
    
    if (isKnownNotInteger(node->child1().node()) || isKnownNotInteger(node->child2().node())) {
        GPRFlushedCallResult result(this);
        GPRReg resultPayloadGPR = result.gpr();
    
        arg1.use();
        arg2.use();

        flushRegisters();
        callOperation(helperFunction, resultPayloadGPR, arg1.jsValueRegs(), arg2.jsValueRegs());
        m_jit.exceptionCheck();
        
        booleanResult(resultPayloadGPR, node, UseChildrenCalledExplicitly);
    } else {
        GPRTemporary resultPayload(this, Reuse, arg1, PayloadWord);
        GPRReg resultPayloadGPR = resultPayload.gpr();

        arg1.use();
        arg2.use();
    
        if (!isKnownInteger(node->child1().node()))
            slowPath.append(m_jit.branch32(MacroAssembler::NotEqual, arg1TagGPR, JITCompiler::TrustedImm32(JSValue::Int32Tag)));
        if (!isKnownInteger(node->child2().node()))
            slowPath.append(m_jit.branch32(MacroAssembler::NotEqual, arg2TagGPR, JITCompiler::TrustedImm32(JSValue::Int32Tag)));

        m_jit.compare32(cond, arg1PayloadGPR, arg2PayloadGPR, resultPayloadGPR);
    
        if (!isKnownInteger(node->child1().node()) || !isKnownInteger(node->child2().node())) {
            addSlowPathGenerator(std::make_unique<CompareAndBoxBooleanSlowPathGenerator<JITCompiler::JumpList>>(
                    slowPath, this, helperFunction, resultPayloadGPR, arg1TagGPR,
                    arg1PayloadGPR, arg2TagGPR, arg2PayloadGPR));
        }
        
        booleanResult(resultPayloadGPR, node, UseChildrenCalledExplicitly);
    }
}

void SpeculativeJIT::nonSpeculativePeepholeStrictEq(Node* node, Node* branchNode, bool invert)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    // The branch instruction will branch to the taken block.
    // If taken is next, switch taken with notTaken & invert the branch condition so we can fall through.
    if (taken == nextBlock()) {
        invert = !invert;
        BasicBlock* tmp = taken;
        taken = notTaken;
        notTaken = tmp;
    }
    
    JSValueOperand arg1(this, node->child1());
    JSValueOperand arg2(this, node->child2());
    GPRReg arg1PayloadGPR = arg1.payloadGPR();
    GPRReg arg2PayloadGPR = arg2.payloadGPR();
    JSValueRegs arg1Regs = arg1.jsValueRegs();
    JSValueRegs arg2Regs = arg2.jsValueRegs();
    
    GPRTemporary resultPayload(this, Reuse, arg1, PayloadWord);
    GPRReg resultPayloadGPR = resultPayload.gpr();
    
    arg1.use();
    arg2.use();
    
    if (isKnownCell(node->child1().node()) && isKnownCell(node->child2().node())) {
        // see if we get lucky: if the arguments are cells and they reference the same
        // cell, then they must be strictly equal.
        branchPtr(JITCompiler::Equal, arg1PayloadGPR, arg2PayloadGPR, invert ? notTaken : taken);
        
        silentSpillAllRegisters(resultPayloadGPR);
        callOperation(operationCompareStrictEqCell, resultPayloadGPR, arg1Regs, arg2Regs);
        m_jit.exceptionCheck();
        silentFillAllRegisters(resultPayloadGPR);
        
        branchTest32(invert ? JITCompiler::Zero : JITCompiler::NonZero, resultPayloadGPR, taken);
    } else {
        // FIXME: Add fast paths for twoCells, number etc.

        silentSpillAllRegisters(resultPayloadGPR);
        callOperation(operationCompareStrictEq, resultPayloadGPR, arg1Regs, arg2Regs);
        m_jit.exceptionCheck();
        silentFillAllRegisters(resultPayloadGPR);
        
        branchTest32(invert ? JITCompiler::Zero : JITCompiler::NonZero, resultPayloadGPR, taken);
    }
    
    jump(notTaken);
}

void SpeculativeJIT::nonSpeculativeNonPeepholeStrictEq(Node* node, bool invert)
{
    JSValueOperand arg1(this, node->child1());
    JSValueOperand arg2(this, node->child2());
    GPRReg arg1PayloadGPR = arg1.payloadGPR();
    GPRReg arg2PayloadGPR = arg2.payloadGPR();
    JSValueRegs arg1Regs = arg1.jsValueRegs();
    JSValueRegs arg2Regs = arg2.jsValueRegs();
    
    GPRTemporary resultPayload(this, Reuse, arg1, PayloadWord);
    GPRReg resultPayloadGPR = resultPayload.gpr();
    
    arg1.use();
    arg2.use();
    
    if (isKnownCell(node->child1().node()) && isKnownCell(node->child2().node())) {
        // see if we get lucky: if the arguments are cells and they reference the same
        // cell, then they must be strictly equal.
        // FIXME: this should flush registers instead of silent spill/fill.
        JITCompiler::Jump notEqualCase = m_jit.branchPtr(JITCompiler::NotEqual, arg1PayloadGPR, arg2PayloadGPR);
        
        m_jit.move(JITCompiler::TrustedImm32(!invert), resultPayloadGPR);
        JITCompiler::Jump done = m_jit.jump();

        notEqualCase.link(&m_jit);
        
        silentSpillAllRegisters(resultPayloadGPR);
        callOperation(operationCompareStrictEqCell, resultPayloadGPR, arg1Regs, arg2Regs);
        m_jit.exceptionCheck();
        silentFillAllRegisters(resultPayloadGPR);
        
        m_jit.andPtr(JITCompiler::TrustedImm32(1), resultPayloadGPR);
        
        done.link(&m_jit);
    } else {
        // FIXME: Add fast paths.

        silentSpillAllRegisters(resultPayloadGPR);
        callOperation(operationCompareStrictEq, resultPayloadGPR, arg1Regs, arg2Regs);
        silentFillAllRegisters(resultPayloadGPR);
        m_jit.exceptionCheck();
        
        m_jit.andPtr(JITCompiler::TrustedImm32(1), resultPayloadGPR);
    }

    booleanResult(resultPayloadGPR, node, UseChildrenCalledExplicitly);
}

void SpeculativeJIT::compileMiscStrictEq(Node* node)
{
    JSValueOperand op1(this, node->child1(), ManualOperandSpeculation);
    JSValueOperand op2(this, node->child2(), ManualOperandSpeculation);
    GPRTemporary result(this);
    
    if (node->child1().useKind() == MiscUse)
        speculateMisc(node->child1(), op1.jsValueRegs());
    if (node->child2().useKind() == MiscUse)
        speculateMisc(node->child2(), op2.jsValueRegs());
    
    m_jit.move(TrustedImm32(0), result.gpr());
    JITCompiler::Jump notEqual = m_jit.branch32(JITCompiler::NotEqual, op1.tagGPR(), op2.tagGPR());
    m_jit.compare32(JITCompiler::Equal, op1.payloadGPR(), op2.payloadGPR(), result.gpr());
    notEqual.link(&m_jit);
    booleanResult(result.gpr(), node);
}

void SpeculativeJIT::emitCall(Node* node)
{
    CallLinkInfo::CallType callType;
    bool isVarargs = false;
    bool isForwardVarargs = false;
    bool isTail = false;
    bool isDirect = false;
    bool isEmulatedTail = false;
    switch (node->op()) {
    case Call:
    case CallEval:
        callType = CallLinkInfo::Call;
        break;
    case TailCall:
        callType = CallLinkInfo::TailCall;
        isTail = true;
        break;
    case TailCallInlinedCaller:
        callType = CallLinkInfo::Call;
        isEmulatedTail = true;
        break;
    case Construct:
        callType = CallLinkInfo::Construct;
        break;
    case CallVarargs:
        callType = CallLinkInfo::CallVarargs;
        isVarargs = true;
        break;
    case TailCallVarargs:
        callType = CallLinkInfo::TailCallVarargs;
        isVarargs = true;
        isTail = true;
        break;
    case TailCallVarargsInlinedCaller:
        callType = CallLinkInfo::CallVarargs;
        isVarargs = true;
        isEmulatedTail = true;
        break;
    case ConstructVarargs:
        callType = CallLinkInfo::ConstructVarargs;
        isVarargs = true;
        break;
    case CallForwardVarargs:
        callType = CallLinkInfo::CallVarargs;
        isForwardVarargs = true;
        break;
    case TailCallForwardVarargs:
        callType = CallLinkInfo::TailCallVarargs;
        isTail = true;
        isForwardVarargs = true;
        break;
    case TailCallForwardVarargsInlinedCaller:
        callType = CallLinkInfo::CallVarargs;
        isEmulatedTail = true;
        isForwardVarargs = true;
        break;
    case ConstructForwardVarargs:
        callType = CallLinkInfo::ConstructVarargs;
        isForwardVarargs = true;
        break;
    case DirectCall:
        callType = CallLinkInfo::DirectCall;
        isDirect = true;
        break;
    case DirectConstruct:
        callType = CallLinkInfo::DirectConstruct;
        isDirect = true;
        break;
    case DirectTailCall:
        callType = CallLinkInfo::DirectTailCall;
        isTail = true;
        isDirect = true;
        break;
    case DirectTailCallInlinedCaller:
        callType = CallLinkInfo::DirectCall;
        isEmulatedTail = true;
        isDirect = true;
        break;
    default:
        DFG_CRASH(m_jit.graph(), node, "bad node type");
        break;
    }

    Edge calleeEdge = m_jit.graph().child(node, 0);
    GPRReg calleeTagGPR = InvalidGPRReg;
    GPRReg calleePayloadGPR = InvalidGPRReg;
    CallFrameShuffleData shuffleData;
    
    ExecutableBase* executable = nullptr;
    FunctionExecutable* functionExecutable = nullptr;
    if (isDirect) {
        executable = node->castOperand<ExecutableBase*>();
        functionExecutable = jsDynamicCast<FunctionExecutable*>(*m_jit.vm(), executable);
    }
    
    unsigned numPassedArgs = 0;
    unsigned numAllocatedArgs = 0;
    
    // Gotta load the arguments somehow. Varargs is trickier.
    if (isVarargs || isForwardVarargs) {
        RELEASE_ASSERT(!isDirect);
        CallVarargsData* data = node->callVarargsData();

        unsigned numUsedStackSlots = m_jit.graph().m_nextMachineLocal;
        
        if (isForwardVarargs) {
            flushRegisters();
            if (node->child3())
                use(node->child3());
            
            GPRReg scratchGPR1;
            GPRReg scratchGPR2;
            GPRReg scratchGPR3;
            
            scratchGPR1 = JITCompiler::selectScratchGPR();
            scratchGPR2 = JITCompiler::selectScratchGPR(scratchGPR1);
            scratchGPR3 = JITCompiler::selectScratchGPR(scratchGPR1, scratchGPR2);
            
            m_jit.move(TrustedImm32(numUsedStackSlots), scratchGPR2);
            JITCompiler::JumpList slowCase;
            InlineCallFrame* inlineCallFrame;
            if (node->child3())
                inlineCallFrame = node->child3()->origin.semantic.inlineCallFrame;
            else
                inlineCallFrame = node->origin.semantic.inlineCallFrame;
            // emitSetupVarargsFrameFastCase modifies the stack pointer if it succeeds.
            emitSetupVarargsFrameFastCase(m_jit, scratchGPR2, scratchGPR1, scratchGPR2, scratchGPR3, inlineCallFrame, data->firstVarArgOffset, slowCase);
            JITCompiler::Jump done = m_jit.jump();
            slowCase.link(&m_jit);
            callOperation(operationThrowStackOverflowForVarargs);
            m_jit.exceptionCheck();
            m_jit.abortWithReason(DFGVarargsThrowingPathDidNotThrow);
            done.link(&m_jit);
        } else {
            GPRReg argumentsPayloadGPR;
            GPRReg argumentsTagGPR;
            GPRReg scratchGPR1;
            GPRReg scratchGPR2;
            GPRReg scratchGPR3;
        
            auto loadArgumentsGPR = [&] (GPRReg reservedGPR) {
                if (reservedGPR != InvalidGPRReg)
                    lock(reservedGPR);
                JSValueOperand arguments(this, node->child3());
                argumentsTagGPR = arguments.tagGPR();
                argumentsPayloadGPR = arguments.payloadGPR();
                if (reservedGPR != InvalidGPRReg)
                    unlock(reservedGPR);
                flushRegisters();
                
                scratchGPR1 = JITCompiler::selectScratchGPR(argumentsPayloadGPR, argumentsTagGPR, reservedGPR);
                scratchGPR2 = JITCompiler::selectScratchGPR(argumentsPayloadGPR, argumentsTagGPR, scratchGPR1, reservedGPR);
                scratchGPR3 = JITCompiler::selectScratchGPR(argumentsPayloadGPR, argumentsTagGPR, scratchGPR1, scratchGPR2, reservedGPR);
            };
            
            loadArgumentsGPR(InvalidGPRReg);
        
            DFG_ASSERT(m_jit.graph(), node, isFlushed());

            // Right now, arguments is in argumentsTagGPR/argumentsPayloadGPR and the register file is
            // flushed.
            callOperation(operationSizeFrameForVarargs, GPRInfo::returnValueGPR, JSValueRegs(argumentsTagGPR, argumentsPayloadGPR), numUsedStackSlots, data->firstVarArgOffset);
            m_jit.exceptionCheck();
            
            // Now we have the argument count of the callee frame, but we've lost the arguments operand.
            // Reconstruct the arguments operand while preserving the callee frame.
            loadArgumentsGPR(GPRInfo::returnValueGPR);
            m_jit.move(TrustedImm32(numUsedStackSlots), scratchGPR1);
            emitSetVarargsFrame(m_jit, GPRInfo::returnValueGPR, false, scratchGPR1, scratchGPR1);
            m_jit.addPtr(TrustedImm32(-(sizeof(CallerFrameAndPC) + WTF::roundUpToMultipleOf(stackAlignmentBytes(), 6 * sizeof(void*)))), scratchGPR1, JITCompiler::stackPointerRegister);
            
            callOperation(operationSetupVarargsFrame, GPRInfo::returnValueGPR, scratchGPR1, JSValueRegs(argumentsTagGPR, argumentsPayloadGPR), data->firstVarArgOffset, GPRInfo::returnValueGPR);
            m_jit.exceptionCheck();
            m_jit.addPtr(TrustedImm32(sizeof(CallerFrameAndPC)), GPRInfo::returnValueGPR, JITCompiler::stackPointerRegister);
        }
        
        DFG_ASSERT(m_jit.graph(), node, isFlushed());
        
        // We don't need the arguments array anymore.
        if (isVarargs)
            use(node->child3());

        // Now set up the "this" argument.
        JSValueOperand thisArgument(this, node->child2());
        GPRReg thisArgumentTagGPR = thisArgument.tagGPR();
        GPRReg thisArgumentPayloadGPR = thisArgument.payloadGPR();
        thisArgument.use();
        
        m_jit.store32(thisArgumentTagGPR, JITCompiler::calleeArgumentTagSlot(0));
        m_jit.store32(thisArgumentPayloadGPR, JITCompiler::calleeArgumentPayloadSlot(0));
    } else {        
        // The call instruction's first child is either the function (normal call) or the
        // receiver (method call). subsequent children are the arguments.
        numPassedArgs = node->numChildren() - 1;
        numAllocatedArgs = numPassedArgs;
        
        if (functionExecutable) {
            // Allocate more args if this would let us avoid arity checks. This is throttled by
            // CallLinkInfo's limit. It's probably good to throttle it - if the callee wants a
            // ginormous amount of argument space then it's better for them to do it so that when we
            // make calls to other things, we don't waste space.
            unsigned desiredNumAllocatedArgs = static_cast<unsigned>(functionExecutable->parameterCount()) + 1;
            if (desiredNumAllocatedArgs <= Options::maximumDirectCallStackSize()) {
                numAllocatedArgs = std::max(numAllocatedArgs, desiredNumAllocatedArgs);
                
                // Whoever converts to DirectCall should do this adjustment. It's too late for us to
                // do this adjustment now since we will have already emitted code that relied on the
                // value of m_parameterSlots.
                DFG_ASSERT(
                    m_jit.graph(), node,
                    Graph::parameterSlotsForArgCount(numAllocatedArgs)
                    <= m_jit.graph().m_parameterSlots);
            }
        }

        if (isTail) {
            JSValueOperand callee(this, calleeEdge);
            calleeTagGPR = callee.tagGPR();
            calleePayloadGPR = callee.payloadGPR();
            if (!isDirect)
                use(calleeEdge);

            shuffleData.numLocals = m_jit.graph().frameRegisterCount();
            shuffleData.callee = ValueRecovery::inPair(calleeTagGPR, calleePayloadGPR);
            shuffleData.args.resize(numAllocatedArgs);
            shuffleData.numPassedArgs = numPassedArgs;

            for (unsigned i = 0; i < numPassedArgs; ++i) {
                Edge argEdge = m_jit.graph().varArgChild(node, i + 1);
                GenerationInfo& info = generationInfo(argEdge.node());
                if (!isDirect)
                    use(argEdge);
                shuffleData.args[i] = info.recovery(argEdge->virtualRegister());
            }
            
            for (unsigned i = numPassedArgs; i < numAllocatedArgs; ++i)
                shuffleData.args[i] = ValueRecovery::constant(jsUndefined());
        } else {
            m_jit.store32(MacroAssembler::TrustedImm32(numPassedArgs), m_jit.calleeFramePayloadSlot(CallFrameSlot::argumentCount));
        
            for (unsigned i = 0; i < numPassedArgs; i++) {
                Edge argEdge = m_jit.graph().m_varArgChildren[node->firstChild() + 1 + i];
                JSValueOperand arg(this, argEdge);
                GPRReg argTagGPR = arg.tagGPR();
                GPRReg argPayloadGPR = arg.payloadGPR();
                use(argEdge);
            
                m_jit.store32(argTagGPR, m_jit.calleeArgumentTagSlot(i));
                m_jit.store32(argPayloadGPR, m_jit.calleeArgumentPayloadSlot(i));
            }
            
            for (unsigned i = numPassedArgs; i < numAllocatedArgs; ++i)
                m_jit.storeTrustedValue(jsUndefined(), JITCompiler::calleeArgumentSlot(i));
        }
    }

    if (!isTail || isVarargs || isForwardVarargs) {
        JSValueOperand callee(this, calleeEdge);
        calleeTagGPR = callee.tagGPR();
        calleePayloadGPR = callee.payloadGPR();
        use(calleeEdge);
        m_jit.store32(calleePayloadGPR, m_jit.calleeFramePayloadSlot(CallFrameSlot::callee));
        m_jit.store32(calleeTagGPR, m_jit.calleeFrameTagSlot(CallFrameSlot::callee));

        if (!isTail)
            flushRegisters();
    }

    JITCompiler::DataLabelPtr targetToCheck;
    JITCompiler::JumpList slowPath;

    CodeOrigin staticOrigin = node->origin.semantic;
    ASSERT(!isTail || !staticOrigin.inlineCallFrame || !staticOrigin.inlineCallFrame->getCallerSkippingTailCalls());
    ASSERT(!isEmulatedTail || (staticOrigin.inlineCallFrame && staticOrigin.inlineCallFrame->getCallerSkippingTailCalls()));
    CodeOrigin dynamicOrigin =
        isEmulatedTail ? *staticOrigin.inlineCallFrame->getCallerSkippingTailCalls() : staticOrigin;
    CallSiteIndex callSite = m_jit.recordCallSiteAndGenerateExceptionHandlingOSRExitIfNeeded(dynamicOrigin, m_stream->size());
    
    CallLinkInfo* info = m_jit.codeBlock()->addCallLinkInfo();
    info->setUpCall(callType, node->origin.semantic, calleePayloadGPR);
    
    auto setResultAndResetStack = [&] () {
        GPRFlushedCallResult resultPayload(this);
        GPRFlushedCallResult2 resultTag(this);
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg resultTagGPR = resultTag.gpr();
        
        m_jit.setupResults(resultPayloadGPR, resultTagGPR);

        jsValueResult(resultTagGPR, resultPayloadGPR, node, DataFormatJS, UseChildrenCalledExplicitly);
        // After the calls are done, we need to reestablish our stack
        // pointer. We rely on this for varargs calls, calls with arity
        // mismatch (the callframe is slided) and tail calls.
        m_jit.addPtr(TrustedImm32(m_jit.graph().stackPointerOffset() * sizeof(Register)), GPRInfo::callFrameRegister, JITCompiler::stackPointerRegister);
    };
    
    if (node->op() == CallEval) {
        // We want to call operationCallEval but we don't want to overwrite the parameter area in
        // which we have created a prototypical eval call frame. This means that we have to
        // subtract stack to make room for the call. Lucky for us, at this point we have the whole
        // register file to ourselves.
        
        m_jit.emitStoreCallSiteIndex(callSite);
        m_jit.addPtr(TrustedImm32(-static_cast<ptrdiff_t>(sizeof(CallerFrameAndPC))), JITCompiler::stackPointerRegister, GPRInfo::regT0);
        m_jit.storePtr(GPRInfo::callFrameRegister, JITCompiler::Address(GPRInfo::regT0, CallFrame::callerFrameOffset()));
        
        // Now we need to make room for:
        // - The caller frame and PC of a call to operationCallEval.
        // - Potentially two arguments on the stack.
        unsigned requiredBytes = sizeof(CallerFrameAndPC) + sizeof(ExecState*) * 2;
        requiredBytes = WTF::roundUpToMultipleOf(stackAlignmentBytes(), requiredBytes);
        m_jit.subPtr(TrustedImm32(requiredBytes), JITCompiler::stackPointerRegister);
        m_jit.setupArgumentsWithExecState(GPRInfo::regT0);
        prepareForExternalCall();
        m_jit.appendCall(operationCallEval);
        m_jit.exceptionCheck();
        JITCompiler::Jump done = m_jit.branch32(JITCompiler::NotEqual, GPRInfo::returnValueGPR2, TrustedImm32(JSValue::EmptyValueTag));
        
        // This is the part where we meant to make a normal call. Oops.
        m_jit.addPtr(TrustedImm32(requiredBytes), JITCompiler::stackPointerRegister);
        m_jit.load32(JITCompiler::calleeFrameSlot(CallFrameSlot::callee).withOffset(PayloadOffset), GPRInfo::regT0);
        m_jit.load32(JITCompiler::calleeFrameSlot(CallFrameSlot::callee).withOffset(TagOffset), GPRInfo::regT1);
        m_jit.emitDumbVirtualCall(info);
        
        done.link(&m_jit);
        setResultAndResetStack();
        return;
    }

    if (isDirect) {
        info->setExecutableDuringCompilation(executable);
        info->setMaxNumArguments(numAllocatedArgs);

        if (isTail) {
            RELEASE_ASSERT(node->op() == DirectTailCall);
            
            JITCompiler::PatchableJump patchableJump = m_jit.patchableJump();
            JITCompiler::Label mainPath = m_jit.label();
            
            m_jit.emitStoreCallSiteIndex(callSite);
            
            info->setFrameShuffleData(shuffleData);
            CallFrameShuffler(m_jit, shuffleData).prepareForTailCall();
            
            JITCompiler::Call call = m_jit.nearTailCall();
            
            JITCompiler::Label slowPath = m_jit.label();
            patchableJump.m_jump.linkTo(slowPath, &m_jit);
            
            silentSpillAllRegisters(InvalidGPRReg);
            callOperation(operationLinkDirectCall, info, calleePayloadGPR);
            silentFillAllRegisters(InvalidGPRReg);
            m_jit.exceptionCheck();
            m_jit.jump().linkTo(mainPath, &m_jit);
            
            useChildren(node);
            
            m_jit.addJSDirectTailCall(patchableJump, call, slowPath, info);
            return;
        }
        
        JITCompiler::Label mainPath = m_jit.label();
        
        m_jit.emitStoreCallSiteIndex(callSite);
        
        JITCompiler::Call call = m_jit.nearCall();
        JITCompiler::Jump done = m_jit.jump();
        
        JITCompiler::Label slowPath = m_jit.label();
        if (isX86())
            m_jit.pop(JITCompiler::selectScratchGPR(calleePayloadGPR));

        callOperation(operationLinkDirectCall, info, calleePayloadGPR);
        m_jit.exceptionCheck();
        m_jit.jump().linkTo(mainPath, &m_jit);
        
        done.link(&m_jit);
        
        setResultAndResetStack();
        
        m_jit.addJSDirectCall(call, slowPath, info);
        return;
    }
    
    m_jit.emitStoreCallSiteIndex(callSite);
    
    slowPath.append(m_jit.branchIfNotCell(JSValueRegs(calleeTagGPR, calleePayloadGPR)));
    slowPath.append(m_jit.branchPtrWithPatch(MacroAssembler::NotEqual, calleePayloadGPR, targetToCheck));

    if (isTail) {
        if (node->op() == TailCall) {
            info->setFrameShuffleData(shuffleData);
            CallFrameShuffler(m_jit, shuffleData).prepareForTailCall();
        } else {
            m_jit.emitRestoreCalleeSaves();
            m_jit.prepareForTailCallSlow();
        }
    }

    JITCompiler::Call fastCall = isTail ? m_jit.nearTailCall() : m_jit.nearCall();

    JITCompiler::Jump done = m_jit.jump();

    slowPath.link(&m_jit);

    if (node->op() == TailCall) {
        CallFrameShuffler callFrameShuffler(m_jit, shuffleData);
        callFrameShuffler.setCalleeJSValueRegs(JSValueRegs(
            GPRInfo::regT1, GPRInfo::regT0));
        callFrameShuffler.prepareForSlowPath();
    } else {
        // Callee payload needs to be in regT0, tag in regT1
        if (calleeTagGPR == GPRInfo::regT0) {
            if (calleePayloadGPR == GPRInfo::regT1)
                m_jit.swap(GPRInfo::regT1, GPRInfo::regT0);
            else {
                m_jit.move(calleeTagGPR, GPRInfo::regT1);
                m_jit.move(calleePayloadGPR, GPRInfo::regT0);
            }
        } else {
            m_jit.move(calleePayloadGPR, GPRInfo::regT0);
            m_jit.move(calleeTagGPR, GPRInfo::regT1);
        }

        if (isTail)
            m_jit.emitRestoreCalleeSaves();
    }

    m_jit.move(TrustedImmPtr(info), GPRInfo::regT2);
    JITCompiler::Call slowCall = m_jit.nearCall();

    done.link(&m_jit);

    if (isTail)
        m_jit.abortWithReason(JITDidReturnFromTailCall);
    else
        setResultAndResetStack();

    m_jit.addJSCall(fastCall, slowCall, targetToCheck, info);
}

template<bool strict>
GPRReg SpeculativeJIT::fillSpeculateInt32Internal(Edge edge, DataFormat& returnFormat)
{
    AbstractValue& value = m_state.forNode(edge);
    SpeculatedType type = value.m_type;
    ASSERT(edge.useKind() != KnownInt32Use || !(value.m_type & ~SpecInt32Only));

    m_interpreter.filter(value, SpecInt32Only);
    if (value.isClear()) {
        terminateSpeculativeExecution(Uncountable, JSValueRegs(), 0);
        returnFormat = DataFormatInt32;
        return allocate();
    }

    VirtualRegister virtualRegister = edge->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);

    switch (info.registerFormat()) {
    case DataFormatNone: {
        if (edge->hasConstant()) {
            ASSERT(edge->isInt32Constant());
            GPRReg gpr = allocate();
            m_jit.move(MacroAssembler::Imm32(edge->asInt32()), gpr);
            m_gprs.retain(gpr, virtualRegister, SpillOrderConstant);
            info.fillInt32(*m_stream, gpr);
            returnFormat = DataFormatInt32;
            return gpr;
        }

        DataFormat spillFormat = info.spillFormat();

        ASSERT_UNUSED(spillFormat, (spillFormat & DataFormatJS) || spillFormat == DataFormatInt32);

        // If we know this was spilled as an integer we can fill without checking.
        if (type & ~SpecInt32Only)
            speculationCheck(BadType, JSValueSource(JITCompiler::addressFor(virtualRegister)), edge, m_jit.branch32(MacroAssembler::NotEqual, JITCompiler::tagFor(virtualRegister), TrustedImm32(JSValue::Int32Tag)));

        GPRReg gpr = allocate();
        m_jit.load32(JITCompiler::payloadFor(virtualRegister), gpr);
        m_gprs.retain(gpr, virtualRegister, SpillOrderSpilled);
        info.fillInt32(*m_stream, gpr);
        returnFormat = DataFormatInt32;
        return gpr;
    }

    case DataFormatJSInt32:
    case DataFormatJS: {
        // Check the value is an integer.
        GPRReg tagGPR = info.tagGPR();
        GPRReg payloadGPR = info.payloadGPR();
        m_gprs.lock(tagGPR);
        m_gprs.lock(payloadGPR);
        if (type & ~SpecInt32Only)
            speculationCheck(BadType, JSValueRegs(tagGPR, payloadGPR), edge, m_jit.branch32(MacroAssembler::NotEqual, tagGPR, TrustedImm32(JSValue::Int32Tag)));
        m_gprs.unlock(tagGPR);
        m_gprs.release(tagGPR);
        m_gprs.release(payloadGPR);
        m_gprs.retain(payloadGPR, virtualRegister, SpillOrderInteger);
        info.fillInt32(*m_stream, payloadGPR);
        // If !strict we're done, return.
        returnFormat = DataFormatInt32;
        return payloadGPR;
    }

    case DataFormatInt32: {
        GPRReg gpr = info.gpr();
        m_gprs.lock(gpr);
        returnFormat = DataFormatInt32;
        return gpr;
    }

    case DataFormatCell:
    case DataFormatBoolean:
    case DataFormatJSDouble:
    case DataFormatJSCell:
    case DataFormatJSBoolean:
    case DataFormatDouble:
    case DataFormatStorage:
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return InvalidGPRReg;
    }
}

GPRReg SpeculativeJIT::fillSpeculateInt32(Edge edge, DataFormat& returnFormat)
{
    return fillSpeculateInt32Internal<false>(edge, returnFormat);
}

GPRReg SpeculativeJIT::fillSpeculateInt32Strict(Edge edge)
{
    DataFormat mustBeDataFormatInt32;
    GPRReg result = fillSpeculateInt32Internal<true>(edge, mustBeDataFormatInt32);
    ASSERT(mustBeDataFormatInt32 == DataFormatInt32);
    return result;
}

FPRReg SpeculativeJIT::fillSpeculateDouble(Edge edge)
{
    ASSERT(isDouble(edge.useKind()));
    ASSERT(edge->hasDoubleResult());
    VirtualRegister virtualRegister = edge->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);

    if (info.registerFormat() == DataFormatNone) {

        if (edge->hasConstant()) {
            RELEASE_ASSERT(edge->isNumberConstant());
            FPRReg fpr = fprAllocate();
            m_jit.loadDouble(TrustedImmPtr(m_jit.addressOfDoubleConstant(edge.node())), fpr);
            m_fprs.retain(fpr, virtualRegister, SpillOrderConstant);
            info.fillDouble(*m_stream, fpr);
            return fpr;
        }
        
        RELEASE_ASSERT(info.spillFormat() == DataFormatDouble);
        FPRReg fpr = fprAllocate();
        m_jit.loadDouble(JITCompiler::addressFor(virtualRegister), fpr);
        m_fprs.retain(fpr, virtualRegister, SpillOrderSpilled);
        info.fillDouble(*m_stream, fpr);
        return fpr;
    }

    RELEASE_ASSERT(info.registerFormat() == DataFormatDouble);
    FPRReg fpr = info.fpr();
    m_fprs.lock(fpr);
    return fpr;
}

GPRReg SpeculativeJIT::fillSpeculateCell(Edge edge)
{
    AbstractValue& value = m_state.forNode(edge);
    SpeculatedType type = value.m_type;
    ASSERT((edge.useKind() != KnownCellUse && edge.useKind() != KnownStringUse) || !(value.m_type & ~SpecCell));

    m_interpreter.filter(value, SpecCell);
    if (value.isClear()) {
        terminateSpeculativeExecution(Uncountable, JSValueRegs(), 0);
        return allocate();
    }

    VirtualRegister virtualRegister = edge->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);

    switch (info.registerFormat()) {
    case DataFormatNone: {
        if (edge->hasConstant()) {
            GPRReg gpr = allocate();
            m_gprs.retain(gpr, virtualRegister, SpillOrderConstant);
            m_jit.move(TrustedImmPtr(edge->constant()), gpr);
            info.fillCell(*m_stream, gpr);
            return gpr;
        }

        ASSERT((info.spillFormat() & DataFormatJS) || info.spillFormat() == DataFormatCell);
        if (type & ~SpecCell) {
            speculationCheck(
                BadType,
                JSValueSource(JITCompiler::addressFor(virtualRegister)),
                edge,
                m_jit.branch32(
                    MacroAssembler::NotEqual,
                    JITCompiler::tagFor(virtualRegister),
                    TrustedImm32(JSValue::CellTag)));
        }
        GPRReg gpr = allocate();
        m_jit.load32(JITCompiler::payloadFor(virtualRegister), gpr);
        m_gprs.retain(gpr, virtualRegister, SpillOrderSpilled);
        info.fillCell(*m_stream, gpr);
        return gpr;
    }

    case DataFormatCell: {
        GPRReg gpr = info.gpr();
        m_gprs.lock(gpr);
        return gpr;
    }

    case DataFormatJSCell:
    case DataFormatJS: {
        GPRReg tagGPR = info.tagGPR();
        GPRReg payloadGPR = info.payloadGPR();
        m_gprs.lock(tagGPR);
        m_gprs.lock(payloadGPR);
        if (type & ~SpecCell) {
            speculationCheck(
                BadType, JSValueRegs(tagGPR, payloadGPR), edge,
                m_jit.branchIfNotCell(info.jsValueRegs()));
        }
        m_gprs.unlock(tagGPR);
        m_gprs.release(tagGPR);
        m_gprs.release(payloadGPR);
        m_gprs.retain(payloadGPR, virtualRegister, SpillOrderCell);
        info.fillCell(*m_stream, payloadGPR);
        return payloadGPR;
    }

    case DataFormatJSInt32:
    case DataFormatInt32:
    case DataFormatJSDouble:
    case DataFormatJSBoolean:
    case DataFormatBoolean:
    case DataFormatDouble:
    case DataFormatStorage:
        RELEASE_ASSERT_NOT_REACHED();

    default:
        RELEASE_ASSERT_NOT_REACHED();
        return InvalidGPRReg;
    }
}

GPRReg SpeculativeJIT::fillSpeculateBoolean(Edge edge)
{
    AbstractValue& value = m_state.forNode(edge);
    SpeculatedType type = value.m_type;
    ASSERT(edge.useKind() != KnownBooleanUse || !(value.m_type & ~SpecBoolean));

    m_interpreter.filter(value, SpecBoolean);
    if (value.isClear()) {
        terminateSpeculativeExecution(Uncountable, JSValueRegs(), 0);
        return allocate();
    }

    VirtualRegister virtualRegister = edge->virtualRegister();
    GenerationInfo& info = generationInfoFromVirtualRegister(virtualRegister);

    switch (info.registerFormat()) {
    case DataFormatNone: {
        if (edge->hasConstant()) {
            JSValue jsValue = edge->asJSValue();
            GPRReg gpr = allocate();
            m_gprs.retain(gpr, virtualRegister, SpillOrderConstant);
            m_jit.move(MacroAssembler::TrustedImm32(jsValue.asBoolean()), gpr);
            info.fillBoolean(*m_stream, gpr);
            return gpr;
        }

        ASSERT((info.spillFormat() & DataFormatJS) || info.spillFormat() == DataFormatBoolean);

        if (type & ~SpecBoolean)
            speculationCheck(BadType, JSValueSource(JITCompiler::addressFor(virtualRegister)), edge, m_jit.branch32(MacroAssembler::NotEqual, JITCompiler::tagFor(virtualRegister), TrustedImm32(JSValue::BooleanTag)));

        GPRReg gpr = allocate();
        m_jit.load32(JITCompiler::payloadFor(virtualRegister), gpr);
        m_gprs.retain(gpr, virtualRegister, SpillOrderSpilled);
        info.fillBoolean(*m_stream, gpr);
        return gpr;
    }

    case DataFormatBoolean: {
        GPRReg gpr = info.gpr();
        m_gprs.lock(gpr);
        return gpr;
    }

    case DataFormatJSBoolean:
    case DataFormatJS: {
        GPRReg tagGPR = info.tagGPR();
        GPRReg payloadGPR = info.payloadGPR();
        m_gprs.lock(tagGPR);
        m_gprs.lock(payloadGPR);
        if (type & ~SpecBoolean)
            speculationCheck(BadType, JSValueRegs(tagGPR, payloadGPR), edge, m_jit.branch32(MacroAssembler::NotEqual, tagGPR, TrustedImm32(JSValue::BooleanTag)));

        m_gprs.unlock(tagGPR);
        m_gprs.release(tagGPR);
        m_gprs.release(payloadGPR);
        m_gprs.retain(payloadGPR, virtualRegister, SpillOrderBoolean);
        info.fillBoolean(*m_stream, payloadGPR);
        return payloadGPR;
    }

    case DataFormatJSInt32:
    case DataFormatInt32:
    case DataFormatJSDouble:
    case DataFormatJSCell:
    case DataFormatCell:
    case DataFormatDouble:
    case DataFormatStorage:
        RELEASE_ASSERT_NOT_REACHED();

    default:
        RELEASE_ASSERT_NOT_REACHED();
        return InvalidGPRReg;
    }
}

void SpeculativeJIT::compileObjectEquality(Node* node)
{
    SpeculateCellOperand op1(this, node->child1());
    SpeculateCellOperand op2(this, node->child2());
    GPRReg op1GPR = op1.gpr();
    GPRReg op2GPR = op2.gpr();
    
    if (masqueradesAsUndefinedWatchpointIsStillValid()) {
        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op1GPR), node->child1(), SpecObject, m_jit.branchIfNotObject(op1GPR));
        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op2GPR), node->child2(), SpecObject, m_jit.branchIfNotObject(op2GPR));
    } else {
        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op1GPR), node->child1(), SpecObject, m_jit.branchIfNotObject(op1GPR));
        speculationCheck(BadType, JSValueSource::unboxedCell(op1GPR), node->child1(),
            m_jit.branchTest8(
                MacroAssembler::NonZero, 
                MacroAssembler::Address(op1GPR, JSCell::typeInfoFlagsOffset()),
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));

        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op2GPR), node->child2(), SpecObject, m_jit.branchIfNotObject(op2GPR));
        speculationCheck(BadType, JSValueSource::unboxedCell(op2GPR), node->child2(),
            m_jit.branchTest8(
                MacroAssembler::NonZero,
                MacroAssembler::Address(op2GPR, JSCell::typeInfoFlagsOffset()),
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));
    }
    
    GPRTemporary resultPayload(this, Reuse, op2);
    GPRReg resultPayloadGPR = resultPayload.gpr();
    
    MacroAssembler::Jump falseCase = m_jit.branchPtr(MacroAssembler::NotEqual, op1GPR, op2GPR);
    m_jit.move(TrustedImm32(1), resultPayloadGPR);
    MacroAssembler::Jump done = m_jit.jump();
    falseCase.link(&m_jit);
    m_jit.move(TrustedImm32(0), resultPayloadGPR);
    done.link(&m_jit);

    booleanResult(resultPayloadGPR, node);
}

void SpeculativeJIT::compileObjectStrictEquality(Edge objectChild, Edge otherChild)
{
    SpeculateCellOperand op1(this, objectChild);
    JSValueOperand op2(this, otherChild);

    GPRReg op1GPR = op1.gpr();
    GPRReg op2GPR = op2.payloadGPR();

    DFG_TYPE_CHECK(JSValueSource::unboxedCell(op1GPR), objectChild, SpecObject, m_jit.branchIfNotObject(op1GPR));

    GPRTemporary resultPayload(this, Reuse, op1);
    GPRReg resultPayloadGPR = resultPayload.gpr();
    
    MacroAssembler::Jump op2CellJump = m_jit.branchIfCell(op2.jsValueRegs());
    
    m_jit.move(TrustedImm32(0), resultPayloadGPR);
    MacroAssembler::Jump op2NotCellJump = m_jit.jump();
    
    // At this point we know that we can perform a straight-forward equality comparison on pointer
    // values because we are doing strict equality.
    op2CellJump.link(&m_jit);
    m_jit.compare32(MacroAssembler::Equal, op1GPR, op2GPR, resultPayloadGPR);
    
    op2NotCellJump.link(&m_jit);
    booleanResult(resultPayloadGPR, m_currentNode);
}
    
void SpeculativeJIT::compilePeepHoleObjectStrictEquality(Edge objectChild, Edge otherChild, Node* branchNode)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;

    SpeculateCellOperand op1(this, objectChild);
    JSValueOperand op2(this, otherChild);
    
    GPRReg op1GPR = op1.gpr();
    GPRReg op2GPR = op2.payloadGPR();

    DFG_TYPE_CHECK(JSValueSource::unboxedCell(op1GPR), objectChild, SpecObject, m_jit.branchIfNotObject(op1GPR));

    branch32(MacroAssembler::NotEqual, op2.tagGPR(), TrustedImm32(JSValue::CellTag), notTaken);
    
    if (taken == nextBlock()) {
        branch32(MacroAssembler::NotEqual, op1GPR, op2GPR, notTaken);
        jump(taken);
    } else {
        branch32(MacroAssembler::Equal, op1GPR, op2GPR, taken);
        jump(notTaken);
    }
}

void SpeculativeJIT::compileObjectToObjectOrOtherEquality(Edge leftChild, Edge rightChild)
{
    SpeculateCellOperand op1(this, leftChild);
    JSValueOperand op2(this, rightChild, ManualOperandSpeculation);
    GPRTemporary result(this);
    
    GPRReg op1GPR = op1.gpr();
    GPRReg op2TagGPR = op2.tagGPR();
    GPRReg op2PayloadGPR = op2.payloadGPR();
    GPRReg resultGPR = result.gpr();

    bool masqueradesAsUndefinedWatchpointValid =
        masqueradesAsUndefinedWatchpointIsStillValid();

    if (masqueradesAsUndefinedWatchpointValid) {
        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op1GPR), leftChild, SpecObject, m_jit.branchIfNotObject(op1GPR));
    } else {
        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op1GPR), leftChild, SpecObject, m_jit.branchIfNotObject(op1GPR));
        speculationCheck(BadType, JSValueSource::unboxedCell(op1GPR), leftChild, 
            m_jit.branchTest8(
                MacroAssembler::NonZero, 
                MacroAssembler::Address(op1GPR, JSCell::typeInfoFlagsOffset()), 
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));
    }
    
    
    // It seems that most of the time when programs do a == b where b may be either null/undefined
    // or an object, b is usually an object. Balance the branches to make that case fast.
    MacroAssembler::Jump rightNotCell = m_jit.branchIfNotCell(op2.jsValueRegs());
    
    // We know that within this branch, rightChild must be a cell.
    if (masqueradesAsUndefinedWatchpointValid) {
        DFG_TYPE_CHECK(
            JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild, (~SpecCell) | SpecObject, m_jit.branchIfNotObject(op2PayloadGPR));
    } else {
        DFG_TYPE_CHECK(
            JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild, (~SpecCell) | SpecObject, m_jit.branchIfNotObject(op2PayloadGPR));
        speculationCheck(BadType, JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild, 
            m_jit.branchTest8(
                MacroAssembler::NonZero, 
                MacroAssembler::Address(op2PayloadGPR, JSCell::typeInfoFlagsOffset()), 
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));
    }
    
    // At this point we know that we can perform a straight-forward equality comparison on pointer
    // values because both left and right are pointers to objects that have no special equality
    // protocols.
    MacroAssembler::Jump falseCase = m_jit.branchPtr(MacroAssembler::NotEqual, op1GPR, op2PayloadGPR);
    MacroAssembler::Jump trueCase = m_jit.jump();
    
    rightNotCell.link(&m_jit);
    
    // We know that within this branch, rightChild must not be a cell. Check if that is enough to
    // prove that it is either null or undefined.
    if (needsTypeCheck(rightChild, SpecCell | SpecOther)) {
        m_jit.or32(TrustedImm32(1), op2TagGPR, resultGPR);
        
        typeCheck(
            JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild, SpecCell | SpecOther,
            m_jit.branch32(
                MacroAssembler::NotEqual, resultGPR,
                MacroAssembler::TrustedImm32(JSValue::NullTag)));
    }
    
    falseCase.link(&m_jit);
    m_jit.move(TrustedImm32(0), resultGPR);
    MacroAssembler::Jump done = m_jit.jump();
    trueCase.link(&m_jit);
    m_jit.move(TrustedImm32(1), resultGPR);
    done.link(&m_jit);
    
    booleanResult(resultGPR, m_currentNode);
}

void SpeculativeJIT::compilePeepHoleObjectToObjectOrOtherEquality(Edge leftChild, Edge rightChild, Node* branchNode)
{
    BasicBlock* taken = branchNode->branchData()->taken.block;
    BasicBlock* notTaken = branchNode->branchData()->notTaken.block;
    
    SpeculateCellOperand op1(this, leftChild);
    JSValueOperand op2(this, rightChild, ManualOperandSpeculation);
    GPRTemporary result(this);
    
    GPRReg op1GPR = op1.gpr();
    GPRReg op2TagGPR = op2.tagGPR();
    GPRReg op2PayloadGPR = op2.payloadGPR();
    GPRReg resultGPR = result.gpr();

    bool masqueradesAsUndefinedWatchpointValid =
        masqueradesAsUndefinedWatchpointIsStillValid();

    if (masqueradesAsUndefinedWatchpointValid) {
        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op1GPR), leftChild, SpecObject, m_jit.branchIfNotObject(op1GPR));
    } else {
        DFG_TYPE_CHECK(
            JSValueSource::unboxedCell(op1GPR), leftChild, SpecObject, m_jit.branchIfNotObject(op1GPR));
        speculationCheck(BadType, JSValueSource::unboxedCell(op1GPR), leftChild,
            m_jit.branchTest8(
                MacroAssembler::NonZero, 
                MacroAssembler::Address(op1GPR, JSCell::typeInfoFlagsOffset()), 
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));
    }
    
    // It seems that most of the time when programs do a == b where b may be either null/undefined
    // or an object, b is usually an object. Balance the branches to make that case fast.
    MacroAssembler::Jump rightNotCell = m_jit.branchIfNotCell(op2.jsValueRegs());
    
    // We know that within this branch, rightChild must be a cell.
    if (masqueradesAsUndefinedWatchpointValid) {
        DFG_TYPE_CHECK(
            JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild, (~SpecCell) | SpecObject,
            m_jit.branchIfNotObject(op2PayloadGPR));
    } else {
        DFG_TYPE_CHECK(
            JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild, (~SpecCell) | SpecObject,
            m_jit.branchIfNotObject(op2PayloadGPR));
        speculationCheck(BadType, JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild,
            m_jit.branchTest8(
                MacroAssembler::NonZero, 
                MacroAssembler::Address(op2PayloadGPR, JSCell::typeInfoFlagsOffset()), 
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined)));
    }
    
    // At this point we know that we can perform a straight-forward equality comparison on pointer
    // values because both left and right are pointers to objects that have no special equality
    // protocols.
    branch32(MacroAssembler::Equal, op1GPR, op2PayloadGPR, taken);
    
    // We know that within this branch, rightChild must not be a cell. Check if that is enough to
    // prove that it is either null or undefined.
    if (!needsTypeCheck(rightChild, SpecCell | SpecOther))
        rightNotCell.link(&m_jit);
    else {
        jump(notTaken, ForceJump);
        
        rightNotCell.link(&m_jit);
        m_jit.or32(TrustedImm32(1), op2TagGPR, resultGPR);
        
        typeCheck(
            JSValueRegs(op2TagGPR, op2PayloadGPR), rightChild, SpecCell | SpecOther,
            m_jit.branch32(
                MacroAssembler::NotEqual, resultGPR,
                MacroAssembler::TrustedImm32(JSValue::NullTag)));
    }
    
    jump(notTaken);
}

void SpeculativeJIT::compileSymbolUntypedEquality(Node* node, Edge symbolEdge, Edge untypedEdge)
{
    SpeculateCellOperand symbol(this, symbolEdge);
    JSValueOperand untyped(this, untypedEdge);

    GPRReg symbolGPR = symbol.gpr();
    GPRReg untypedGPR = untyped.payloadGPR();

    speculateSymbol(symbolEdge, symbolGPR);

    GPRTemporary resultPayload(this, Reuse, symbol);
    GPRReg resultPayloadGPR = resultPayload.gpr();

    MacroAssembler::Jump untypedCellJump = m_jit.branchIfCell(untyped.jsValueRegs());

    m_jit.move(TrustedImm32(0), resultPayloadGPR);
    MacroAssembler::Jump untypedNotCellJump = m_jit.jump();

    // At this point we know that we can perform a straight-forward equality comparison on pointer
    // values because we are doing strict equality.
    untypedCellJump.link(&m_jit);
    m_jit.compare32(MacroAssembler::Equal, symbolGPR, untypedGPR, resultPayloadGPR);

    untypedNotCellJump.link(&m_jit);
    booleanResult(resultPayloadGPR, node);
}

void SpeculativeJIT::compileInt32Compare(Node* node, MacroAssembler::RelationalCondition condition)
{
    SpeculateInt32Operand op1(this, node->child1());
    SpeculateInt32Operand op2(this, node->child2());
    GPRTemporary resultPayload(this);
    
    m_jit.compare32(condition, op1.gpr(), op2.gpr(), resultPayload.gpr());
    
    // If we add a DataFormatBool, we should use it here.
    booleanResult(resultPayload.gpr(), node);
}

void SpeculativeJIT::compileDoubleCompare(Node* node, MacroAssembler::DoubleCondition condition)
{
    SpeculateDoubleOperand op1(this, node->child1());
    SpeculateDoubleOperand op2(this, node->child2());
    GPRTemporary resultPayload(this);
    
    m_jit.move(TrustedImm32(1), resultPayload.gpr());
    MacroAssembler::Jump trueCase = m_jit.branchDouble(condition, op1.fpr(), op2.fpr());
    m_jit.move(TrustedImm32(0), resultPayload.gpr());
    trueCase.link(&m_jit);
    
    booleanResult(resultPayload.gpr(), node);
}

void SpeculativeJIT::compileObjectOrOtherLogicalNot(Edge nodeUse)
{
    JSValueOperand value(this, nodeUse, ManualOperandSpeculation);
    GPRTemporary resultPayload(this);
    GPRReg valueTagGPR = value.tagGPR();
    GPRReg valuePayloadGPR = value.payloadGPR();
    GPRReg resultPayloadGPR = resultPayload.gpr();
    GPRTemporary structure;
    GPRReg structureGPR = InvalidGPRReg;

    bool masqueradesAsUndefinedWatchpointValid =
        masqueradesAsUndefinedWatchpointIsStillValid();

    if (!masqueradesAsUndefinedWatchpointValid) {
        // The masquerades as undefined case will use the structure register, so allocate it here.
        // Do this at the top of the function to avoid branching around a register allocation.
        GPRTemporary realStructure(this);
        structure.adopt(realStructure);
        structureGPR = structure.gpr();
    }

    MacroAssembler::Jump notCell = m_jit.branchIfNotCell(value.jsValueRegs());
    if (masqueradesAsUndefinedWatchpointValid) {
        DFG_TYPE_CHECK(
            JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse, (~SpecCell) | SpecObject,
            m_jit.branchIfNotObject(valuePayloadGPR));
    } else {
        DFG_TYPE_CHECK(
            JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse, (~SpecCell) | SpecObject,
            m_jit.branchIfNotObject(valuePayloadGPR));

        MacroAssembler::Jump isNotMasqueradesAsUndefined = 
            m_jit.branchTest8(
                MacroAssembler::Zero, 
                MacroAssembler::Address(valuePayloadGPR, JSCell::typeInfoFlagsOffset()), 
                MacroAssembler::TrustedImm32(MasqueradesAsUndefined));

        m_jit.loadPtr(MacroAssembler::Address(valuePayloadGPR, JSCell::structureIDOffset()), structureGPR);
        speculationCheck(BadType, JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse, 
            m_jit.branchPtr(
                MacroAssembler::Equal, 
                MacroAssembler::Address(structureGPR, Structure::globalObjectOffset()), 
                TrustedImmPtr::weakPointer(m_jit.graph(), m_jit.graph().globalObjectFor(m_currentNode->origin.semantic))));

        isNotMasqueradesAsUndefined.link(&m_jit);
    }
    m_jit.move(TrustedImm32(0), resultPayloadGPR);
    MacroAssembler::Jump done = m_jit.jump();
    
    notCell.link(&m_jit);
 
    COMPILE_ASSERT((JSValue::UndefinedTag | 1) == JSValue::NullTag, UndefinedTag_OR_1_EQUALS_NullTag);
    if (needsTypeCheck(nodeUse, SpecCell | SpecOther)) {
        m_jit.or32(TrustedImm32(1), valueTagGPR, resultPayloadGPR);
        typeCheck(
            JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse, SpecCell | SpecOther,
            m_jit.branch32(
                MacroAssembler::NotEqual, 
                resultPayloadGPR, 
                TrustedImm32(JSValue::NullTag)));
    }
    m_jit.move(TrustedImm32(1), resultPayloadGPR);
    
    done.link(&m_jit);
    
    booleanResult(resultPayloadGPR, m_currentNode);
}

void SpeculativeJIT::compileLogicalNot(Node* node)
{
    switch (node->child1().useKind()) {
    case BooleanUse:
    case KnownBooleanUse: {
        SpeculateBooleanOperand value(this, node->child1());
        GPRTemporary result(this, Reuse, value);
        m_jit.xor32(TrustedImm32(1), value.gpr(), result.gpr());
        booleanResult(result.gpr(), node);
        return;
    }
        
    case ObjectOrOtherUse: {
        compileObjectOrOtherLogicalNot(node->child1());
        return;
    }
        
    case Int32Use: {
        SpeculateInt32Operand value(this, node->child1());
        GPRTemporary resultPayload(this, Reuse, value);
        m_jit.compare32(MacroAssembler::Equal, value.gpr(), MacroAssembler::TrustedImm32(0), resultPayload.gpr());
        booleanResult(resultPayload.gpr(), node);
        return;
    }
        
    case DoubleRepUse: {
        SpeculateDoubleOperand value(this, node->child1());
        FPRTemporary scratch(this);
        GPRTemporary resultPayload(this);
        m_jit.move(TrustedImm32(0), resultPayload.gpr());
        MacroAssembler::Jump nonZero = m_jit.branchDoubleNonZero(value.fpr(), scratch.fpr());
        m_jit.move(TrustedImm32(1), resultPayload.gpr());
        nonZero.link(&m_jit);
        booleanResult(resultPayload.gpr(), node);
        return;
    }

    case UntypedUse: {
        JSValueOperand arg1(this, node->child1());
        GPRTemporary result(this);
        GPRTemporary temp(this);
        FPRTemporary valueFPR(this);
        FPRTemporary tempFPR(this);

        GPRReg resultGPR = result.gpr();

        bool shouldCheckMasqueradesAsUndefined = !masqueradesAsUndefinedWatchpointIsStillValid();
        JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
        bool negateResult = true;
        m_jit.emitConvertValueToBoolean(arg1.jsValueRegs(), resultGPR, temp.gpr(), valueFPR.fpr(), tempFPR.fpr(), shouldCheckMasqueradesAsUndefined, globalObject, negateResult);
        booleanResult(resultGPR, node);
        return;
    }
    case StringUse:
        return compileStringZeroLength(node);

    case StringOrOtherUse:
        return compileLogicalNotStringOrOther(node);

    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

void SpeculativeJIT::emitObjectOrOtherBranch(Edge nodeUse, BasicBlock* taken, BasicBlock* notTaken)
{
    JSValueOperand value(this, nodeUse, ManualOperandSpeculation);
    GPRTemporary scratch(this);
    GPRReg valueTagGPR = value.tagGPR();
    GPRReg valuePayloadGPR = value.payloadGPR();
    GPRReg scratchGPR = scratch.gpr();
    
    MacroAssembler::Jump notCell = m_jit.branchIfNotCell(value.jsValueRegs());
    if (masqueradesAsUndefinedWatchpointIsStillValid()) {
        DFG_TYPE_CHECK(
            JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse, (~SpecCell) | SpecObject,
            m_jit.branchIfNotObject(valuePayloadGPR));
    } else {
        DFG_TYPE_CHECK(
            JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse, (~SpecCell) | SpecObject,
            m_jit.branchIfNotObject(valuePayloadGPR));

        JITCompiler::Jump isNotMasqueradesAsUndefined = m_jit.branchTest8(
            JITCompiler::Zero, 
            MacroAssembler::Address(valuePayloadGPR, JSCell::typeInfoFlagsOffset()), 
            TrustedImm32(MasqueradesAsUndefined));

        m_jit.loadPtr(MacroAssembler::Address(valuePayloadGPR, JSCell::structureIDOffset()), scratchGPR);
        speculationCheck(BadType, JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse,
            m_jit.branchPtr(
                MacroAssembler::Equal, 
                MacroAssembler::Address(scratchGPR, Structure::globalObjectOffset()), 
                TrustedImmPtr::weakPointer(m_jit.graph(), m_jit.graph().globalObjectFor(m_currentNode->origin.semantic))));

        isNotMasqueradesAsUndefined.link(&m_jit);
    }
    jump(taken, ForceJump);
    
    notCell.link(&m_jit);
    
    COMPILE_ASSERT((JSValue::UndefinedTag | 1) == JSValue::NullTag, UndefinedTag_OR_1_EQUALS_NullTag);
    if (needsTypeCheck(nodeUse, SpecCell | SpecOther)) {
        m_jit.or32(TrustedImm32(1), valueTagGPR, scratchGPR);
        typeCheck(
            JSValueRegs(valueTagGPR, valuePayloadGPR), nodeUse, SpecCell | SpecOther,
            m_jit.branch32(MacroAssembler::NotEqual, scratchGPR, TrustedImm32(JSValue::NullTag)));
    }

    jump(notTaken);
    
    noResult(m_currentNode);
}

void SpeculativeJIT::emitBranch(Node* node)
{
    BasicBlock* taken = node->branchData()->taken.block;
    BasicBlock* notTaken = node->branchData()->notTaken.block;

    switch (node->child1().useKind()) {
    case BooleanUse:
    case KnownBooleanUse: {
        SpeculateBooleanOperand value(this, node->child1());
        MacroAssembler::ResultCondition condition = MacroAssembler::NonZero;

        if (taken == nextBlock()) {
            condition = MacroAssembler::Zero;
            BasicBlock* tmp = taken;
            taken = notTaken;
            notTaken = tmp;
        }

        branchTest32(condition, value.gpr(), TrustedImm32(1), taken);
        jump(notTaken);

        noResult(node);
        return;
    }
    
    case ObjectOrOtherUse: {
        emitObjectOrOtherBranch(node->child1(), taken, notTaken);
        return;
    }

    case StringUse: {
        emitStringBranch(node->child1(), taken, notTaken);
        return;
    }

    case StringOrOtherUse: {
        emitStringOrOtherBranch(node->child1(), taken, notTaken);
        return;
    }

    case DoubleRepUse:
    case Int32Use: {
        if (node->child1().useKind() == Int32Use) {
            bool invert = false;
            
            if (taken == nextBlock()) {
                invert = true;
                BasicBlock* tmp = taken;
                taken = notTaken;
                notTaken = tmp;
            }

            SpeculateInt32Operand value(this, node->child1());
            branchTest32(invert ? MacroAssembler::Zero : MacroAssembler::NonZero, value.gpr(), taken);
        } else {
            SpeculateDoubleOperand value(this, node->child1());
            FPRTemporary scratch(this);
            branchDoubleNonZero(value.fpr(), scratch.fpr(), taken);
        }
        
        jump(notTaken);
        
        noResult(node);
        return;
    }
    
    case UntypedUse: {
        JSValueOperand value(this, node->child1());
        FPRTemporary valueFPR(this);
        FPRTemporary tempFPR(this);
        GPRTemporary result(this);
        GPRTemporary temp(this);

        JSValueRegs valueRegs = value.jsValueRegs();
        GPRReg resultGPR = result.gpr();
    
        use(node->child1());

        bool shouldCheckMasqueradesAsUndefined = !masqueradesAsUndefinedWatchpointIsStillValid();
        JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
        m_jit.emitConvertValueToBoolean(valueRegs, resultGPR, temp.gpr(), valueFPR.fpr(), tempFPR.fpr(), shouldCheckMasqueradesAsUndefined, globalObject);
        branchTest32(JITCompiler::Zero, resultGPR, notTaken);
        jump(taken, ForceJump);

        noResult(node, UseChildrenCalledExplicitly);
        return;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

template<typename BaseOperandType, typename PropertyOperandType, typename ValueOperandType, typename TagType>
void SpeculativeJIT::compileContiguousPutByVal(Node* node, BaseOperandType& base, PropertyOperandType& property, ValueOperandType& value, GPRReg valuePayloadReg, TagType valueTag)
{
    Edge child4 = m_jit.graph().varArgChild(node, 3);

    ArrayMode arrayMode = node->arrayMode();
    
    GPRReg baseReg = base.gpr();
    GPRReg propertyReg = property.gpr();
    
    StorageOperand storage(this, child4);
    GPRReg storageReg = storage.gpr();

    if (node->op() == PutByValAlias) {
        // Store the value to the array.
        GPRReg propertyReg = property.gpr();
        m_jit.store32(valueTag, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
        m_jit.store32(valuePayloadReg, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
        
        noResult(node);
        return;
    }
    
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
        
        m_jit.add32(TrustedImm32(1), propertyReg);
        m_jit.store32(propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength()));
        m_jit.sub32(TrustedImm32(1), propertyReg);
        
        inBounds.link(&m_jit);
    }
    
    m_jit.store32(valueTag, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
    m_jit.store32(valuePayloadReg, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
    
    base.use();
    property.use();
    value.use();
    storage.use();
    
    if (arrayMode.isOutOfBounds()) {
        if (node->op() == PutByValDirect) {
            addSlowPathGenerator(slowPathCall(
                slowCase, this,
                m_jit.codeBlock()->isStrictMode() ? operationPutByValDirectBeyondArrayBoundsStrict : operationPutByValDirectBeyondArrayBoundsNonStrict,
                NoResult, baseReg, propertyReg, valueTag, valuePayloadReg));
        } else {
            addSlowPathGenerator(slowPathCall(
                slowCase, this,
                m_jit.codeBlock()->isStrictMode() ? operationPutByValBeyondArrayBoundsStrict : operationPutByValBeyondArrayBoundsNonStrict,
                NoResult, baseReg, propertyReg, valueTag, valuePayloadReg));
        }
    }

    noResult(node, UseChildrenCalledExplicitly);    
}

void SpeculativeJIT::compile(Node* node)
{
    NodeType op = node->op();

#if ENABLE(DFG_REGISTER_ALLOCATION_VALIDATION)
    m_jit.clearRegisterAllocationOffsets();
#endif

    switch (op) {
    case JSConstant:
    case DoubleConstant:
    case PhantomDirectArguments:
    case PhantomClonedArguments:
        initConstantInfo(node);
        break;

    case LazyJSConstant:
        compileLazyJSConstant(node);
        break;

    case Identity: {
        speculate(node, node->child1());
        switch (node->child1().useKind()) {
        case DoubleRepUse:
        case DoubleRepRealUse: {
            SpeculateDoubleOperand op(this, node->child1());
            doubleResult(op.fpr(), node);
            break;
        }
        case Int52RepUse: 
        case AnyIntUse:
        case DoubleRepAnyIntUse: {
            RELEASE_ASSERT_NOT_REACHED();   
            break;
        }
        default: {
            JSValueOperand op(this, node->child1());
            GPRTemporary resultTag(this, Reuse, op, TagWord);
            GPRTemporary resultPayload(this, Reuse, op, PayloadWord);
            GPRReg sourceTag = op.tagGPR();
            GPRReg sourcePayload = op.payloadGPR();
            GPRReg resultTagGPR = resultTag.gpr();
            GPRReg resultPayloadGPR = resultPayload.gpr();
            m_jit.move(sourceTag, resultTagGPR);
            m_jit.move(sourcePayload, resultPayloadGPR);
            jsValueResult(resultTagGPR, resultPayloadGPR, node);
            break;
        }
        } // switch
        break;
    }

    case GetLocal: {
        AbstractValue& value = m_state.variables().operand(node->local());

        // If the CFA is tracking this variable and it found that the variable
        // cannot have been assigned, then don't attempt to proceed.
        if (value.isClear()) {
            m_compileOkay = false;
            break;
        }
        
        switch (node->variableAccessData()->flushFormat()) {
        case FlushedDouble: {
            FPRTemporary result(this);
            m_jit.loadDouble(JITCompiler::addressFor(node->machineLocal()), result.fpr());
            VirtualRegister virtualRegister = node->virtualRegister();
            m_fprs.retain(result.fpr(), virtualRegister, SpillOrderDouble);
            generationInfoFromVirtualRegister(virtualRegister).initDouble(node, node->refCount(), result.fpr());
            break;
        }
        
        case FlushedInt32: {
            GPRTemporary result(this);
            m_jit.load32(JITCompiler::payloadFor(node->machineLocal()), result.gpr());
            
            // Like int32Result, but don't useChildren - our children are phi nodes,
            // and don't represent values within this dataflow with virtual registers.
            VirtualRegister virtualRegister = node->virtualRegister();
            m_gprs.retain(result.gpr(), virtualRegister, SpillOrderInteger);
            generationInfoFromVirtualRegister(virtualRegister).initInt32(node, node->refCount(), result.gpr());
            break;
        }
        
        case FlushedCell: {
            GPRTemporary result(this);
            m_jit.load32(JITCompiler::payloadFor(node->machineLocal()), result.gpr());
            
            // Like cellResult, but don't useChildren - our children are phi nodes,
            // and don't represent values within this dataflow with virtual registers.
            VirtualRegister virtualRegister = node->virtualRegister();
            m_gprs.retain(result.gpr(), virtualRegister, SpillOrderCell);
            generationInfoFromVirtualRegister(virtualRegister).initCell(node, node->refCount(), result.gpr());
            break;
        }
            
        case FlushedBoolean: {
            GPRTemporary result(this);
            m_jit.load32(JITCompiler::payloadFor(node->machineLocal()), result.gpr());
            
            // Like booleanResult, but don't useChildren - our children are phi nodes,
            // and don't represent values within this dataflow with virtual registers.
            VirtualRegister virtualRegister = node->virtualRegister();
            m_gprs.retain(result.gpr(), virtualRegister, SpillOrderBoolean);
            generationInfoFromVirtualRegister(virtualRegister).initBoolean(node, node->refCount(), result.gpr());
            break;
        }
            
        case FlushedJSValue: {
            GPRTemporary result(this);
            GPRTemporary tag(this);
            m_jit.load32(JITCompiler::payloadFor(node->machineLocal()), result.gpr());
            m_jit.load32(JITCompiler::tagFor(node->machineLocal()), tag.gpr());
            
            // Like jsValueResult, but don't useChildren - our children are phi nodes,
            // and don't represent values within this dataflow with virtual registers.
            VirtualRegister virtualRegister = node->virtualRegister();
            m_gprs.retain(result.gpr(), virtualRegister, SpillOrderJS);
            m_gprs.retain(tag.gpr(), virtualRegister, SpillOrderJS);
            
            generationInfoFromVirtualRegister(virtualRegister).initJSValue(node, node->refCount(), tag.gpr(), result.gpr(), DataFormatJS);
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
        break;
    }
        
    case GetLocalUnlinked: {
        GPRTemporary payload(this);
        GPRTemporary tag(this);
        m_jit.load32(JITCompiler::payloadFor(node->unlinkedMachineLocal()), payload.gpr());
        m_jit.load32(JITCompiler::tagFor(node->unlinkedMachineLocal()), tag.gpr());
        jsValueResult(tag.gpr(), payload.gpr(), node);
        break;
    }

    case MovHint: {
        compileMovHint(m_currentNode);
        noResult(node);
        break;
    }
        
    case ZombieHint: {
        recordSetLocal(m_currentNode->unlinkedLocal(), VirtualRegister(), DataFormatDead);
        noResult(node);
        break;
    }

    case ExitOK: {
        noResult(node);
        break;
    }
        
    case SetLocal: {
        switch (node->variableAccessData()->flushFormat()) {
        case FlushedDouble: {
            SpeculateDoubleOperand value(this, node->child1());
            m_jit.storeDouble(value.fpr(), JITCompiler::addressFor(node->machineLocal()));
            noResult(node);
            // Indicate that it's no longer necessary to retrieve the value of
            // this bytecode variable from registers or other locations in the stack,
            // but that it is stored as a double.
            recordSetLocal(DataFormatDouble);
            break;
        }
            
        case FlushedInt32: {
            SpeculateInt32Operand value(this, node->child1());
            m_jit.store32(value.gpr(), JITCompiler::payloadFor(node->machineLocal()));
            noResult(node);
            recordSetLocal(DataFormatInt32);
            break;
        }
            
        case FlushedCell: {
            SpeculateCellOperand cell(this, node->child1());
            GPRReg cellGPR = cell.gpr();
            m_jit.storePtr(cellGPR, JITCompiler::payloadFor(node->machineLocal()));
            noResult(node);
            recordSetLocal(DataFormatCell);
            break;
        }
            
        case FlushedBoolean: {
            SpeculateBooleanOperand value(this, node->child1());
            m_jit.store32(value.gpr(), JITCompiler::payloadFor(node->machineLocal()));
            noResult(node);
            recordSetLocal(DataFormatBoolean);
            break;
        }
            
        case FlushedJSValue: {
            JSValueOperand value(this, node->child1());
            m_jit.store32(value.payloadGPR(), JITCompiler::payloadFor(node->machineLocal()));
            m_jit.store32(value.tagGPR(), JITCompiler::tagFor(node->machineLocal()));
            noResult(node);
            recordSetLocal(dataFormatFor(node->variableAccessData()->flushFormat()));
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }

    case SetArgument:
        // This is a no-op; it just marks the fact that the argument is being used.
        // But it may be profitable to use this as a hook to run speculation checks
        // on arguments, thereby allowing us to trivially eliminate such checks if
        // the argument is not used.
        recordSetLocal(dataFormatFor(node->variableAccessData()->flushFormat()));
        break;

    case BitAnd:
    case BitOr:
    case BitXor:
        compileBitwiseOp(node);
        break;

    case BitRShift:
    case BitLShift:
    case BitURShift:
        compileShiftOp(node);
        break;

    case UInt32ToNumber: {
        compileUInt32ToNumber(node);
        break;
    }
        
    case DoubleAsInt32: {
        compileDoubleAsInt32(node);
        break;
    }

    case ValueToInt32: {
        compileValueToInt32(node);
        break;
    }
        
    case DoubleRep: {
        compileDoubleRep(node);
        break;
    }
        
    case ValueRep: {
        compileValueRep(node);
        break;
    }

    case ValueAdd:
        compileValueAdd(node);
        break;

    case StrCat: {
        JSValueOperand op1(this, node->child1(), ManualOperandSpeculation);
        JSValueOperand op2(this, node->child2(), ManualOperandSpeculation);
        JSValueOperand op3(this, node->child3(), ManualOperandSpeculation);
        
        JSValueRegs op1Regs = op1.jsValueRegs();
        JSValueRegs op2Regs = op2.jsValueRegs();
        JSValueRegs op3Regs;

        if (node->child3())
            op3Regs = op3.jsValueRegs();
        
        flushRegisters();

        GPRFlushedCallResult result(this);
        if (node->child3())
            callOperation(operationStrCat3, result.gpr(), op1Regs, op2Regs, op3Regs);
        else
            callOperation(operationStrCat2, result.gpr(), op1Regs, op2Regs);
        m_jit.exceptionCheck();
        
        cellResult(result.gpr(), node);
        break;
    }

    case ArithAdd:
        compileArithAdd(node);
        break;

    case ArithClz32:
        compileArithClz32(node);
        break;

    case MakeRope:
        compileMakeRope(node);
        break;

    case ArithSub:
        compileArithSub(node);
        break;

    case ArithNegate:
        compileArithNegate(node);
        break;

    case ArithMul:
        compileArithMul(node);
        break;

    case ArithDiv: {
        compileArithDiv(node);
        break;
    }

    case ArithMod: {
        compileArithMod(node);
        break;
    }

    case ArithPow: {
        compileArithPow(node);
        break;
    }

    case ArithAbs:
        compileArithAbs(node);
        break;

    case ArithMin:
    case ArithMax: {
        switch (node->binaryUseKind()) {
        case Int32Use: {
            SpeculateStrictInt32Operand op1(this, node->child1());
            SpeculateStrictInt32Operand op2(this, node->child2());
            GPRTemporary result(this, Reuse, op1);

            GPRReg op1GPR = op1.gpr();
            GPRReg op2GPR = op2.gpr();
            GPRReg resultGPR = result.gpr();

            MacroAssembler::Jump op1Less = m_jit.branch32(op == ArithMin ? MacroAssembler::LessThan : MacroAssembler::GreaterThan, op1GPR, op2GPR);
            m_jit.move(op2GPR, resultGPR);
            if (op1GPR != resultGPR) {
                MacroAssembler::Jump done = m_jit.jump();
                op1Less.link(&m_jit);
                m_jit.move(op1GPR, resultGPR);
                done.link(&m_jit);
            } else
                op1Less.link(&m_jit);
            
            int32Result(resultGPR, node);
            break;
        }
        
        case DoubleRepUse: {
            SpeculateDoubleOperand op1(this, node->child1());
            SpeculateDoubleOperand op2(this, node->child2());
            FPRTemporary result(this, op1);

            FPRReg op1FPR = op1.fpr();
            FPRReg op2FPR = op2.fpr();
            FPRReg resultFPR = result.fpr();

            MacroAssembler::JumpList done;
        
            MacroAssembler::Jump op1Less = m_jit.branchDouble(op == ArithMin ? MacroAssembler::DoubleLessThan : MacroAssembler::DoubleGreaterThan, op1FPR, op2FPR);
        
            // op2 is eather the lesser one or one of then is NaN
            MacroAssembler::Jump op2Less = m_jit.branchDouble(op == ArithMin ? MacroAssembler::DoubleGreaterThanOrEqual : MacroAssembler::DoubleLessThanOrEqual, op1FPR, op2FPR);
        
            // Unordered case. We don't know which of op1, op2 is NaN. Manufacture NaN by adding 
            // op1 + op2 and putting it into result.
            m_jit.addDouble(op1FPR, op2FPR, resultFPR);
            done.append(m_jit.jump());
        
            op2Less.link(&m_jit);
            m_jit.moveDouble(op2FPR, resultFPR);
        
            if (op1FPR != resultFPR) {
                done.append(m_jit.jump());
            
                op1Less.link(&m_jit);
                m_jit.moveDouble(op1FPR, resultFPR);
            } else
                op1Less.link(&m_jit);
        
            done.link(&m_jit);
        
            doubleResult(resultFPR, node);
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }

    case ArithSqrt:
        compileArithSqrt(node);
        break;

    case ArithFRound:
        compileArithFRound(node);
        break;

    case ArithRandom:
        compileArithRandom(node);
        break;

    case ArithRound:
    case ArithFloor:
    case ArithCeil:
    case ArithTrunc:
        compileArithRounding(node);
        break;

    case ArithSin:
        compileArithSin(node);
        break;

    case ArithCos:
        compileArithCos(node);
        break;

    case ArithTan:
        compileArithTan(node);
        break;

    case ArithLog:
        compileArithLog(node);
        break;

    case LogicalNot:
        compileLogicalNot(node);
        break;

    case CompareLess:
        if (compare(node, JITCompiler::LessThan, JITCompiler::DoubleLessThan, operationCompareLess))
            return;
        break;

    case CompareLessEq:
        if (compare(node, JITCompiler::LessThanOrEqual, JITCompiler::DoubleLessThanOrEqual, operationCompareLessEq))
            return;
        break;

    case CompareGreater:
        if (compare(node, JITCompiler::GreaterThan, JITCompiler::DoubleGreaterThan, operationCompareGreater))
            return;
        break;

    case CompareGreaterEq:
        if (compare(node, JITCompiler::GreaterThanOrEqual, JITCompiler::DoubleGreaterThanOrEqual, operationCompareGreaterEq))
            return;
        break;

    case CompareEq:
        if (compare(node, JITCompiler::Equal, JITCompiler::DoubleEqual, operationCompareEq))
            return;
        break;

    case CompareStrictEq:
        if (compileStrictEq(node))
            return;
        break;
        
    case CompareEqPtr:
        compileCompareEqPtr(node);
        break;

    case StringCharCodeAt: {
        compileGetCharCodeAt(node);
        break;
    }

    case StringCharAt: {
        // Relies on StringCharAt node having same basic layout as GetByVal
        compileGetByValOnString(node);
        break;
    }

    case StringFromCharCode: {
        compileFromCharCode(node);
        break;
    }
        
    case CheckArray: {
        checkArray(node);
        break;
    }
        
    case Arrayify:
    case ArrayifyToStructure: {
        arrayify(node);
        break;
    }

    case GetByVal: {
        switch (node->arrayMode().type()) {
        case Array::SelectUsingPredictions:
        case Array::ForceExit:
            RELEASE_ASSERT_NOT_REACHED();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
            terminateSpeculativeExecution(InadequateCoverage, JSValueRegs(), 0);
#endif
            break;
        case Array::Undecided: {
            SpeculateStrictInt32Operand index(this, node->child2());
            GPRTemporary resultTag(this, Reuse, index);
            GPRTemporary resultPayload(this);

            GPRReg indexGPR = index.gpr();
            GPRReg resultTagGPR = resultTag.gpr();
            GPRReg resultPayloadGPR = resultPayload.gpr();

            speculationCheck(OutOfBounds, JSValueRegs(), node,
                m_jit.branch32(MacroAssembler::LessThan, indexGPR, MacroAssembler::TrustedImm32(0)));

            use(node->child1());
            index.use();

            m_jit.move(MacroAssembler::TrustedImm32(JSValue::UndefinedTag), resultTagGPR);
            m_jit.move(MacroAssembler::TrustedImm32(0), resultPayloadGPR);
            jsValueResult(resultTagGPR, resultPayloadGPR, node, UseChildrenCalledExplicitly);
            break;
        }
        case Array::Generic: {
            SpeculateCellOperand base(this, node->child1()); // Save a register, speculate cell. We'll probably be right.
            JSValueOperand property(this, node->child2());
            GPRReg baseGPR = base.gpr();
            JSValueRegs propertyRegs = property.jsValueRegs();
            
            flushRegisters();
            GPRFlushedCallResult2 resultTag(this);
            GPRFlushedCallResult resultPayload(this);
            callOperation(operationGetByValCell, JSValueRegs(resultTag.gpr(), resultPayload.gpr()), baseGPR, propertyRegs);
            m_jit.exceptionCheck();
            
            jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
            break;
        }
        case Array::Int32:
        case Array::Contiguous: {
            if (node->arrayMode().isInBounds()) {
                SpeculateStrictInt32Operand property(this, node->child2());
                StorageOperand storage(this, node->child3());
            
                GPRReg propertyReg = property.gpr();
                GPRReg storageReg = storage.gpr();
            
                if (!m_compileOkay)
                    return;
            
                speculationCheck(OutOfBounds, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength())));
            
                GPRTemporary resultPayload(this);
                if (node->arrayMode().type() == Array::Int32) {
                    ASSERT(!node->arrayMode().isSaneChain());
                    
                    speculationCheck(
                        OutOfBounds, JSValueRegs(), 0,
                        m_jit.branch32(
                            MacroAssembler::Equal,
                            MacroAssembler::BaseIndex(
                                storageReg, propertyReg, MacroAssembler::TimesEight, TagOffset),
                            TrustedImm32(JSValue::EmptyValueTag)));
                    m_jit.load32(
                        MacroAssembler::BaseIndex(
                            storageReg, propertyReg, MacroAssembler::TimesEight, PayloadOffset),
                        resultPayload.gpr());
                    int32Result(resultPayload.gpr(), node);
                    break;
                }
                
                GPRTemporary resultTag(this);
                m_jit.load32(
                    MacroAssembler::BaseIndex(
                        storageReg, propertyReg, MacroAssembler::TimesEight, TagOffset),
                    resultTag.gpr());
                m_jit.load32(
                    MacroAssembler::BaseIndex(
                        storageReg, propertyReg, MacroAssembler::TimesEight, PayloadOffset),
                    resultPayload.gpr());
                if (node->arrayMode().isSaneChain()) {
                    JITCompiler::Jump notHole = m_jit.branch32(
                        MacroAssembler::NotEqual, resultTag.gpr(),
                        TrustedImm32(JSValue::EmptyValueTag));
                    m_jit.move(TrustedImm32(JSValue::UndefinedTag), resultTag.gpr());
                    m_jit.move(TrustedImm32(0), resultPayload.gpr());
                    notHole.link(&m_jit);
                } else {
                    speculationCheck(
                        LoadFromHole, JSValueRegs(), 0,
                        m_jit.branch32(
                            MacroAssembler::Equal, resultTag.gpr(),
                            TrustedImm32(JSValue::EmptyValueTag)));
                }
                jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
                break;
            }

            SpeculateCellOperand base(this, node->child1());
            SpeculateStrictInt32Operand property(this, node->child2());
            StorageOperand storage(this, node->child3());
            
            GPRReg baseReg = base.gpr();
            GPRReg propertyReg = property.gpr();
            GPRReg storageReg = storage.gpr();
            
            if (!m_compileOkay)
                return;
            
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            GPRReg resultTagReg = resultTag.gpr();
            GPRReg resultPayloadReg = resultPayload.gpr();
            
            MacroAssembler::JumpList slowCases;

            slowCases.append(m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength())));
            
            m_jit.load32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)), resultTagReg);
            m_jit.load32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)), resultPayloadReg);
            slowCases.append(m_jit.branch32(MacroAssembler::Equal, resultTagReg, TrustedImm32(JSValue::EmptyValueTag)));
            
            addSlowPathGenerator(
                slowPathCall(
                    slowCases, this, operationGetByValArrayInt,
                    JSValueRegs(resultTagReg, resultPayloadReg), baseReg, propertyReg));
            
            jsValueResult(resultTagReg, resultPayloadReg, node);
            break;
        }
        case Array::Double: {
            if (node->arrayMode().isInBounds()) {
                SpeculateStrictInt32Operand property(this, node->child2());
                StorageOperand storage(this, node->child3());
            
                GPRReg propertyReg = property.gpr();
                GPRReg storageReg = storage.gpr();
            
                if (!m_compileOkay)
                    return;
            
                speculationCheck(OutOfBounds, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength())));
            
                FPRTemporary result(this);
                m_jit.loadDouble(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight), result.fpr());
                if (!node->arrayMode().isSaneChain())
                    speculationCheck(LoadFromHole, JSValueRegs(), 0, m_jit.branchDouble(MacroAssembler::DoubleNotEqualOrUnordered, result.fpr(), result.fpr()));
                doubleResult(result.fpr(), node);
                break;
            }

            SpeculateCellOperand base(this, node->child1());
            SpeculateStrictInt32Operand property(this, node->child2());
            StorageOperand storage(this, node->child3());
            
            GPRReg baseReg = base.gpr();
            GPRReg propertyReg = property.gpr();
            GPRReg storageReg = storage.gpr();
            
            if (!m_compileOkay)
                return;
            
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            FPRTemporary temp(this);
            GPRReg resultTagReg = resultTag.gpr();
            GPRReg resultPayloadReg = resultPayload.gpr();
            FPRReg tempReg = temp.fpr();
            
            MacroAssembler::JumpList slowCases;
            
            slowCases.append(m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, Butterfly::offsetOfPublicLength())));
            
            m_jit.loadDouble(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight), tempReg);
            slowCases.append(m_jit.branchDouble(MacroAssembler::DoubleNotEqualOrUnordered, tempReg, tempReg));
            boxDouble(tempReg, resultTagReg, resultPayloadReg);

            addSlowPathGenerator(
                slowPathCall(
                    slowCases, this, operationGetByValArrayInt,
                    JSValueRegs(resultTagReg, resultPayloadReg), baseReg, propertyReg));
            
            jsValueResult(resultTagReg, resultPayloadReg, node);
            break;
        }
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage: {
            if (node->arrayMode().isInBounds()) {
                SpeculateStrictInt32Operand property(this, node->child2());
                StorageOperand storage(this, node->child3());
                GPRReg propertyReg = property.gpr();
                GPRReg storageReg = storage.gpr();
        
                if (!m_compileOkay)
                    return;

                speculationCheck(OutOfBounds, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, ArrayStorage::vectorLengthOffset())));

                GPRTemporary resultTag(this);
                GPRTemporary resultPayload(this);

                m_jit.load32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), resultTag.gpr());
                speculationCheck(LoadFromHole, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::Equal, resultTag.gpr(), TrustedImm32(JSValue::EmptyValueTag)));
                m_jit.load32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), resultPayload.gpr());
            
                jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
                break;
            }

            SpeculateCellOperand base(this, node->child1());
            SpeculateStrictInt32Operand property(this, node->child2());
            StorageOperand storage(this, node->child3());
            GPRReg propertyReg = property.gpr();
            GPRReg storageReg = storage.gpr();
            GPRReg baseReg = base.gpr();

            if (!m_compileOkay)
                return;

            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            GPRReg resultTagReg = resultTag.gpr();
            GPRReg resultPayloadReg = resultPayload.gpr();

            JITCompiler::Jump outOfBounds = m_jit.branch32(
                MacroAssembler::AboveOrEqual, propertyReg,
                MacroAssembler::Address(storageReg, ArrayStorage::vectorLengthOffset()));

            m_jit.load32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), resultTagReg);
            JITCompiler::Jump hole = m_jit.branch32(
                MacroAssembler::Equal, resultTag.gpr(), TrustedImm32(JSValue::EmptyValueTag));
            m_jit.load32(MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), resultPayloadReg);
            
            JITCompiler::JumpList slowCases;
            slowCases.append(outOfBounds);
            slowCases.append(hole);
            addSlowPathGenerator(
                slowPathCall(
                    slowCases, this, operationGetByValArrayInt,
                    JSValueRegs(resultTagReg, resultPayloadReg),
                    baseReg, propertyReg));

            jsValueResult(resultTagReg, resultPayloadReg, node);
            break;
        }
        case Array::String:
            compileGetByValOnString(node);
            break;
        case Array::DirectArguments:
            compileGetByValOnDirectArguments(node);
            break;
        case Array::ScopedArguments:
            compileGetByValOnScopedArguments(node);
            break;
        default: {
            TypedArrayType type = node->arrayMode().typedArrayType();
            if (isInt(type))
                compileGetByValOnIntTypedArray(node, type);
            else
                compileGetByValOnFloatTypedArray(node, type);
        } }
        break;
    }
    
    case ToLowerCase: {
        compileToLowerCase(node);
        break;
    }

    case GetByValWithThis: {
        JSValueOperand base(this, node->child1());
        JSValueRegs baseRegs = base.jsValueRegs();
        JSValueOperand thisValue(this, node->child2());
        JSValueRegs thisValueRegs = thisValue.jsValueRegs();
        JSValueOperand subscript(this, node->child3());
        JSValueRegs subscriptRegs = subscript.jsValueRegs();

        GPRFlushedCallResult resultPayload(this);
        GPRFlushedCallResult2 resultTag(this);
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg resultTagGPR = resultTag.gpr();

        flushRegisters();
        callOperation(operationGetByValWithThis, JSValueRegs(resultTagGPR, resultPayloadGPR), baseRegs, thisValueRegs, subscriptRegs);
        m_jit.exceptionCheck();

        jsValueResult(resultTagGPR, resultPayloadGPR, node);
        break;
    }

    case PutByValDirect:
    case PutByVal:
    case PutByValAlias: {
        Edge child1 = m_jit.graph().varArgChild(node, 0);
        Edge child2 = m_jit.graph().varArgChild(node, 1);
        Edge child3 = m_jit.graph().varArgChild(node, 2);
        Edge child4 = m_jit.graph().varArgChild(node, 3);
        
        ArrayMode arrayMode = node->arrayMode().modeForPut();
        bool alreadyHandled = false;
        
        switch (arrayMode.type()) {
        case Array::SelectUsingPredictions:
        case Array::ForceExit:
            RELEASE_ASSERT_NOT_REACHED();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
            terminateSpeculativeExecution(InadequateCoverage, JSValueRegs(), 0);
            alreadyHandled = true;
#endif
            break;
        case Array::Generic: {
            ASSERT(node->op() == PutByVal || node->op() == PutByValDirect);
            
            SpeculateCellOperand base(this, child1); // Save a register, speculate cell. We'll probably be right.
            JSValueOperand property(this, child2);
            JSValueOperand value(this, child3);
            GPRReg baseGPR = base.gpr();
            JSValueRegs propertyRegs = property.jsValueRegs();
            JSValueRegs valueRegs = value.jsValueRegs();
            
            flushRegisters();
            if (node->op() == PutByValDirect)
                callOperation(m_jit.codeBlock()->isStrictMode() ? operationPutByValDirectCellStrict : operationPutByValDirectCellNonStrict, baseGPR, propertyRegs, valueRegs);
            else
                callOperation(m_jit.codeBlock()->isStrictMode() ? operationPutByValCellStrict : operationPutByValCellNonStrict, baseGPR, propertyRegs, valueRegs);
            m_jit.exceptionCheck();
            
            noResult(node);
            alreadyHandled = true;
            break;
        }
        default:
            break;
        }
        
        if (alreadyHandled)
            break;
        
        SpeculateCellOperand base(this, child1);
        SpeculateStrictInt32Operand property(this, child2);
        
        GPRReg baseReg = base.gpr();
        GPRReg propertyReg = property.gpr();

        switch (arrayMode.type()) {
        case Array::Int32: {
            SpeculateInt32Operand value(this, child3);

            GPRReg valuePayloadReg = value.gpr();
        
            if (!m_compileOkay)
                return;
            
            compileContiguousPutByVal(node, base, property, value, valuePayloadReg, TrustedImm32(JSValue::Int32Tag));
            break;
        }
        case Array::Contiguous: {
            JSValueOperand value(this, child3);

            GPRReg valueTagReg = value.tagGPR();
            GPRReg valuePayloadReg = value.payloadGPR();
        
            if (!m_compileOkay)
                return;

            compileContiguousPutByVal(node, base, property, value, valuePayloadReg, valueTagReg);
            break;
        }
        case Array::Double: {
            compileDoublePutByVal(node, base, property);
            break;
        }
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage: {
            JSValueOperand value(this, child3);

            GPRReg valueTagReg = value.tagGPR();
            GPRReg valuePayloadReg = value.payloadGPR();
            
            if (!m_compileOkay)
                return;

            StorageOperand storage(this, child4);
            GPRReg storageReg = storage.gpr();

            if (node->op() == PutByValAlias) {
                // Store the value to the array.
                GPRReg propertyReg = property.gpr();
                m_jit.store32(value.tagGPR(), MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
                m_jit.store32(value.payloadGPR(), MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
                
                noResult(node);
                break;
            }

            MacroAssembler::JumpList slowCases;

            MacroAssembler::Jump beyondArrayBounds = m_jit.branch32(MacroAssembler::AboveOrEqual, propertyReg, MacroAssembler::Address(storageReg, ArrayStorage::vectorLengthOffset()));
            if (!arrayMode.isOutOfBounds())
                speculationCheck(OutOfBounds, JSValueRegs(), 0, beyondArrayBounds);
            else
                slowCases.append(beyondArrayBounds);

            // Check if we're writing to a hole; if so increment m_numValuesInVector.
            if (arrayMode.isInBounds()) {
                speculationCheck(
                    StoreToHole, JSValueRegs(), 0,
                    m_jit.branch32(MacroAssembler::Equal, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), TrustedImm32(JSValue::EmptyValueTag)));
            } else {
                MacroAssembler::Jump notHoleValue = m_jit.branch32(MacroAssembler::NotEqual, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), TrustedImm32(JSValue::EmptyValueTag));
                if (arrayMode.isSlowPut()) {
                    // This is sort of strange. If we wanted to optimize this code path, we would invert
                    // the above branch. But it's simply not worth it since this only happens if we're
                    // already having a bad time.
                    slowCases.append(m_jit.jump());
                } else {
                    m_jit.add32(TrustedImm32(1), MacroAssembler::Address(storageReg, ArrayStorage::numValuesInVectorOffset()));
                
                    // If we're writing to a hole we might be growing the array; 
                    MacroAssembler::Jump lengthDoesNotNeedUpdate = m_jit.branch32(MacroAssembler::Below, propertyReg, MacroAssembler::Address(storageReg, ArrayStorage::lengthOffset()));
                    m_jit.add32(TrustedImm32(1), propertyReg);
                    m_jit.store32(propertyReg, MacroAssembler::Address(storageReg, ArrayStorage::lengthOffset()));
                    m_jit.sub32(TrustedImm32(1), propertyReg);
                
                    lengthDoesNotNeedUpdate.link(&m_jit);
                }
                notHoleValue.link(&m_jit);
            }
    
            // Store the value to the array.
            m_jit.store32(valueTagReg, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
            m_jit.store32(valuePayloadReg, MacroAssembler::BaseIndex(storageReg, propertyReg, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));

            base.use();
            property.use();
            value.use();
            storage.use();
            
            if (!slowCases.empty()) {
                if (node->op() == PutByValDirect) {
                    addSlowPathGenerator(slowPathCall(
                        slowCases, this,
                        m_jit.codeBlock()->isStrictMode() ? operationPutByValDirectBeyondArrayBoundsStrict : operationPutByValDirectBeyondArrayBoundsNonStrict,
                        NoResult, baseReg, propertyReg, JSValueRegs(valueTagReg, valuePayloadReg)));
                } else {
                    addSlowPathGenerator(slowPathCall(
                        slowCases, this,
                        m_jit.codeBlock()->isStrictMode() ? operationPutByValBeyondArrayBoundsStrict : operationPutByValBeyondArrayBoundsNonStrict,
                        NoResult, baseReg, propertyReg, JSValueRegs(valueTagReg, valuePayloadReg)));
                }
            }

            noResult(node, UseChildrenCalledExplicitly);
            break;
        }
            
        default: {
            TypedArrayType type = arrayMode.typedArrayType();
            if (isInt(type))
                compilePutByValForIntTypedArray(base.gpr(), property.gpr(), node, type);
            else
                compilePutByValForFloatTypedArray(base.gpr(), property.gpr(), node, type);
        } }
        break;
    }

    case PutByValWithThis: {
#if CPU(X86)
        // We don't have enough registers on X86 to do this
        // without setting up the call frame incrementally.
        unsigned index = 0;
        m_jit.poke(GPRInfo::callFrameRegister, index++);

        {
            JSValueOperand base(this, m_jit.graph().varArgChild(node, 0));
            GPRReg baseTag = base.tagGPR();
            GPRReg basePayload = base.payloadGPR();

            JSValueOperand thisValue(this, m_jit.graph().varArgChild(node, 1));
            GPRReg thisValueTag = thisValue.tagGPR();
            GPRReg thisValuePayload = thisValue.payloadGPR();

            JSValueOperand property(this, m_jit.graph().varArgChild(node, 2));
            GPRReg propertyTag = property.tagGPR();
            GPRReg propertyPayload = property.payloadGPR();

            m_jit.poke(basePayload, index++);
            m_jit.poke(baseTag, index++);

            m_jit.poke(thisValuePayload, index++);
            m_jit.poke(thisValueTag, index++);

            m_jit.poke(propertyPayload, index++);
            m_jit.poke(propertyTag, index++);

            flushRegisters();
        }

        JSValueOperand value(this, m_jit.graph().varArgChild(node, 3));
        GPRReg valueTag = value.tagGPR();
        GPRReg valuePayload = value.payloadGPR();
        m_jit.poke(valuePayload, index++);
        m_jit.poke(valueTag, index++);

        flushRegisters();
        appendCall(m_jit.isStrictModeFor(node->origin.semantic) ? operationPutByValWithThisStrict : operationPutByValWithThis);
        m_jit.exceptionCheck();
#elif CPU(MIPS)
        // We don't have enough registers on MIPS either but the ABI is a little different.
        unsigned index = 4;
        m_jit.move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
        {
            JSValueOperand base(this, m_jit.graph().varArgChild(node, 0));
            GPRReg baseTag = base.tagGPR();
            GPRReg basePayload = base.payloadGPR();

            JSValueOperand thisValue(this, m_jit.graph().varArgChild(node, 1));
            GPRReg thisValueTag = thisValue.tagGPR();
            GPRReg thisValuePayload = thisValue.payloadGPR();

            JSValueOperand property(this, m_jit.graph().varArgChild(node, 2));
            GPRReg propertyTag = property.tagGPR();
            GPRReg propertyPayload = property.payloadGPR();

            // for operationPutByValWithThis[Strict](), base is a 64 bits
            // argument, so it should be double word aligned on the stack.
            // This requirement still applies when it's in argument registers
            // instead of on the stack.
            m_jit.move(basePayload, GPRInfo::argumentGPR2);
            m_jit.move(baseTag, GPRInfo::argumentGPR3);

            m_jit.poke(thisValuePayload, index++);
            m_jit.poke(thisValueTag, index++);

            m_jit.poke(propertyPayload, index++);
            m_jit.poke(propertyTag, index++);

            flushRegisters();
        }

        JSValueOperand value(this, m_jit.graph().varArgChild(node, 3));
        GPRReg valueTag = value.tagGPR();
        GPRReg valuePayload = value.payloadGPR();
        m_jit.poke(valuePayload, index++);
        m_jit.poke(valueTag, index++);

        flushRegisters();
        appendCall(m_jit.isStrictModeFor(node->origin.semantic) ? operationPutByValWithThisStrict : operationPutByValWithThis);
        m_jit.exceptionCheck();
#else
        static_assert(GPRInfo::numberOfRegisters >= 8, "We are assuming we have enough registers to make this call without incrementally setting up the arguments.");

        JSValueOperand base(this, m_jit.graph().varArgChild(node, 0));
        JSValueRegs baseRegs = base.jsValueRegs();

        JSValueOperand thisValue(this, m_jit.graph().varArgChild(node, 1));
        JSValueRegs thisRegs = thisValue.jsValueRegs();

        JSValueOperand property(this, m_jit.graph().varArgChild(node, 2));
        JSValueRegs propertyRegs = property.jsValueRegs();

        JSValueOperand value(this, m_jit.graph().varArgChild(node, 3));
        JSValueRegs valueRegs = value.jsValueRegs();

        flushRegisters();
        callOperation(m_jit.isStrictModeFor(node->origin.semantic) ? operationPutByValWithThisStrict : operationPutByValWithThis,
            NoResult, baseRegs, thisRegs, propertyRegs, valueRegs);
        m_jit.exceptionCheck();
#endif // CPU(X86)

        noResult(node);
        break;
    }

    case RegExpExec: {
        SpeculateCellOperand globalObject(this, node->child1());
        GPRReg globalObjectGPR = globalObject.gpr();
        
        if (node->child2().useKind() == RegExpObjectUse) {
            if (node->child3().useKind() == StringUse) {
                SpeculateCellOperand base(this, node->child2());
                SpeculateCellOperand argument(this, node->child3());
                GPRReg baseGPR = base.gpr();
                GPRReg argumentGPR = argument.gpr();
                speculateRegExpObject(node->child2(), baseGPR);
                speculateString(node->child3(), argumentGPR);
                
                flushRegisters();
                GPRFlushedCallResult2 resultTag(this);
                GPRFlushedCallResult resultPayload(this);
                callOperation(
                    operationRegExpExecString, JSValueRegs(resultTag.gpr(), resultPayload.gpr()),
                    globalObjectGPR, baseGPR, argumentGPR);
                m_jit.exceptionCheck();
                
                jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
                break;
            }
            
            SpeculateCellOperand base(this, node->child2());
            JSValueOperand argument(this, node->child3());
            GPRReg baseGPR = base.gpr();
            JSValueRegs argumentRegs = argument.jsValueRegs();
            speculateRegExpObject(node->child2(), baseGPR);
        
            flushRegisters();
            GPRFlushedCallResult2 resultTag(this);
            GPRFlushedCallResult resultPayload(this);
            callOperation(
                operationRegExpExec, JSValueRegs(resultTag.gpr(), resultPayload.gpr()), globalObjectGPR, baseGPR, argumentRegs);
            m_jit.exceptionCheck();
        
            jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
            break;
        }
        
        JSValueOperand base(this, node->child2());
        JSValueOperand argument(this, node->child3());
        JSValueRegs baseRegs = base.jsValueRegs();
        JSValueRegs argumentRegs = argument.jsValueRegs();
        
        flushRegisters();
        GPRFlushedCallResult2 resultTag(this);
        GPRFlushedCallResult resultPayload(this);
        callOperation(
            operationRegExpExecGeneric, JSValueRegs(resultTag.gpr(), resultPayload.gpr()), globalObjectGPR, baseRegs, argumentRegs);
        m_jit.exceptionCheck();
        
        jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
        break;
    }
        
    case RegExpTest: {
        SpeculateCellOperand globalObject(this, node->child1());
        GPRReg globalObjectGPR = globalObject.gpr();
        
        if (node->child2().useKind() == RegExpObjectUse) {
            if (node->child3().useKind() == StringUse) {
                SpeculateCellOperand base(this, node->child2());
                SpeculateCellOperand argument(this, node->child3());
                GPRReg baseGPR = base.gpr();
                GPRReg argumentGPR = argument.gpr();
                speculateRegExpObject(node->child2(), baseGPR);
                speculateString(node->child3(), argumentGPR);
                
                flushRegisters();
                GPRFlushedCallResult result(this);
                callOperation(
                    operationRegExpTestString, result.gpr(), globalObjectGPR, baseGPR, argumentGPR);
                m_jit.exceptionCheck();
                
                booleanResult(result.gpr(), node);
                break;
            }
            
            SpeculateCellOperand base(this, node->child2());
            JSValueOperand argument(this, node->child3());
            GPRReg baseGPR = base.gpr();
            JSValueRegs argumentRegs = argument.jsValueRegs();
            speculateRegExpObject(node->child2(), baseGPR);
        
            flushRegisters();
            GPRFlushedCallResult result(this);
            callOperation(
                operationRegExpTest, result.gpr(), globalObjectGPR, baseGPR, argumentRegs);
            m_jit.exceptionCheck();
        
            booleanResult(result.gpr(), node);
            break;
        }
        
        JSValueOperand base(this, node->child2());
        JSValueOperand argument(this, node->child3());
        JSValueRegs baseRegs = base.jsValueRegs();
        JSValueRegs argumentRegs = argument.jsValueRegs();

        flushRegisters();
        GPRFlushedCallResult result(this);
        callOperation(
            operationRegExpTestGeneric, result.gpr(), globalObjectGPR, baseRegs, argumentRegs);
        m_jit.exceptionCheck();
        
        booleanResult(result.gpr(), node);
        break;
    }

    case StringReplace:
    case StringReplaceRegExp: {
        if (node->child1().useKind() == StringUse
            && node->child2().useKind() == RegExpObjectUse
            && node->child3().useKind() == StringUse) {
            if (JSString* replace = node->child3()->dynamicCastConstant<JSString*>(*m_jit.vm())) {
                if (!replace->length()) {
                    SpeculateCellOperand string(this, node->child1());
                    SpeculateCellOperand regExp(this, node->child2());
                    GPRReg stringGPR = string.gpr();
                    GPRReg regExpGPR = regExp.gpr();
                    speculateString(node->child1(), stringGPR);
                    speculateRegExpObject(node->child2(), regExpGPR);

                    flushRegisters();
                    GPRFlushedCallResult2 resultTag(this);
                    GPRFlushedCallResult resultPayload(this);
                    callOperation(
                        operationStringProtoFuncReplaceRegExpEmptyStr, JSValueRegs(resultTag.gpr(), resultPayload.gpr()), stringGPR, regExpGPR);
                    m_jit.exceptionCheck();
                    cellResult(resultPayload.gpr(), node);
                    break;
                }
            }
            
            SpeculateCellOperand string(this, node->child1());
            SpeculateCellOperand regExp(this, node->child2());
            SpeculateCellOperand replace(this, node->child3());
            GPRReg stringGPR = string.gpr();
            GPRReg regExpGPR = regExp.gpr();
            GPRReg replaceGPR = replace.gpr();
            speculateString(node->child1(), stringGPR);
            speculateRegExpObject(node->child2(), regExpGPR);
            speculateString(node->child3(), replaceGPR);
            
            flushRegisters();
            GPRFlushedCallResult2 resultTag(this);
            GPRFlushedCallResult resultPayload(this);
            callOperation(
                operationStringProtoFuncReplaceRegExpString, JSValueRegs(resultTag.gpr(), resultPayload.gpr()),
                stringGPR, regExpGPR, replaceGPR);
            m_jit.exceptionCheck();
            cellResult(resultPayload.gpr(), node);
            break;
        }

        // If we fixed up the edge of child2, we inserted a Check(@child2, String).
        OperandSpeculationMode child2SpeculationMode = AutomaticOperandSpeculation;
        if (node->child2().useKind() == StringUse)
            child2SpeculationMode = ManualOperandSpeculation;
        
        JSValueOperand string(this, node->child1());
        JSValueOperand search(this, node->child2(), child2SpeculationMode);
        JSValueOperand replace(this, node->child3());
        JSValueRegs stringRegs = string.jsValueRegs();
        JSValueRegs searchRegs = search.jsValueRegs();
        JSValueRegs replaceRegs = replace.jsValueRegs();
        
        flushRegisters();
        GPRFlushedCallResult2 resultTag(this);
        GPRFlushedCallResult resultPayload(this);
        callOperation(
            operationStringProtoFuncReplaceGeneric, JSValueRegs(resultTag.gpr(), resultPayload.gpr()),
            stringRegs, searchRegs, replaceRegs); 
        m_jit.exceptionCheck();
        cellResult(resultPayload.gpr(), node);
        break;
    }

    case GetRegExpObjectLastIndex: {
        compileGetRegExpObjectLastIndex(node);
        break;
    }
        
    case SetRegExpObjectLastIndex: {
        compileSetRegExpObjectLastIndex(node);
        break;
    }
        
    case RecordRegExpCachedResult: {
        compileRecordRegExpCachedResult(node);
        break;
    }
        
    case ArrayPush: {
        ASSERT(node->arrayMode().isJSArray());
        
        SpeculateCellOperand base(this, node->child1());
        GPRTemporary storageLength(this);
        
        GPRReg baseGPR = base.gpr();
        GPRReg storageLengthGPR = storageLength.gpr();
        
        StorageOperand storage(this, node->child3());
        GPRReg storageGPR = storage.gpr();
        
        switch (node->arrayMode().type()) {
        case Array::Int32: {
            SpeculateInt32Operand value(this, node->child2());
            GPRReg valuePayloadGPR = value.gpr();
            
            m_jit.load32(MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()), storageLengthGPR);
            MacroAssembler::Jump slowPath = m_jit.branch32(MacroAssembler::AboveOrEqual, storageLengthGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfVectorLength()));
            m_jit.store32(TrustedImm32(JSValue::Int32Tag), MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
            m_jit.store32(valuePayloadGPR, MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
            m_jit.add32(TrustedImm32(1), storageLengthGPR);
            m_jit.store32(storageLengthGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()));
            m_jit.move(TrustedImm32(JSValue::Int32Tag), storageGPR);
            
            addSlowPathGenerator(
                slowPathCall(
                    slowPath, this, operationArrayPush,
                    JSValueRegs(storageGPR, storageLengthGPR),
                    TrustedImm32(JSValue::Int32Tag), valuePayloadGPR, baseGPR));
        
            jsValueResult(storageGPR, storageLengthGPR, node);
            break;
        }
            
        case Array::Contiguous: {
            JSValueOperand value(this, node->child2());
            GPRReg valueTagGPR = value.tagGPR();
            GPRReg valuePayloadGPR = value.payloadGPR();

            m_jit.load32(MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()), storageLengthGPR);
            MacroAssembler::Jump slowPath = m_jit.branch32(MacroAssembler::AboveOrEqual, storageLengthGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfVectorLength()));
            m_jit.store32(valueTagGPR, MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
            m_jit.store32(valuePayloadGPR, MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
            m_jit.add32(TrustedImm32(1), storageLengthGPR);
            m_jit.store32(storageLengthGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()));
            m_jit.move(TrustedImm32(JSValue::Int32Tag), storageGPR);
            
            addSlowPathGenerator(
                slowPathCall(
                    slowPath, this, operationArrayPush,
                    JSValueRegs(storageGPR, storageLengthGPR),
                    JSValueRegs(valueTagGPR, valuePayloadGPR), baseGPR));
        
            jsValueResult(storageGPR, storageLengthGPR, node);
            break;
        }
            
        case Array::Double: {
            SpeculateDoubleOperand value(this, node->child2());
            FPRReg valueFPR = value.fpr();

            DFG_TYPE_CHECK(
                JSValueRegs(), node->child2(), SpecDoubleReal,
                m_jit.branchDouble(MacroAssembler::DoubleNotEqualOrUnordered, valueFPR, valueFPR));
            
            m_jit.load32(MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()), storageLengthGPR);
            MacroAssembler::Jump slowPath = m_jit.branch32(MacroAssembler::AboveOrEqual, storageLengthGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfVectorLength()));
            m_jit.storeDouble(valueFPR, MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight));
            m_jit.add32(TrustedImm32(1), storageLengthGPR);
            m_jit.store32(storageLengthGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()));
            m_jit.move(TrustedImm32(JSValue::Int32Tag), storageGPR);
            
            addSlowPathGenerator(
                slowPathCall(
                    slowPath, this, operationArrayPushDouble,
                    JSValueRegs(storageGPR, storageLengthGPR),
                    valueFPR, baseGPR));
        
            jsValueResult(storageGPR, storageLengthGPR, node);
            break;
        }
            
        case Array::ArrayStorage: {
            JSValueOperand value(this, node->child2());
            GPRReg valueTagGPR = value.tagGPR();
            GPRReg valuePayloadGPR = value.payloadGPR();

            m_jit.load32(MacroAssembler::Address(storageGPR, ArrayStorage::lengthOffset()), storageLengthGPR);
        
            // Refuse to handle bizarre lengths.
            speculationCheck(Uncountable, JSValueRegs(), 0, m_jit.branch32(MacroAssembler::Above, storageLengthGPR, TrustedImm32(0x7ffffffe)));
        
            MacroAssembler::Jump slowPath = m_jit.branch32(MacroAssembler::AboveOrEqual, storageLengthGPR, MacroAssembler::Address(storageGPR, ArrayStorage::vectorLengthOffset()));
        
            m_jit.store32(valueTagGPR, MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
            m_jit.store32(valuePayloadGPR, MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
        
            m_jit.add32(TrustedImm32(1), storageLengthGPR);
            m_jit.store32(storageLengthGPR, MacroAssembler::Address(storageGPR, ArrayStorage::lengthOffset()));
            m_jit.add32(TrustedImm32(1), MacroAssembler::Address(storageGPR, OBJECT_OFFSETOF(ArrayStorage, m_numValuesInVector)));
            m_jit.move(TrustedImm32(JSValue::Int32Tag), storageGPR);
        
            addSlowPathGenerator(slowPathCall(slowPath, this, operationArrayPush, JSValueRegs(storageGPR, storageLengthGPR),
                JSValueRegs(valueTagGPR, valuePayloadGPR), baseGPR));
        
            jsValueResult(storageGPR, storageLengthGPR, node);
            break;
        }
            
        default:
            CRASH();
            break;
        }
        break;
    }
        
    case ArrayPop: {
        ASSERT(node->arrayMode().isJSArray());
        
        SpeculateCellOperand base(this, node->child1());
        StorageOperand storage(this, node->child2());
        GPRTemporary valueTag(this);
        GPRTemporary valuePayload(this);
        
        GPRReg baseGPR = base.gpr();
        GPRReg valueTagGPR = valueTag.gpr();
        GPRReg valuePayloadGPR = valuePayload.gpr();
        GPRReg storageGPR = storage.gpr();
        
        switch (node->arrayMode().type()) {
        case Array::Int32:
        case Array::Contiguous: {
            m_jit.load32(
                MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()), valuePayloadGPR);
            MacroAssembler::Jump undefinedCase =
                m_jit.branchTest32(MacroAssembler::Zero, valuePayloadGPR);
            m_jit.sub32(TrustedImm32(1), valuePayloadGPR);
            m_jit.store32(
                valuePayloadGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()));
            m_jit.load32(
                MacroAssembler::BaseIndex(storageGPR, valuePayloadGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)),
                valueTagGPR);
            MacroAssembler::Jump slowCase = m_jit.branch32(MacroAssembler::Equal, valueTagGPR, TrustedImm32(JSValue::EmptyValueTag));
            m_jit.store32(
                MacroAssembler::TrustedImm32(JSValue::EmptyValueTag),
                MacroAssembler::BaseIndex(storageGPR, valuePayloadGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
            m_jit.load32(
                MacroAssembler::BaseIndex(storageGPR, valuePayloadGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)),
                valuePayloadGPR);

            addSlowPathGenerator(
                slowPathMove(
                    undefinedCase, this,
                    MacroAssembler::TrustedImm32(jsUndefined().tag()), valueTagGPR,
                    MacroAssembler::TrustedImm32(jsUndefined().payload()), valuePayloadGPR));
            addSlowPathGenerator(
                slowPathCall(
                    slowCase, this, operationArrayPopAndRecoverLength,
                    JSValueRegs(valueTagGPR, valuePayloadGPR), baseGPR));
            
            jsValueResult(valueTagGPR, valuePayloadGPR, node);
            break;
        }
            
        case Array::Double: {
            FPRTemporary temp(this);
            FPRReg tempFPR = temp.fpr();
            
            m_jit.load32(
                MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()), valuePayloadGPR);
            MacroAssembler::Jump undefinedCase =
                m_jit.branchTest32(MacroAssembler::Zero, valuePayloadGPR);
            m_jit.sub32(TrustedImm32(1), valuePayloadGPR);
            m_jit.store32(
                valuePayloadGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength()));
            m_jit.loadDouble(
                MacroAssembler::BaseIndex(storageGPR, valuePayloadGPR, MacroAssembler::TimesEight),
                tempFPR);
            MacroAssembler::Jump slowCase = m_jit.branchDouble(MacroAssembler::DoubleNotEqualOrUnordered, tempFPR, tempFPR);
            JSValue nan = JSValue(JSValue::EncodeAsDouble, PNaN);
            m_jit.store32(
                MacroAssembler::TrustedImm32(nan.u.asBits.tag),
                MacroAssembler::BaseIndex(storageGPR, valuePayloadGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
            m_jit.store32(
                MacroAssembler::TrustedImm32(nan.u.asBits.payload),
                MacroAssembler::BaseIndex(storageGPR, valuePayloadGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
            boxDouble(tempFPR, valueTagGPR, valuePayloadGPR);

            addSlowPathGenerator(
                slowPathMove(
                    undefinedCase, this,
                    MacroAssembler::TrustedImm32(jsUndefined().tag()), valueTagGPR,
                    MacroAssembler::TrustedImm32(jsUndefined().payload()), valuePayloadGPR));
            addSlowPathGenerator(
                slowPathCall(
                    slowCase, this, operationArrayPopAndRecoverLength,
                    JSValueRegs(valueTagGPR, valuePayloadGPR), baseGPR));
            
            jsValueResult(valueTagGPR, valuePayloadGPR, node);
            break;
        }

        case Array::ArrayStorage: {
            GPRTemporary storageLength(this);
            GPRReg storageLengthGPR = storageLength.gpr();

            m_jit.load32(MacroAssembler::Address(storageGPR, ArrayStorage::lengthOffset()), storageLengthGPR);
        
            JITCompiler::JumpList setUndefinedCases;
            setUndefinedCases.append(m_jit.branchTest32(MacroAssembler::Zero, storageLengthGPR));
        
            m_jit.sub32(TrustedImm32(1), storageLengthGPR);
        
            MacroAssembler::Jump slowCase = m_jit.branch32(MacroAssembler::AboveOrEqual, storageLengthGPR, MacroAssembler::Address(storageGPR, ArrayStorage::vectorLengthOffset()));
        
            m_jit.load32(MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), valueTagGPR);
            m_jit.load32(MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), valuePayloadGPR);
        
            m_jit.store32(storageLengthGPR, MacroAssembler::Address(storageGPR, ArrayStorage::lengthOffset()));

            setUndefinedCases.append(m_jit.branch32(MacroAssembler::Equal, TrustedImm32(JSValue::EmptyValueTag), valueTagGPR));
        
            m_jit.store32(TrustedImm32(JSValue::EmptyValueTag), MacroAssembler::BaseIndex(storageGPR, storageLengthGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0]) + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));

            m_jit.sub32(TrustedImm32(1), MacroAssembler::Address(storageGPR, OBJECT_OFFSETOF(ArrayStorage, m_numValuesInVector)));
        
            addSlowPathGenerator(
                slowPathMove(
                    setUndefinedCases, this,
                    MacroAssembler::TrustedImm32(jsUndefined().tag()), valueTagGPR,
                    MacroAssembler::TrustedImm32(jsUndefined().payload()), valuePayloadGPR));
        
            addSlowPathGenerator(
                slowPathCall(
                    slowCase, this, operationArrayPop,
                    JSValueRegs(valueTagGPR, valuePayloadGPR), baseGPR));

            jsValueResult(valueTagGPR, valuePayloadGPR, node);
            break;
        }
            
        default:
            CRASH();
            break;
        }
        break;
    }

    case ArraySlice: {
        compileArraySlice(node);
        break;
    }

    case DFG::Jump: {
        jump(node->targetBlock());
        noResult(node);
        break;
    }

    case Branch:
        emitBranch(node);
        break;
        
    case Switch:
        emitSwitch(node);
        break;

    case Return: {
        ASSERT(GPRInfo::callFrameRegister != GPRInfo::regT2);
        ASSERT(GPRInfo::regT1 != GPRInfo::returnValueGPR);
        ASSERT(GPRInfo::returnValueGPR != GPRInfo::callFrameRegister);

        // Return the result in returnValueGPR.
        JSValueOperand op1(this, node->child1());
        op1.fill();
        if (op1.isDouble())
            boxDouble(op1.fpr(), GPRInfo::returnValueGPR2, GPRInfo::returnValueGPR);
        else {
            if (op1.payloadGPR() == GPRInfo::returnValueGPR2 && op1.tagGPR() == GPRInfo::returnValueGPR)
                m_jit.swap(GPRInfo::returnValueGPR, GPRInfo::returnValueGPR2);
            else if (op1.payloadGPR() == GPRInfo::returnValueGPR2) {
                m_jit.move(op1.payloadGPR(), GPRInfo::returnValueGPR);
                m_jit.move(op1.tagGPR(), GPRInfo::returnValueGPR2);
            } else {
                m_jit.move(op1.tagGPR(), GPRInfo::returnValueGPR2);
                m_jit.move(op1.payloadGPR(), GPRInfo::returnValueGPR);
            }
        }

        m_jit.emitRestoreCalleeSaves();
        m_jit.emitFunctionEpilogue();
        m_jit.ret();
        
        noResult(node);
        break;
    }
        
    case Throw:
    case ThrowStaticError: {
        // We expect that throw statements are rare and are intended to exit the code block
        // anyway, so we just OSR back to the old JIT for now.
        terminateSpeculativeExecution(Uncountable, JSValueRegs(), 0);
        break;
    }
        
    case BooleanToNumber: {
        switch (node->child1().useKind()) {
        case BooleanUse: {
            SpeculateBooleanOperand value(this, node->child1());
            GPRTemporary result(this); // FIXME: We could reuse, but on speculation fail would need recovery to restore tag (akin to add).
            
            m_jit.move(value.gpr(), result.gpr());

            int32Result(result.gpr(), node);
            break;
        }
            
        case UntypedUse: {
            JSValueOperand value(this, node->child1());
            
            if (!m_interpreter.needsTypeCheck(node->child1(), SpecBoolInt32 | SpecBoolean)) {
                GPRTemporary result(this);
                
                GPRReg valueGPR = value.payloadGPR();
                GPRReg resultGPR = result.gpr();
                
                m_jit.move(valueGPR, resultGPR);
                int32Result(result.gpr(), node);
                break;
            }
            
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            
            GPRReg valueTagGPR = value.tagGPR();
            GPRReg valuePayloadGPR = value.payloadGPR();
            GPRReg resultTagGPR = resultTag.gpr();
            GPRReg resultPayloadGPR = resultPayload.gpr();
            
            m_jit.move(valuePayloadGPR, resultPayloadGPR);
            JITCompiler::Jump isBoolean = m_jit.branch32(
                JITCompiler::Equal, valueTagGPR, TrustedImm32(JSValue::BooleanTag));
            m_jit.move(valueTagGPR, resultTagGPR);
            JITCompiler::Jump done = m_jit.jump();
            isBoolean.link(&m_jit);
            m_jit.move(TrustedImm32(JSValue::Int32Tag), resultTagGPR);
            done.link(&m_jit);
            
            jsValueResult(resultTagGPR, resultPayloadGPR, node);
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }
        
    case ToPrimitive: {
        RELEASE_ASSERT(node->child1().useKind() == UntypedUse);
        JSValueOperand argument(this, node->child1());
        GPRTemporary resultTag(this, Reuse, argument, TagWord);
        GPRTemporary resultPayload(this, Reuse, argument, PayloadWord);
        
        GPRReg argumentTagGPR = argument.tagGPR();
        GPRReg argumentPayloadGPR = argument.payloadGPR();
        GPRReg resultTagGPR = resultTag.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        
        argument.use();
        
        MacroAssembler::Jump alreadyPrimitive = m_jit.branchIfNotCell(argument.jsValueRegs());
        MacroAssembler::Jump notPrimitive = m_jit.branchIfObject(argumentPayloadGPR);
        
        alreadyPrimitive.link(&m_jit);
        m_jit.move(argumentTagGPR, resultTagGPR);
        m_jit.move(argumentPayloadGPR, resultPayloadGPR);
        
        addSlowPathGenerator(
            slowPathCall(
                notPrimitive, this, operationToPrimitive,
                JSValueRegs(resultTagGPR, resultPayloadGPR), JSValueRegs(argumentTagGPR, argumentPayloadGPR)));
        
        jsValueResult(resultTagGPR, resultPayloadGPR, node, UseChildrenCalledExplicitly);
        break;
    }

    case ToNumber: {
        JSValueOperand argument(this, node->child1());
        GPRTemporary resultTag(this, Reuse, argument, TagWord);
        GPRTemporary resultPayload(this, Reuse, argument, PayloadWord);

        GPRReg argumentPayloadGPR = argument.payloadGPR();
        GPRReg argumentTagGPR = argument.tagGPR();
        JSValueRegs argumentRegs = argument.jsValueRegs();
        JSValueRegs resultRegs(resultTag.gpr(), resultPayload.gpr());

        argument.use();

        // We have several attempts to remove ToNumber. But ToNumber still exists.
        // It means that converting non-numbers to numbers by this ToNumber is not rare.
        // Instead of the slow path generator, we emit callOperation here.
        if (!(m_state.forNode(node->child1()).m_type & SpecBytecodeNumber)) {
            flushRegisters();
            callOperation(operationToNumber, resultRegs, argumentRegs);
            m_jit.exceptionCheck();
        } else {
            MacroAssembler::Jump notNumber;
            {
                GPRTemporary scratch(this);
                notNumber = m_jit.branchIfNotNumber(argument.jsValueRegs(), scratch.gpr());
            }
            m_jit.move(argumentTagGPR, resultRegs.tagGPR());
            m_jit.move(argumentPayloadGPR, resultRegs.payloadGPR());
            MacroAssembler::Jump done = m_jit.jump();

            notNumber.link(&m_jit);
            silentSpillAllRegisters(resultRegs);
            callOperation(operationToNumber, resultRegs, argumentRegs);
            silentFillAllRegisters(resultRegs);
            m_jit.exceptionCheck();

            done.link(&m_jit);
        }

        jsValueResult(resultRegs.tagGPR(), resultRegs.payloadGPR(), node, UseChildrenCalledExplicitly);
        break;
    }
        
    case ToString:
    case CallStringConstructor: {
        compileToStringOrCallStringConstructorOnCell(node);
        break;
    }
        
    case NewStringObject: {
        compileNewStringObject(node);
        break;
    }
        
    case NewArray: {
        JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
        if (!globalObject->isHavingABadTime() && !hasAnyArrayStorage(node->indexingType())) {
            RegisteredStructure structure = m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(node->indexingType()));
            ASSERT(structure->indexingType() == node->indexingType());
            ASSERT(
                hasUndecided(structure->indexingType())
                || hasInt32(structure->indexingType())
                || hasDouble(structure->indexingType())
                || hasContiguous(structure->indexingType()));

            unsigned numElements = node->numChildren();
            
            GPRTemporary result(this);
            GPRTemporary storage(this);
            
            GPRReg resultGPR = result.gpr();
            GPRReg storageGPR = storage.gpr();

            emitAllocateRawObject(resultGPR, structure, storageGPR, numElements, numElements);
            
            // At this point, one way or another, resultGPR and storageGPR have pointers to
            // the JSArray and the Butterfly, respectively.
            
            ASSERT(!hasUndecided(structure->indexingType()) || !node->numChildren());
            
            for (unsigned operandIdx = 0; operandIdx < node->numChildren(); ++operandIdx) {
                Edge use = m_jit.graph().m_varArgChildren[node->firstChild() + operandIdx];
                switch (node->indexingType()) {
                case ALL_BLANK_INDEXING_TYPES:
                case ALL_UNDECIDED_INDEXING_TYPES:
                    CRASH();
                    break;
                case ALL_DOUBLE_INDEXING_TYPES: {
                    SpeculateDoubleOperand operand(this, use);
                    FPRReg opFPR = operand.fpr();
                    DFG_TYPE_CHECK(
                        JSValueRegs(), use, SpecDoubleReal,
                        m_jit.branchDouble(MacroAssembler::DoubleNotEqualOrUnordered, opFPR, opFPR));
        
                    m_jit.storeDouble(opFPR, MacroAssembler::Address(storageGPR, sizeof(double) * operandIdx));
                    break;
                }
                case ALL_INT32_INDEXING_TYPES: {
                    SpeculateInt32Operand operand(this, use);
                    m_jit.store32(TrustedImm32(JSValue::Int32Tag), MacroAssembler::Address(storageGPR, sizeof(JSValue) * operandIdx + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
                    m_jit.store32(operand.gpr(), MacroAssembler::Address(storageGPR, sizeof(JSValue) * operandIdx + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
                    break;
                }
                case ALL_CONTIGUOUS_INDEXING_TYPES: {
                    JSValueOperand operand(this, m_jit.graph().m_varArgChildren[node->firstChild() + operandIdx]);
                    GPRReg opTagGPR = operand.tagGPR();
                    GPRReg opPayloadGPR = operand.payloadGPR();
                    m_jit.store32(opTagGPR, MacroAssembler::Address(storageGPR, sizeof(JSValue) * operandIdx + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
                    m_jit.store32(opPayloadGPR, MacroAssembler::Address(storageGPR, sizeof(JSValue) * operandIdx + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
                    break;
                }
                default:
                    CRASH();
                    break;
                }
            }
            
            // Yuck, we should *really* have a way of also returning the storageGPR. But
            // that's the least of what's wrong with this code. We really shouldn't be
            // allocating the array after having computed - and probably spilled to the
            // stack - all of the things that will go into the array. The solution to that
            // bigger problem will also likely fix the redundancy in reloading the storage
            // pointer that we currently have.
            
            cellResult(resultGPR, node);
            break;
        }
        
        if (!node->numChildren()) {
            flushRegisters();
            GPRFlushedCallResult result(this);
            callOperation(
                operationNewEmptyArray, result.gpr(), m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(node->indexingType())));
            m_jit.exceptionCheck();
            cellResult(result.gpr(), node);
            break;
        }
        
        size_t scratchSize = sizeof(EncodedJSValue) * node->numChildren();
        ScratchBuffer* scratchBuffer = m_jit.vm()->scratchBufferForSize(scratchSize);
        EncodedJSValue* buffer = scratchBuffer ? static_cast<EncodedJSValue*>(scratchBuffer->dataBuffer()) : 0;
        
        for (unsigned operandIdx = 0; operandIdx < node->numChildren(); ++operandIdx) {
            // Need to perform the speculations that this node promises to perform. If we're
            // emitting code here and the indexing type is not array storage then there is
            // probably something hilarious going on and we're already failing at all the
            // things, but at least we're going to be sound.
            Edge use = m_jit.graph().m_varArgChildren[node->firstChild() + operandIdx];
            switch (node->indexingType()) {
            case ALL_BLANK_INDEXING_TYPES:
            case ALL_UNDECIDED_INDEXING_TYPES:
                CRASH();
                break;
            case ALL_DOUBLE_INDEXING_TYPES: {
                SpeculateDoubleOperand operand(this, use);
                FPRReg opFPR = operand.fpr();
                DFG_TYPE_CHECK(
                    JSValueRegs(), use, SpecFullRealNumber,
                    m_jit.branchDouble(MacroAssembler::DoubleNotEqualOrUnordered, opFPR, opFPR));
                
                m_jit.storeDouble(opFPR, TrustedImmPtr(reinterpret_cast<char*>(buffer + operandIdx)));
                break;
            }
            case ALL_INT32_INDEXING_TYPES: {
                SpeculateInt32Operand operand(this, use);
                GPRReg opGPR = operand.gpr();
                m_jit.store32(TrustedImm32(JSValue::Int32Tag), reinterpret_cast<char*>(buffer + operandIdx) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag));
                m_jit.store32(opGPR, reinterpret_cast<char*>(buffer + operandIdx) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload));
                break;
            }
            case ALL_CONTIGUOUS_INDEXING_TYPES:
            case ALL_ARRAY_STORAGE_INDEXING_TYPES: {
                JSValueOperand operand(this, m_jit.graph().m_varArgChildren[node->firstChild() + operandIdx]);
                GPRReg opTagGPR = operand.tagGPR();
                GPRReg opPayloadGPR = operand.payloadGPR();
                
                m_jit.store32(opTagGPR, reinterpret_cast<char*>(buffer + operandIdx) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag));
                m_jit.store32(opPayloadGPR, reinterpret_cast<char*>(buffer + operandIdx) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload));
                operand.use();
                break;
            }
            default:
                CRASH();
                break;
            }
        }
        
        switch (node->indexingType()) {
        case ALL_DOUBLE_INDEXING_TYPES:
        case ALL_INT32_INDEXING_TYPES:
            useChildren(node);
            break;
        default:
            break;
        }
        
        flushRegisters();

        if (scratchSize) {
            GPRTemporary scratch(this);

            // Tell GC mark phase how much of the scratch buffer is active during call.
            m_jit.move(TrustedImmPtr(scratchBuffer->activeLengthPtr()), scratch.gpr());
            m_jit.storePtr(TrustedImmPtr(scratchSize), scratch.gpr());
        }

        GPRFlushedCallResult result(this);
        
        callOperation(
            operationNewArray, result.gpr(), m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(node->indexingType())),
            static_cast<void*>(buffer), node->numChildren());
        m_jit.exceptionCheck();

        if (scratchSize) {
            GPRTemporary scratch(this);

            m_jit.move(TrustedImmPtr(scratchBuffer->activeLengthPtr()), scratch.gpr());
            m_jit.storePtr(TrustedImmPtr(0), scratch.gpr());
        }

        cellResult(result.gpr(), node, UseChildrenCalledExplicitly);
        break;
    }

    case NewArrayWithSpread: {
        compileNewArrayWithSpread(node);
        break;
    }

    case Spread: {
        compileSpread(node);
        break;
    }

    case NewArrayWithSize: {
        JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
        if (!globalObject->isHavingABadTime() && !hasAnyArrayStorage(node->indexingType())) {
            SpeculateStrictInt32Operand size(this, node->child1());
            GPRTemporary result(this);

            GPRReg sizeGPR = size.gpr();
            GPRReg resultGPR = result.gpr();

            compileAllocateNewArrayWithSize(globalObject, resultGPR, sizeGPR, node->indexingType());
            cellResult(resultGPR, node);
            break;
        }
        
        SpeculateStrictInt32Operand size(this, node->child1());
        GPRReg sizeGPR = size.gpr();
        flushRegisters();
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();
        GPRReg structureGPR = selectScratchGPR(sizeGPR);
        MacroAssembler::Jump bigLength = m_jit.branch32(MacroAssembler::AboveOrEqual, sizeGPR, TrustedImm32(MIN_ARRAY_STORAGE_CONSTRUCTION_LENGTH));
        m_jit.move(TrustedImmPtr(m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(node->indexingType()))), structureGPR);
        MacroAssembler::Jump done = m_jit.jump();
        bigLength.link(&m_jit);
        m_jit.move(TrustedImmPtr(m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithArrayStorage))), structureGPR);
        done.link(&m_jit);
        callOperation(operationNewArrayWithSize, resultGPR, structureGPR, sizeGPR, nullptr);
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
        break;
    }
        
    case NewArrayBuffer: {
        JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
        IndexingType indexingType = node->indexingType();
        if (!globalObject->isHavingABadTime() && !hasAnyArrayStorage(indexingType)) {
            unsigned numElements = node->numConstants();
            
            GPRTemporary result(this);
            GPRTemporary storage(this);
            
            GPRReg resultGPR = result.gpr();
            GPRReg storageGPR = storage.gpr();

            emitAllocateRawObject(resultGPR, m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(indexingType)), storageGPR, numElements, numElements);
            
            if (node->indexingType() == ArrayWithDouble) {
                JSValue* data = m_jit.codeBlock()->constantBuffer(node->startConstant());
                for (unsigned index = 0; index < node->numConstants(); ++index) {
                    union {
                        int32_t halves[2];
                        double value;
                    } u;
                    u.value = data[index].asNumber();
                    m_jit.store32(Imm32(u.halves[0]), MacroAssembler::Address(storageGPR, sizeof(double) * index));
                    m_jit.store32(Imm32(u.halves[1]), MacroAssembler::Address(storageGPR, sizeof(double) * index + sizeof(int32_t)));
                }
            } else {
                int32_t* data = bitwise_cast<int32_t*>(m_jit.codeBlock()->constantBuffer(node->startConstant()));
                for (unsigned index = 0; index < node->numConstants() * 2; ++index) {
                    m_jit.store32(
                        Imm32(data[index]), MacroAssembler::Address(storageGPR, sizeof(int32_t) * index));
                }
            }
            
            cellResult(resultGPR, node);
            break;
        }
        
        flushRegisters();
        GPRFlushedCallResult result(this);
        
        callOperation(operationNewArrayBuffer, result.gpr(), m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(node->indexingType())), node->startConstant(), node->numConstants());
        m_jit.exceptionCheck();
        
        cellResult(result.gpr(), node);
        break;
    }
        
    case NewTypedArray: {
        switch (node->child1().useKind()) {
        case Int32Use:
            compileNewTypedArray(node);
            break;
        case UntypedUse: {
            JSValueOperand argument(this, node->child1());
            JSValueRegs argumentRegs = argument.jsValueRegs();
            
            flushRegisters();
            
            GPRFlushedCallResult result(this);
            GPRReg resultGPR = result.gpr();
            
            JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);
            callOperation(
                operationNewTypedArrayWithOneArgumentForType(node->typedArrayType()),
                resultGPR, m_jit.graph().registerStructure(globalObject->typedArrayStructureConcurrently(node->typedArrayType())), argumentRegs);
            m_jit.exceptionCheck();
            
            cellResult(resultGPR, node);
            break;
        }
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }
        
    case NewRegexp: {
        flushRegisters();
        GPRFlushedCallResult resultPayload(this);
        GPRFlushedCallResult2 resultTag(this);
        
        RegExp* regexp = node->castOperand<RegExp*>();
        callOperation(operationNewRegexp, JSValueRegs(resultTag.gpr(), resultPayload.gpr()), regexp);
        m_jit.exceptionCheck();
        
        // FIXME: make the callOperation above explicitly return a cell result, or jitAssert the tag is a cell tag.
        cellResult(resultPayload.gpr(), node);
        break;
    }

    case CallObjectConstructor: {
        compileCallObjectConstructor(node);
        break;
    }
        
    case ToThis: {
        ASSERT(node->child1().useKind() == UntypedUse);
        JSValueOperand thisValue(this, node->child1());
        GPRTemporary temp(this);
        GPRTemporary tempTag(this);
        GPRReg thisValuePayloadGPR = thisValue.payloadGPR();
        GPRReg thisValueTagGPR = thisValue.tagGPR();
        GPRReg tempGPR = temp.gpr();
        GPRReg tempTagGPR = tempTag.gpr();
        
        MacroAssembler::JumpList slowCases;
        slowCases.append(m_jit.branchIfNotCell(thisValue.jsValueRegs()));
        slowCases.append(
            m_jit.branchTest8(                             
                MacroAssembler::NonZero,
                MacroAssembler::Address(thisValuePayloadGPR, JSCell::typeInfoFlagsOffset()),
                MacroAssembler::TrustedImm32(OverridesToThis)));
        m_jit.move(thisValuePayloadGPR, tempGPR);
        m_jit.move(thisValueTagGPR, tempTagGPR);
        J_JITOperation_EJ function;
        if (m_jit.graph().executableFor(node->origin.semantic)->isStrictMode())
            function = operationToThisStrict;
        else
            function = operationToThis;
        addSlowPathGenerator(
            slowPathCall(
                slowCases, this, function,
                JSValueRegs(tempTagGPR, tempGPR), JSValueRegs(thisValueTagGPR, thisValuePayloadGPR)));

        jsValueResult(tempTagGPR, tempGPR, node);
        break;
    }

    case CreateThis: {
        // Note that there is not so much profit to speculate here. The only things we
        // speculate on are (1) that it's a cell, since that eliminates cell checks
        // later if the proto is reused, and (2) if we have a FinalObject prediction
        // then we speculate because we want to get recompiled if it isn't (since
        // otherwise we'd start taking slow path a lot).
        
        SpeculateCellOperand callee(this, node->child1());
        GPRTemporary result(this);
        GPRTemporary allocator(this);
        GPRTemporary structure(this);
        GPRTemporary scratch(this);
        
        GPRReg calleeGPR = callee.gpr();
        GPRReg resultGPR = result.gpr();
        GPRReg allocatorGPR = allocator.gpr();
        GPRReg structureGPR = structure.gpr();
        GPRReg scratchGPR = scratch.gpr();
        // Rare data is only used to access the allocator & structure
        // We can avoid using an additional GPR this way
        GPRReg rareDataGPR = structureGPR;
        GPRReg inlineCapacityGPR = rareDataGPR;
        
        MacroAssembler::JumpList slowPath;

        slowPath.append(m_jit.branch8(JITCompiler::NotEqual,
            JITCompiler::Address(calleeGPR, JSCell::typeInfoTypeOffset()), TrustedImm32(JSFunctionType)));
        m_jit.loadPtr(JITCompiler::Address(calleeGPR, JSFunction::offsetOfRareData()), rareDataGPR);
        slowPath.append(m_jit.branchTestPtr(MacroAssembler::Zero, rareDataGPR));
        m_jit.loadPtr(JITCompiler::Address(rareDataGPR, FunctionRareData::offsetOfObjectAllocationProfile() + ObjectAllocationProfile::offsetOfAllocator()), allocatorGPR);
        m_jit.loadPtr(JITCompiler::Address(rareDataGPR, FunctionRareData::offsetOfObjectAllocationProfile() + ObjectAllocationProfile::offsetOfStructure()), structureGPR);
        slowPath.append(m_jit.branchTestPtr(MacroAssembler::Zero, allocatorGPR));
        emitAllocateJSObject(resultGPR, nullptr, allocatorGPR, structureGPR, TrustedImmPtr(0), scratchGPR, slowPath);

        m_jit.loadPtr(JITCompiler::Address(calleeGPR, JSFunction::offsetOfRareData()), rareDataGPR);
        m_jit.load32(JITCompiler::Address(rareDataGPR, FunctionRareData::offsetOfObjectAllocationProfile() + ObjectAllocationProfile::offsetOfInlineCapacity()), inlineCapacityGPR);
        m_jit.emitInitializeInlineStorage(resultGPR, inlineCapacityGPR);

        addSlowPathGenerator(slowPathCall(slowPath, this, operationCreateThis, resultGPR, calleeGPR, node->inlineCapacity()));
        
        cellResult(resultGPR, node);
        break;
    }

    case NewObject: {
        GPRTemporary result(this);
        GPRTemporary allocator(this);
        GPRTemporary scratch(this);
        
        GPRReg resultGPR = result.gpr();
        GPRReg allocatorGPR = allocator.gpr();
        GPRReg scratchGPR = scratch.gpr();
        
        MacroAssembler::JumpList slowPath;
        
        RegisteredStructure structure = node->structure();
        size_t allocationSize = JSFinalObject::allocationSize(structure->inlineCapacity());
        MarkedAllocator* allocatorPtr = subspaceFor<JSFinalObject>(*m_jit.vm())->allocatorFor(allocationSize);

        m_jit.move(TrustedImmPtr(allocatorPtr), allocatorGPR);
        emitAllocateJSObject(resultGPR, allocatorPtr, allocatorGPR, TrustedImmPtr(structure), TrustedImmPtr(0), scratchGPR, slowPath);
        m_jit.emitInitializeInlineStorage(resultGPR, structure->inlineCapacity());

        addSlowPathGenerator(slowPathCall(slowPath, this, operationNewObject, resultGPR, structure));
        
        cellResult(resultGPR, node);
        break;
    }

    case GetCallee: {
        GPRTemporary result(this);
        m_jit.loadPtr(JITCompiler::payloadFor(CallFrameSlot::callee), result.gpr());
        cellResult(result.gpr(), node);
        break;
    }
        
    case GetArgumentCountIncludingThis: {
        GPRTemporary result(this);
        m_jit.load32(JITCompiler::payloadFor(CallFrameSlot::argumentCount), result.gpr());
        int32Result(result.gpr(), node);
        break;
    }
        
    case GetScope:
        compileGetScope(node);
        break;
            
    case SkipScope:
        compileSkipScope(node);
        break;
        
    case GetGlobalObject:
        compileGetGlobalObject(node);
        break;
        
    case GetClosureVar: {
        SpeculateCellOperand base(this, node->child1());
        GPRTemporary resultTag(this);
        GPRTemporary resultPayload(this);
        GPRReg baseGPR = base.gpr();
        GPRReg resultTagGPR = resultTag.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        m_jit.load32(JITCompiler::Address(baseGPR, JSEnvironmentRecord::offsetOfVariable(node->scopeOffset()) + TagOffset), resultTagGPR);
        m_jit.load32(JITCompiler::Address(baseGPR, JSEnvironmentRecord::offsetOfVariable(node->scopeOffset()) + PayloadOffset), resultPayloadGPR);
        jsValueResult(resultTagGPR, resultPayloadGPR, node);
        break;
    }
    
    case PutClosureVar: {
        SpeculateCellOperand base(this, node->child1());
        JSValueOperand value(this, node->child2());

        GPRReg baseGPR = base.gpr();
        GPRReg valueTagGPR = value.tagGPR();
        GPRReg valuePayloadGPR = value.payloadGPR();

        m_jit.store32(valueTagGPR, JITCompiler::Address(baseGPR, JSEnvironmentRecord::offsetOfVariable(node->scopeOffset()) + TagOffset));
        m_jit.store32(valuePayloadGPR, JITCompiler::Address(baseGPR, JSEnvironmentRecord::offsetOfVariable(node->scopeOffset()) + PayloadOffset));
        noResult(node);
        break;
    }

    case TryGetById: {
        compileTryGetById(node);
        break;
    }

    case GetById: {
        // FIXME https://bugs.webkit.org/show_bug.cgi?id=161158
        // dedup with SpeculativeJIT::compileTryGetById and 64-bit version of this.
        switch (node->child1().useKind()) {
        case CellUse: {
            SpeculateCellOperand base(this, node->child1());
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this, Reuse, base);
            
            GPRReg baseGPR = base.gpr();
            GPRReg resultTagGPR = resultTag.gpr();
            GPRReg resultPayloadGPR = resultPayload.gpr();

            base.use();
            
            cachedGetById(node->origin.semantic, InvalidGPRReg, baseGPR, resultTagGPR, resultPayloadGPR, node->identifierNumber());
            
            jsValueResult(resultTagGPR, resultPayloadGPR, node, UseChildrenCalledExplicitly);
            break;
        }
        
        case UntypedUse: {
            JSValueOperand base(this, node->child1());
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this, Reuse, base, TagWord);
        
            GPRReg baseTagGPR = base.tagGPR();
            GPRReg basePayloadGPR = base.payloadGPR();
            GPRReg resultTagGPR = resultTag.gpr();
            GPRReg resultPayloadGPR = resultPayload.gpr();
        
            base.use();
        
            JITCompiler::Jump notCell = m_jit.branchIfNotCell(base.jsValueRegs());
        
            cachedGetById(node->origin.semantic, baseTagGPR, basePayloadGPR, resultTagGPR, resultPayloadGPR, node->identifierNumber(), notCell);
        
            jsValueResult(resultTagGPR, resultPayloadGPR, node, UseChildrenCalledExplicitly);
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }

    case GetByIdWithThis: {
        if (node->child1().useKind() == CellUse && node->child2().useKind() == CellUse) {
            SpeculateCellOperand base(this, node->child1());
            SpeculateCellOperand thisValue(this, node->child2());
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            
            GPRReg baseGPR = base.gpr();
            GPRReg thisGPR = thisValue.gpr();
            GPRReg resultTagGPR = resultTag.gpr();
            GPRReg resultPayloadGPR = resultPayload.gpr();
            
            cachedGetByIdWithThis(node->origin.semantic, InvalidGPRReg, baseGPR, InvalidGPRReg, thisGPR, resultTagGPR, resultPayloadGPR, node->identifierNumber());
            
            jsValueResult(resultTagGPR, resultPayloadGPR, node);
        } else {
            JSValueOperand base(this, node->child1());
            JSValueOperand thisValue(this, node->child2());
            GPRTemporary resultTag(this);
            GPRTemporary resultPayload(this);
            
            GPRReg baseTagGPR = base.tagGPR();
            GPRReg basePayloadGPR = base.payloadGPR();
            GPRReg thisTagGPR = thisValue.tagGPR();
            GPRReg thisPayloadGPR = thisValue.payloadGPR();
            GPRReg resultTagGPR = resultTag.gpr();
            GPRReg resultPayloadGPR = resultPayload.gpr();
            
            JITCompiler::JumpList notCellList;
            notCellList.append(m_jit.branchIfNotCell(base.jsValueRegs()));
            notCellList.append(m_jit.branchIfNotCell(thisValue.jsValueRegs()));
            
            cachedGetByIdWithThis(node->origin.semantic, baseTagGPR, basePayloadGPR, thisTagGPR, thisPayloadGPR, resultTagGPR, resultPayloadGPR, node->identifierNumber(), notCellList);
            
            jsValueResult(resultTagGPR, resultPayloadGPR, node);
        }
        
        break;
    }

    case GetByIdFlush: {
        if (!node->prediction()) {
            terminateSpeculativeExecution(InadequateCoverage, JSValueRegs(), 0);
            break;
        }
        
        switch (node->child1().useKind()) {
        case CellUse: {
            SpeculateCellOperand base(this, node->child1());
            
            GPRReg baseGPR = base.gpr();

            GPRFlushedCallResult resultPayload(this);
            GPRFlushedCallResult2 resultTag(this);
            GPRReg resultPayloadGPR = resultPayload.gpr();
            GPRReg resultTagGPR = resultTag.gpr();

            base.use();
            
            flushRegisters();
            
            cachedGetById(node->origin.semantic, InvalidGPRReg, baseGPR, resultTagGPR, resultPayloadGPR, node->identifierNumber(), JITCompiler::Jump(), DontSpill);
            
            jsValueResult(resultTagGPR, resultPayloadGPR, node, UseChildrenCalledExplicitly);
            break;
        }
        
        case UntypedUse: {
            JSValueOperand base(this, node->child1());
            GPRReg baseTagGPR = base.tagGPR();
            GPRReg basePayloadGPR = base.payloadGPR();

            GPRFlushedCallResult resultPayload(this);
            GPRFlushedCallResult2 resultTag(this);
            GPRReg resultPayloadGPR = resultPayload.gpr();
            GPRReg resultTagGPR = resultTag.gpr();

            base.use();
        
            flushRegisters();
        
            JITCompiler::Jump notCell = m_jit.branchIfNotCell(base.jsValueRegs());
        
            cachedGetById(node->origin.semantic, baseTagGPR, basePayloadGPR, resultTagGPR, resultPayloadGPR, node->identifierNumber(), notCell, DontSpill);
        
            jsValueResult(resultTagGPR, resultPayloadGPR, node, UseChildrenCalledExplicitly);
            break;
        }
            
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }

    case GetArrayLength:
        compileGetArrayLength(node);
        break;

    case DeleteById: {
        compileDeleteById(node);
        break;
    }

    case DeleteByVal: {
        compileDeleteByVal(node);
        break;
    }
        
    case CheckCell: {
        SpeculateCellOperand cell(this, node->child1());
        speculationCheck(BadCell, JSValueSource::unboxedCell(cell.gpr()), node->child1(), m_jit.branchWeakPtr(JITCompiler::NotEqual, cell.gpr(), node->cellOperand()->cell()));
        noResult(node);
        break;
    }

    case CheckNotEmpty: {
        JSValueOperand operand(this, node->child1());
        GPRReg tagGPR = operand.tagGPR();
        speculationCheck(TDZFailure, JSValueSource(), nullptr, m_jit.branch32(JITCompiler::Equal, tagGPR, TrustedImm32(JSValue::EmptyValueTag)));
        noResult(node);
        break;
    }

    case CheckStringIdent:
        compileCheckStringIdent(node);
        break;

    case GetExecutable: {
        SpeculateCellOperand function(this, node->child1());
        GPRTemporary result(this, Reuse, function);
        GPRReg functionGPR = function.gpr();
        GPRReg resultGPR = result.gpr();
        speculateCellType(node->child1(), functionGPR, SpecFunction, JSFunctionType);
        m_jit.loadPtr(JITCompiler::Address(functionGPR, JSFunction::offsetOfExecutable()), resultGPR);
        cellResult(resultGPR, node);
        break;
    }
        
    case CheckStructure: {
        compileCheckStructure(node);
        break;
    }
        
    case PutStructure: {
        RegisteredStructure oldStructure = node->transition()->previous;
        RegisteredStructure newStructure = node->transition()->next;

        m_jit.jitCode()->common.notifyCompilingStructureTransition(m_jit.graph().m_plan, m_jit.codeBlock(), node);

        SpeculateCellOperand base(this, node->child1());
        GPRReg baseGPR = base.gpr();
        
        ASSERT_UNUSED(oldStructure, oldStructure->indexingType() == newStructure->indexingType());
        ASSERT(oldStructure->typeInfo().type() == newStructure->typeInfo().type());
        ASSERT(oldStructure->typeInfo().inlineTypeFlags() == newStructure->typeInfo().inlineTypeFlags());
        m_jit.storePtr(TrustedImmPtr(newStructure), MacroAssembler::Address(baseGPR, JSCell::structureIDOffset()));
        
        noResult(node);
        break;
    }
        
    case AllocatePropertyStorage:
        compileAllocatePropertyStorage(node);
        break;
        
    case ReallocatePropertyStorage:
        compileReallocatePropertyStorage(node);
        break;
        
    case NukeStructureAndSetButterfly:
        compileNukeStructureAndSetButterfly(node);
        break;
        
    case GetButterfly:
        compileGetButterfly(node);
        break;

    case GetIndexedPropertyStorage: {
        compileGetIndexedPropertyStorage(node);
        break;
    }

    case ConstantStoragePointer: {
        compileConstantStoragePointer(node);
        break;
    }
        
    case GetTypedArrayByteOffset: {
        compileGetTypedArrayByteOffset(node);
        break;
    }
        
    case GetByOffset: {
        StorageOperand storage(this, node->child1());
        GPRTemporary resultTag(this, Reuse, storage);
        GPRTemporary resultPayload(this);
        
        GPRReg storageGPR = storage.gpr();
        GPRReg resultTagGPR = resultTag.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        
        StorageAccessData& storageAccessData = node->storageAccessData();
        
        m_jit.load32(JITCompiler::Address(storageGPR, offsetRelativeToBase(storageAccessData.offset) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload)), resultPayloadGPR);
        m_jit.load32(JITCompiler::Address(storageGPR, offsetRelativeToBase(storageAccessData.offset) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag)), resultTagGPR);
        
        jsValueResult(resultTagGPR, resultPayloadGPR, node);
        break;
    }
        
    case GetGetterSetterByOffset: {
        StorageOperand storage(this, node->child1());
        GPRTemporary resultPayload(this);
        
        GPRReg storageGPR = storage.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        
        StorageAccessData& storageAccessData = node->storageAccessData();
        
        m_jit.load32(JITCompiler::Address(storageGPR, offsetRelativeToBase(storageAccessData.offset) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload)), resultPayloadGPR);
        
        cellResult(resultPayloadGPR, node);
        break;
    }
        
    case GetGetter: {
        SpeculateCellOperand op1(this, node->child1());
        GPRTemporary result(this, Reuse, op1);
        
        GPRReg op1GPR = op1.gpr();
        GPRReg resultGPR = result.gpr();
        
        m_jit.loadPtr(JITCompiler::Address(op1GPR, GetterSetter::offsetOfGetter()), resultGPR);
        
        cellResult(resultGPR, node);
        break;
    }
        
    case GetSetter: {
        SpeculateCellOperand op1(this, node->child1());
        GPRTemporary result(this, Reuse, op1);
        
        GPRReg op1GPR = op1.gpr();
        GPRReg resultGPR = result.gpr();
        
        m_jit.loadPtr(JITCompiler::Address(op1GPR, GetterSetter::offsetOfSetter()), resultGPR);
        
        cellResult(resultGPR, node);
        break;
    }
        
    case PutByOffset: {
        StorageOperand storage(this, node->child1());
        JSValueOperand value(this, node->child3());

        GPRReg storageGPR = storage.gpr();
        GPRReg valueTagGPR = value.tagGPR();
        GPRReg valuePayloadGPR = value.payloadGPR();

        speculate(node, node->child2());

        StorageAccessData& storageAccessData = node->storageAccessData();
        
        m_jit.storePtr(valueTagGPR, JITCompiler::Address(storageGPR, offsetRelativeToBase(storageAccessData.offset) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag)));
        m_jit.storePtr(valuePayloadGPR, JITCompiler::Address(storageGPR, offsetRelativeToBase(storageAccessData.offset) + OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload)));
        
        noResult(node);
        break;
    }

    case PutByIdFlush: {
        SpeculateCellOperand base(this, node->child1());
        JSValueOperand value(this, node->child2());
        GPRTemporary scratch(this);

        GPRReg baseGPR = base.gpr();
        GPRReg valueTagGPR = value.tagGPR();
        GPRReg valuePayloadGPR = value.payloadGPR();
        GPRReg scratchGPR = scratch.gpr();
        flushRegisters();

        cachedPutById(node->origin.semantic, baseGPR, valueTagGPR, valuePayloadGPR, scratchGPR, node->identifierNumber(), NotDirect, MacroAssembler::Jump(), DontSpill);

        noResult(node);
        break;
    }
        
    case PutById: {
        SpeculateCellOperand base(this, node->child1());
        JSValueOperand value(this, node->child2());
        GPRTemporary scratch(this);
        
        GPRReg baseGPR = base.gpr();
        GPRReg valueTagGPR = value.tagGPR();
        GPRReg valuePayloadGPR = value.payloadGPR();
        GPRReg scratchGPR = scratch.gpr();
        
        cachedPutById(node->origin.semantic, baseGPR, valueTagGPR, valuePayloadGPR, scratchGPR, node->identifierNumber(), NotDirect);
        
        noResult(node);
        break;
    }

    case PutByIdDirect: {
        SpeculateCellOperand base(this, node->child1());
        JSValueOperand value(this, node->child2());
        GPRTemporary scratch(this);
        
        GPRReg baseGPR = base.gpr();
        GPRReg valueTagGPR = value.tagGPR();
        GPRReg valuePayloadGPR = value.payloadGPR();
        GPRReg scratchGPR = scratch.gpr();
        
        cachedPutById(node->origin.semantic, baseGPR, valueTagGPR, valuePayloadGPR, scratchGPR, node->identifierNumber(), Direct);

        noResult(node);
        break;
    }

    case PutByIdWithThis: {
        JSValueOperand base(this, node->child1());
        JSValueRegs baseRegs = base.jsValueRegs();
        JSValueOperand thisValue(this, node->child2());
        JSValueRegs thisRegs = thisValue.jsValueRegs();
        JSValueOperand value(this, node->child3());
        JSValueRegs valueRegs = value.jsValueRegs();

        flushRegisters();
        callOperation(m_jit.isStrictModeFor(node->origin.semantic) ? operationPutByIdWithThisStrict : operationPutByIdWithThis,
            NoResult, baseRegs, thisRegs, valueRegs, identifierUID(node->identifierNumber()));
        m_jit.exceptionCheck();

        noResult(node);
        break;
    }

    case PutGetterById:
    case PutSetterById: {
        compilePutAccessorById(node);
        break;
    }

    case PutGetterSetterById: {
        compilePutGetterSetterById(node);
        break;
    }

    case PutGetterByVal:
    case PutSetterByVal: {
        compilePutAccessorByVal(node);
        break;
    }

    case DefineDataProperty: {
        compileDefineDataProperty(node);
        break;
    }

    case DefineAccessorProperty: {
        compileDefineAccessorProperty(node);
        break;
    }

    case GetGlobalLexicalVariable:
    case GetGlobalVar: {
        GPRTemporary resultPayload(this);
        GPRTemporary resultTag(this);

        m_jit.move(TrustedImmPtr(node->variablePointer()), resultPayload.gpr());
        m_jit.load32(JITCompiler::Address(resultPayload.gpr(), OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag)), resultTag.gpr());
        m_jit.load32(JITCompiler::Address(resultPayload.gpr(), OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload)), resultPayload.gpr());

        jsValueResult(resultTag.gpr(), resultPayload.gpr(), node);
        break;
    }

    case PutGlobalVariable: {
        JSValueOperand value(this, node->child2());

        // FIXME: if we happen to have a spare register - and _ONLY_ if we happen to have
        // a spare register - a good optimization would be to put the register pointer into
        // a register and then do a zero offset store followed by a four-offset store (or
        // vice-versa depending on endianness).
        m_jit.store32(value.tagGPR(), node->variablePointer()->tagPointer());
        m_jit.store32(value.payloadGPR(), node->variablePointer()->payloadPointer());

        noResult(node);
        break;
    }

    case NotifyWrite: {
        compileNotifyWrite(node);
        break;
    }

    case ParseInt: {
        compileParseInt(node);
        break;
    }

    case CheckTypeInfoFlags: {
        compileCheckTypeInfoFlags(node);
        break;
    }

    case OverridesHasInstance: {

        Node* hasInstanceValueNode = node->child2().node();
        JSFunction* defaultHasInstanceFunction = jsCast<JSFunction*>(node->cellOperand()->value());

        MacroAssembler::JumpList notDefaultHasInstanceValue;
        SpeculateCellOperand base(this, node->child1());
        JSValueOperand hasInstanceValue(this, node->child2());
        GPRTemporary result(this);

        GPRReg baseGPR = base.gpr();
        GPRReg resultGPR = result.gpr();

        // It would be great if constant folding handled automatically the case where we knew the hasInstance function
        // was a constant. Unfortunately, the folding rule for OverridesHasInstance is in the strength reduction phase
        // since it relies on OSR information. https://bugs.webkit.org/show_bug.cgi?id=154832
        if (!hasInstanceValueNode->isCellConstant() || defaultHasInstanceFunction != hasInstanceValueNode->asCell()) {
            JSValueRegs hasInstanceValueRegs = hasInstanceValue.jsValueRegs();
            notDefaultHasInstanceValue.append(m_jit.branchIfNotCell(hasInstanceValueRegs));
            notDefaultHasInstanceValue.append(m_jit.branchPtr(MacroAssembler::NotEqual, hasInstanceValueRegs.payloadGPR(), TrustedImmPtr(node->cellOperand())));
        }

        // Check that constructor 'ImplementsDefaultHasInstance'.
        m_jit.test8(MacroAssembler::Zero, MacroAssembler::Address(baseGPR, JSCell::typeInfoFlagsOffset()), MacroAssembler::TrustedImm32(ImplementsDefaultHasInstance), resultGPR);
        MacroAssembler::Jump done = m_jit.jump();

        if (!notDefaultHasInstanceValue.empty()) {
            notDefaultHasInstanceValue.link(&m_jit);
            moveTrueTo(resultGPR);
        }

        done.link(&m_jit);
        booleanResult(resultGPR, node);
        break;
    }

    case InstanceOf: {
        compileInstanceOf(node);
        break;
    }

    case InstanceOfCustom: {
        compileInstanceOfCustom(node);
        break;
    }

    case IsEmpty: {        
        JSValueOperand value(this, node->child1());
        GPRTemporary result(this, Reuse, value, TagWord);
        m_jit.comparePtr(JITCompiler::Equal, value.tagGPR(), TrustedImm32(JSValue::EmptyValueTag), result.gpr());
        booleanResult(result.gpr(), node);
        break;
    }

    case IsUndefined: {
        JSValueOperand value(this, node->child1());
        GPRTemporary result(this);
        GPRTemporary localGlobalObject(this);
        GPRTemporary remoteGlobalObject(this);

        JITCompiler::Jump isCell = m_jit.branchIfCell(value.jsValueRegs());
        
        m_jit.compare32(JITCompiler::Equal, value.tagGPR(), TrustedImm32(JSValue::UndefinedTag), result.gpr());
        JITCompiler::Jump done = m_jit.jump();
        
        isCell.link(&m_jit);
        JITCompiler::Jump notMasqueradesAsUndefined;
        if (masqueradesAsUndefinedWatchpointIsStillValid()) {
            m_jit.move(TrustedImm32(0), result.gpr());
            notMasqueradesAsUndefined = m_jit.jump();
        } else {
            JITCompiler::Jump isMasqueradesAsUndefined = m_jit.branchTest8(
                JITCompiler::NonZero, 
                JITCompiler::Address(value.payloadGPR(), JSCell::typeInfoFlagsOffset()), 
                TrustedImm32(MasqueradesAsUndefined));
            m_jit.move(TrustedImm32(0), result.gpr());
            notMasqueradesAsUndefined = m_jit.jump();
            
            isMasqueradesAsUndefined.link(&m_jit);
            GPRReg localGlobalObjectGPR = localGlobalObject.gpr();
            GPRReg remoteGlobalObjectGPR = remoteGlobalObject.gpr();
            m_jit.move(TrustedImmPtr::weakPointer(m_jit.graph(), m_jit.globalObjectFor(node->origin.semantic)), localGlobalObjectGPR);
            m_jit.loadPtr(JITCompiler::Address(value.payloadGPR(), JSCell::structureIDOffset()), result.gpr());
            m_jit.loadPtr(JITCompiler::Address(result.gpr(), Structure::globalObjectOffset()), remoteGlobalObjectGPR); 
            m_jit.compare32(JITCompiler::Equal, localGlobalObjectGPR, remoteGlobalObjectGPR, result.gpr());
        }

        notMasqueradesAsUndefined.link(&m_jit);
        done.link(&m_jit);
        booleanResult(result.gpr(), node);
        break;
    }

    case IsBoolean: {
        JSValueOperand value(this, node->child1());
        GPRTemporary result(this, Reuse, value, TagWord);
        
        m_jit.compare32(JITCompiler::Equal, value.tagGPR(), JITCompiler::TrustedImm32(JSValue::BooleanTag), result.gpr());
        booleanResult(result.gpr(), node);
        break;
    }

    case IsNumber: {
        JSValueOperand value(this, node->child1());
        GPRTemporary result(this, Reuse, value, TagWord);
        
        m_jit.add32(TrustedImm32(1), value.tagGPR(), result.gpr());
        m_jit.compare32(JITCompiler::Below, result.gpr(), JITCompiler::TrustedImm32(JSValue::LowestTag + 1), result.gpr());
        booleanResult(result.gpr(), node);
        break;
    }

    case IsObject: {
        JSValueOperand value(this, node->child1());
        GPRTemporary result(this, Reuse, value, TagWord);

        JITCompiler::Jump isNotCell = m_jit.branchIfNotCell(value.jsValueRegs());

        m_jit.compare8(JITCompiler::AboveOrEqual,
            JITCompiler::Address(value.payloadGPR(), JSCell::typeInfoTypeOffset()),
            TrustedImm32(ObjectType),
            result.gpr());
        JITCompiler::Jump done = m_jit.jump();

        isNotCell.link(&m_jit);
        m_jit.move(TrustedImm32(0), result.gpr());

        done.link(&m_jit);
        booleanResult(result.gpr(), node);
        break;
    }

    case IsObjectOrNull: {
        compileIsObjectOrNull(node);
        break;
    }

    case IsFunction: {
        compileIsFunction(node);
        break;
    }

    case IsCellWithType: {
        compileIsCellWithType(node);
        break;
    }

    case IsTypedArrayView: {
        compileIsTypedArrayView(node);
        break;
    }

    case TypeOf: {
        compileTypeOf(node);
        break;
    }

    case MapHash: {
        JSValueOperand input(this, node->child1());

        JSValueRegs inputRegs = input.jsValueRegs();

        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();

        flushRegisters();
        callOperation(operationMapHash, resultGPR, inputRegs);
        m_jit.exceptionCheck();
        int32Result(resultGPR, node);
        break;
    }

    case GetMapBucket: {
        SpeculateCellOperand map(this, node->child1());
        JSValueOperand key(this, node->child2());
        SpeculateInt32Operand hash(this, node->child3());
        GPRFlushedCallResult result(this);

        GPRReg mapGPR = map.gpr();
        JSValueRegs keyRegs = key.jsValueRegs();
        GPRReg hashGPR = hash.gpr();
        GPRReg resultGPR = result.gpr();

        if (node->child1().useKind() == MapObjectUse)
            speculateMapObject(node->child1(), mapGPR);
        else if (node->child1().useKind() == SetObjectUse)
            speculateSetObject(node->child1(), mapGPR);
        else
            RELEASE_ASSERT_NOT_REACHED();

        flushRegisters();
        if (node->child1().useKind() == MapObjectUse)
            callOperation(operationJSMapFindBucket, resultGPR, mapGPR, keyRegs, hashGPR);
        else
            callOperation(operationJSSetFindBucket, resultGPR, mapGPR, keyRegs, hashGPR);
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
        break;
    }

    case LoadFromJSMapBucket: {
        SpeculateCellOperand bucket(this, node->child1());
        GPRTemporary resultPayload(this);
        GPRTemporary resultTag(this);

        GPRReg bucketGPR = bucket.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg resultTagGPR = resultTag.gpr();

        auto notBucket = m_jit.branchTestPtr(MacroAssembler::Zero, bucketGPR);
        m_jit.loadValue(MacroAssembler::Address(bucketGPR, HashMapBucket<HashMapBucketDataKeyValue>::offsetOfValue()), JSValueRegs(resultTagGPR, resultPayloadGPR));
        auto done = m_jit.jump();

        notBucket.link(&m_jit);
        m_jit.move(TrustedImm32(JSValue::UndefinedTag), resultTagGPR);
        m_jit.move(TrustedImm32(0), resultPayloadGPR);
        done.link(&m_jit);
        jsValueResult(resultTagGPR, resultPayloadGPR, node);
        break;
    }

    case IsNonEmptyMapBucket: {
        SpeculateCellOperand bucket(this, node->child1());
        GPRTemporary result(this);

        GPRReg bucketGPR = bucket.gpr();
        GPRReg resultGPR = result.gpr();

        m_jit.comparePtr(MacroAssembler::NotEqual, bucketGPR, TrustedImm32(0), resultGPR);
        booleanResult(resultGPR, node);
        break;
    }

    case Flush:
        break;

    case Call:
    case TailCall:
    case TailCallInlinedCaller:
    case Construct:
    case CallVarargs:
    case TailCallVarargs:
    case TailCallVarargsInlinedCaller:
    case ConstructVarargs:
    case CallForwardVarargs:
    case TailCallForwardVarargs:
    case TailCallForwardVarargsInlinedCaller:
    case ConstructForwardVarargs:
    case CallEval:
    case DirectCall:
    case DirectConstruct:
    case DirectTailCall:
    case DirectTailCallInlinedCaller:
        emitCall(node);
        break;

    case LoadVarargs: {
        LoadVarargsData* data = node->loadVarargsData();
        
        GPRReg argumentsTagGPR;
        GPRReg argumentsPayloadGPR;
        JSValueRegs argumentsRegs;
        {
            JSValueOperand arguments(this, node->child1());
            argumentsRegs = arguments.jsValueRegs();
            flushRegisters();
        }
        
        callOperation(operationSizeOfVarargs, GPRInfo::returnValueGPR, argumentsRegs, data->offset);
        m_jit.exceptionCheck();
        
        lock(GPRInfo::returnValueGPR);
        {
            JSValueOperand arguments(this, node->child1());
            argumentsTagGPR = arguments.tagGPR();
            argumentsPayloadGPR = arguments.payloadGPR();
            argumentsRegs = arguments.jsValueRegs();
            flushRegisters();
        }
        unlock(GPRInfo::returnValueGPR);
        
        // FIXME: There is a chance that we will call an effectful length property twice. This is safe
        // from the standpoint of the VM's integrity, but it's subtly wrong from a spec compliance
        // standpoint. The best solution would be one where we can exit *into* the op_call_varargs right
        // past the sizing.
        // https://bugs.webkit.org/show_bug.cgi?id=141448

        GPRReg argCountIncludingThisGPR =
            JITCompiler::selectScratchGPR(GPRInfo::returnValueGPR, argumentsTagGPR, argumentsPayloadGPR);
        
        m_jit.add32(TrustedImm32(1), GPRInfo::returnValueGPR, argCountIncludingThisGPR);
        speculationCheck(
            VarargsOverflow, JSValueSource(), Edge(), m_jit.branch32(
                MacroAssembler::Above,
                argCountIncludingThisGPR,
                TrustedImm32(data->limit)));
        
        m_jit.store32(argCountIncludingThisGPR, JITCompiler::payloadFor(data->machineCount));
        
        callOperation(operationLoadVarargs, data->machineStart.offset(), argumentsRegs, data->offset, GPRInfo::returnValueGPR, data->mandatoryMinimum);
        m_jit.exceptionCheck();
        
        noResult(node);
        break;
    }
        
    case ForwardVarargs: {
        compileForwardVarargs(node);
        break;
    }
        
    case CreateActivation: {
        compileCreateActivation(node);
        break;
    }
        
    case CreateDirectArguments: {
        compileCreateDirectArguments(node);
        break;
    }
        
    case GetFromArguments: {
        compileGetFromArguments(node);
        break;
    }
        
    case PutToArguments: {
        compilePutToArguments(node);
        break;
    }

    case GetArgument: {
        compileGetArgument(node);
        break;
    }
        
    case CreateScopedArguments: {
        compileCreateScopedArguments(node);
        break;
    }
        
    case CreateClonedArguments: {
        compileCreateClonedArguments(node);
        break;
    }

    case CreateRest: {
        compileCreateRest(node);
        break;
    }

    case GetRestLength: {
        compileGetRestLength(node);
        break;
    }

    case NewFunction:
    case NewGeneratorFunction:
    case NewAsyncFunction:
        compileNewFunction(node);
        break;

    case SetFunctionName:
        compileSetFunctionName(node);
        break;

    case In:
        compileIn(node);
        break;

    case HasOwnProperty: {
#if CPU(X86)
        ASSERT(node->child2().useKind() == UntypedUse);
        SpeculateCellOperand object(this, node->child1());
        JSValueOperand key(this, node->child2());
        GPRTemporary result(this, Reuse, object);

        JSValueRegs keyRegs = key.jsValueRegs();
        GPRReg objectGPR = object.gpr();
        GPRReg resultGPR = result.gpr();

        speculateObject(node->child1());

        flushRegisters();
        callOperation(operationHasOwnProperty, resultGPR, objectGPR, keyRegs);
        booleanResult(resultGPR, node);
#else
        SpeculateCellOperand object(this, node->child1());
        GPRTemporary uniquedStringImpl(this);
        GPRTemporary temp(this);
        GPRTemporary hash(this);
        GPRTemporary structureID(this);
        GPRTemporary result(this);

        std::optional<SpeculateCellOperand> keyAsCell;
        std::optional<JSValueOperand> keyAsValue;
        JSValueRegs keyRegs;
        if (node->child2().useKind() == UntypedUse) {
            keyAsValue.emplace(this, node->child2());
            keyRegs = keyAsValue->jsValueRegs();
        } else {
            ASSERT(node->child2().useKind() == StringUse || node->child2().useKind() == SymbolUse);
            keyAsCell.emplace(this, node->child2());
            keyRegs = JSValueRegs::payloadOnly(keyAsCell->gpr());
        }

        GPRReg objectGPR = object.gpr();
        GPRReg implGPR = uniquedStringImpl.gpr();
        GPRReg tempGPR = temp.gpr();
        GPRReg hashGPR = hash.gpr();
        GPRReg structureIDGPR = structureID.gpr();
        GPRReg resultGPR = result.gpr();

        speculateObject(node->child1());

        MacroAssembler::JumpList slowPath;
        switch (node->child2().useKind()) {
        case SymbolUse: {
            speculateSymbol(node->child2(), keyRegs.payloadGPR());
            m_jit.loadPtr(MacroAssembler::Address(keyRegs.payloadGPR(), Symbol::offsetOfSymbolImpl()), implGPR);
            break;
        }
        case StringUse: {
            speculateString(node->child2(), keyRegs.payloadGPR());
            m_jit.loadPtr(MacroAssembler::Address(keyRegs.payloadGPR(), JSString::offsetOfValue()), implGPR);
            slowPath.append(m_jit.branchTestPtr(MacroAssembler::Zero, implGPR));
            slowPath.append(m_jit.branchTest32(
                MacroAssembler::Zero, MacroAssembler::Address(implGPR, StringImpl::flagsOffset()),
                MacroAssembler::TrustedImm32(StringImpl::flagIsAtomic())));
            break;
        }
        case UntypedUse: {
            slowPath.append(m_jit.branchIfNotCell(keyRegs));
            auto isNotString = m_jit.branchIfNotString(keyRegs.payloadGPR());
            m_jit.loadPtr(MacroAssembler::Address(keyRegs.payloadGPR(), JSString::offsetOfValue()), implGPR);
            slowPath.append(m_jit.branchTestPtr(MacroAssembler::Zero, implGPR));
            slowPath.append(m_jit.branchTest32(
                MacroAssembler::Zero, MacroAssembler::Address(implGPR, StringImpl::flagsOffset()),
                MacroAssembler::TrustedImm32(StringImpl::flagIsAtomic())));
            auto hasUniquedImpl = m_jit.jump();

            isNotString.link(&m_jit);
            slowPath.append(m_jit.branchIfNotSymbol(keyRegs.payloadGPR()));
            m_jit.loadPtr(MacroAssembler::Address(keyRegs.payloadGPR(), Symbol::offsetOfSymbolImpl()), implGPR);

            hasUniquedImpl.link(&m_jit);
            break;
        }
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }

        // Note that we don't test if the hash is zero here. AtomicStringImpl's can't have a zero
        // hash, however, a SymbolImpl may. But, because this is a cache, we don't care. We only
        // ever load the result from the cache if the cache entry matches what we are querying for.
        // So we either get super lucky and use zero for the hash and somehow collide with the entity
        // we're looking for, or we realize we're comparing against another entity, and go to the
        // slow path anyways.
        m_jit.load32(MacroAssembler::Address(implGPR, UniquedStringImpl::flagsOffset()), hashGPR);
        m_jit.urshift32(MacroAssembler::TrustedImm32(StringImpl::s_flagCount), hashGPR);
        m_jit.load32(MacroAssembler::Address(objectGPR, JSCell::structureIDOffset()), structureIDGPR);
        m_jit.add32(structureIDGPR, hashGPR);
        m_jit.and32(TrustedImm32(HasOwnPropertyCache::mask), hashGPR);
        m_jit.mul32(TrustedImm32(sizeof(HasOwnPropertyCache::Entry)), hashGPR, hashGPR);
        ASSERT(m_jit.vm()->hasOwnPropertyCache());
        m_jit.move(TrustedImmPtr(m_jit.vm()->hasOwnPropertyCache()), tempGPR);
        slowPath.append(m_jit.branchPtr(MacroAssembler::NotEqual, 
            MacroAssembler::BaseIndex(tempGPR, hashGPR, MacroAssembler::TimesOne, HasOwnPropertyCache::Entry::offsetOfImpl()), implGPR));
        m_jit.load8(MacroAssembler::BaseIndex(tempGPR, hashGPR, MacroAssembler::TimesOne, HasOwnPropertyCache::Entry::offsetOfResult()), resultGPR);
        m_jit.load32(MacroAssembler::BaseIndex(tempGPR, hashGPR, MacroAssembler::TimesOne, HasOwnPropertyCache::Entry::offsetOfStructureID()), tempGPR);
        slowPath.append(m_jit.branch32(MacroAssembler::NotEqual, tempGPR, structureIDGPR));
        auto done = m_jit.jump();

        slowPath.link(&m_jit);
        silentSpillAllRegisters(resultGPR);
        if (node->child2().useKind() != UntypedUse) {
            m_jit.move(MacroAssembler::TrustedImm32(JSValue::CellTag), tempGPR);
            keyRegs = JSValueRegs(tempGPR, keyRegs.payloadGPR());
        }
        callOperation(operationHasOwnProperty, resultGPR, objectGPR, keyRegs);
        silentFillAllRegisters(resultGPR);
        m_jit.exceptionCheck();

        done.link(&m_jit);
        booleanResult(resultGPR, node);
#endif // CPU(X86)
        break;
    }

    case StoreBarrier:
    case FencedStoreBarrier: {
        compileStoreBarrier(node);
        break;
    }

    case GetEnumerableLength: {
        SpeculateCellOperand enumerator(this, node->child1());
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();

        m_jit.load32(MacroAssembler::Address(enumerator.gpr(), JSPropertyNameEnumerator::indexedLengthOffset()), resultGPR);
        int32Result(resultGPR, node);
        break;
    }
    case HasGenericProperty: {
        JSValueOperand base(this, node->child1());
        SpeculateCellOperand property(this, node->child2());
        GPRFlushedCallResult resultPayload(this);
        GPRFlushedCallResult2 resultTag(this);
        JSValueRegs baseRegs = base.jsValueRegs();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg resultTagGPR = resultTag.gpr();

        flushRegisters();
        callOperation(operationHasGenericProperty, JSValueRegs(resultTagGPR, resultPayloadGPR), baseRegs, property.gpr());
        m_jit.exceptionCheck();
        booleanResult(resultPayloadGPR, node);
        break;
    }
    case HasStructureProperty: {
        JSValueOperand base(this, node->child1());
        SpeculateCellOperand property(this, node->child2());
        SpeculateCellOperand enumerator(this, node->child3());
        GPRTemporary resultPayload(this);
        GPRTemporary resultTag(this);

        GPRReg baseTagGPR = base.tagGPR();
        GPRReg basePayloadGPR = base.payloadGPR();
        GPRReg propertyGPR = property.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg resultTagGPR = resultTag.gpr();

        m_jit.load32(MacroAssembler::Address(basePayloadGPR, JSCell::structureIDOffset()), resultTagGPR);
        MacroAssembler::Jump wrongStructure = m_jit.branch32(MacroAssembler::NotEqual, 
            resultTagGPR,
            MacroAssembler::Address(enumerator.gpr(), JSPropertyNameEnumerator::cachedStructureIDOffset()));

        moveTrueTo(resultPayloadGPR);
        MacroAssembler::Jump done = m_jit.jump();

        done.link(&m_jit);

        addSlowPathGenerator(slowPathCall(wrongStructure, this, operationHasGenericProperty, 
            JSValueRegs(resultTagGPR, resultPayloadGPR), JSValueRegs(baseTagGPR, basePayloadGPR), propertyGPR));
        booleanResult(resultPayloadGPR, node);
        break;
    }
    case HasIndexedProperty: {
        SpeculateCellOperand base(this, node->child1());
        SpeculateInt32Operand index(this, node->child2());
        GPRTemporary resultPayload(this);
        GPRTemporary resultTag(this);

        GPRReg baseGPR = base.gpr();
        GPRReg indexGPR = index.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg resultTagGPR = resultTag.gpr();

        MacroAssembler::JumpList slowCases;
        ArrayMode mode = node->arrayMode();
        switch (mode.type()) {
        case Array::Int32:
        case Array::Contiguous: {
            ASSERT(!!node->child3());
            StorageOperand storage(this, node->child3());
            GPRTemporary scratch(this);
            
            GPRReg storageGPR = storage.gpr();
            GPRReg scratchGPR = scratch.gpr();

            slowCases.append(m_jit.branch32(MacroAssembler::AboveOrEqual, indexGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength())));
            m_jit.load32(MacroAssembler::BaseIndex(storageGPR, indexGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)), scratchGPR);
            slowCases.append(m_jit.branch32(MacroAssembler::Equal, scratchGPR, TrustedImm32(JSValue::EmptyValueTag)));
            break;
        }
        case Array::Double: {
            ASSERT(!!node->child3());
            StorageOperand storage(this, node->child3());
            FPRTemporary scratch(this);
            FPRReg scratchFPR = scratch.fpr();
            GPRReg storageGPR = storage.gpr();

            slowCases.append(m_jit.branch32(MacroAssembler::AboveOrEqual, indexGPR, MacroAssembler::Address(storageGPR, Butterfly::offsetOfPublicLength())));
            m_jit.loadDouble(MacroAssembler::BaseIndex(storageGPR, indexGPR, MacroAssembler::TimesEight), scratchFPR);
            slowCases.append(m_jit.branchDouble(MacroAssembler::DoubleNotEqualOrUnordered, scratchFPR, scratchFPR));
            break;
        }
        case Array::ArrayStorage: {
            ASSERT(!!node->child3());
            StorageOperand storage(this, node->child3());
            GPRTemporary scratch(this);

            GPRReg storageGPR = storage.gpr();
            GPRReg scratchGPR = scratch.gpr();

            slowCases.append(m_jit.branch32(MacroAssembler::AboveOrEqual, indexGPR, MacroAssembler::Address(storageGPR, ArrayStorage::vectorLengthOffset())));
            m_jit.load32(MacroAssembler::BaseIndex(storageGPR, indexGPR, MacroAssembler::TimesEight, ArrayStorage::vectorOffset() + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), scratchGPR);
            slowCases.append(m_jit.branch32(MacroAssembler::Equal, scratchGPR, TrustedImm32(JSValue::EmptyValueTag)));
            break;
        }
        default: {
            slowCases.append(m_jit.jump());
            break;
        }
        }

        moveTrueTo(resultPayloadGPR);
        MacroAssembler::Jump done = m_jit.jump();

        addSlowPathGenerator(slowPathCall(slowCases, this, operationHasIndexedProperty, JSValueRegs(resultTagGPR, resultPayloadGPR), baseGPR, indexGPR, static_cast<int32_t>(node->internalMethodType())));
        
        done.link(&m_jit);
        booleanResult(resultPayloadGPR, node);
        break;
    }
    case GetDirectPname: {
        Edge& baseEdge = m_jit.graph().varArgChild(node, 0);
        Edge& propertyEdge = m_jit.graph().varArgChild(node, 1);

        SpeculateCellOperand base(this, baseEdge);
        SpeculateCellOperand property(this, propertyEdge);
        GPRReg baseGPR = base.gpr();
        GPRReg propertyGPR = property.gpr();

#if CPU(X86)
        GPRFlushedCallResult resultPayload(this);
        GPRFlushedCallResult2 resultTag(this);
        GPRTemporary scratch(this);

        GPRReg resultTagGPR = resultTag.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg scratchGPR = scratch.gpr();

        // Not enough registers on X86 for this code, so always use the slow path.
        flushRegisters();
        m_jit.move(MacroAssembler::TrustedImm32(JSValue::CellTag), scratchGPR);
        callOperation(operationGetByValCell, JSValueRegs(resultTagGPR, resultPayloadGPR), baseGPR, JSValueRegs(scratchGPR, propertyGPR));
        m_jit.exceptionCheck();
#else
        GPRTemporary resultPayload(this);
        GPRTemporary resultTag(this);
        GPRTemporary scratch(this);

        GPRReg resultTagGPR = resultTag.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();
        GPRReg scratchGPR = scratch.gpr();

        Edge& indexEdge = m_jit.graph().varArgChild(node, 2);
        Edge& enumeratorEdge = m_jit.graph().varArgChild(node, 3);

        SpeculateInt32Operand index(this, indexEdge);
        SpeculateCellOperand enumerator(this, enumeratorEdge);

        GPRReg indexGPR = index.gpr();
        GPRReg enumeratorGPR = enumerator.gpr();

        MacroAssembler::JumpList slowPath;

        // Check the structure
        m_jit.load32(MacroAssembler::Address(baseGPR, JSCell::structureIDOffset()), scratchGPR);
        slowPath.append(
            m_jit.branch32(
                MacroAssembler::NotEqual, 
                scratchGPR,
                MacroAssembler::Address(
                    enumeratorGPR, JSPropertyNameEnumerator::cachedStructureIDOffset())));
        
        // Compute the offset
        // If index is less than the enumerator's cached inline storage, then it's an inline access
        MacroAssembler::Jump outOfLineAccess = m_jit.branch32(MacroAssembler::AboveOrEqual, 
            indexGPR, MacroAssembler::Address(enumeratorGPR, JSPropertyNameEnumerator::cachedInlineCapacityOffset()));

        m_jit.move(indexGPR, scratchGPR);
        m_jit.signExtend32ToPtr(scratchGPR, scratchGPR);
        m_jit.load32(MacroAssembler::BaseIndex(baseGPR, scratchGPR, MacroAssembler::TimesEight, JSObject::offsetOfInlineStorage() + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), resultTagGPR);
        m_jit.load32(MacroAssembler::BaseIndex(baseGPR, scratchGPR, MacroAssembler::TimesEight, JSObject::offsetOfInlineStorage() + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), resultPayloadGPR);

        MacroAssembler::Jump done = m_jit.jump();
        
        // Otherwise it's out of line
        outOfLineAccess.link(&m_jit);
        m_jit.move(indexGPR, scratchGPR);
        m_jit.sub32(MacroAssembler::Address(enumeratorGPR, JSPropertyNameEnumerator::cachedInlineCapacityOffset()), scratchGPR);
        m_jit.neg32(scratchGPR);
        m_jit.signExtend32ToPtr(scratchGPR, scratchGPR);
        // We use resultPayloadGPR as a temporary here. We have to make sure clobber it after getting the 
        // value out of indexGPR and enumeratorGPR because resultPayloadGPR could reuse either of those registers.
        m_jit.loadPtr(MacroAssembler::Address(baseGPR, JSObject::butterflyOffset()), resultPayloadGPR);
        int32_t offsetOfFirstProperty = static_cast<int32_t>(offsetInButterfly(firstOutOfLineOffset)) * sizeof(EncodedJSValue);
        m_jit.load32(MacroAssembler::BaseIndex(resultPayloadGPR, scratchGPR, MacroAssembler::TimesEight, offsetOfFirstProperty + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), resultTagGPR);
        m_jit.load32(MacroAssembler::BaseIndex(resultPayloadGPR, scratchGPR, MacroAssembler::TimesEight, offsetOfFirstProperty + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), resultPayloadGPR);

        done.link(&m_jit);

        addSlowPathGenerator(slowPathCall(slowPath, this, operationGetByValCell, JSValueRegs(resultTagGPR, resultPayloadGPR), baseGPR, propertyGPR));
#endif

        jsValueResult(resultTagGPR, resultPayloadGPR, node);
        break;
    }
    case GetPropertyEnumerator: {
        SpeculateCellOperand base(this, node->child1());
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();

        flushRegisters();
        callOperation(operationGetPropertyEnumerator, resultGPR, base.gpr());
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
        break;
    }
    case GetEnumeratorStructurePname:
    case GetEnumeratorGenericPname: {
        SpeculateCellOperand enumerator(this, node->child1());
        SpeculateInt32Operand index(this, node->child2());
        GPRTemporary scratch(this);
        GPRTemporary resultPayload(this);
        GPRTemporary resultTag(this);

        GPRReg enumeratorGPR = enumerator.gpr();
        GPRReg indexGPR = index.gpr();
        GPRReg scratchGPR = scratch.gpr();
        GPRReg resultTagGPR = resultTag.gpr();
        GPRReg resultPayloadGPR = resultPayload.gpr();

        MacroAssembler::Jump inBounds = m_jit.branch32(MacroAssembler::Below, indexGPR,
            MacroAssembler::Address(enumeratorGPR, (op == GetEnumeratorStructurePname)
                ? JSPropertyNameEnumerator::endStructurePropertyIndexOffset()
                : JSPropertyNameEnumerator::endGenericPropertyIndexOffset()));

        m_jit.move(MacroAssembler::TrustedImm32(JSValue::NullTag), resultTagGPR);
        m_jit.move(MacroAssembler::TrustedImm32(0), resultPayloadGPR);

        MacroAssembler::Jump done = m_jit.jump();
        inBounds.link(&m_jit);

        m_jit.loadPtr(MacroAssembler::Address(enumeratorGPR, JSPropertyNameEnumerator::cachedPropertyNamesVectorOffset()), scratchGPR);
        m_jit.loadPtr(MacroAssembler::BaseIndex(scratchGPR, indexGPR, MacroAssembler::ScalePtr), resultPayloadGPR);
        m_jit.move(MacroAssembler::TrustedImm32(JSValue::CellTag), resultTagGPR);

        done.link(&m_jit);
        jsValueResult(resultTagGPR, resultPayloadGPR, node);
        break;
    }
    case ToIndexString: {
        SpeculateInt32Operand index(this, node->child1());
        GPRFlushedCallResult result(this);
        GPRReg resultGPR = result.gpr();

        flushRegisters();
        callOperation(operationToIndexString, resultGPR, index.gpr());
        m_jit.exceptionCheck();
        cellResult(resultGPR, node);
        break;
    }
    case ProfileType: {
        JSValueOperand value(this, node->child1());
        GPRTemporary scratch1(this);
        GPRTemporary scratch2(this);
        GPRTemporary scratch3(this);

        GPRReg scratch1GPR = scratch1.gpr();
        GPRReg scratch2GPR = scratch2.gpr();
        GPRReg scratch3GPR = scratch3.gpr();

        JITCompiler::Jump isTDZValue = m_jit.branch32(JITCompiler::Equal, value.tagGPR(), TrustedImm32(JSValue::EmptyValueTag));

        // Load the TypeProfilerLog into Scratch2.
        TypeProfilerLog* cachedTypeProfilerLog = m_jit.vm()->typeProfilerLog();
        m_jit.move(TrustedImmPtr(cachedTypeProfilerLog), scratch2GPR);

        // Load the next LogEntry into Scratch1.
        m_jit.loadPtr(MacroAssembler::Address(scratch2GPR, TypeProfilerLog::currentLogEntryOffset()), scratch1GPR);

        // Store the JSValue onto the log entry.
        m_jit.store32(value.tagGPR(), MacroAssembler::Address(scratch1GPR, TypeProfilerLog::LogEntry::valueOffset() + OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
        m_jit.store32(value.payloadGPR(), MacroAssembler::Address(scratch1GPR, TypeProfilerLog::LogEntry::valueOffset() + OBJECT_OFFSETOF(JSValue, u.asBits.payload)));

        // Store the structureID of the cell if valueGPR is a cell, otherwise, store 0 on the log entry.
        MacroAssembler::Jump isNotCell = m_jit.branchIfNotCell(value.jsValueRegs());
        m_jit.load32(MacroAssembler::Address(value.payloadGPR(), JSCell::structureIDOffset()), scratch3GPR);
        m_jit.store32(scratch3GPR, MacroAssembler::Address(scratch1GPR, TypeProfilerLog::LogEntry::structureIDOffset()));
        MacroAssembler::Jump skipIsCell = m_jit.jump();
        isNotCell.link(&m_jit);
        m_jit.store32(TrustedImm32(0), MacroAssembler::Address(scratch1GPR, TypeProfilerLog::LogEntry::structureIDOffset()));
        skipIsCell.link(&m_jit);

        // Store the typeLocation on the log entry.
        TypeLocation* cachedTypeLocation = node->typeLocation();
        m_jit.move(TrustedImmPtr(cachedTypeLocation), scratch3GPR);
        m_jit.storePtr(scratch3GPR, MacroAssembler::Address(scratch1GPR, TypeProfilerLog::LogEntry::locationOffset()));

        // Increment the current log entry.
        m_jit.addPtr(TrustedImm32(sizeof(TypeProfilerLog::LogEntry)), scratch1GPR);
        m_jit.storePtr(scratch1GPR, MacroAssembler::Address(scratch2GPR, TypeProfilerLog::currentLogEntryOffset()));
        MacroAssembler::Jump clearLog = m_jit.branchPtr(MacroAssembler::Equal, scratch1GPR, TrustedImmPtr(cachedTypeProfilerLog->logEndPtr()));
        addSlowPathGenerator(
            slowPathCall(clearLog, this, operationProcessTypeProfilerLogDFG, NoResult));

        isTDZValue.link(&m_jit);

        noResult(node);
        break;
    }
    case ProfileControlFlow: {
        GPRTemporary scratch1(this);
        BasicBlockLocation* basicBlockLocation = node->basicBlockLocation();
        basicBlockLocation->emitExecuteCode(m_jit, scratch1.gpr());
        noResult(node);
        break;
    }

    case LogShadowChickenPrologue: {
        flushRegisters();
        prepareForExternalCall();
        m_jit.emitStoreCodeOrigin(node->origin.semantic);

        GPRTemporary scratch1(this, GPRInfo::nonArgGPR0); // This must be a non-argument GPR.
        GPRReg scratch1Reg = scratch1.gpr();
        GPRTemporary shadowPacket(this);
        GPRReg shadowPacketReg = shadowPacket.gpr();
        GPRTemporary scratch2(this);
        GPRReg scratch2Reg = scratch2.gpr();

        m_jit.ensureShadowChickenPacket(shadowPacketReg, scratch1Reg, scratch2Reg);

        SpeculateCellOperand scope(this, node->child1());
        GPRReg scopeReg = scope.gpr();

        m_jit.logShadowChickenProloguePacket(shadowPacketReg, scratch1Reg, scopeReg);
        noResult(node);
        break;
    }

    case LogShadowChickenTail: {
        flushRegisters();
        prepareForExternalCall();
        CallSiteIndex callSiteIndex = m_jit.emitStoreCodeOrigin(node->origin.semantic);

        GPRTemporary scratch1(this, GPRInfo::nonArgGPR0); // This must be a non-argument GPR.
        GPRReg scratch1Reg = scratch1.gpr();
        GPRTemporary shadowPacket(this);
        GPRReg shadowPacketReg = shadowPacket.gpr();
        GPRTemporary scratch2(this);
        GPRReg scratch2Reg = scratch2.gpr();

        m_jit.ensureShadowChickenPacket(shadowPacketReg, scratch1Reg, scratch2Reg);

        JSValueOperand thisValue(this, node->child1());
        JSValueRegs thisRegs = thisValue.jsValueRegs();
        SpeculateCellOperand scope(this, node->child2());
        GPRReg scopeReg = scope.gpr();

        m_jit.logShadowChickenTailPacket(shadowPacketReg, thisRegs, scopeReg, m_jit.codeBlock(), callSiteIndex);
        noResult(node);
        break;
    }

    case ForceOSRExit: {
        terminateSpeculativeExecution(InadequateCoverage, JSValueRegs(), 0);
        break;
    }

    case InvalidationPoint:
        emitInvalidationPoint(node);
        break;

    case CheckTraps:
        if (Options::usePollingTraps())
            compileCheckTraps(node);
        else
            noResult(node); // This is a no-op.
        break;

    case CountExecution:
        m_jit.add64(TrustedImm32(1), MacroAssembler::AbsoluteAddress(node->executionCounter()->address()));
        break;

    case Phantom:
    case Check:
        DFG_NODE_DO_TO_CHILDREN(m_jit.graph(), node, speculate);
        noResult(node);
        break;

    case PhantomLocal:
    case LoopHint:
        // This is a no-op.
        noResult(node);
        break;
        
    case MaterializeNewObject:
        compileMaterializeNewObject(node);
        break;

    case PutDynamicVar: {
        compilePutDynamicVar(node);
        break;
    }

    case GetDynamicVar: {
        compileGetDynamicVar(node);
        break;
    }

    case ResolveScope: {
        compileResolveScope(node);
        break;
    }

    case CallDOM:
        compileCallDOM(node);
        break;

    case CallDOMGetter:
        compileCallDOMGetter(node);
        break;

    case CheckDOM:
        compileCheckDOM(node);
        break;

    case Unreachable:
        unreachable(node);
        break;

    case LastNodeType:
    case Phi:
    case Upsilon:
    case ExtractOSREntryLocal:
    case CheckTierUpInLoop:
    case CheckTierUpAtReturn:
    case CheckTierUpAndOSREnter:
    case Int52Rep:
    case FiatInt52:
    case Int52Constant:
    case CheckInBounds:
    case ArithIMul:
    case MultiGetByOffset:
    case MultiPutByOffset:
    case CheckBadCell:
    case BottomValue:
    case PhantomNewObject:
    case PhantomNewFunction:
    case PhantomNewGeneratorFunction:
    case PhantomNewAsyncFunction:
    case PhantomCreateActivation:
    case PutHint:
    case CheckStructureImmediate:
    case MaterializeCreateActivation:
    case PutStack:
    case KillStack:
    case GetStack:
    case GetMyArgumentByVal:
    case GetMyArgumentByValOutOfBounds:
    case PhantomCreateRest:
    case PhantomSpread:
    case PhantomNewArrayWithSpread:
        DFG_CRASH(m_jit.graph(), node, "unexpected node in DFG backend");
        break;
    }

    if (!m_compileOkay)
        return;
    
    if (node->hasResult() && node->mustGenerate())
        use(node);
}

void SpeculativeJIT::moveTrueTo(GPRReg gpr)
{
    m_jit.move(TrustedImm32(1), gpr);
}

void SpeculativeJIT::moveFalseTo(GPRReg gpr)
{
    m_jit.move(TrustedImm32(0), gpr);
}

void SpeculativeJIT::blessBoolean(GPRReg)
{
}

void SpeculativeJIT::compileArithRandom(Node* node)
{
    JSGlobalObject* globalObject = m_jit.graph().globalObjectFor(node->origin.semantic);

    flushRegisters();

    FPRResult result(this);
    callOperation(operationRandom, result.fpr(), globalObject);
    // operationRandom does not raise any exception.
    doubleResult(result.fpr(), node);
}

void SpeculativeJIT::emitInitializeButterfly(GPRReg storageGPR, GPRReg sizeGPR, JSValueRegs emptyValueRegs, GPRReg scratchGPR)
{
    m_jit.move(sizeGPR, scratchGPR);
    MacroAssembler::Jump done = m_jit.branchTest32(MacroAssembler::Zero, scratchGPR);
    MacroAssembler::Label loop = m_jit.label();
    m_jit.sub32(TrustedImm32(1), scratchGPR);
    m_jit.store32(emptyValueRegs.tagGPR(), MacroAssembler::BaseIndex(storageGPR, scratchGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.tag)));
    m_jit.store32(emptyValueRegs.payloadGPR(), MacroAssembler::BaseIndex(storageGPR, scratchGPR, MacroAssembler::TimesEight, OBJECT_OFFSETOF(JSValue, u.asBits.payload)));
    m_jit.branchTest32(MacroAssembler::NonZero, scratchGPR).linkTo(loop, &m_jit);
    done.link(&m_jit);
}

void SpeculativeJIT::compileAllocateNewArrayWithSize(JSGlobalObject* globalObject, GPRReg resultGPR, GPRReg sizeGPR, IndexingType indexingType, bool shouldConvertLargeSizeToArrayStorage)
{
    GPRTemporary storage(this);
    GPRTemporary scratch(this);
    GPRTemporary scratch2(this);
    
    GPRReg storageGPR = storage.gpr();
    GPRReg scratchGPR = scratch.gpr();
    GPRReg scratch2GPR = scratch2.gpr();
    
    m_jit.move(TrustedImmPtr(0), storageGPR);
            
    MacroAssembler::JumpList slowCases;
    if (shouldConvertLargeSizeToArrayStorage)
        slowCases.append(m_jit.branch32(MacroAssembler::AboveOrEqual, sizeGPR, TrustedImm32(MIN_ARRAY_STORAGE_CONSTRUCTION_LENGTH)));

    // We can use result as a scratch for this.
    emitAllocateButterfly(storageGPR, sizeGPR, scratchGPR, scratch2GPR, resultGPR, slowCases);

    JSValue hole;
    if (hasDouble(indexingType))
        hole = JSValue(JSValue::EncodeAsDouble, PNaN);
    else
        hole = JSValue();
    JSValueRegs emptyValueRegs(scratchGPR, scratch2GPR);
    m_jit.move(TrustedImm32(hole.tag()), emptyValueRegs.tagGPR());
    m_jit.move(TrustedImm32(hole.payload()), emptyValueRegs.payloadGPR());
    // We can use result as a scratch for this.
    emitInitializeButterfly(storageGPR, sizeGPR, emptyValueRegs, resultGPR);
            
    RegisteredStructure structure = m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(indexingType));
    emitAllocateJSObject<JSArray>(resultGPR, TrustedImmPtr(structure), storageGPR, scratchGPR, scratch2GPR, slowCases);
            
    addSlowPathGenerator(std::make_unique<CallArrayAllocatorWithVariableSizeSlowPathGenerator>(
        slowCases, this, operationNewArrayWithSize, resultGPR,
        structure,
        shouldConvertLargeSizeToArrayStorage ? m_jit.graph().registerStructure(globalObject->arrayStructureForIndexingTypeDuringAllocation(ArrayWithArrayStorage)) : structure,
        sizeGPR, storageGPR));
}

#endif

} } // namespace JSC::DFG

#endif
