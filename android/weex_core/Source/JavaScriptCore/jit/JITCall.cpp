/*
 * Copyright (C) 2008, 2013-2016 Apple Inc. All rights reserved.
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
#if USE(JSVALUE64)
#include "JIT.h"

#include "CallFrameShuffler.h"
#include "CodeBlock.h"
#include "JITInlines.h"
#include "JSArray.h"
#include "JSFunction.h"
#include "Interpreter.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"
#include "ResultType.h"
#include "SetupVarargsFrame.h"
#include "StackAlignment.h"
#include "ThunkGenerators.h"
#include <wtf/StringPrintStream.h>


namespace JSC {

void JIT::emitPutCallResult(Instruction* instruction)
{
    int dst = instruction[1].u.operand;
    emitValueProfilingSite();
    emitPutVirtualRegister(dst);
}

void JIT::compileSetupVarargsFrame(OpcodeID opcode, Instruction* instruction, CallLinkInfo* info)
{
    int thisValue = instruction[3].u.operand;
    int arguments = instruction[4].u.operand;
    int firstFreeRegister = instruction[5].u.operand;
    int firstVarArgOffset = instruction[6].u.operand;

    emitGetVirtualRegister(arguments, regT1);
    Z_JITOperation_EJZZ sizeOperation;
    if (opcode == op_tail_call_forward_arguments)
        sizeOperation = operationSizeFrameForForwardArguments;
    else
        sizeOperation = operationSizeFrameForVarargs;
    callOperation(sizeOperation, regT1, -firstFreeRegister, firstVarArgOffset);
    move(TrustedImm32(-firstFreeRegister), regT1);
    emitSetVarargsFrame(*this, returnValueGPR, false, regT1, regT1);
    addPtr(TrustedImm32(-(sizeof(CallerFrameAndPC) + WTF::roundUpToMultipleOf(stackAlignmentBytes(), 5 * sizeof(void*)))), regT1, stackPointerRegister);
    emitGetVirtualRegister(arguments, regT2);
    F_JITOperation_EFJZZ setupOperation;
    if (opcode == op_tail_call_forward_arguments)
        setupOperation = operationSetupForwardArgumentsFrame;
    else
        setupOperation = operationSetupVarargsFrame;
    callOperation(setupOperation, regT1, regT2, firstVarArgOffset, regT0);
    move(returnValueGPR, regT1);

    // Profile the argument count.
    load32(Address(regT1, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset), regT2);
    load32(info->addressOfMaxNumArguments(), regT0);
    Jump notBiggest = branch32(Above, regT0, regT2);
    store32(regT2, info->addressOfMaxNumArguments());
    notBiggest.link(this);
    
    // Initialize 'this'.
    emitGetVirtualRegister(thisValue, regT0);
    store64(regT0, Address(regT1, CallFrame::thisArgumentOffset() * static_cast<int>(sizeof(Register))));

    addPtr(TrustedImm32(sizeof(CallerFrameAndPC)), regT1, stackPointerRegister);
}

void JIT::compileCallEval(Instruction* instruction)
{
    addPtr(TrustedImm32(-static_cast<ptrdiff_t>(sizeof(CallerFrameAndPC))), stackPointerRegister, regT1);
    storePtr(callFrameRegister, Address(regT1, CallFrame::callerFrameOffset()));

    addPtr(TrustedImm32(stackPointerOffsetFor(m_codeBlock) * sizeof(Register)), callFrameRegister, stackPointerRegister);
    checkStackPointerAlignment();

    callOperation(operationCallEval, regT1);

    addSlowCase(branch64(Equal, regT0, TrustedImm64(JSValue::encode(JSValue()))));

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

    load64(Address(stackPointerRegister, sizeof(Register) * CallFrameSlot::callee - sizeof(CallerFrameAndPC)), regT0);
    emitDumbVirtualCall(info);
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
    COMPILE_ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_construct), call_and_construct_opcodes_must_be_same_length);
    COMPILE_ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_call_varargs), call_and_call_varargs_opcodes_must_be_same_length);
    COMPILE_ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_construct_varargs), call_and_construct_varargs_opcodes_must_be_same_length);
    COMPILE_ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_tail_call), call_and_tail_call_opcodes_must_be_same_length);
    COMPILE_ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_tail_call_varargs), call_and_tail_call_varargs_opcodes_must_be_same_length);
    COMPILE_ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_tail_call_forward_arguments), call_and_tail_call_forward_arguments_opcodes_must_be_same_length);

    CallLinkInfo* info = nullptr;
    if (opcodeID != op_call_eval)
        info = m_codeBlock->addCallLinkInfo();
    if (opcodeID == op_call_varargs || opcodeID == op_construct_varargs || opcodeID == op_tail_call_varargs || opcodeID == op_tail_call_forward_arguments)
        compileSetupVarargsFrame(opcodeID, instruction, info);
    else {
        int argCount = instruction[3].u.operand;
        int registerOffset = -instruction[4].u.operand;

        if (opcodeID == op_call && shouldEmitProfiling()) {
            emitGetVirtualRegister(registerOffset + CallFrame::argumentOffsetIncludingThis(0), regT0);
            Jump done = emitJumpIfNotJSCell(regT0);
            load32(Address(regT0, JSCell::structureIDOffset()), regT0);
            store32(regT0, instruction[OPCODE_LENGTH(op_call) - 2].u.arrayProfile->addressOfLastSeenStructureID());
            done.link(this);
        }
    
        addPtr(TrustedImm32(registerOffset * sizeof(Register) + sizeof(CallerFrameAndPC)), callFrameRegister, stackPointerRegister);
        store32(TrustedImm32(argCount), Address(stackPointerRegister, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset - sizeof(CallerFrameAndPC)));
    } // SP holds newCallFrame + sizeof(CallerFrameAndPC), with ArgumentCount initialized.
    
    uint32_t bytecodeOffset = instruction - m_codeBlock->instructions().begin();
    uint32_t locationBits = CallSiteIndex(bytecodeOffset).bits();
    store32(TrustedImm32(locationBits), Address(callFrameRegister, CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + TagOffset));

    emitGetVirtualRegister(callee, regT0); // regT0 holds callee.
    store64(regT0, Address(stackPointerRegister, CallFrameSlot::callee * static_cast<int>(sizeof(Register)) - sizeof(CallerFrameAndPC)));

    if (opcodeID == op_call_eval) {
        compileCallEval(instruction);
        return;
    }

    DataLabelPtr addressOfLinkedFunctionCheck;
    Jump slowCase = branchPtrWithPatch(NotEqual, regT0, addressOfLinkedFunctionCheck, TrustedImmPtr(0));
    addSlowCase(slowCase);

    ASSERT(m_callCompilationInfo.size() == callLinkInfoIndex);
    info->setUpCall(CallLinkInfo::callTypeFor(opcodeID), CodeOrigin(m_bytecodeOffset), regT0);
    m_callCompilationInfo.append(CallCompilationInfo());
    m_callCompilationInfo[callLinkInfoIndex].hotPathBegin = addressOfLinkedFunctionCheck;
    m_callCompilationInfo[callLinkInfoIndex].callLinkInfo = info;

    if (opcodeID == op_tail_call) {
        CallFrameShuffleData shuffleData;
        shuffleData.numPassedArgs = instruction[3].u.operand;
        shuffleData.tagTypeNumber = GPRInfo::tagTypeNumberRegister;
        shuffleData.numLocals =
            instruction[4].u.operand - sizeof(CallerFrameAndPC) / sizeof(Register);
        shuffleData.args.resize(instruction[3].u.operand);
        for (int i = 0; i < instruction[3].u.operand; ++i) {
            shuffleData.args[i] =
                ValueRecovery::displacedInJSStack(
                    virtualRegisterForArgument(i) - instruction[4].u.operand,
                    DataFormatJS);
        }
        shuffleData.callee =
            ValueRecovery::inGPR(regT0, DataFormatJS);
        shuffleData.setupCalleeSaveRegisters(m_codeBlock);
        info->setFrameShuffleData(shuffleData);
        CallFrameShuffler(*this, shuffleData).prepareForTailCall();
        m_callCompilationInfo[callLinkInfoIndex].hotPathOther = emitNakedTailCall();
        return;
    }

    if (opcodeID == op_tail_call_varargs || opcodeID == op_tail_call_forward_arguments) {
        emitRestoreCalleeSaves();
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

    if (opcodeID == op_tail_call || opcodeID == op_tail_call_varargs || opcodeID == op_tail_call_forward_arguments)
        emitRestoreCalleeSaves();

    move(TrustedImmPtr(m_callCompilationInfo[callLinkInfoIndex].callLinkInfo), regT2);

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

} // namespace JSC

#endif // USE(JSVALUE64)
#endif // ENABLE(JIT)
