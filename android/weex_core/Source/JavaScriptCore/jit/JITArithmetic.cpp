/*
 * Copyright (C) 2008, 2015-2016 Apple Inc. All rights reserved.
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
#include "JIT.h"

#include "ArithProfile.h"
#include "CodeBlock.h"
#include "JITAddGenerator.h"
#include "JITBitAndGenerator.h"
#include "JITBitOrGenerator.h"
#include "JITBitXorGenerator.h"
#include "JITDivGenerator.h"
#include "JITInlines.h"
#include "JITLeftShiftGenerator.h"
#include "JITMathIC.h"
#include "JITMulGenerator.h"
#include "JITNegGenerator.h"
#include "JITOperations.h"
#include "JITRightShiftGenerator.h"
#include "JITSubGenerator.h"
#include "JSArray.h"
#include "JSFunction.h"
#include "Interpreter.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"
#include "ResultType.h"
#include "SlowPathCall.h"

namespace JSC {

void JIT::emit_op_jless(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jless, op1, op2, target, LessThan);
}

void JIT::emit_op_jlesseq(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jlesseq, op1, op2, target, LessThanOrEqual);
}

void JIT::emit_op_jgreater(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jgreater, op1, op2, target, GreaterThan);
}

void JIT::emit_op_jgreatereq(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jgreatereq, op1, op2, target, GreaterThanOrEqual);
}

void JIT::emit_op_jnless(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jnless, op1, op2, target, GreaterThanOrEqual);
}

void JIT::emit_op_jnlesseq(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jnlesseq, op1, op2, target, GreaterThan);
}

void JIT::emit_op_jngreater(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jngreater, op1, op2, target, LessThanOrEqual);
}

void JIT::emit_op_jngreatereq(Instruction* currentInstruction)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJump(op_jngreatereq, op1, op2, target, LessThan);
}

void JIT::emitSlow_op_jless(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleLessThan, operationCompareLess, false, iter);
}

void JIT::emitSlow_op_jlesseq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleLessThanOrEqual, operationCompareLessEq, false, iter);
}

void JIT::emitSlow_op_jgreater(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleGreaterThan, operationCompareGreater, false, iter);
}

void JIT::emitSlow_op_jgreatereq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleGreaterThanOrEqual, operationCompareGreaterEq, false, iter);
}

void JIT::emitSlow_op_jnless(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleGreaterThanOrEqualOrUnordered, operationCompareLess, true, iter);
}

void JIT::emitSlow_op_jnlesseq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleGreaterThanOrUnordered, operationCompareLessEq, true, iter);
}

void JIT::emitSlow_op_jngreater(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleLessThanOrEqualOrUnordered, operationCompareGreater, true, iter);
}

void JIT::emitSlow_op_jngreatereq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int op1 = currentInstruction[1].u.operand;
    int op2 = currentInstruction[2].u.operand;
    unsigned target = currentInstruction[3].u.operand;

    emit_compareAndJumpSlow(op1, op2, target, DoubleLessThanOrUnordered, operationCompareGreaterEq, true, iter);
}

#if USE(JSVALUE64)

void JIT::emit_op_unsigned(Instruction* currentInstruction)
{
    int result = currentInstruction[1].u.operand;
    int op1 = currentInstruction[2].u.operand;
    
    emitGetVirtualRegister(op1, regT0);
    emitJumpSlowCaseIfNotInt(regT0);
    addSlowCase(branch32(LessThan, regT0, TrustedImm32(0)));
    emitTagInt(regT0, regT0);
    emitPutVirtualRegister(result, regT0);
}

void JIT::emitSlow_op_unsigned(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_unsigned);
    slowPathCall.call();
}

void JIT::emit_compareAndJump(OpcodeID, int op1, int op2, unsigned target, RelationalCondition condition)
{
    // We generate inline code for the following cases in the fast path:
    // - int immediate to constant int immediate
    // - constant int immediate to int immediate
    // - int immediate to int immediate

    if (isOperandConstantChar(op1)) {
        emitGetVirtualRegister(op2, regT0);
        addSlowCase(emitJumpIfNotJSCell(regT0));
        JumpList failures;
        emitLoadCharacterString(regT0, regT0, failures);
        addSlowCase(failures);
        addJump(branch32(commute(condition), regT0, Imm32(asString(getConstantOperand(op1))->tryGetValue()[0])), target);
        return;
    }
    if (isOperandConstantChar(op2)) {
        emitGetVirtualRegister(op1, regT0);
        addSlowCase(emitJumpIfNotJSCell(regT0));
        JumpList failures;
        emitLoadCharacterString(regT0, regT0, failures);
        addSlowCase(failures);
        addJump(branch32(condition, regT0, Imm32(asString(getConstantOperand(op2))->tryGetValue()[0])), target);
        return;
    }
    if (isOperandConstantInt(op2)) {
        emitGetVirtualRegister(op1, regT0);
        emitJumpSlowCaseIfNotInt(regT0);
        int32_t op2imm = getOperandConstantInt(op2);
        addJump(branch32(condition, regT0, Imm32(op2imm)), target);
    } else if (isOperandConstantInt(op1)) {
        emitGetVirtualRegister(op2, regT1);
        emitJumpSlowCaseIfNotInt(regT1);
        int32_t op1imm = getOperandConstantInt(op1);
        addJump(branch32(commute(condition), regT1, Imm32(op1imm)), target);
    } else {
        emitGetVirtualRegisters(op1, regT0, op2, regT1);
        emitJumpSlowCaseIfNotInt(regT0);
        emitJumpSlowCaseIfNotInt(regT1);

        addJump(branch32(condition, regT0, regT1), target);
    }
}

void JIT::emit_compareAndJumpSlow(int op1, int op2, unsigned target, DoubleCondition condition, size_t (JIT_OPERATION *operation)(ExecState*, EncodedJSValue, EncodedJSValue), bool invert, Vector<SlowCaseEntry>::iterator& iter)
{
    COMPILE_ASSERT(OPCODE_LENGTH(op_jless) == OPCODE_LENGTH(op_jlesseq), OPCODE_LENGTH_op_jlesseq_equals_op_jless);
    COMPILE_ASSERT(OPCODE_LENGTH(op_jless) == OPCODE_LENGTH(op_jnless), OPCODE_LENGTH_op_jnless_equals_op_jless);
    COMPILE_ASSERT(OPCODE_LENGTH(op_jless) == OPCODE_LENGTH(op_jnlesseq), OPCODE_LENGTH_op_jnlesseq_equals_op_jless);
    COMPILE_ASSERT(OPCODE_LENGTH(op_jless) == OPCODE_LENGTH(op_jgreater), OPCODE_LENGTH_op_jgreater_equals_op_jless);
    COMPILE_ASSERT(OPCODE_LENGTH(op_jless) == OPCODE_LENGTH(op_jgreatereq), OPCODE_LENGTH_op_jgreatereq_equals_op_jless);
    COMPILE_ASSERT(OPCODE_LENGTH(op_jless) == OPCODE_LENGTH(op_jngreater), OPCODE_LENGTH_op_jngreater_equals_op_jless);
    COMPILE_ASSERT(OPCODE_LENGTH(op_jless) == OPCODE_LENGTH(op_jngreatereq), OPCODE_LENGTH_op_jngreatereq_equals_op_jless);
    
    // We generate inline code for the following cases in the slow path:
    // - floating-point number to constant int immediate
    // - constant int immediate to floating-point number
    // - floating-point number to floating-point number.
    if (isOperandConstantChar(op1) || isOperandConstantChar(op2)) {
        linkSlowCase(iter);
        linkSlowCase(iter);
        linkSlowCase(iter);
        linkSlowCase(iter);

        emitGetVirtualRegister(op1, argumentGPR0);
        emitGetVirtualRegister(op2, argumentGPR1);
        callOperation(operation, argumentGPR0, argumentGPR1);
        emitJumpSlowToHot(branchTest32(invert ? Zero : NonZero, returnValueGPR), target);
        return;
    }

    if (isOperandConstantInt(op2)) {
        linkSlowCase(iter);

        if (supportsFloatingPoint()) {
            Jump fail1 = emitJumpIfNotNumber(regT0);
            add64(tagTypeNumberRegister, regT0);
            move64ToDouble(regT0, fpRegT0);

            int32_t op2imm = getConstantOperand(op2).asInt32();

            move(Imm32(op2imm), regT1);
            convertInt32ToDouble(regT1, fpRegT1);

            emitJumpSlowToHot(branchDouble(condition, fpRegT0, fpRegT1), target);

            emitJumpSlowToHot(jump(), OPCODE_LENGTH(op_jless));

            fail1.link(this);
        }

        emitGetVirtualRegister(op2, regT1);
        callOperation(operation, regT0, regT1);
        emitJumpSlowToHot(branchTest32(invert ? Zero : NonZero, returnValueGPR), target);
    } else if (isOperandConstantInt(op1)) {
        linkSlowCase(iter);

        if (supportsFloatingPoint()) {
            Jump fail1 = emitJumpIfNotNumber(regT1);
            add64(tagTypeNumberRegister, regT1);
            move64ToDouble(regT1, fpRegT1);

            int32_t op1imm = getConstantOperand(op1).asInt32();

            move(Imm32(op1imm), regT0);
            convertInt32ToDouble(regT0, fpRegT0);

            emitJumpSlowToHot(branchDouble(condition, fpRegT0, fpRegT1), target);

            emitJumpSlowToHot(jump(), OPCODE_LENGTH(op_jless));

            fail1.link(this);
        }

        emitGetVirtualRegister(op1, regT2);
        callOperation(operation, regT2, regT1);
        emitJumpSlowToHot(branchTest32(invert ? Zero : NonZero, returnValueGPR), target);
    } else {
        linkSlowCase(iter);

        if (supportsFloatingPoint()) {
            Jump fail1 = emitJumpIfNotNumber(regT0);
            Jump fail2 = emitJumpIfNotNumber(regT1);
            Jump fail3 = emitJumpIfInt(regT1);
            add64(tagTypeNumberRegister, regT0);
            add64(tagTypeNumberRegister, regT1);
            move64ToDouble(regT0, fpRegT0);
            move64ToDouble(regT1, fpRegT1);

            emitJumpSlowToHot(branchDouble(condition, fpRegT0, fpRegT1), target);

            emitJumpSlowToHot(jump(), OPCODE_LENGTH(op_jless));

            fail1.link(this);
            fail2.link(this);
            fail3.link(this);
        }

        linkSlowCase(iter);
        callOperation(operation, regT0, regT1);
        emitJumpSlowToHot(branchTest32(invert ? Zero : NonZero, returnValueGPR), target);
    }
}

void JIT::emit_op_inc(Instruction* currentInstruction)
{
    int srcDst = currentInstruction[1].u.operand;

    emitGetVirtualRegister(srcDst, regT0);
    emitJumpSlowCaseIfNotInt(regT0);
    addSlowCase(branchAdd32(Overflow, TrustedImm32(1), regT0));
    emitTagInt(regT0, regT0);
    emitPutVirtualRegister(srcDst);
}

void JIT::emitSlow_op_inc(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_inc);
    slowPathCall.call();
}

void JIT::emit_op_dec(Instruction* currentInstruction)
{
    int srcDst = currentInstruction[1].u.operand;

    emitGetVirtualRegister(srcDst, regT0);
    emitJumpSlowCaseIfNotInt(regT0);
    addSlowCase(branchSub32(Overflow, TrustedImm32(1), regT0));
    emitTagInt(regT0, regT0);
    emitPutVirtualRegister(srcDst);
}

void JIT::emitSlow_op_dec(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_dec);
    slowPathCall.call();
}

/* ------------------------------ BEGIN: OP_MOD ------------------------------ */

