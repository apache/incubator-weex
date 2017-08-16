/*
 * Copyright (C) 2008, 2013-2015 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)
#if USE(JSVALUE32_64)
#include "JIT.h"

#include "CodeBlock.h"
#include "Interpreter.h"
#include "JITInlines.h"
#include "JSArray.h"
#include "JSFunction.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"
#include "ResultType.h"
#include "SetupVarargsFrame.h"
#include "StackAlignment.h"
#include <wtf/StringPrintStream.h>


namespace JSC {

void JIT::emitPutCallResult(Instruction* instruction)
{
    int dst = instruction[1].u.operand;
    emitValueProfilingSite();
    emitStore(dst, regT1, regT0);
}

void JIT::emit_op_ret(Instruction* currentInstruction)
{
    unsigned dst = currentInstruction[1].u.operand;

    emitLoad(dst, regT1, regT0);

    checkStackPointerAlignment();
    emitRestoreCalleeSaves();
    emitFunctionEpilogue();
    ret();
}

void JIT::emitSlow_op_call(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_call, currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_tail_call(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_tail_call, currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_call_eval(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_call_eval, currentInstruction, iter, m_callLinkInfoIndex);
}
 
void JIT::emitSlow_op_call_varargs(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_call_varargs, currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_tail_call_varargs(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_tail_call_varargs, currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emitSlow_op_tail_call_forward_arguments(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_tail_call_forward_arguments, currentInstruction, iter, m_callLinkInfoIndex++);
}
    
void JIT::emitSlow_op_construct_varargs(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_construct_varargs, currentInstruction, iter, m_callLinkInfoIndex++);
}
    
void JIT::emitSlow_op_construct(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    compileOpCallSlowCase(op_construct, currentInstruction, iter, m_callLinkInfoIndex++);
}

void JIT::emit_op_call(Instruction* currentInstruction)
{
    compileOpCall(op_call, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_tail_call(Instruction* currentInstruction)
{
    compileOpCall(op_tail_call, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_call_eval(Instruction* currentInstruction)
{
    compileOpCall(op_call_eval, currentInstruction, m_callLinkInfoIndex);
}

void JIT::emit_op_call_varargs(Instruction* currentInstruction)
{
    compileOpCall(op_call_varargs, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_tail_call_varargs(Instruction* currentInstruction)
{
    compileOpCall(op_tail_call_varargs, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::emit_op_tail_call_forward_arguments(Instruction* currentInstruction)
{
    compileOpCall(op_tail_call_forward_arguments, currentInstruction, m_callLinkInfoIndex++);
}
    
void JIT::emit_op_construct_varargs(Instruction* currentInstruction)
{
    compileOpCall(op_construct_varargs, currentInstruction, m_callLinkInfoIndex++);
}
    
void JIT::emit_op_construct(Instruction* currentInstruction)
{
    compileOpCall(op_construct, currentInstruction, m_callLinkInfoIndex++);
}

void JIT::compileSetupVarargsFrame(OpcodeID opcode, Instruction* instruction, CallLinkInfo* info)
{
    int thisValue = instruction[3].u.operand;
    int arguments = instruction[4].u.operand;
    int firstFreeRegister = instruction[5].u.operand;
    int firstVarArgOffset = instruction[6].u.operand;

    emitLoad(arguments, regT1, regT0);
    Z_JITOperation_EJZZ sizeOperation;
    if (opcode == op_tail_call_forward_arguments)
        sizeOperation = operationSizeFrameForForwardArguments;
    else
        sizeOperation = operationSizeFrameForVarargs;
    callOperation(sizeOperation, regT1, regT0, -firstFreeRegister, firstVarArgOffset);
    move(TrustedImm32(-firstFreeRegister), regT1);
    emitSetVarargsFrame(*this, returnValueGPR, false, regT1, regT1);
    addPtr(TrustedImm32(-(sizeof(CallerFrameAndPC) + WTF::roundUpToMultipleOf(stackAlignmentBytes(), 6 * sizeof(void*)))), regT1, stackPointerRegister);
    emitLoad(arguments, regT2, regT4);
    F_JITOperation_EFJZZ setupOperation;
    if (opcode == op_tail_call_forward_arguments)
        setupOperation = operationSetupForwardArgumentsFrame;
    else
        setupOperation = operationSetupVarargsFrame;
    callOperation(setupOperation, regT1, regT2, regT4, firstVarArgOffset, regT0);
    move(returnValueGPR, regT1);

    // Profile the argument count.
    load32(Address(regT1, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset), regT2);
    load32(info->addressOfMaxNumArguments(), regT0);
    Jump notBiggest = branch32(Above, regT0, regT2);
    store32(regT2, info->addressOfMaxNumArguments());
    notBiggest.link(this);
    
    // Initialize 'this'.
    emitLoad(thisValue, regT2, regT0);
    store32(regT0, Address(regT1, PayloadOffset + (CallFrame::thisArgumentOffset() * static_cast<int>(sizeof(Register)))));
    store32(regT2, Address(regT1, TagOffset + (CallFrame::thisArgumentOffset() * static_cast<int>(sizeof(Register)))));
    
    addPtr(TrustedImm32(sizeof(CallerFrameAndPC)), regT1, stackPointerRegister);
}

void JIT::compileCallEval(Instruction* instruction)
{
    addPtr(TrustedImm32(-static_cast<ptrdiff_t>(sizeof(CallerFrameAndPC))), stackPointerRegister, regT1);
    storePtr(callFrameRegister, Address(regT1, CallFrame::callerFrameOffset()));

    addPtr(TrustedImm32(stackPointerOffsetFor(m_codeBlock) * sizeof(Register)), callFrameRegister, stackPointerRegister);

    callOperation(operationCallEval, regT1);

    addSlowCase(branch32(Equal, regT1, TrustedImm32(JSValue::EmptyValueTag)));

    sampleCodeBlock(m_codeBlock);
    
    emitPutCallResult(instruction);
}

void JIT::compileCallEvalSlowCase(Instruction* instruction, Vector<SlowCaseEntry>::iterator& iter)
{
    CallLinkInfo* info = m_codeBlock->addCallLinkInfo();
    info->setUpCall(CallLinkInfo::Call, CodeOrigin(m_bytecodeOffset), regT0);

    linkSlowCase(iter);

    int registerOffset = -instruction[4].u.operand;

    addPtr(TrustedImm32(registerOffset * sizeof(Register) + sizeof(CallerFrameAndPC)), callFrameRegister, stackPointerRegister);

    move(TrustedImmPtr(info), regT2);

    emitLoad(CallFrameSlot::callee, regT1, regT0);
    MacroAssemblerCodeRef virtualThunk = virtualThunkFor(m_vm, *info);
    info->setSlowStub(createJITStubRoutine(virtualThunk, *m_vm, nullptr, true));
    emitNakedCall(virtualThunk.code());
    addPtr(TrustedImm32(stackPointerOffsetFor(m_codeBlock) * sizeof(Register)), callFrameRegister, stackPointerRegister);
    checkStackPointerAlignment();

    sampleCodeBlock(m_codeBlock);
    
    emitPutCallResult(instruction);
}

void JIT::compileOpCall(OpcodeID opcodeID, Instruction* instruction, unsigned callLinkInfoIndex)
{
    int callee = instruction[2].u.operand;

    /* Caller always:
        - Updates callFrameRegister to callee callFrame.
        - Initializes ArgumentCount; CallerFrame; Callee.

       For a JS call:
        - Callee initializes ReturnPC; CodeBlock.
        - Callee restores callFrameRegister before return.

       For a non-JS call:
        - Caller initializes ReturnPC; CodeBlock.
        - Caller restores callFrameRegister after return.
    */
    CallLinkInfo* info = nullptr;
    if (opcodeID != op_call_eval)
        info = m_codeBlock->addCallLinkInfo();
    if (opcodeID == op_call_varargs || opcodeID == op_construct_varargs || opcodeID == op_tail_call_varargs || opcodeID == op_tail_call_forward_arguments)
        compileSetupVarargsFrame(opcodeID, instruction, info);
    else {
        int argCount = instruction[3].u.operand;
        int registerOffset = -instruction[4].u.operand;
        
        if (opcodeID == op_call && shouldEmitProfiling()) {
            emitLoad(registerOffset + CallFrame::argumentOffsetIncludingThis(0), regT0, regT1);
            Jump done = branch32(NotEqual, regT0, TrustedImm32(JSValue::CellTag));
            loadPtr(Address(regT1, JSCell::structureIDOffset()), regT1);
            storePtr(regT1, instruction[OPCODE_LENGTH(op_call) - 2].u.arrayProfile->addressOfLastSeenStructureID());
            done.link(this);
        }
    
        addPtr(TrustedImm32(registerOffset * sizeof(Register) + sizeof(CallerFrameAndPC)), callFrameRegister, stackPointerRegister);

        store32(TrustedImm32(argCount), Address(stackPointerRegister, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset - sizeof(CallerFrameAndPC)));
    } // SP holds newCallFrame + sizeof(CallerFrameAndPC), with ArgumentCount initialized.
    
    uint32_t locationBits = CallSiteIndex(instruction).bits();
    store32(TrustedImm32(locationBits), tagFor(CallFrameSlot::argumentCount, callFrameRegister));
    emitLoad(callee, regT1, regT0); // regT1, regT0 holds callee.

    store32(regT0, Address(stackPointerRegister, CallFrameSlot::callee * static_cast<int>(sizeof(Register)) + PayloadOffset - sizeof(CallerFrameAndPC)));
    store32(regT1, Address(stackPointerRegister, CallFrameSlot::callee * static_cast<int>(sizeof(Register)) + TagOffset - sizeof(CallerFrameAndPC)));

    if (opcodeID == op_call_eval) {
        compileCallEval(instruction);
        return;
    }

    if (opcodeID == op_tail_call || opcodeID == op_tail_call_varargs)
        emitRestoreCalleeSaves();

    addSlowCase(branch32(NotEqual, regT1, TrustedImm32(JSValue::CellTag)));

    DataLabelPtr addressOfLinkedFunctionCheck;
    Jump slowCase = branchPtrWithPatch(NotEqual, regT0, addressOfLinkedFunctionCheck, TrustedImmPtr(0));

    addSlowCase(slowCase);

    ASSERT(m_callCompilationInfo.size() == callLinkInfoIndex);
    info->setUpCall(CallLinkInfo::callTypeFor(opcodeID), CodeOrigin(m_bytecodeOffset), regT0);
    m_callCompilationInfo.append(CallCompilationInfo());
    m_callCompilationInfo[callLinkInfoIndex].hotPathBegin = addressOfLinkedFunctionCheck;
    m_callCompilationInfo[callLinkInfoIndex].callLinkInfo = info;

    checkStackPointerAlignment();
    if (opcodeID == op_tail_call || opcodeID == op_tail_call_varargs || opcodeID == op_tail_call_forward_arguments) {
        prepareForTailCallSlow();
        m_callCompilationInfo[callLinkInfoIndex].hotPathOther = emitNakedTailCall();
        return;
    }

    m_callCompilationInfo[callLinkInfoIndex].hotPathOther = emitNakedCall();

    addPtr(TrustedImm32(stackPointerOffsetFor(m_codeBlock) * sizeof(Register)), callFrameRegister, stackPointerRegister);
    checkStackPointerAlignment();

    sampleCodeBlock(m_codeBlock);
    emitPutCallResult(instruction);
}

