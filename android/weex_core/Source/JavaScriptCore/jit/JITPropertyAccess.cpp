/*
 * Copyright (C) 2008, 2009, 2014, 2015 Apple Inc. All rights reserved.
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

#include "CodeBlock.h"
#include "DirectArguments.h"
#include "GCAwareJITStubRoutine.h"
#include "GetterSetter.h"
#include "Interpreter.h"
#include "JITInlines.h"
#include "JSArray.h"
#include "JSEnvironmentRecord.h"
#include "JSFunction.h"
#include "LinkBuffer.h"
#include "ResultType.h"
#include "ScopedArguments.h"
#include "ScopedArgumentsTable.h"
#include "SlowPathCall.h"
#include "StructureStubInfo.h"
#include <wtf/StringPrintStream.h>


namespace JSC {
#if USE(JSVALUE64)

JIT::CodeRef JIT::stringGetByValStubGenerator(VM* vm)
{
    JSInterfaceJIT jit(vm);
    JumpList failures;
    failures.append(jit.branchStructure(
        NotEqual, 
        Address(regT0, JSCell::structureIDOffset()), 
        vm->stringStructure.get()));

    // Load string length to regT2, and start the process of loading the data pointer into regT0
    jit.load32(Address(regT0, ThunkHelpers::jsStringLengthOffset()), regT2);
    jit.loadPtr(Address(regT0, ThunkHelpers::jsStringValueOffset()), regT0);
    failures.append(jit.branchTest32(Zero, regT0));

    // Do an unsigned compare to simultaneously filter negative indices as well as indices that are too large
    failures.append(jit.branch32(AboveOrEqual, regT1, regT2));
    
    // Load the character
    JumpList is16Bit;
    JumpList cont8Bit;
    // Load the string flags
    jit.loadPtr(Address(regT0, StringImpl::flagsOffset()), regT2);
    jit.loadPtr(Address(regT0, StringImpl::dataOffset()), regT0);
    is16Bit.append(jit.branchTest32(Zero, regT2, TrustedImm32(StringImpl::flagIs8Bit())));
    jit.load8(BaseIndex(regT0, regT1, TimesOne, 0), regT0);
    cont8Bit.append(jit.jump());
    is16Bit.link(&jit);
    jit.load16(BaseIndex(regT0, regT1, TimesTwo, 0), regT0);
    cont8Bit.link(&jit);

    failures.append(jit.branch32(AboveOrEqual, regT0, TrustedImm32(0x100)));
    jit.move(TrustedImmPtr(vm->smallStrings.singleCharacterStrings()), regT1);
    jit.loadPtr(BaseIndex(regT1, regT0, ScalePtr, 0), regT0);
    jit.ret();
    
    failures.link(&jit);
    jit.move(TrustedImm32(0), regT0);
    jit.ret();
    
    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("String get_by_val stub"));
}

void JIT::emit_op_get_by_val(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int property = currentInstruction[3].u.operand;
    ArrayProfile* profile = currentInstruction[4].u.arrayProfile;
    ByValInfo* byValInfo = m_codeBlock->addByValInfo();

    emitGetVirtualRegister(base, regT0);
    bool propertyNameIsIntegerConstant = isOperandConstantInt(property);
    if (propertyNameIsIntegerConstant)
        move(Imm32(getOperandConstantInt(property)), regT1);
    else
        emitGetVirtualRegister(property, regT1);

    emitJumpSlowCaseIfNotJSCell(regT0, base);

    PatchableJump notIndex;
    if (!propertyNameIsIntegerConstant) {
        notIndex = emitPatchableJumpIfNotInt(regT1);
        addSlowCase(notIndex);

        // This is technically incorrect - we're zero-extending an int32. On the hot path this doesn't matter.
        // We check the value as if it was a uint32 against the m_vectorLength - which will always fail if
        // number was signed since m_vectorLength is always less than intmax (since the total allocation
        // size is always less than 4Gb). As such zero extending will have been correct (and extending the value
        // to 64-bits is necessary since it's used in the address calculation). We zero extend rather than sign
        // extending since it makes it easier to re-tag the value in the slow case.
        zeroExtend32ToPtr(regT1, regT1);
    }

    emitArrayProfilingSiteWithCell(regT0, regT2, profile);
    and32(TrustedImm32(IndexingShapeMask), regT2);

    PatchableJump badType;
    JumpList slowCases;

    JITArrayMode mode = chooseArrayMode(profile);
    switch (mode) {
    case JITInt32:
        slowCases = emitInt32GetByVal(currentInstruction, badType);
        break;
    case JITDouble:
        slowCases = emitDoubleGetByVal(currentInstruction, badType);
        break;
    case JITContiguous:
        slowCases = emitContiguousGetByVal(currentInstruction, badType);
        break;
    case JITArrayStorage:
        slowCases = emitArrayStorageGetByVal(currentInstruction, badType);
        break;
    default:
        CRASH();
        break;
    }
    
    addSlowCase(badType);
    addSlowCase(slowCases);
    
    Label done = label();
    
    if (!ASSERT_DISABLED) {
        Jump resultOK = branchTest64(NonZero, regT0);
        abortWithReason(JITGetByValResultIsNotEmpty);
        resultOK.link(this);
    }

    emitValueProfilingSite();
    emitPutVirtualRegister(dst);

    Label nextHotPath = label();

    m_byValCompilationInfo.append(ByValCompilationInfo(byValInfo, m_bytecodeOffset, notIndex, badType, mode, profile, done, nextHotPath));
}

JIT::JumpList JIT::emitDoubleLoad(Instruction*, PatchableJump& badType)
{
    JumpList slowCases;
    
    badType = patchableBranch32(NotEqual, regT2, TrustedImm32(DoubleShape));
    loadPtr(Address(regT0, JSObject::butterflyOffset()), regT2);
    slowCases.append(branch32(AboveOrEqual, regT1, Address(regT2, Butterfly::offsetOfPublicLength())));
    loadDouble(BaseIndex(regT2, regT1, TimesEight), fpRegT0);
    slowCases.append(branchDouble(DoubleNotEqualOrUnordered, fpRegT0, fpRegT0));
    
    return slowCases;
}

JIT::JumpList JIT::emitContiguousLoad(Instruction*, PatchableJump& badType, IndexingType expectedShape)
{
    JumpList slowCases;
    
    badType = patchableBranch32(NotEqual, regT2, TrustedImm32(expectedShape));
    loadPtr(Address(regT0, JSObject::butterflyOffset()), regT2);
    slowCases.append(branch32(AboveOrEqual, regT1, Address(regT2, Butterfly::offsetOfPublicLength())));
    load64(BaseIndex(regT2, regT1, TimesEight), regT0);
    slowCases.append(branchTest64(Zero, regT0));
    
    return slowCases;
}

JIT::JumpList JIT::emitArrayStorageLoad(Instruction*, PatchableJump& badType)
{
    JumpList slowCases;

    add32(TrustedImm32(-ArrayStorageShape), regT2, regT3);
    badType = patchableBranch32(Above, regT3, TrustedImm32(SlowPutArrayStorageShape - ArrayStorageShape));

    loadPtr(Address(regT0, JSObject::butterflyOffset()), regT2);
    slowCases.append(branch32(AboveOrEqual, regT1, Address(regT2, ArrayStorage::vectorLengthOffset())));

    load64(BaseIndex(regT2, regT1, TimesEight, ArrayStorage::vectorOffset()), regT0);
    slowCases.append(branchTest64(Zero, regT0));
    
    return slowCases;
}

JITGetByIdGenerator JIT::emitGetByValWithCachedId(ByValInfo* byValInfo, Instruction* currentInstruction, const Identifier& propertyName, Jump& fastDoneCase, Jump& slowDoneCase, JumpList& slowCases)
{
    // base: regT0
    // property: regT1
    // scratch: regT3

    int dst = currentInstruction[1].u.operand;

    slowCases.append(emitJumpIfNotJSCell(regT1));
    emitByValIdentifierCheck(byValInfo, regT1, regT3, propertyName, slowCases);

    JITGetByIdGenerator gen(
        m_codeBlock, CodeOrigin(m_bytecodeOffset), CallSiteIndex(m_bytecodeOffset), RegisterSet::stubUnavailableRegisters(),
        propertyName.impl(), JSValueRegs(regT0), JSValueRegs(regT0), AccessType::Get);
    gen.generateFastPath(*this);

    fastDoneCase = jump();

    Label coldPathBegin = label();
    gen.slowPathJump().link(this);

    Call call = callOperation(WithProfile, operationGetByIdOptimize, dst, gen.stubInfo(), regT0, propertyName.impl());
    gen.reportSlowPathCall(coldPathBegin, call);
    slowDoneCase = jump();

    return gen;
}

void JIT::emitSlow_op_get_by_val(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int property = currentInstruction[3].u.operand;
    ByValInfo* byValInfo = m_byValCompilationInfo[m_byValInstructionIndex].byValInfo;
    
    linkSlowCaseIfNotJSCell(iter, base); // base cell check

    if (!isOperandConstantInt(property))
        linkSlowCase(iter); // property int32 check
    Jump nonCell = jump();
    linkSlowCase(iter); // base array check
    Jump notString = branchStructure(NotEqual, 
        Address(regT0, JSCell::structureIDOffset()), 
        m_vm->stringStructure.get());
    emitNakedCall(CodeLocationLabel(m_vm->getCTIStub(stringGetByValStubGenerator).code()));
    Jump failed = branchTest64(Zero, regT0);
    emitPutVirtualRegister(dst, regT0);
    emitJumpSlowToHot(jump(), OPCODE_LENGTH(op_get_by_val));
    failed.link(this);
    notString.link(this);
    nonCell.link(this);
    
    linkSlowCase(iter); // vector length check
    linkSlowCase(iter); // empty value
    
    Label slowPath = label();
    
    emitGetVirtualRegister(base, regT0);
    emitGetVirtualRegister(property, regT1);
    Call call = callOperation(operationGetByValOptimize, dst, regT0, regT1, byValInfo);

    m_byValCompilationInfo[m_byValInstructionIndex].slowPathTarget = slowPath;
    m_byValCompilationInfo[m_byValInstructionIndex].returnAddress = call;
    m_byValInstructionIndex++;

    emitValueProfilingSite();
}

void JIT::emit_op_put_by_val(Instruction* currentInstruction)
{
    int base = currentInstruction[1].u.operand;
    int property = currentInstruction[2].u.operand;
    ArrayProfile* profile = currentInstruction[4].u.arrayProfile;
    ByValInfo* byValInfo = m_codeBlock->addByValInfo();

    emitGetVirtualRegister(base, regT0);
    bool propertyNameIsIntegerConstant = isOperandConstantInt(property);
    if (propertyNameIsIntegerConstant)
        move(Imm32(getOperandConstantInt(property)), regT1);
    else
        emitGetVirtualRegister(property, regT1);

    emitJumpSlowCaseIfNotJSCell(regT0, base);
    PatchableJump notIndex;
    if (!propertyNameIsIntegerConstant) {
        notIndex = emitPatchableJumpIfNotInt(regT1);
        addSlowCase(notIndex);
        // See comment in op_get_by_val.
        zeroExtend32ToPtr(regT1, regT1);
    }
    emitArrayProfilingSiteWithCell(regT0, regT2, profile);
    and32(TrustedImm32(IndexingShapeMask), regT2);
    
    PatchableJump badType;
    JumpList slowCases;
    
    JITArrayMode mode = chooseArrayMode(profile);
    switch (mode) {
    case JITInt32:
        slowCases = emitInt32PutByVal(currentInstruction, badType);
        break;
    case JITDouble:
        slowCases = emitDoublePutByVal(currentInstruction, badType);
        break;
    case JITContiguous:
        slowCases = emitContiguousPutByVal(currentInstruction, badType);
        break;
    case JITArrayStorage:
        slowCases = emitArrayStoragePutByVal(currentInstruction, badType);
        break;
    default:
        CRASH();
        break;
    }
    
    addSlowCase(badType);
    addSlowCase(slowCases);
    
    Label done = label();
    
    m_byValCompilationInfo.append(ByValCompilationInfo(byValInfo, m_bytecodeOffset, notIndex, badType, mode, profile, done, done));
}

void JIT::emit_op_put_by_val_with_this(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_put_by_val_with_this);
    slowPathCall.call();
}

JIT::JumpList JIT::emitGenericContiguousPutByVal(Instruction* currentInstruction, PatchableJump& badType, IndexingType indexingShape)
{
    int value = currentInstruction[3].u.operand;
    ArrayProfile* profile = currentInstruction[4].u.arrayProfile;
    
    JumpList slowCases;

    badType = patchableBranch32(NotEqual, regT2, TrustedImm32(indexingShape));
    
    loadPtr(Address(regT0, JSObject::butterflyOffset()), regT2);
    Jump outOfBounds = branch32(AboveOrEqual, regT1, Address(regT2, Butterfly::offsetOfPublicLength()));

    Label storeResult = label();
    emitGetVirtualRegister(value, regT3);
    switch (indexingShape) {
    case Int32Shape:
        slowCases.append(emitJumpIfNotInt(regT3));
        store64(regT3, BaseIndex(regT2, regT1, TimesEight));
        break;
    case DoubleShape: {
        Jump notInt = emitJumpIfNotInt(regT3);
        convertInt32ToDouble(regT3, fpRegT0);
        Jump ready = jump();
        notInt.link(this);
        add64(tagTypeNumberRegister, regT3);
        move64ToDouble(regT3, fpRegT0);
        slowCases.append(branchDouble(DoubleNotEqualOrUnordered, fpRegT0, fpRegT0));
        ready.link(this);
        storeDouble(fpRegT0, BaseIndex(regT2, regT1, TimesEight));
        break;
    }
    case ContiguousShape:
        store64(regT3, BaseIndex(regT2, regT1, TimesEight));
        emitWriteBarrier(currentInstruction[1].u.operand, value, ShouldFilterValue);
        break;
    default:
        CRASH();
        break;
    }
    
    Jump done = jump();
    outOfBounds.link(this);
    
    slowCases.append(branch32(AboveOrEqual, regT1, Address(regT2, Butterfly::offsetOfVectorLength())));
    
    emitArrayProfileStoreToHoleSpecialCase(profile);
    
    add32(TrustedImm32(1), regT1, regT3);
    store32(regT3, Address(regT2, Butterfly::offsetOfPublicLength()));
    jump().linkTo(storeResult, this);
    
    done.link(this);
    
    return slowCases;
}

JIT::JumpList JIT::emitArrayStoragePutByVal(Instruction* currentInstruction, PatchableJump& badType)
{
    int value = currentInstruction[3].u.operand;
    ArrayProfile* profile = currentInstruction[4].u.arrayProfile;
    
    JumpList slowCases;
    
    badType = patchableBranch32(NotEqual, regT2, TrustedImm32(ArrayStorageShape));
    loadPtr(Address(regT0, JSObject::butterflyOffset()), regT2);
    slowCases.append(branch32(AboveOrEqual, regT1, Address(regT2, ArrayStorage::vectorLengthOffset())));

    Jump empty = branchTest64(Zero, BaseIndex(regT2, regT1, TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0])));

    Label storeResult(this);
    emitGetVirtualRegister(value, regT3);
    store64(regT3, BaseIndex(regT2, regT1, TimesEight, OBJECT_OFFSETOF(ArrayStorage, m_vector[0])));
    emitWriteBarrier(currentInstruction[1].u.operand, value, ShouldFilterValue);
    Jump end = jump();
    
    empty.link(this);
    emitArrayProfileStoreToHoleSpecialCase(profile);
    add32(TrustedImm32(1), Address(regT2, ArrayStorage::numValuesInVectorOffset()));
    branch32(Below, regT1, Address(regT2, ArrayStorage::lengthOffset())).linkTo(storeResult, this);

    add32(TrustedImm32(1), regT1);
    store32(regT1, Address(regT2, ArrayStorage::lengthOffset()));
    sub32(TrustedImm32(1), regT1);
    jump().linkTo(storeResult, this);

    end.link(this);
    
    return slowCases;
}

JITPutByIdGenerator JIT::emitPutByValWithCachedId(ByValInfo* byValInfo, Instruction* currentInstruction, PutKind putKind, const Identifier& propertyName, JumpList& doneCases, JumpList& slowCases)
{
    // base: regT0
    // property: regT1
    // scratch: regT2

    int base = currentInstruction[1].u.operand;
    int value = currentInstruction[3].u.operand;

    slowCases.append(emitJumpIfNotJSCell(regT1));
    emitByValIdentifierCheck(byValInfo, regT1, regT1, propertyName, slowCases);

    // Write barrier breaks the registers. So after issuing the write barrier,
    // reload the registers.
    emitGetVirtualRegisters(base, regT0, value, regT1);

    JITPutByIdGenerator gen(
        m_codeBlock, CodeOrigin(m_bytecodeOffset), CallSiteIndex(m_bytecodeOffset), RegisterSet::stubUnavailableRegisters(),
        JSValueRegs(regT0), JSValueRegs(regT1), regT2, m_codeBlock->ecmaMode(), putKind);
    gen.generateFastPath(*this);
    emitWriteBarrier(base, value, ShouldFilterBase);
    doneCases.append(jump());

    Label coldPathBegin = label();
    gen.slowPathJump().link(this);

    Call call = callOperation(gen.slowPathFunction(), gen.stubInfo(), regT1, regT0, propertyName.impl());
    gen.reportSlowPathCall(coldPathBegin, call);
    doneCases.append(jump());

    return gen;
}

void JIT::emitSlow_op_put_by_val(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int base = currentInstruction[1].u.operand;
    int property = currentInstruction[2].u.operand;
    int value = currentInstruction[3].u.operand;
    JITArrayMode mode = m_byValCompilationInfo[m_byValInstructionIndex].arrayMode;
    ByValInfo* byValInfo = m_byValCompilationInfo[m_byValInstructionIndex].byValInfo;

    linkSlowCaseIfNotJSCell(iter, base); // base cell check
    if (!isOperandConstantInt(property))
        linkSlowCase(iter); // property int32 check
    linkSlowCase(iter); // base not array check
    
    linkSlowCase(iter); // out of bounds
    
    switch (mode) {
    case JITInt32:
    case JITDouble:
        linkSlowCase(iter); // value type check
        break;
    default:
        break;
    }
    
    Label slowPath = label();

    emitGetVirtualRegister(base, regT0);
    emitGetVirtualRegister(property, regT1);
    emitGetVirtualRegister(value, regT2);
    bool isDirect = m_interpreter->getOpcodeID(currentInstruction->u.opcode) == op_put_by_val_direct;
    Call call = callOperation(isDirect ? operationDirectPutByValOptimize : operationPutByValOptimize, regT0, regT1, regT2, byValInfo);

    m_byValCompilationInfo[m_byValInstructionIndex].slowPathTarget = slowPath;
    m_byValCompilationInfo[m_byValInstructionIndex].returnAddress = call;
    m_byValInstructionIndex++;
}

void JIT::emit_op_put_by_index(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    emitGetVirtualRegister(currentInstruction[3].u.operand, regT1);
    callOperation(operationPutByIndex, regT0, currentInstruction[2].u.operand, regT1);
}

void JIT::emit_op_put_getter_by_id(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    int32_t options = currentInstruction[3].u.operand;
    emitGetVirtualRegister(currentInstruction[4].u.operand, regT1);
    callOperation(operationPutGetterById, regT0, m_codeBlock->identifier(currentInstruction[2].u.operand).impl(), options, regT1);
}

void JIT::emit_op_put_setter_by_id(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    int32_t options = currentInstruction[3].u.operand;
    emitGetVirtualRegister(currentInstruction[4].u.operand, regT1);
    callOperation(operationPutSetterById, regT0, m_codeBlock->identifier(currentInstruction[2].u.operand).impl(), options, regT1);
}

void JIT::emit_op_put_getter_setter_by_id(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    int32_t attribute = currentInstruction[3].u.operand;
    emitGetVirtualRegister(currentInstruction[4].u.operand, regT1);
    emitGetVirtualRegister(currentInstruction[5].u.operand, regT2);
    callOperation(operationPutGetterSetter, regT0, m_codeBlock->identifier(currentInstruction[2].u.operand).impl(), attribute, regT1, regT2);
}

void JIT::emit_op_put_getter_by_val(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT1);
    int32_t attributes = currentInstruction[3].u.operand;
    emitGetVirtualRegister(currentInstruction[4].u.operand, regT2);
    callOperation(operationPutGetterByVal, regT0, regT1, attributes, regT2);
}

void JIT::emit_op_put_setter_by_val(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT1);
    int32_t attributes = currentInstruction[3].u.operand;
    emitGetVirtualRegister(currentInstruction[4].u.operand, regT2);
    callOperation(operationPutSetterByVal, regT0, regT1, attributes, regT2);
}

void JIT::emit_op_del_by_id(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int property = currentInstruction[3].u.operand;
    emitGetVirtualRegister(base, regT0);
    callOperation(operationDeleteByIdJSResult, dst, regT0, m_codeBlock->identifier(property).impl());
}

void JIT::emit_op_del_by_val(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int property = currentInstruction[3].u.operand;
    emitGetVirtualRegister(base, regT0);
    emitGetVirtualRegister(property, regT1);
    callOperation(operationDeleteByValJSResult, dst, regT0, regT1);
}

void JIT::emit_op_try_get_by_id(Instruction* currentInstruction)
{
    int resultVReg = currentInstruction[1].u.operand;
    int baseVReg = currentInstruction[2].u.operand;
    const Identifier* ident = &(m_codeBlock->identifier(currentInstruction[3].u.operand));

    emitGetVirtualRegister(baseVReg, regT0);

    emitJumpSlowCaseIfNotJSCell(regT0, baseVReg);

    JITGetByIdGenerator gen(
        m_codeBlock, CodeOrigin(m_bytecodeOffset), CallSiteIndex(m_bytecodeOffset), RegisterSet::stubUnavailableRegisters(),
        ident->impl(), JSValueRegs(regT0), JSValueRegs(regT0), AccessType::TryGet);
    gen.generateFastPath(*this);
    addSlowCase(gen.slowPathJump());
    m_getByIds.append(gen);
    
    emitValueProfilingSite();
    emitPutVirtualRegister(resultVReg);
}

void JIT::emitSlow_op_try_get_by_id(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int resultVReg = currentInstruction[1].u.operand;
    int baseVReg = currentInstruction[2].u.operand;
    const Identifier* ident = &(m_codeBlock->identifier(currentInstruction[3].u.operand));

    linkSlowCaseIfNotJSCell(iter, baseVReg);
    linkSlowCase(iter);

    JITGetByIdGenerator& gen = m_getByIds[m_getByIdIndex++];

    Label coldPathBegin = label();

    Call call = callOperation(operationTryGetByIdOptimize, resultVReg, gen.stubInfo(), regT0, ident->impl());
    
    gen.reportSlowPathCall(coldPathBegin, call);
}

void JIT::emit_op_get_by_id(Instruction* currentInstruction)
{
    int resultVReg = currentInstruction[1].u.operand;
    int baseVReg = currentInstruction[2].u.operand;
    const Identifier* ident = &(m_codeBlock->identifier(currentInstruction[3].u.operand));

    emitGetVirtualRegister(baseVReg, regT0);
    
    emitJumpSlowCaseIfNotJSCell(regT0, baseVReg);
    
    if (*ident == m_vm->propertyNames->length && shouldEmitProfiling())
        emitArrayProfilingSiteForBytecodeIndexWithCell(regT0, regT1, m_bytecodeOffset);

    JITGetByIdGenerator gen(
        m_codeBlock, CodeOrigin(m_bytecodeOffset), CallSiteIndex(m_bytecodeOffset), RegisterSet::stubUnavailableRegisters(),
        ident->impl(), JSValueRegs(regT0), JSValueRegs(regT0), AccessType::Get);
    gen.generateFastPath(*this);
    addSlowCase(gen.slowPathJump());
    m_getByIds.append(gen);

    emitValueProfilingSite();
    emitPutVirtualRegister(resultVReg);
}

void JIT::emit_op_get_by_id_with_this(Instruction* currentInstruction)
{
    int resultVReg = currentInstruction[1].u.operand;
    int baseVReg = currentInstruction[2].u.operand;
    int thisVReg = currentInstruction[3].u.operand;
    const Identifier* ident = &(m_codeBlock->identifier(currentInstruction[4].u.operand));

    emitGetVirtualRegister(baseVReg, regT0);
    emitGetVirtualRegister(thisVReg, regT1);
    emitJumpSlowCaseIfNotJSCell(regT0, baseVReg);
    emitJumpSlowCaseIfNotJSCell(regT1, thisVReg);

    JITGetByIdWithThisGenerator gen(
        m_codeBlock, CodeOrigin(m_bytecodeOffset), CallSiteIndex(m_bytecodeOffset), RegisterSet::stubUnavailableRegisters(),
        ident->impl(), JSValueRegs(regT0), JSValueRegs(regT0), JSValueRegs(regT1), AccessType::GetWithThis);
    gen.generateFastPath(*this);
    addSlowCase(gen.slowPathJump());
    m_getByIdsWithThis.append(gen);

    emitValueProfilingSite();
    emitPutVirtualRegister(resultVReg);
}

void JIT::emit_op_get_by_val_with_this(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_get_by_val_with_this);
    slowPathCall.call();
}

void JIT::emitSlow_op_get_by_id(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int resultVReg = currentInstruction[1].u.operand;
    int baseVReg = currentInstruction[2].u.operand;
    const Identifier* ident = &(m_codeBlock->identifier(currentInstruction[3].u.operand));

    linkSlowCaseIfNotJSCell(iter, baseVReg);
    linkSlowCase(iter);

    JITGetByIdGenerator& gen = m_getByIds[m_getByIdIndex++];
    
    Label coldPathBegin = label();
    
    Call call = callOperation(WithProfile, operationGetByIdOptimize, resultVReg, gen.stubInfo(), regT0, ident->impl());

    gen.reportSlowPathCall(coldPathBegin, call);
}

void JIT::emitSlow_op_get_by_id_with_this(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int resultVReg = currentInstruction[1].u.operand;
    int baseVReg = currentInstruction[2].u.operand;
    int thisVReg = currentInstruction[3].u.operand;
    const Identifier* ident = &(m_codeBlock->identifier(currentInstruction[4].u.operand));

    linkSlowCaseIfNotJSCell(iter, baseVReg);
    linkSlowCaseIfNotJSCell(iter, thisVReg);
    linkSlowCase(iter);

    JITGetByIdWithThisGenerator& gen = m_getByIdsWithThis[m_getByIdWithThisIndex++];
    
    Label coldPathBegin = label();
    
    Call call = callOperation(WithProfile, operationGetByIdWithThisOptimize, resultVReg, gen.stubInfo(), regT0, regT1, ident->impl());

    gen.reportSlowPathCall(coldPathBegin, call);
}

void JIT::emit_op_put_by_id(Instruction* currentInstruction)
{
    int baseVReg = currentInstruction[1].u.operand;
    int valueVReg = currentInstruction[3].u.operand;
    unsigned direct = currentInstruction[8].u.putByIdFlags & PutByIdIsDirect;

    // In order to be able to patch both the Structure, and the object offset, we store one pointer,
    // to just after the arguments have been loaded into registers 'hotPathBegin', and we generate code
    // such that the Structure & offset are always at the same distance from this.

    emitGetVirtualRegisters(baseVReg, regT0, valueVReg, regT1);

    emitJumpSlowCaseIfNotJSCell(regT0, baseVReg);

    JITPutByIdGenerator gen(
        m_codeBlock, CodeOrigin(m_bytecodeOffset), CallSiteIndex(m_bytecodeOffset), RegisterSet::stubUnavailableRegisters(),
        JSValueRegs(regT0), JSValueRegs(regT1), regT2, m_codeBlock->ecmaMode(),
        direct ? Direct : NotDirect);
    
    gen.generateFastPath(*this);
    addSlowCase(gen.slowPathJump());
    
    emitWriteBarrier(baseVReg, valueVReg, ShouldFilterBase);

    m_putByIds.append(gen);
}

void JIT::emit_op_put_by_id_with_this(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_put_by_id_with_this);
    slowPathCall.call();
}

void JIT::emitSlow_op_put_by_id(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int baseVReg = currentInstruction[1].u.operand;
    const Identifier* ident = &(m_codeBlock->identifier(currentInstruction[2].u.operand));

    linkSlowCaseIfNotJSCell(iter, baseVReg);
    linkSlowCase(iter);

    Label coldPathBegin(this);
    
    JITPutByIdGenerator& gen = m_putByIds[m_putByIdIndex++];

    Call call = callOperation(
        gen.slowPathFunction(), gen.stubInfo(), regT1, regT0, ident->impl());

    gen.reportSlowPathCall(coldPathBegin, call);
}

void JIT::emitVarInjectionCheck(bool needsVarInjectionChecks)
{
    if (!needsVarInjectionChecks)
        return;
    addSlowCase(branch8(Equal, AbsoluteAddress(m_codeBlock->globalObject()->varInjectionWatchpoint()->addressOfState()), TrustedImm32(IsInvalidated)));
}

void JIT::emitResolveClosure(int dst, int scope, bool needsVarInjectionChecks, unsigned depth)
{
    emitVarInjectionCheck(needsVarInjectionChecks);
    emitGetVirtualRegister(scope, regT0);
    for (unsigned i = 0; i < depth; ++i)
        loadPtr(Address(regT0, JSScope::offsetOfNext()), regT0);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_resolve_scope(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int scope = currentInstruction[2].u.operand;
    ResolveType resolveType = static_cast<ResolveType>(copiedInstruction(currentInstruction)[4].u.operand);
    unsigned depth = currentInstruction[5].u.operand;

    auto emitCode = [&] (ResolveType resolveType) {
        switch (resolveType) {
        case GlobalProperty:
        case GlobalVar:
        case GlobalPropertyWithVarInjectionChecks:
        case GlobalVarWithVarInjectionChecks:
        case GlobalLexicalVar:
        case GlobalLexicalVarWithVarInjectionChecks: {
            JSScope* constantScope = JSScope::constantScopeForCodeBlock(resolveType, m_codeBlock);
            RELEASE_ASSERT(constantScope);
            emitVarInjectionCheck(needsVarInjectionChecks(resolveType));
            move(TrustedImmPtr(constantScope), regT0);
            emitPutVirtualRegister(dst);
            break;
        }
        case ClosureVar:
        case ClosureVarWithVarInjectionChecks:
            emitResolveClosure(dst, scope, needsVarInjectionChecks(resolveType), depth);
            break;
        case ModuleVar:
            move(TrustedImmPtr(currentInstruction[6].u.jsCell.get()), regT0);
            emitPutVirtualRegister(dst);
            break;
        case Dynamic:
            addSlowCase(jump());
            break;
        case LocalClosureVar:
        case UnresolvedProperty:
        case UnresolvedPropertyWithVarInjectionChecks:
            RELEASE_ASSERT_NOT_REACHED();
        }
    };

    switch (resolveType) {
    case UnresolvedProperty:
    case UnresolvedPropertyWithVarInjectionChecks: {
        JumpList skipToEnd;
        load32(&currentInstruction[4], regT0);

        Jump notGlobalProperty = branch32(NotEqual, regT0, TrustedImm32(GlobalProperty));
        emitCode(GlobalProperty);
        skipToEnd.append(jump());
        notGlobalProperty.link(this);

        Jump notGlobalPropertyWithVarInjections = branch32(NotEqual, regT0, TrustedImm32(GlobalPropertyWithVarInjectionChecks));
        emitCode(GlobalPropertyWithVarInjectionChecks);
        skipToEnd.append(jump());
        notGlobalPropertyWithVarInjections.link(this);

        Jump notGlobalLexicalVar = branch32(NotEqual, regT0, TrustedImm32(GlobalLexicalVar));
        emitCode(GlobalLexicalVar);
        skipToEnd.append(jump());
        notGlobalLexicalVar.link(this);

        Jump notGlobalLexicalVarWithVarInjections = branch32(NotEqual, regT0, TrustedImm32(GlobalLexicalVarWithVarInjectionChecks));
        emitCode(GlobalLexicalVarWithVarInjectionChecks);
        skipToEnd.append(jump());
        notGlobalLexicalVarWithVarInjections.link(this);

        addSlowCase(jump());
        skipToEnd.link(this);
        break;
    }

    default:
        emitCode(resolveType);
        break;
    }
}

void JIT::emitSlow_op_resolve_scope(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    ResolveType resolveType = static_cast<ResolveType>(copiedInstruction(currentInstruction)[4].u.operand);
    if (resolveType == GlobalProperty || resolveType == GlobalVar || resolveType == ClosureVar || resolveType == GlobalLexicalVar || resolveType == ModuleVar)
        return;

    if (resolveType == UnresolvedProperty || resolveType == UnresolvedPropertyWithVarInjectionChecks) {
        linkSlowCase(iter); // var injections check for GlobalPropertyWithVarInjectionChecks.
        linkSlowCase(iter); // var injections check for GlobalLexicalVarWithVarInjectionChecks.
    }

    linkSlowCase(iter);
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_resolve_scope);
    slowPathCall.call();
}

void JIT::emitLoadWithStructureCheck(int scope, Structure** structureSlot)
{
    loadPtr(structureSlot, regT1);
    emitGetVirtualRegister(scope, regT0);
    addSlowCase(branchTestPtr(Zero, regT1));
    load32(Address(regT1, Structure::structureIDOffset()), regT1);
    addSlowCase(branch32(NotEqual, Address(regT0, JSCell::structureIDOffset()), regT1));
}

void JIT::emitGetVarFromPointer(JSValue* operand, GPRReg reg)
{
    loadPtr(operand, reg);
}

void JIT::emitGetVarFromIndirectPointer(JSValue** operand, GPRReg reg)
{
    loadPtr(operand, reg);
    loadPtr(reg, reg);
}

void JIT::emitGetClosureVar(int scope, uintptr_t operand)
{
    emitGetVirtualRegister(scope, regT0);
    loadPtr(Address(regT0, JSEnvironmentRecord::offsetOfVariables() + operand * sizeof(Register)), regT0);
}

void JIT::emit_op_get_from_scope(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int scope = currentInstruction[2].u.operand;
    ResolveType resolveType = GetPutInfo(copiedInstruction(currentInstruction)[4].u.operand).resolveType();
    Structure** structureSlot = currentInstruction[5].u.structure.slot();
    uintptr_t* operandSlot = reinterpret_cast<uintptr_t*>(&currentInstruction[6].u.pointer);

    auto emitCode = [&] (ResolveType resolveType, bool indirectLoadForOperand) {
        switch (resolveType) {
        case GlobalProperty:
        case GlobalPropertyWithVarInjectionChecks: {
            emitLoadWithStructureCheck(scope, structureSlot); // Structure check covers var injection.
            GPRReg base = regT0;
            GPRReg result = regT0;
            GPRReg offset = regT1;
            GPRReg scratch = regT2;
            
            load32(operandSlot, offset);
            if (!ASSERT_DISABLED) {
                Jump isOutOfLine = branch32(GreaterThanOrEqual, offset, TrustedImm32(firstOutOfLineOffset));
                abortWithReason(JITOffsetIsNotOutOfLine);
                isOutOfLine.link(this);
            }
            loadPtr(Address(base, JSObject::butterflyOffset()), scratch);
            neg32(offset);
            signExtend32ToPtr(offset, offset);
            load64(BaseIndex(scratch, offset, TimesEight, (firstOutOfLineOffset - 2) * sizeof(EncodedJSValue)), result);
            break;
        }
        case GlobalVar:
        case GlobalVarWithVarInjectionChecks:
        case GlobalLexicalVar:
        case GlobalLexicalVarWithVarInjectionChecks:
            emitVarInjectionCheck(needsVarInjectionChecks(resolveType));
            if (indirectLoadForOperand)
                emitGetVarFromIndirectPointer(bitwise_cast<JSValue**>(operandSlot), regT0);
            else
                emitGetVarFromPointer(bitwise_cast<JSValue*>(*operandSlot), regT0);
            if (resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks) // TDZ check.
                addSlowCase(branchTest64(Zero, regT0));
            break;
        case ClosureVar:
        case ClosureVarWithVarInjectionChecks:
            emitVarInjectionCheck(needsVarInjectionChecks(resolveType));
            emitGetClosureVar(scope, *operandSlot);
            break;
        case Dynamic:
            addSlowCase(jump());
            break;
        case LocalClosureVar:
        case ModuleVar:
        case UnresolvedProperty:
        case UnresolvedPropertyWithVarInjectionChecks:
            RELEASE_ASSERT_NOT_REACHED();
        }
    };

    switch (resolveType) {
    case UnresolvedProperty:
    case UnresolvedPropertyWithVarInjectionChecks: {
        JumpList skipToEnd;
        load32(&currentInstruction[4], regT0);
        and32(TrustedImm32(GetPutInfo::typeBits), regT0); // Load ResolveType into T0

        Jump isGlobalProperty = branch32(Equal, regT0, TrustedImm32(GlobalProperty));
        Jump notGlobalPropertyWithVarInjections = branch32(NotEqual, regT0, TrustedImm32(GlobalPropertyWithVarInjectionChecks));
        isGlobalProperty.link(this);
        emitCode(GlobalProperty, false);
        skipToEnd.append(jump());
        notGlobalPropertyWithVarInjections.link(this);

        Jump notGlobalLexicalVar = branch32(NotEqual, regT0, TrustedImm32(GlobalLexicalVar));
        emitCode(GlobalLexicalVar, true);
        skipToEnd.append(jump());
        notGlobalLexicalVar.link(this);

        Jump notGlobalLexicalVarWithVarInjections = branch32(NotEqual, regT0, TrustedImm32(GlobalLexicalVarWithVarInjectionChecks));
        emitCode(GlobalLexicalVarWithVarInjectionChecks, true);
        skipToEnd.append(jump());
        notGlobalLexicalVarWithVarInjections.link(this);

        addSlowCase(jump());

        skipToEnd.link(this);
        break;
    }

    default:
        emitCode(resolveType, false);
        break;
    }
    emitPutVirtualRegister(dst);
    emitValueProfilingSite();
}

void JIT::emitSlow_op_get_from_scope(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int dst = currentInstruction[1].u.operand;
    ResolveType resolveType = GetPutInfo(copiedInstruction(currentInstruction)[4].u.operand).resolveType();

    if (resolveType == GlobalVar || resolveType == ClosureVar)
        return;

    if (resolveType == GlobalProperty || resolveType == GlobalPropertyWithVarInjectionChecks)
        linkSlowCase(iter); // bad structure

    if (resolveType == GlobalLexicalVarWithVarInjectionChecks) // Var injections check.
        linkSlowCase(iter);

    if (resolveType == UnresolvedProperty || resolveType == UnresolvedPropertyWithVarInjectionChecks) {
        // GlobalProperty/GlobalPropertyWithVarInjectionChecks
        linkSlowCase(iter); // emitLoadWithStructureCheck
        linkSlowCase(iter); // emitLoadWithStructureCheck
        // GlobalLexicalVar
        linkSlowCase(iter); // TDZ check.
        // GlobalLexicalVarWithVarInjectionChecks.
        linkSlowCase(iter); // var injection check.
        linkSlowCase(iter); // TDZ check.
    }

    linkSlowCase(iter);

    callOperation(WithProfile, operationGetFromScope, dst, currentInstruction);
}

void JIT::emitPutGlobalVariable(JSValue* operand, int value, WatchpointSet* set)
{
    emitGetVirtualRegister(value, regT0);
    emitNotifyWrite(set);
    storePtr(regT0, operand);
}
void JIT::emitPutGlobalVariableIndirect(JSValue** addressOfOperand, int value, WatchpointSet** indirectWatchpointSet)
{
    emitGetVirtualRegister(value, regT0);
    loadPtr(indirectWatchpointSet, regT1);
    emitNotifyWrite(regT1);
    loadPtr(addressOfOperand, regT1);
    storePtr(regT0, regT1);
}

void JIT::emitPutClosureVar(int scope, uintptr_t operand, int value, WatchpointSet* set)
{
    emitGetVirtualRegister(value, regT1);
    emitGetVirtualRegister(scope, regT0);
    emitNotifyWrite(set);
    storePtr(regT1, Address(regT0, JSEnvironmentRecord::offsetOfVariables() + operand * sizeof(Register)));
}

void JIT::emit_op_put_to_scope(Instruction* currentInstruction)
{
    int scope = currentInstruction[1].u.operand;
    int value = currentInstruction[3].u.operand;
    GetPutInfo getPutInfo = GetPutInfo(copiedInstruction(currentInstruction)[4].u.operand);
    ResolveType resolveType = getPutInfo.resolveType();
    Structure** structureSlot = currentInstruction[5].u.structure.slot();
    uintptr_t* operandSlot = reinterpret_cast<uintptr_t*>(&currentInstruction[6].u.pointer);

    auto emitCode = [&] (ResolveType resolveType, bool indirectLoadForOperand) {
        switch (resolveType) {
        case GlobalProperty:
        case GlobalPropertyWithVarInjectionChecks: {
            emitLoadWithStructureCheck(scope, structureSlot); // Structure check covers var injection.
            emitGetVirtualRegister(value, regT2);
            
            loadPtr(Address(regT0, JSObject::butterflyOffset()), regT0);
            loadPtr(operandSlot, regT1);
            negPtr(regT1);
            storePtr(regT2, BaseIndex(regT0, regT1, TimesEight, (firstOutOfLineOffset - 2) * sizeof(EncodedJSValue)));
            emitWriteBarrier(m_codeBlock->globalObject(), value, ShouldFilterValue);
            break;
        }
        case GlobalVar:
        case GlobalVarWithVarInjectionChecks:
        case GlobalLexicalVar:
        case GlobalLexicalVarWithVarInjectionChecks: {
            JSScope* constantScope = JSScope::constantScopeForCodeBlock(resolveType, m_codeBlock);
            RELEASE_ASSERT(constantScope);
            emitVarInjectionCheck(needsVarInjectionChecks(resolveType));
            if (!isInitialization(getPutInfo.initializationMode()) && (resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks)) {
                // We need to do a TDZ check here because we can't always prove we need to emit TDZ checks statically.
                if (indirectLoadForOperand)
                    emitGetVarFromIndirectPointer(bitwise_cast<JSValue**>(operandSlot), regT0);
                else
                    emitGetVarFromPointer(bitwise_cast<JSValue*>(*operandSlot), regT0);
                addSlowCase(branchTest64(Zero, regT0));
            }
            if (indirectLoadForOperand)
                emitPutGlobalVariableIndirect(bitwise_cast<JSValue**>(operandSlot), value, bitwise_cast<WatchpointSet**>(&currentInstruction[5]));
            else
                emitPutGlobalVariable(bitwise_cast<JSValue*>(*operandSlot), value, currentInstruction[5].u.watchpointSet);
            emitWriteBarrier(constantScope, value, ShouldFilterValue);
            break;
        }
        case LocalClosureVar:
        case ClosureVar:
        case ClosureVarWithVarInjectionChecks:
            emitVarInjectionCheck(needsVarInjectionChecks(resolveType));
            emitPutClosureVar(scope, *operandSlot, value, currentInstruction[5].u.watchpointSet);
            emitWriteBarrier(scope, value, ShouldFilterValue);
            break;
        case ModuleVar:
        case Dynamic:
            addSlowCase(jump());
            break;
        case UnresolvedProperty:
        case UnresolvedPropertyWithVarInjectionChecks:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
    };

    switch (resolveType) {
    case UnresolvedProperty:
    case UnresolvedPropertyWithVarInjectionChecks: {
        JumpList skipToEnd;
        load32(&currentInstruction[4], regT0);
        and32(TrustedImm32(GetPutInfo::typeBits), regT0); // Load ResolveType into T0

        Jump isGlobalProperty = branch32(Equal, regT0, TrustedImm32(GlobalProperty));
        Jump notGlobalPropertyWithVarInjections = branch32(NotEqual, regT0, TrustedImm32(GlobalPropertyWithVarInjectionChecks));
        isGlobalProperty.link(this);
        emitCode(GlobalProperty, false);
        skipToEnd.append(jump());
        notGlobalPropertyWithVarInjections.link(this);

        Jump notGlobalLexicalVar = branch32(NotEqual, regT0, TrustedImm32(GlobalLexicalVar));
        emitCode(GlobalLexicalVar, true);
        skipToEnd.append(jump());
        notGlobalLexicalVar.link(this);

        Jump notGlobalLexicalVarWithVarInjections = branch32(NotEqual, regT0, TrustedImm32(GlobalLexicalVarWithVarInjectionChecks));
        emitCode(GlobalLexicalVarWithVarInjectionChecks, true);
        skipToEnd.append(jump());
        notGlobalLexicalVarWithVarInjections.link(this);

        addSlowCase(jump());

        skipToEnd.link(this);
        break;
    }

    default:
        emitCode(resolveType, false);
        break;
    }
}

void JIT::emitSlow_op_put_to_scope(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    GetPutInfo getPutInfo = GetPutInfo(copiedInstruction(currentInstruction)[4].u.operand);
    ResolveType resolveType = getPutInfo.resolveType();
    unsigned linkCount = 0;
    if (resolveType != GlobalVar && resolveType != ClosureVar && resolveType != LocalClosureVar && resolveType != GlobalLexicalVar)
        linkCount++;
    if (resolveType == GlobalVar || resolveType == GlobalVarWithVarInjectionChecks 
        || resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks 
        || resolveType == ClosureVar || resolveType == ClosureVarWithVarInjectionChecks
        || resolveType == LocalClosureVar)
        linkCount++;
    if (resolveType == GlobalProperty || resolveType == GlobalPropertyWithVarInjectionChecks)
        linkCount++; // bad structure
    if (!isInitialization(getPutInfo.initializationMode()) && (resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks)) // TDZ check.
        linkCount++;
    if (resolveType == UnresolvedProperty || resolveType == UnresolvedPropertyWithVarInjectionChecks) {
        // GlobalProperty/GlobalPropertyWithVarInjectionsCheck
        linkCount++; // emitLoadWithStructureCheck
        linkCount++; // emitLoadWithStructureCheck

        // GlobalLexicalVar
        bool needsTDZCheck = !isInitialization(getPutInfo.initializationMode());
        if (needsTDZCheck)
            linkCount++;
        linkCount++; // Notify write check.

        // GlobalLexicalVarWithVarInjectionsCheck
        linkCount++; // var injection check.
        if (needsTDZCheck)
            linkCount++;
        linkCount++; // Notify write check.
    }
    if (!linkCount)
        return;
    while (linkCount--)
        linkSlowCase(iter);

    if (resolveType == ModuleVar) {
        JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_throw_strict_mode_readonly_property_write_error);
        slowPathCall.call();
    } else
        callOperation(operationPutToScope, currentInstruction);
}

void JIT::emit_op_get_from_arguments(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int arguments = currentInstruction[2].u.operand;
    int index = currentInstruction[3].u.operand;
    
    emitGetVirtualRegister(arguments, regT0);
    load64(Address(regT0, DirectArguments::storageOffset() + index * sizeof(WriteBarrier<Unknown>)), regT0);
    emitValueProfilingSite();
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_put_to_arguments(Instruction* currentInstruction)
{
    int arguments = currentInstruction[1].u.operand;
    int index = currentInstruction[2].u.operand;
    int value = currentInstruction[3].u.operand;
    
    emitGetVirtualRegister(arguments, regT0);
    emitGetVirtualRegister(value, regT1);
    store64(regT1, Address(regT0, DirectArguments::storageOffset() + index * sizeof(WriteBarrier<Unknown>)));

    emitWriteBarrier(arguments, value, ShouldFilterValue);
}

#endif // USE(JSVALUE64)

#if USE(JSVALUE64)
void JIT::emitWriteBarrier(unsigned owner, unsigned value, WriteBarrierMode mode)
{
    Jump valueNotCell;
    if (mode == ShouldFilterValue || mode == ShouldFilterBaseAndValue) {
        emitGetVirtualRegister(value, regT0);
        valueNotCell = branchTest64(NonZero, regT0, tagMaskRegister);
    }
    
    emitGetVirtualRegister(owner, regT0);
    Jump ownerNotCell;
    if (mode == ShouldFilterBaseAndValue || mode == ShouldFilterBase)
        ownerNotCell = branchTest64(NonZero, regT0, tagMaskRegister);

    Jump ownerIsRememberedOrInEden = barrierBranch(regT0, regT1);
    callOperation(operationWriteBarrierSlowPath, regT0);
    ownerIsRememberedOrInEden.link(this);

    if (mode == ShouldFilterBaseAndValue || mode == ShouldFilterBase)
        ownerNotCell.link(this);
    if (mode == ShouldFilterValue || mode == ShouldFilterBaseAndValue) 
        valueNotCell.link(this);
}

void JIT::emitWriteBarrier(JSCell* owner, unsigned value, WriteBarrierMode mode)
{
    emitGetVirtualRegister(value, regT0);
    Jump valueNotCell;
    if (mode == ShouldFilterValue)
        valueNotCell = branchTest64(NonZero, regT0, tagMaskRegister);

    emitWriteBarrier(owner);

    if (mode == ShouldFilterValue) 
        valueNotCell.link(this);
}

#else // USE(JSVALUE64)

void JIT::emitWriteBarrier(unsigned owner, unsigned value, WriteBarrierMode mode)
{
    Jump valueNotCell;
    if (mode == ShouldFilterValue || mode == ShouldFilterBaseAndValue) {
        emitLoadTag(value, regT0);
        valueNotCell = branch32(NotEqual, regT0, TrustedImm32(JSValue::CellTag));
    }
    
    emitLoad(owner, regT0, regT1);
    Jump ownerNotCell;
    if (mode == ShouldFilterBase || mode == ShouldFilterBaseAndValue)
        ownerNotCell = branch32(NotEqual, regT0, TrustedImm32(JSValue::CellTag));

    Jump ownerIsRememberedOrInEden = barrierBranch(regT1, regT2);
    callOperation(operationWriteBarrierSlowPath, regT1);
    ownerIsRememberedOrInEden.link(this);

    if (mode == ShouldFilterBase || mode == ShouldFilterBaseAndValue)
        ownerNotCell.link(this);
    if (mode == ShouldFilterValue || mode == ShouldFilterBaseAndValue) 
        valueNotCell.link(this);
}

void JIT::emitWriteBarrier(JSCell* owner, unsigned value, WriteBarrierMode mode)
{
    Jump valueNotCell;
    if (mode == ShouldFilterValue) {
        emitLoadTag(value, regT0);
        valueNotCell = branch32(NotEqual, regT0, TrustedImm32(JSValue::CellTag));
    }

    emitWriteBarrier(owner);

    if (mode == ShouldFilterValue) 
        valueNotCell.link(this);
}

#endif // USE(JSVALUE64)

void JIT::emitWriteBarrier(JSCell* owner)
{
    Jump ownerIsRememberedOrInEden = barrierBranch(owner, regT0);
    callOperation(operationWriteBarrierSlowPath, owner);
    ownerIsRememberedOrInEden.link(this);
}

void JIT::emitByValIdentifierCheck(ByValInfo* byValInfo, RegisterID cell, RegisterID scratch, const Identifier& propertyName, JumpList& slowCases)
{
    if (propertyName.isSymbol())
        slowCases.append(branchPtr(NotEqual, cell, TrustedImmPtr(byValInfo->cachedSymbol.get())));
    else {
        slowCases.append(branchStructure(NotEqual, Address(cell, JSCell::structureIDOffset()), m_vm->stringStructure.get()));
        loadPtr(Address(cell, JSString::offsetOfValue()), scratch);
        slowCases.append(branchPtr(NotEqual, scratch, TrustedImmPtr(propertyName.impl())));
    }
}

void JIT::privateCompileGetByVal(ByValInfo* byValInfo, ReturnAddressPtr returnAddress, JITArrayMode arrayMode)
{
    Instruction* currentInstruction = m_codeBlock->instructions().begin() + byValInfo->bytecodeIndex;
    
    PatchableJump badType;
    JumpList slowCases;
    
    switch (arrayMode) {
    case JITInt32:
        slowCases = emitInt32GetByVal(currentInstruction, badType);
        break;
    case JITDouble:
        slowCases = emitDoubleGetByVal(currentInstruction, badType);
        break;
    case JITContiguous:
        slowCases = emitContiguousGetByVal(currentInstruction, badType);
        break;
    case JITArrayStorage:
        slowCases = emitArrayStorageGetByVal(currentInstruction, badType);
        break;
    case JITDirectArguments:
        slowCases = emitDirectArgumentsGetByVal(currentInstruction, badType);
        break;
    case JITScopedArguments:
        slowCases = emitScopedArgumentsGetByVal(currentInstruction, badType);
        break;
    default:
        TypedArrayType type = typedArrayTypeForJITArrayMode(arrayMode);
        if (isInt(type))
            slowCases = emitIntTypedArrayGetByVal(currentInstruction, badType, type);
        else 
            slowCases = emitFloatTypedArrayGetByVal(currentInstruction, badType, type);
        break;
    }
    
    Jump done = jump();

    LinkBuffer patchBuffer(*m_vm, *this, m_codeBlock);
    
    patchBuffer.link(badType, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    patchBuffer.link(slowCases, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    
    patchBuffer.link(done, byValInfo->badTypeJump.labelAtOffset(byValInfo->badTypeJumpToDone));
    
    byValInfo->stubRoutine = FINALIZE_CODE_FOR_STUB(
        m_codeBlock, patchBuffer,
        ("Baseline get_by_val stub for %s, return point %p", toCString(*m_codeBlock).data(), returnAddress.value()));
    
    MacroAssembler::repatchJump(byValInfo->badTypeJump, CodeLocationLabel(byValInfo->stubRoutine->code().code()));
    MacroAssembler::repatchCall(CodeLocationCall(MacroAssemblerCodePtr(returnAddress)), FunctionPtr(operationGetByValGeneric));
}

void JIT::privateCompileGetByValWithCachedId(ByValInfo* byValInfo, ReturnAddressPtr returnAddress, const Identifier& propertyName)
{
    Instruction* currentInstruction = m_codeBlock->instructions().begin() + byValInfo->bytecodeIndex;

    Jump fastDoneCase;
    Jump slowDoneCase;
    JumpList slowCases;

    JITGetByIdGenerator gen = emitGetByValWithCachedId(byValInfo, currentInstruction, propertyName, fastDoneCase, slowDoneCase, slowCases);

    ConcurrentJSLocker locker(m_codeBlock->m_lock);
    LinkBuffer patchBuffer(*m_vm, *this, m_codeBlock);
    patchBuffer.link(slowCases, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    patchBuffer.link(fastDoneCase, byValInfo->badTypeJump.labelAtOffset(byValInfo->badTypeJumpToDone));
    patchBuffer.link(slowDoneCase, byValInfo->badTypeJump.labelAtOffset(byValInfo->badTypeJumpToNextHotPath));
    if (!m_exceptionChecks.empty())
        patchBuffer.link(m_exceptionChecks, byValInfo->exceptionHandler);

    for (const auto& callSite : m_calls) {
        if (callSite.to)
            patchBuffer.link(callSite.from, FunctionPtr(callSite.to));
    }
    gen.finalize(patchBuffer);

    byValInfo->stubRoutine = FINALIZE_CODE_FOR_STUB(
        m_codeBlock, patchBuffer,
        ("Baseline get_by_val with cached property name '%s' stub for %s, return point %p", propertyName.impl()->utf8().data(), toCString(*m_codeBlock).data(), returnAddress.value()));
    byValInfo->stubInfo = gen.stubInfo();

    MacroAssembler::repatchJump(byValInfo->notIndexJump, CodeLocationLabel(byValInfo->stubRoutine->code().code()));
    MacroAssembler::repatchCall(CodeLocationCall(MacroAssemblerCodePtr(returnAddress)), FunctionPtr(operationGetByValGeneric));
}

void JIT::privateCompilePutByVal(ByValInfo* byValInfo, ReturnAddressPtr returnAddress, JITArrayMode arrayMode)
{
    Instruction* currentInstruction = m_codeBlock->instructions().begin() + byValInfo->bytecodeIndex;
    
    PatchableJump badType;
    JumpList slowCases;

    bool needsLinkForWriteBarrier = false;

    switch (arrayMode) {
    case JITInt32:
        slowCases = emitInt32PutByVal(currentInstruction, badType);
        break;
    case JITDouble:
        slowCases = emitDoublePutByVal(currentInstruction, badType);
        break;
    case JITContiguous:
        slowCases = emitContiguousPutByVal(currentInstruction, badType);
        needsLinkForWriteBarrier = true;
        break;
    case JITArrayStorage:
        slowCases = emitArrayStoragePutByVal(currentInstruction, badType);
        needsLinkForWriteBarrier = true;
        break;
    default:
        TypedArrayType type = typedArrayTypeForJITArrayMode(arrayMode);
        if (isInt(type))
            slowCases = emitIntTypedArrayPutByVal(currentInstruction, badType, type);
        else 
            slowCases = emitFloatTypedArrayPutByVal(currentInstruction, badType, type);
        break;
    }
    
    Jump done = jump();

    LinkBuffer patchBuffer(*m_vm, *this, m_codeBlock);
    patchBuffer.link(badType, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    patchBuffer.link(slowCases, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    patchBuffer.link(done, byValInfo->badTypeJump.labelAtOffset(byValInfo->badTypeJumpToDone));
    if (needsLinkForWriteBarrier) {
        ASSERT(m_calls.last().to == operationWriteBarrierSlowPath);
        patchBuffer.link(m_calls.last().from, operationWriteBarrierSlowPath);
    }
    
    bool isDirect = m_interpreter->getOpcodeID(currentInstruction->u.opcode) == op_put_by_val_direct;
    if (!isDirect) {
        byValInfo->stubRoutine = FINALIZE_CODE_FOR_STUB(
            m_codeBlock, patchBuffer,
            ("Baseline put_by_val stub for %s, return point %p", toCString(*m_codeBlock).data(), returnAddress.value()));
        
    } else {
        byValInfo->stubRoutine = FINALIZE_CODE_FOR_STUB(
            m_codeBlock, patchBuffer,
            ("Baseline put_by_val_direct stub for %s, return point %p", toCString(*m_codeBlock).data(), returnAddress.value()));
    }
    MacroAssembler::repatchJump(byValInfo->badTypeJump, CodeLocationLabel(byValInfo->stubRoutine->code().code()));
    MacroAssembler::repatchCall(CodeLocationCall(MacroAssemblerCodePtr(returnAddress)), FunctionPtr(isDirect ? operationDirectPutByValGeneric : operationPutByValGeneric));
}

void JIT::privateCompilePutByValWithCachedId(ByValInfo* byValInfo, ReturnAddressPtr returnAddress, PutKind putKind, const Identifier& propertyName)
{
    Instruction* currentInstruction = m_codeBlock->instructions().begin() + byValInfo->bytecodeIndex;

    JumpList doneCases;
    JumpList slowCases;

    JITPutByIdGenerator gen = emitPutByValWithCachedId(byValInfo, currentInstruction, putKind, propertyName, doneCases, slowCases);

    ConcurrentJSLocker locker(m_codeBlock->m_lock);
    LinkBuffer patchBuffer(*m_vm, *this, m_codeBlock);
    patchBuffer.link(slowCases, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    patchBuffer.link(doneCases, byValInfo->badTypeJump.labelAtOffset(byValInfo->badTypeJumpToDone));
    if (!m_exceptionChecks.empty())
        patchBuffer.link(m_exceptionChecks, byValInfo->exceptionHandler);

    for (const auto& callSite : m_calls) {
        if (callSite.to)
            patchBuffer.link(callSite.from, FunctionPtr(callSite.to));
    }
    gen.finalize(patchBuffer);

    byValInfo->stubRoutine = FINALIZE_CODE_FOR_STUB(
        m_codeBlock, patchBuffer,
        ("Baseline put_by_val%s with cached property name '%s' stub for %s, return point %p", (putKind == Direct) ? "_direct" : "", propertyName.impl()->utf8().data(), toCString(*m_codeBlock).data(), returnAddress.value()));
    byValInfo->stubInfo = gen.stubInfo();

    MacroAssembler::repatchJump(byValInfo->notIndexJump, CodeLocationLabel(byValInfo->stubRoutine->code().code()));
    MacroAssembler::repatchCall(CodeLocationCall(MacroAssemblerCodePtr(returnAddress)), FunctionPtr(putKind == Direct ? operationDirectPutByValGeneric : operationPutByValGeneric));
}


JIT::JumpList JIT::emitDirectArgumentsGetByVal(Instruction*, PatchableJump& badType)
{
    JumpList slowCases;
    
#if USE(JSVALUE64)
    RegisterID base = regT0;
    RegisterID property = regT1;
    JSValueRegs result = JSValueRegs(regT0);
    RegisterID scratch = regT3;
#else
    RegisterID base = regT0;
    RegisterID property = regT2;
    JSValueRegs result = JSValueRegs(regT1, regT0);
    RegisterID scratch = regT3;
#endif

    load8(Address(base, JSCell::typeInfoTypeOffset()), scratch);
    badType = patchableBranch32(NotEqual, scratch, TrustedImm32(DirectArgumentsType));
    
    slowCases.append(branch32(AboveOrEqual, property, Address(base, DirectArguments::offsetOfLength())));
    slowCases.append(branchTestPtr(NonZero, Address(base, DirectArguments::offsetOfMappedArguments())));
    
    zeroExtend32ToPtr(property, scratch);
    loadValue(BaseIndex(base, scratch, TimesEight, DirectArguments::storageOffset()), result);
    
    return slowCases;
}

JIT::JumpList JIT::emitScopedArgumentsGetByVal(Instruction*, PatchableJump& badType)
{
    JumpList slowCases;
    
#if USE(JSVALUE64)
    RegisterID base = regT0;
    RegisterID property = regT1;
    JSValueRegs result = JSValueRegs(regT0);
    RegisterID scratch = regT3;
    RegisterID scratch2 = regT4;
#else
    RegisterID base = regT0;
    RegisterID property = regT2;
    JSValueRegs result = JSValueRegs(regT1, regT0);
    RegisterID scratch = regT3;
    RegisterID scratch2 = regT4;
#endif

    load8(Address(base, JSCell::typeInfoTypeOffset()), scratch);
    badType = patchableBranch32(NotEqual, scratch, TrustedImm32(ScopedArgumentsType));
    slowCases.append(branch32(AboveOrEqual, property, Address(base, ScopedArguments::offsetOfTotalLength())));
    
    loadPtr(Address(base, ScopedArguments::offsetOfTable()), scratch);
    load32(Address(scratch, ScopedArgumentsTable::offsetOfLength()), scratch2);
    Jump overflowCase = branch32(AboveOrEqual, property, scratch2);
    loadPtr(Address(base, ScopedArguments::offsetOfScope()), scratch2);
    loadPtr(Address(scratch, ScopedArgumentsTable::offsetOfArguments()), scratch);
    load32(BaseIndex(scratch, property, TimesFour), scratch);
    slowCases.append(branch32(Equal, scratch, TrustedImm32(ScopeOffset::invalidOffset)));
    loadValue(BaseIndex(scratch2, scratch, TimesEight, JSEnvironmentRecord::offsetOfVariables()), result);
    Jump done = jump();
    overflowCase.link(this);
    sub32(property, scratch2);
    neg32(scratch2);
    loadValue(BaseIndex(base, scratch2, TimesEight, ScopedArguments::overflowStorageOffset()), result);
    slowCases.append(branchIfEmpty(result));
    done.link(this);
    
    return slowCases;
}

JIT::JumpList JIT::emitIntTypedArrayGetByVal(Instruction*, PatchableJump& badType, TypedArrayType type)
{
    ASSERT(isInt(type));
    
    // The best way to test the array type is to use the classInfo. We need to do so without
    // clobbering the register that holds the indexing type, base, and property.

#if USE(JSVALUE64)
    RegisterID base = regT0;
    RegisterID property = regT1;
    RegisterID resultPayload = regT0;
    RegisterID scratch = regT3;
#else
    RegisterID base = regT0;
    RegisterID property = regT2;
    RegisterID resultPayload = regT0;
    RegisterID resultTag = regT1;
    RegisterID scratch = regT3;
#endif
    
    JumpList slowCases;
    
    load8(Address(base, JSCell::typeInfoTypeOffset()), scratch);
    badType = patchableBranch32(NotEqual, scratch, TrustedImm32(typeForTypedArrayType(type)));
    slowCases.append(branch32(AboveOrEqual, property, Address(base, JSArrayBufferView::offsetOfLength())));
    loadPtr(Address(base, JSArrayBufferView::offsetOfVector()), scratch);
    
    switch (elementSize(type)) {
    case 1:
        if (JSC::isSigned(type))
            load8SignedExtendTo32(BaseIndex(scratch, property, TimesOne), resultPayload);
        else
            load8(BaseIndex(scratch, property, TimesOne), resultPayload);
        break;
    case 2:
        if (JSC::isSigned(type))
            load16SignedExtendTo32(BaseIndex(scratch, property, TimesTwo), resultPayload);
        else
            load16(BaseIndex(scratch, property, TimesTwo), resultPayload);
        break;
    case 4:
        load32(BaseIndex(scratch, property, TimesFour), resultPayload);
        break;
    default:
        CRASH();
    }
    
    Jump done;
    if (type == TypeUint32) {
        Jump canBeInt = branch32(GreaterThanOrEqual, resultPayload, TrustedImm32(0));
        
        convertInt32ToDouble(resultPayload, fpRegT0);
        addDouble(AbsoluteAddress(&twoToThe32), fpRegT0);
#if USE(JSVALUE64)
        moveDoubleTo64(fpRegT0, resultPayload);
        sub64(tagTypeNumberRegister, resultPayload);
#else
        moveDoubleToInts(fpRegT0, resultPayload, resultTag);
#endif
        
        done = jump();
        canBeInt.link(this);
    }

#if USE(JSVALUE64)
    or64(tagTypeNumberRegister, resultPayload);
#else
    move(TrustedImm32(JSValue::Int32Tag), resultTag);
#endif
    if (done.isSet())
        done.link(this);
    return slowCases;
}

JIT::JumpList JIT::emitFloatTypedArrayGetByVal(Instruction*, PatchableJump& badType, TypedArrayType type)
{
    ASSERT(isFloat(type));
    
#if USE(JSVALUE64)
    RegisterID base = regT0;
    RegisterID property = regT1;
    RegisterID resultPayload = regT0;
    RegisterID scratch = regT3;
#else
    RegisterID base = regT0;
    RegisterID property = regT2;
    RegisterID resultPayload = regT0;
    RegisterID resultTag = regT1;
    RegisterID scratch = regT3;
#endif
    
    JumpList slowCases;

    load8(Address(base, JSCell::typeInfoTypeOffset()), scratch);
    badType = patchableBranch32(NotEqual, scratch, TrustedImm32(typeForTypedArrayType(type)));
    slowCases.append(branch32(AboveOrEqual, property, Address(base, JSArrayBufferView::offsetOfLength())));
    loadPtr(Address(base, JSArrayBufferView::offsetOfVector()), scratch);
    
    switch (elementSize(type)) {
    case 4:
        loadFloat(BaseIndex(scratch, property, TimesFour), fpRegT0);
        convertFloatToDouble(fpRegT0, fpRegT0);
        break;
    case 8: {
        loadDouble(BaseIndex(scratch, property, TimesEight), fpRegT0);
        break;
    }
    default:
        CRASH();
    }
    
    Jump notNaN = branchDouble(DoubleEqual, fpRegT0, fpRegT0);
    static const double NaN = PNaN;
    loadDouble(TrustedImmPtr(&NaN), fpRegT0);
    notNaN.link(this);
    
#if USE(JSVALUE64)
    moveDoubleTo64(fpRegT0, resultPayload);
    sub64(tagTypeNumberRegister, resultPayload);
#else
    moveDoubleToInts(fpRegT0, resultPayload, resultTag);
#endif
    return slowCases;    
}

JIT::JumpList JIT::emitIntTypedArrayPutByVal(Instruction* currentInstruction, PatchableJump& badType, TypedArrayType type)
{
    ArrayProfile* profile = currentInstruction[4].u.arrayProfile;
    ASSERT(isInt(type));
    
    int value = currentInstruction[3].u.operand;

#if USE(JSVALUE64)
    RegisterID base = regT0;
    RegisterID property = regT1;
    RegisterID earlyScratch = regT3;
    RegisterID lateScratch = regT2;
#else
    RegisterID base = regT0;
    RegisterID property = regT2;
    RegisterID earlyScratch = regT3;
    RegisterID lateScratch = regT1;
#endif
    
    JumpList slowCases;
    
    load8(Address(base, JSCell::typeInfoTypeOffset()), earlyScratch);
    badType = patchableBranch32(NotEqual, earlyScratch, TrustedImm32(typeForTypedArrayType(type)));
    Jump inBounds = branch32(Below, property, Address(base, JSArrayBufferView::offsetOfLength()));
    emitArrayProfileOutOfBoundsSpecialCase(profile);
    slowCases.append(jump());
    inBounds.link(this);
    
#if USE(JSVALUE64)
    emitGetVirtualRegister(value, earlyScratch);
    slowCases.append(emitJumpIfNotInt(earlyScratch));
#else
    emitLoad(value, lateScratch, earlyScratch);
    slowCases.append(branch32(NotEqual, lateScratch, TrustedImm32(JSValue::Int32Tag)));
#endif
    
    // We would be loading this into base as in get_by_val, except that the slow
    // path expects the base to be unclobbered.
    loadPtr(Address(base, JSArrayBufferView::offsetOfVector()), lateScratch);
    
    if (isClamped(type)) {
        ASSERT(elementSize(type) == 1);
        ASSERT(!JSC::isSigned(type));
        Jump inBounds = branch32(BelowOrEqual, earlyScratch, TrustedImm32(0xff));
        Jump tooBig = branch32(GreaterThan, earlyScratch, TrustedImm32(0xff));
        xor32(earlyScratch, earlyScratch);
        Jump clamped = jump();
        tooBig.link(this);
        move(TrustedImm32(0xff), earlyScratch);
        clamped.link(this);
        inBounds.link(this);
    }
    
    switch (elementSize(type)) {
    case 1:
        store8(earlyScratch, BaseIndex(lateScratch, property, TimesOne));
        break;
    case 2:
        store16(earlyScratch, BaseIndex(lateScratch, property, TimesTwo));
        break;
    case 4:
        store32(earlyScratch, BaseIndex(lateScratch, property, TimesFour));
        break;
    default:
        CRASH();
    }
    
    return slowCases;
}

JIT::JumpList JIT::emitFloatTypedArrayPutByVal(Instruction* currentInstruction, PatchableJump& badType, TypedArrayType type)
{
    ArrayProfile* profile = currentInstruction[4].u.arrayProfile;
    ASSERT(isFloat(type));
    
    int value = currentInstruction[3].u.operand;

#if USE(JSVALUE64)
    RegisterID base = regT0;
    RegisterID property = regT1;
    RegisterID earlyScratch = regT3;
    RegisterID lateScratch = regT2;
#else
    RegisterID base = regT0;
    RegisterID property = regT2;
    RegisterID earlyScratch = regT3;
    RegisterID lateScratch = regT1;
#endif
    
    JumpList slowCases;
    
    load8(Address(base, JSCell::typeInfoTypeOffset()), earlyScratch);
    badType = patchableBranch32(NotEqual, earlyScratch, TrustedImm32(typeForTypedArrayType(type)));
    Jump inBounds = branch32(Below, property, Address(base, JSArrayBufferView::offsetOfLength()));
    emitArrayProfileOutOfBoundsSpecialCase(profile);
    slowCases.append(jump());
    inBounds.link(this);
    
#if USE(JSVALUE64)
    emitGetVirtualRegister(value, earlyScratch);
    Jump doubleCase = emitJumpIfNotInt(earlyScratch);
    convertInt32ToDouble(earlyScratch, fpRegT0);
    Jump ready = jump();
    doubleCase.link(this);
    slowCases.append(emitJumpIfNotNumber(earlyScratch));
    add64(tagTypeNumberRegister, earlyScratch);
    move64ToDouble(earlyScratch, fpRegT0);
    ready.link(this);
#else
    emitLoad(value, lateScratch, earlyScratch);
    Jump doubleCase = branch32(NotEqual, lateScratch, TrustedImm32(JSValue::Int32Tag));
    convertInt32ToDouble(earlyScratch, fpRegT0);
    Jump ready = jump();
    doubleCase.link(this);
    slowCases.append(branch32(Above, lateScratch, TrustedImm32(JSValue::LowestTag)));
    moveIntsToDouble(earlyScratch, lateScratch, fpRegT0, fpRegT1);
    ready.link(this);
#endif
    
    // We would be loading this into base as in get_by_val, except that the slow
    // path expects the base to be unclobbered.
    loadPtr(Address(base, JSArrayBufferView::offsetOfVector()), lateScratch);
    
    switch (elementSize(type)) {
    case 4:
        convertDoubleToFloat(fpRegT0, fpRegT0);
        storeFloat(fpRegT0, BaseIndex(lateScratch, property, TimesFour));
        break;
    case 8:
        storeDouble(fpRegT0, BaseIndex(lateScratch, property, TimesEight));
        break;
    default:
        CRASH();
    }
    
    return slowCases;
}

void JIT::emit_op_define_data_property(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_define_data_property);
    slowPathCall.call();
}

void JIT::emit_op_define_accessor_property(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_define_accessor_property);
    slowPathCall.call();
}

} // namespace JSC

#endif // ENABLE(JIT)