#if CPU(X86) || CPU(X86_64)

void JIT::emit_op_mod(Instruction* currentInstruction)
{
    int result = currentInstruction[1].u.operand;
    int op1 = currentInstruction[2].u.operand;
    int op2 = currentInstruction[3].u.operand;

    // Make sure registers are correct for x86 IDIV instructions.
    ASSERT(regT0 == X86Registers::eax);
    auto edx = X86Registers::edx;
    auto ecx = X86Registers::ecx;
    ASSERT(regT4 != edx);
    ASSERT(regT4 != ecx);

    emitGetVirtualRegisters(op1, regT4, op2, ecx);
    emitJumpSlowCaseIfNotInt(regT4);
    emitJumpSlowCaseIfNotInt(ecx);

    move(regT4, regT0);
    addSlowCase(branchTest32(Zero, ecx));
    Jump denominatorNotNeg1 = branch32(NotEqual, ecx, TrustedImm32(-1));
    addSlowCase(branch32(Equal, regT0, TrustedImm32(-2147483647-1)));
    denominatorNotNeg1.link(this);
    x86ConvertToDoubleWord32();
    x86Div32(ecx);
    Jump numeratorPositive = branch32(GreaterThanOrEqual, regT4, TrustedImm32(0));
    addSlowCase(branchTest32(Zero, edx));
    numeratorPositive.link(this);
    emitTagInt(edx, regT0);
    emitPutVirtualRegister(result);
}