void JIT::compileOpCallSlowCase(OpcodeID opcodeID, Instruction* instruction, Vector<SlowCaseEntry>::iterator& iter, unsigned callLinkInfoIndex)
{
    if (opcodeID == op_call_eval) {
        compileCallEvalSlowCase(instruction, iter);
        return;
    }

    linkSlowCase(iter);
    linkSlowCase(iter);

    move(TrustedImmPtr(m_callCompilationInfo[callLinkInfoIndex].callLinkInfo), regT2);

    if (opcodeID == op_tail_call || opcodeID == op_tail_call_varargs)
        emitRestoreCalleeSaves();

    m_callCompilationInfo[callLinkInfoIndex].callReturnLocation = emitNakedCall(m_vm->getCTIStub(linkCallThunkGenerator).code());

    if (opcodeID == op_tail_call || opcodeID == op_tail_call_varargs) {
        abortWithReason(JITDidReturnFromTailCall);
        return;
    }

    addPtr(TrustedImm32(stackPointerOffsetFor(m_codeBlock) * sizeof(Register)), callFrameRegister, stackPointerRegister);
    checkStackPointerAlignment();

    sampleCodeBlock(m_codeBlock);
    emitPutCallResult(instruction);
}

} // namespace JSC

#endif // USE(JSVALUE32_64)
#endif // ENABLE(JIT)