void JIT::emitSlow_op_mod(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_mod);
    slowPathCall.call();
}

#else // CPU(X86) || CPU(X86_64)

void JIT::emit_op_mod(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_mod);
    slowPathCall.call();
}

void JIT::emitSlow_op_mod(Instruction*, Vector<SlowCaseEntry>::iterator&)
{
    UNREACHABLE_FOR_PLATFORM();
}

#endif // CPU(X86) || CPU(X86_64)

/* ------------------------------ END: OP_MOD ------------------------------ */

#endif // USE(JSVALUE64)

void JIT::emit_op_negate(Instruction* currentInstruction)
{
    ArithProfile* arithProfile = m_codeBlock->arithProfileForPC(currentInstruction);
    JITNegIC* negateIC = m_codeBlock->addJITNegIC(arithProfile);
    m_instructionToMathIC.add(currentInstruction, negateIC);
    emitMathICFast(negateIC, currentInstruction, operationArithNegateProfiled, operationArithNegate);
}

void JIT::emitSlow_op_negate(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITNegIC* negIC = bitwise_cast<JITNegIC*>(m_instructionToMathIC.get(currentInstruction));
    emitMathICSlow(negIC, currentInstruction, operationArithNegateProfiledOptimize, operationArithNegateProfiled, operationArithNegateOptimize);
}

template<typename SnippetGenerator>
void JIT::emitBitBinaryOpFastPath(Instruction* currentInstruction)
{
    int result = currentInstruction[1].u.operand;
    int op1 = currentInstruction[2].u.operand;
    int op2 = currentInstruction[3].u.operand;

#if USE(JSVALUE64)
    JSValueRegs leftRegs = JSValueRegs(regT0);
    JSValueRegs rightRegs = JSValueRegs(regT1);
    JSValueRegs resultRegs = leftRegs;
    GPRReg scratchGPR = regT2;
#else
    JSValueRegs leftRegs = JSValueRegs(regT1, regT0);
    JSValueRegs rightRegs = JSValueRegs(regT3, regT2);
    JSValueRegs resultRegs = leftRegs;
    GPRReg scratchGPR = regT4;
#endif

    SnippetOperand leftOperand;
    SnippetOperand rightOperand;

    if (isOperandConstantInt(op1))
        leftOperand.setConstInt32(getOperandConstantInt(op1));
    else if (isOperandConstantInt(op2))
        rightOperand.setConstInt32(getOperandConstantInt(op2));

    RELEASE_ASSERT(!leftOperand.isConst() || !rightOperand.isConst());

    if (!leftOperand.isConst())
        emitGetVirtualRegister(op1, leftRegs);
    if (!rightOperand.isConst())
        emitGetVirtualRegister(op2, rightRegs);

    SnippetGenerator gen(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs, scratchGPR);

    gen.generateFastPath(*this);

    ASSERT(gen.didEmitFastPath());
    gen.endJumpList().link(this);
    emitPutVirtualRegister(result, resultRegs);

    addSlowCase(gen.slowPathJumpList());
}

void JIT::emit_op_bitand(Instruction* currentInstruction)
{
    emitBitBinaryOpFastPath<JITBitAndGenerator>(currentInstruction);
}

void JIT::emitSlow_op_bitand(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_bitand);
    slowPathCall.call();
}

void JIT::emit_op_bitor(Instruction* currentInstruction)
{
    emitBitBinaryOpFastPath<JITBitOrGenerator>(currentInstruction);
}

void JIT::emitSlow_op_bitor(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_bitor);
    slowPathCall.call();
}

void JIT::emit_op_bitxor(Instruction* currentInstruction)
{
    emitBitBinaryOpFastPath<JITBitXorGenerator>(currentInstruction);
}

void JIT::emitSlow_op_bitxor(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_bitxor);
    slowPathCall.call();
}

void JIT::emit_op_lshift(Instruction* currentInstruction)
{
    emitBitBinaryOpFastPath<JITLeftShiftGenerator>(currentInstruction);
}

void JIT::emitSlow_op_lshift(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_lshift);
    slowPathCall.call();
}

void JIT::emitRightShiftFastPath(Instruction* currentInstruction, OpcodeID opcodeID)
{
    ASSERT(opcodeID == op_rshift || opcodeID == op_urshift);

    JITRightShiftGenerator::ShiftType snippetShiftType = opcodeID == op_rshift ?
        JITRightShiftGenerator::SignedShift : JITRightShiftGenerator::UnsignedShift;

    int result = currentInstruction[1].u.operand;
    int op1 = currentInstruction[2].u.operand;
    int op2 = currentInstruction[3].u.operand;

#if USE(JSVALUE64)
    JSValueRegs leftRegs = JSValueRegs(regT0);
    JSValueRegs rightRegs = JSValueRegs(regT1);
    JSValueRegs resultRegs = leftRegs;
    GPRReg scratchGPR = regT2;
    FPRReg scratchFPR = InvalidFPRReg;
#else
    JSValueRegs leftRegs = JSValueRegs(regT1, regT0);
    JSValueRegs rightRegs = JSValueRegs(regT3, regT2);
    JSValueRegs resultRegs = leftRegs;
    GPRReg scratchGPR = regT4;
    FPRReg scratchFPR = fpRegT2;
#endif

    SnippetOperand leftOperand;
    SnippetOperand rightOperand;

    if (isOperandConstantInt(op1))
        leftOperand.setConstInt32(getOperandConstantInt(op1));
    else if (isOperandConstantInt(op2))
        rightOperand.setConstInt32(getOperandConstantInt(op2));

    RELEASE_ASSERT(!leftOperand.isConst() || !rightOperand.isConst());

    if (!leftOperand.isConst())
        emitGetVirtualRegister(op1, leftRegs);
    if (!rightOperand.isConst())
        emitGetVirtualRegister(op2, rightRegs);

    JITRightShiftGenerator gen(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs,
        fpRegT0, scratchGPR, scratchFPR, snippetShiftType);

    gen.generateFastPath(*this);

    ASSERT(gen.didEmitFastPath());
    gen.endJumpList().link(this);
    emitPutVirtualRegister(result, resultRegs);

    addSlowCase(gen.slowPathJumpList());
}

void JIT::emit_op_rshift(Instruction* currentInstruction)
{
    emitRightShiftFastPath(currentInstruction, op_rshift);
}

void JIT::emitSlow_op_rshift(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_rshift);
    slowPathCall.call();
}

void JIT::emit_op_urshift(Instruction* currentInstruction)
{
    emitRightShiftFastPath(currentInstruction, op_urshift);
}

void JIT::emitSlow_op_urshift(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_urshift);
    slowPathCall.call();
}

ALWAYS_INLINE static OperandTypes getOperandTypes(Instruction* instruction)
{
    return OperandTypes(ArithProfile::fromInt(instruction[4].u.operand).lhsResultType(), ArithProfile::fromInt(instruction[4].u.operand).rhsResultType());
}

void JIT::emit_op_add(Instruction* currentInstruction)
{
    ArithProfile* arithProfile = m_codeBlock->arithProfileForPC(currentInstruction);
    JITAddIC* addIC = m_codeBlock->addJITAddIC(arithProfile);
    m_instructionToMathIC.add(currentInstruction, addIC);
    emitMathICFast(addIC, currentInstruction, operationValueAddProfiled, operationValueAdd);
}

void JIT::emitSlow_op_add(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITAddIC* addIC = bitwise_cast<JITAddIC*>(m_instructionToMathIC.get(currentInstruction));
    emitMathICSlow(addIC, currentInstruction, operationValueAddProfiledOptimize, operationValueAddProfiled, operationValueAddOptimize);
}

template <typename Generator, typename ProfiledFunction, typename NonProfiledFunction>
void JIT::emitMathICFast(JITUnaryMathIC<Generator>* mathIC, Instruction* currentInstruction, ProfiledFunction profiledFunction, NonProfiledFunction nonProfiledFunction)
{
    int result = currentInstruction[1].u.operand;
    int operand = currentInstruction[2].u.operand;

#if USE(JSVALUE64)
    // ArithNegate benefits from using the same register as src and dst.
    // Since regT1==argumentGPR1, using regT1 avoid shuffling register to call the slow path.
    JSValueRegs srcRegs = JSValueRegs(regT1);
    JSValueRegs resultRegs = JSValueRegs(regT1);
    GPRReg scratchGPR = regT2;
#else
    JSValueRegs srcRegs = JSValueRegs(regT1, regT0);
    JSValueRegs resultRegs = JSValueRegs(regT3, regT2);
    GPRReg scratchGPR = regT4;
#endif

#if ENABLE(MATH_IC_STATS)
    auto inlineStart = label();
#endif

    mathIC->m_generator = Generator(resultRegs, srcRegs, scratchGPR);

    emitGetVirtualRegister(operand, srcRegs);

    MathICGenerationState& mathICGenerationState = m_instructionToMathICGenerationState.add(currentInstruction, MathICGenerationState()).iterator->value;

    bool generatedInlineCode = mathIC->generateInline(*this, mathICGenerationState);
    if (!generatedInlineCode) {
        ArithProfile* arithProfile = mathIC->arithProfile();
        if (arithProfile && shouldEmitProfiling())
            callOperation(profiledFunction, resultRegs, srcRegs, arithProfile);
        else
            callOperation(nonProfiledFunction, resultRegs, srcRegs);
    } else
        addSlowCase(mathICGenerationState.slowPathJumps);

#if ENABLE(MATH_IC_STATS)
    auto inlineEnd = label();
    addLinkTask([=] (LinkBuffer& linkBuffer) {
        size_t size = static_cast<char*>(linkBuffer.locationOf(inlineEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(inlineStart).executableAddress());
        mathIC->m_generatedCodeSize += size;
    });
#endif

    emitPutVirtualRegister(result, resultRegs);
}

template <typename Generator, typename ProfiledFunction, typename NonProfiledFunction>
void JIT::emitMathICFast(JITBinaryMathIC<Generator>* mathIC, Instruction* currentInstruction, ProfiledFunction profiledFunction, NonProfiledFunction nonProfiledFunction)
{
    int result = currentInstruction[1].u.operand;
    int op1 = currentInstruction[2].u.operand;
    int op2 = currentInstruction[3].u.operand;

#if USE(JSVALUE64)
    OperandTypes types = getOperandTypes(copiedInstruction(currentInstruction));
    JSValueRegs leftRegs = JSValueRegs(regT1);
    JSValueRegs rightRegs = JSValueRegs(regT2);
    JSValueRegs resultRegs = JSValueRegs(regT0);
    GPRReg scratchGPR = regT3;
    FPRReg scratchFPR = fpRegT2;
#else
    OperandTypes types = getOperandTypes(currentInstruction);
    JSValueRegs leftRegs = JSValueRegs(regT1, regT0);
    JSValueRegs rightRegs = JSValueRegs(regT3, regT2);
    JSValueRegs resultRegs = leftRegs;
    GPRReg scratchGPR = regT4;
    FPRReg scratchFPR = fpRegT2;
#endif

    SnippetOperand leftOperand(types.first());
    SnippetOperand rightOperand(types.second());

    if (isOperandConstantInt(op1))
        leftOperand.setConstInt32(getOperandConstantInt(op1));
    else if (isOperandConstantInt(op2))
        rightOperand.setConstInt32(getOperandConstantInt(op2));

    RELEASE_ASSERT(!leftOperand.isConst() || !rightOperand.isConst());

    mathIC->m_generator = Generator(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs, fpRegT0, fpRegT1, scratchGPR, scratchFPR);
    
    ASSERT(!(Generator::isLeftOperandValidConstant(leftOperand) && Generator::isRightOperandValidConstant(rightOperand)));
    
    if (!Generator::isLeftOperandValidConstant(leftOperand))
        emitGetVirtualRegister(op1, leftRegs);
    if (!Generator::isRightOperandValidConstant(rightOperand))
        emitGetVirtualRegister(op2, rightRegs);

#if ENABLE(MATH_IC_STATS)
    auto inlineStart = label();
#endif

    MathICGenerationState& mathICGenerationState = m_instructionToMathICGenerationState.add(currentInstruction, MathICGenerationState()).iterator->value;

    bool generatedInlineCode = mathIC->generateInline(*this, mathICGenerationState);
    if (!generatedInlineCode) {
        if (leftOperand.isConst())
            emitGetVirtualRegister(op1, leftRegs);
        else if (rightOperand.isConst())
            emitGetVirtualRegister(op2, rightRegs);
        ArithProfile* arithProfile = mathIC->arithProfile();
        if (arithProfile && shouldEmitProfiling())
            callOperation(profiledFunction, resultRegs, leftRegs, rightRegs, arithProfile);
        else
            callOperation(nonProfiledFunction, resultRegs, leftRegs, rightRegs);
    } else
        addSlowCase(mathICGenerationState.slowPathJumps);

#if ENABLE(MATH_IC_STATS)
    auto inlineEnd = label();
    addLinkTask([=] (LinkBuffer& linkBuffer) {
        size_t size = static_cast<char*>(linkBuffer.locationOf(inlineEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(inlineStart).executableAddress());
        mathIC->m_generatedCodeSize += size;
    });
#endif

    emitPutVirtualRegister(result, resultRegs);
}

template <typename Generator, typename ProfiledRepatchFunction, typename ProfiledFunction, typename RepatchFunction>
void JIT::emitMathICSlow(JITUnaryMathIC<Generator>* mathIC, Instruction* currentInstruction, ProfiledRepatchFunction profiledRepatchFunction, ProfiledFunction profiledFunction, RepatchFunction repatchFunction)
{
    MathICGenerationState& mathICGenerationState = m_instructionToMathICGenerationState.find(currentInstruction)->value;
    mathICGenerationState.slowPathStart = label();

    int result = currentInstruction[1].u.operand;

#if USE(JSVALUE64)
    JSValueRegs srcRegs = JSValueRegs(regT1);
    JSValueRegs resultRegs = JSValueRegs(regT0);
#else
    JSValueRegs srcRegs = JSValueRegs(regT1, regT0);
    JSValueRegs resultRegs = JSValueRegs(regT3, regT2);
#endif

#if ENABLE(MATH_IC_STATS)
    auto slowPathStart = label();
#endif

    ArithProfile* arithProfile = mathIC->arithProfile();
    if (arithProfile && shouldEmitProfiling()) {
        if (mathICGenerationState.shouldSlowPathRepatch)
            mathICGenerationState.slowPathCall = callOperation(reinterpret_cast<J_JITOperation_EJMic>(profiledRepatchFunction), resultRegs, srcRegs, TrustedImmPtr(mathIC));
        else
            mathICGenerationState.slowPathCall = callOperation(profiledFunction, resultRegs, srcRegs, arithProfile);
    } else
        mathICGenerationState.slowPathCall = callOperation(reinterpret_cast<J_JITOperation_EJMic>(repatchFunction), resultRegs, srcRegs, TrustedImmPtr(mathIC));

#if ENABLE(MATH_IC_STATS)
    auto slowPathEnd = label();
    addLinkTask([=] (LinkBuffer& linkBuffer) {
        size_t size = static_cast<char*>(linkBuffer.locationOf(slowPathEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(slowPathStart).executableAddress());
        mathIC->m_generatedCodeSize += size;
    });
#endif

    emitPutVirtualRegister(result, resultRegs);

    addLinkTask([=] (LinkBuffer& linkBuffer) {
        MathICGenerationState& mathICGenerationState = m_instructionToMathICGenerationState.find(currentInstruction)->value;
        mathIC->finalizeInlineCode(mathICGenerationState, linkBuffer);
    });
}

template <typename Generator, typename ProfiledRepatchFunction, typename ProfiledFunction, typename RepatchFunction>
void JIT::emitMathICSlow(JITBinaryMathIC<Generator>* mathIC, Instruction* currentInstruction, ProfiledRepatchFunction profiledRepatchFunction, ProfiledFunction profiledFunction, RepatchFunction repatchFunction)
{
    MathICGenerationState& mathICGenerationState = m_instructionToMathICGenerationState.find(currentInstruction)->value;
    mathICGenerationState.slowPathStart = label();

    int result = currentInstruction[1].u.operand;
    int op1 = currentInstruction[2].u.operand;
    int op2 = currentInstruction[3].u.operand;

#if USE(JSVALUE64)
    OperandTypes types = getOperandTypes(copiedInstruction(currentInstruction));
    JSValueRegs leftRegs = JSValueRegs(regT1);
    JSValueRegs rightRegs = JSValueRegs(regT2);
    JSValueRegs resultRegs = JSValueRegs(regT0);
#else
    OperandTypes types = getOperandTypes(currentInstruction);
    JSValueRegs leftRegs = JSValueRegs(regT1, regT0);
    JSValueRegs rightRegs = JSValueRegs(regT3, regT2);
    JSValueRegs resultRegs = leftRegs;
#endif
    
    SnippetOperand leftOperand(types.first());
    SnippetOperand rightOperand(types.second());

    if (isOperandConstantInt(op1))
        leftOperand.setConstInt32(getOperandConstantInt(op1));
    else if (isOperandConstantInt(op2))
        rightOperand.setConstInt32(getOperandConstantInt(op2));

    ASSERT(!(Generator::isLeftOperandValidConstant(leftOperand) && Generator::isRightOperandValidConstant(rightOperand)));

    if (Generator::isLeftOperandValidConstant(leftOperand))
        emitGetVirtualRegister(op1, leftRegs);
    else if (Generator::isRightOperandValidConstant(rightOperand))
        emitGetVirtualRegister(op2, rightRegs);

#if ENABLE(MATH_IC_STATS)
    auto slowPathStart = label();
#endif

    ArithProfile* arithProfile = mathIC->arithProfile();
    if (arithProfile && shouldEmitProfiling()) {
        if (mathICGenerationState.shouldSlowPathRepatch)
            mathICGenerationState.slowPathCall = callOperation(bitwise_cast<J_JITOperation_EJJMic>(profiledRepatchFunction), resultRegs, leftRegs, rightRegs, TrustedImmPtr(mathIC));
        else
            mathICGenerationState.slowPathCall = callOperation(profiledFunction, resultRegs, leftRegs, rightRegs, arithProfile);
    } else
        mathICGenerationState.slowPathCall = callOperation(bitwise_cast<J_JITOperation_EJJMic>(repatchFunction), resultRegs, leftRegs, rightRegs, TrustedImmPtr(mathIC));

#if ENABLE(MATH_IC_STATS)
    auto slowPathEnd = label();
    addLinkTask([=] (LinkBuffer& linkBuffer) {
        size_t size = static_cast<char*>(linkBuffer.locationOf(slowPathEnd).executableAddress()) - static_cast<char*>(linkBuffer.locationOf(slowPathStart).executableAddress());
        mathIC->m_generatedCodeSize += size;
    });
#endif

    emitPutVirtualRegister(result, resultRegs);

    addLinkTask([=] (LinkBuffer& linkBuffer) {
        MathICGenerationState& mathICGenerationState = m_instructionToMathICGenerationState.find(currentInstruction)->value;
        mathIC->finalizeInlineCode(mathICGenerationState, linkBuffer);
    });
}

void JIT::emit_op_div(Instruction* currentInstruction)
{
    int result = currentInstruction[1].u.operand;
    int op1 = currentInstruction[2].u.operand;
    int op2 = currentInstruction[3].u.operand;

#if USE(JSVALUE64)
    OperandTypes types = getOperandTypes(copiedInstruction(currentInstruction));
    JSValueRegs leftRegs = JSValueRegs(regT0);
    JSValueRegs rightRegs = JSValueRegs(regT1);
    JSValueRegs resultRegs = leftRegs;
    GPRReg scratchGPR = regT2;
#else
    OperandTypes types = getOperandTypes(currentInstruction);
    JSValueRegs leftRegs = JSValueRegs(regT1, regT0);
    JSValueRegs rightRegs = JSValueRegs(regT3, regT2);
    JSValueRegs resultRegs = leftRegs;
    GPRReg scratchGPR = regT4;
#endif
    FPRReg scratchFPR = fpRegT2;

    ArithProfile* arithProfile = nullptr;
    if (shouldEmitProfiling())
        arithProfile = m_codeBlock->arithProfileForPC(currentInstruction);

    SnippetOperand leftOperand(types.first());
    SnippetOperand rightOperand(types.second());

    if (isOperandConstantInt(op1))
        leftOperand.setConstInt32(getOperandConstantInt(op1));
#if USE(JSVALUE64)
    else if (isOperandConstantDouble(op1))
        leftOperand.setConstDouble(getOperandConstantDouble(op1));
#endif
    else if (isOperandConstantInt(op2))
        rightOperand.setConstInt32(getOperandConstantInt(op2));
#if USE(JSVALUE64)
    else if (isOperandConstantDouble(op2))
        rightOperand.setConstDouble(getOperandConstantDouble(op2));
#endif

    RELEASE_ASSERT(!leftOperand.isConst() || !rightOperand.isConst());

    if (!leftOperand.isConst())
        emitGetVirtualRegister(op1, leftRegs);
    if (!rightOperand.isConst())
        emitGetVirtualRegister(op2, rightRegs);

    JITDivGenerator gen(leftOperand, rightOperand, resultRegs, leftRegs, rightRegs,
        fpRegT0, fpRegT1, scratchGPR, scratchFPR, arithProfile);

    gen.generateFastPath(*this);

    if (gen.didEmitFastPath()) {
        gen.endJumpList().link(this);
        emitPutVirtualRegister(result, resultRegs);

        addSlowCase(gen.slowPathJumpList());
    } else {
        ASSERT(gen.endJumpList().empty());
        ASSERT(gen.slowPathJumpList().empty());
        JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_div);
        slowPathCall.call();
    }
}

void JIT::emitSlow_op_div(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_div);
    slowPathCall.call();
}

void JIT::emit_op_mul(Instruction* currentInstruction)
{
    ArithProfile* arithProfile = m_codeBlock->arithProfileForPC(currentInstruction);
    JITMulIC* mulIC = m_codeBlock->addJITMulIC(arithProfile);
    m_instructionToMathIC.add(currentInstruction, mulIC);
    emitMathICFast(mulIC, currentInstruction, operationValueMulProfiled, operationValueMul);
}

void JIT::emitSlow_op_mul(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);

    JITMulIC* mulIC = bitwise_cast<JITMulIC*>(m_instructionToMathIC.get(currentInstruction));
    emitMathICSlow(mulIC, currentInstruction, operationValueMulProfiledOptimize, operationValueMulProfiled, operationValueMulOptimize);
}

void JIT::emit_op_sub(Instruction* currentInstruction)
{
    ArithProfile* arithProfile = m_codeBlock->arithProfileForPC(currentInstruction);
    JITSubIC* subIC = m_codeBlock->addJITSubIC(arithProfile);
    m_instructionToMathIC.add(currentInstruction, subIC);
    emitMathICFast(subIC, currentInstruction, operationValueSubProfiled, operationValueSub);
}

void JIT::emitSlow_op_sub(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkAllSlowCasesForBytecodeOffset(m_slowCases, iter, m_bytecodeOffset);
    JITSubIC* subIC = bitwise_cast<JITSubIC*>(m_instructionToMathIC.get(currentInstruction));
    emitMathICSlow(subIC, currentInstruction, operationValueSubProfiledOptimize, operationValueSubProfiled, operationValueSubOptimize);
}

void JIT::emit_op_pow(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_pow);
    slowPathCall.call();
}

/* ------------------------------ END: OP_ADD, OP_SUB, OP_MUL, OP_POW ------------------------------ */

} // namespace JSC

#endif // ENABLE(JIT)
