/*
 * Copyright (C) 2009-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
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

#include "BasicBlockLocation.h"
#include "Exception.h"
#include "Heap.h"
#include "Interpreter.h"
#include "JITInlines.h"
#include "JSArray.h"
#include "JSCell.h"
#include "JSFunction.h"
#include "JSPropertyNameEnumerator.h"
#include "LinkBuffer.h"
#include "MaxFrameExtentForSlowPathCall.h"
#include "SlowPathCall.h"
#include "TypeLocation.h"
#include "TypeProfilerLog.h"
#include "VirtualRegister.h"
#include "Watchdog.h"

namespace JSC {

#if USE(JSVALUE64)

JIT::CodeRef JIT::privateCompileCTINativeCall(VM* vm, NativeFunction)
{
    return vm->getCTIStub(nativeCallGenerator);
}

void JIT::emit_op_mov(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src, regT0);
    emitPutVirtualRegister(dst);
}


void JIT::emit_op_end(Instruction* currentInstruction)
{
    RELEASE_ASSERT(returnValueGPR != callFrameRegister);
    emitGetVirtualRegister(currentInstruction[1].u.operand, returnValueGPR);
    emitRestoreCalleeSaves();
    emitFunctionEpilogue();
    ret();
}

void JIT::emit_op_jmp(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[1].u.operand;
    addJump(jump(), target);
}

void JIT::emit_op_new_object(Instruction* currentInstruction)
{
    Structure* structure = currentInstruction[3].u.objectAllocationProfile->structure();
    size_t allocationSize = JSFinalObject::allocationSize(structure->inlineCapacity());
    MarkedAllocator* allocator = subspaceFor<JSFinalObject>(*m_vm)->allocatorFor(allocationSize);

    RegisterID resultReg = regT0;
    RegisterID allocatorReg = regT1;
    RegisterID scratchReg = regT2;

    move(TrustedImmPtr(allocator), allocatorReg);
    if (allocator)
        addSlowCase(Jump());
    JumpList slowCases;
    emitAllocateJSObject(resultReg, allocator, allocatorReg, TrustedImmPtr(structure), TrustedImmPtr(0), scratchReg, slowCases);
    emitInitializeInlineStorage(resultReg, structure->inlineCapacity());
    addSlowCase(slowCases);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_new_object(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    int dst = currentInstruction[1].u.operand;
    Structure* structure = currentInstruction[3].u.objectAllocationProfile->structure();
    callOperation(operationNewObject, structure);
    emitStoreCell(dst, returnValueGPR);
}

void JIT::emit_op_overrides_has_instance(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int constructor = currentInstruction[2].u.operand;
    int hasInstanceValue = currentInstruction[3].u.operand;

    emitGetVirtualRegister(hasInstanceValue, regT0);

    // We don't jump if we know what Symbol.hasInstance would do.
    Jump customhasInstanceValue = branchPtr(NotEqual, regT0, TrustedImmPtr(m_codeBlock->globalObject()->functionProtoHasInstanceSymbolFunction()));

    emitGetVirtualRegister(constructor, regT0);

    // Check that constructor 'ImplementsDefaultHasInstance' i.e. the object is not a C-API user nor a bound function.
    test8(Zero, Address(regT0, JSCell::typeInfoFlagsOffset()), TrustedImm32(ImplementsDefaultHasInstance), regT0);
    emitTagBool(regT0);
    Jump done = jump();

    customhasInstanceValue.link(this);
    move(TrustedImm32(ValueTrue), regT0);

    done.link(this);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_instanceof(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;
    int proto = currentInstruction[3].u.operand;

    // Load the operands (baseVal, proto, and value respectively) into registers.
    // We use regT0 for baseVal since we will be done with this first, and we can then use it for the result.
    emitGetVirtualRegister(value, regT2);
    emitGetVirtualRegister(proto, regT1);

    // Check that proto are cells. baseVal must be a cell - this is checked by the get_by_id for Symbol.hasInstance.
    emitJumpSlowCaseIfNotJSCell(regT2, value);
    emitJumpSlowCaseIfNotJSCell(regT1, proto);

    // Check that prototype is an object
    addSlowCase(emitJumpIfCellNotObject(regT1));
    
    // Optimistically load the result true, and start looping.
    // Initially, regT1 still contains proto and regT2 still contains value.
    // As we loop regT2 will be updated with its prototype, recursively walking the prototype chain.
    move(TrustedImm64(JSValue::encode(jsBoolean(true))), regT0);
    Label loop(this);

    addSlowCase(branch8(Equal, Address(regT2, JSCell::typeInfoTypeOffset()), TrustedImm32(ProxyObjectType)));

    // Load the prototype of the object in regT2.  If this is equal to regT1 - WIN!
    // Otherwise, check if we've hit null - if we have then drop out of the loop, if not go again.
    emitLoadStructure(regT2, regT2, regT3);
    load64(Address(regT2, Structure::prototypeOffset()), regT2);
    Jump isInstance = branchPtr(Equal, regT2, regT1);
    emitJumpIfJSCell(regT2).linkTo(loop, this);

    // We get here either by dropping out of the loop, or if value was not an Object.  Result is false.
    move(TrustedImm64(JSValue::encode(jsBoolean(false))), regT0);

    // isInstance jumps right down to here, to skip setting the result to false (it has already set true).
    isInstance.link(this);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_instanceof_custom(Instruction*)
{
    // This always goes to slow path since we expect it to be rare.
    addSlowCase(jump());
}
    
void JIT::emit_op_is_empty(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;

    emitGetVirtualRegister(value, regT0);
    compare64(Equal, regT0, TrustedImm32(JSValue::encode(JSValue())), regT0);

    emitTagBool(regT0);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_is_undefined(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;
    
    emitGetVirtualRegister(value, regT0);
    Jump isCell = emitJumpIfJSCell(regT0);

    compare64(Equal, regT0, TrustedImm32(ValueUndefined), regT0);
    Jump done = jump();
    
    isCell.link(this);
    Jump isMasqueradesAsUndefined = branchTest8(NonZero, Address(regT0, JSCell::typeInfoFlagsOffset()), TrustedImm32(MasqueradesAsUndefined));
    move(TrustedImm32(0), regT0);
    Jump notMasqueradesAsUndefined = jump();

    isMasqueradesAsUndefined.link(this);
    emitLoadStructure(regT0, regT1, regT2);
    move(TrustedImmPtr(m_codeBlock->globalObject()), regT0);
    loadPtr(Address(regT1, Structure::globalObjectOffset()), regT1);
    comparePtr(Equal, regT0, regT1, regT0);

    notMasqueradesAsUndefined.link(this);
    done.link(this);
    emitTagBool(regT0);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_is_boolean(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;
    
    emitGetVirtualRegister(value, regT0);
    xor64(TrustedImm32(static_cast<int32_t>(ValueFalse)), regT0);
    test64(Zero, regT0, TrustedImm32(static_cast<int32_t>(~1)), regT0);
    emitTagBool(regT0);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_is_number(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;
    
    emitGetVirtualRegister(value, regT0);
    test64(NonZero, regT0, tagTypeNumberRegister, regT0);
    emitTagBool(regT0);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_is_cell_with_type(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;
    int type = currentInstruction[3].u.operand;

    emitGetVirtualRegister(value, regT0);
    Jump isNotCell = emitJumpIfNotJSCell(regT0);

    compare8(Equal, Address(regT0, JSCell::typeInfoTypeOffset()), TrustedImm32(type), regT0);
    emitTagBool(regT0);
    Jump done = jump();

    isNotCell.link(this);
    move(TrustedImm32(ValueFalse), regT0);

    done.link(this);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_is_object(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;

    emitGetVirtualRegister(value, regT0);
    Jump isNotCell = emitJumpIfNotJSCell(regT0);

    compare8(AboveOrEqual, Address(regT0, JSCell::typeInfoTypeOffset()), TrustedImm32(ObjectType), regT0);
    emitTagBool(regT0);
    Jump done = jump();

    isNotCell.link(this);
    move(TrustedImm32(ValueFalse), regT0);

    done.link(this);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_ret(Instruction* currentInstruction)
{
    ASSERT(callFrameRegister != regT1);
    ASSERT(regT1 != returnValueGPR);
    ASSERT(returnValueGPR != callFrameRegister);

    // Return the result in %eax.
    emitGetVirtualRegister(currentInstruction[1].u.operand, returnValueGPR);

    checkStackPointerAlignment();
    emitRestoreCalleeSaves();
    emitFunctionEpilogue();
    ret();
}

void JIT::emit_op_to_primitive(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src, regT0);
    
    Jump isImm = emitJumpIfNotJSCell(regT0);
    addSlowCase(emitJumpIfCellObject(regT0));
    isImm.link(this);

    if (dst != src)
        emitPutVirtualRegister(dst);

}

void JIT::emit_op_set_function_name(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT1);
    callOperation(operationSetFunctionName, regT0, regT1);
}

void JIT::emit_op_strcat(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_strcat);
    slowPathCall.call();
}

void JIT::emit_op_not(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT0);

    // Invert against JSValue(false); if the value was tagged as a boolean, then all bits will be
    // clear other than the low bit (which will be 0 or 1 for false or true inputs respectively).
    // Then invert against JSValue(true), which will add the tag back in, and flip the low bit.
    xor64(TrustedImm32(static_cast<int32_t>(ValueFalse)), regT0);
    addSlowCase(branchTestPtr(NonZero, regT0, TrustedImm32(static_cast<int32_t>(~1))));
    xor64(TrustedImm32(static_cast<int32_t>(ValueTrue)), regT0);

    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_jfalse(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[2].u.operand;

    GPRReg value = regT0;
    GPRReg result = regT1;
    GPRReg scratch = regT2;
    bool shouldCheckMasqueradesAsUndefined = true;

    emitGetVirtualRegister(currentInstruction[1].u.operand, value);
    emitConvertValueToBoolean(JSValueRegs(value), result, scratch, fpRegT0, fpRegT1, shouldCheckMasqueradesAsUndefined, m_codeBlock->globalObject());

    addJump(branchTest32(Zero, result), target);
}

void JIT::emit_op_jeq_null(Instruction* currentInstruction)
{
    int src = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    // First, handle JSCell cases - check MasqueradesAsUndefined bit on the structure.
    Jump isNotMasqueradesAsUndefined = branchTest8(Zero, Address(regT0, JSCell::typeInfoFlagsOffset()), TrustedImm32(MasqueradesAsUndefined));
    emitLoadStructure(regT0, regT2, regT1);
    move(TrustedImmPtr(m_codeBlock->globalObject()), regT0);
    addJump(branchPtr(Equal, Address(regT2, Structure::globalObjectOffset()), regT0), target);
    Jump masqueradesGlobalObjectIsForeign = jump();

    // Now handle the immediate cases - undefined & null
    isImmediate.link(this);
    and64(TrustedImm32(~TagBitUndefined), regT0);
    addJump(branch64(Equal, regT0, TrustedImm64(JSValue::encode(jsNull()))), target);            

    isNotMasqueradesAsUndefined.link(this);
    masqueradesGlobalObjectIsForeign.link(this);
};
void JIT::emit_op_jneq_null(Instruction* currentInstruction)
{
    int src = currentInstruction[1].u.operand;
    unsigned target = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    // First, handle JSCell cases - check MasqueradesAsUndefined bit on the structure.
    addJump(branchTest8(Zero, Address(regT0, JSCell::typeInfoFlagsOffset()), TrustedImm32(MasqueradesAsUndefined)), target);
    emitLoadStructure(regT0, regT2, regT1);
    move(TrustedImmPtr(m_codeBlock->globalObject()), regT0);
    addJump(branchPtr(NotEqual, Address(regT2, Structure::globalObjectOffset()), regT0), target);
    Jump wasNotImmediate = jump();

    // Now handle the immediate cases - undefined & null
    isImmediate.link(this);
    and64(TrustedImm32(~TagBitUndefined), regT0);
    addJump(branch64(NotEqual, regT0, TrustedImm64(JSValue::encode(jsNull()))), target);            

    wasNotImmediate.link(this);
}

void JIT::emit_op_jneq_ptr(Instruction* currentInstruction)
{
    int src = currentInstruction[1].u.operand;
    Special::Pointer ptr = currentInstruction[2].u.specialPointer;
    unsigned target = currentInstruction[3].u.operand;
    
    emitGetVirtualRegister(src, regT0);
    CCallHelpers::Jump equal = branchPtr(Equal, regT0, TrustedImmPtr(actualPointerFor(m_codeBlock, ptr)));
    store32(TrustedImm32(1), &currentInstruction[4].u.operand);
    addJump(jump(), target);
    equal.link(this);
}

void JIT::emit_op_eq(Instruction* currentInstruction)
{
    emitGetVirtualRegisters(currentInstruction[2].u.operand, regT0, currentInstruction[3].u.operand, regT1);
    emitJumpSlowCaseIfNotInt(regT0, regT1, regT2);
    compare32(Equal, regT1, regT0, regT0);
    emitTagBool(regT0);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_jtrue(Instruction* currentInstruction)
{
    unsigned target = currentInstruction[2].u.operand;

    GPRReg value = regT0;
    GPRReg result = regT1;
    GPRReg scratch = regT2;
    bool shouldCheckMasqueradesAsUndefined = true;
    emitGetVirtualRegister(currentInstruction[1].u.operand, value);
    emitConvertValueToBoolean(JSValueRegs(value), result, scratch, fpRegT0, fpRegT1, shouldCheckMasqueradesAsUndefined, m_codeBlock->globalObject());
    addJump(branchTest32(NonZero, result), target);
}

void JIT::emit_op_neq(Instruction* currentInstruction)
{
    emitGetVirtualRegisters(currentInstruction[2].u.operand, regT0, currentInstruction[3].u.operand, regT1);
    emitJumpSlowCaseIfNotInt(regT0, regT1, regT2);
    compare32(NotEqual, regT1, regT0, regT0);
    emitTagBool(regT0);

    emitPutVirtualRegister(currentInstruction[1].u.operand);

}

void JIT::emit_op_throw(Instruction* currentInstruction)
{
    ASSERT(regT0 == returnValueGPR);
    copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    callOperationNoExceptionCheck(operationThrow, regT0);
    jumpToExceptionHandler();
}

void JIT::emit_op_push_with_scope(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_push_with_scope);
    slowPathCall.call();
}

void JIT::compileOpStrictEq(Instruction* currentInstruction, CompileOpStrictEqType type)
{
    int dst = currentInstruction[1].u.operand;
    int src1 = currentInstruction[2].u.operand;
    int src2 = currentInstruction[3].u.operand;

    emitGetVirtualRegisters(src1, regT0, src2, regT1);
    
    // Jump slow if both are cells (to cover strings).
    move(regT0, regT2);
    or64(regT1, regT2);
    addSlowCase(emitJumpIfJSCell(regT2));
    
    // Jump slow if either is a double. First test if it's an integer, which is fine, and then test
    // if it's a double.
    Jump leftOK = emitJumpIfInt(regT0);
    addSlowCase(emitJumpIfNumber(regT0));
    leftOK.link(this);
    Jump rightOK = emitJumpIfInt(regT1);
    addSlowCase(emitJumpIfNumber(regT1));
    rightOK.link(this);

    if (type == OpStrictEq)
        compare64(Equal, regT1, regT0, regT0);
    else
        compare64(NotEqual, regT1, regT0, regT0);
    emitTagBool(regT0);

    emitPutVirtualRegister(dst);
}

void JIT::emit_op_stricteq(Instruction* currentInstruction)
{
    compileOpStrictEq(currentInstruction, OpStrictEq);
}

void JIT::emit_op_nstricteq(Instruction* currentInstruction)
{
    compileOpStrictEq(currentInstruction, OpNStrictEq);
}

void JIT::emit_op_to_number(Instruction* currentInstruction)
{
    int dstVReg = currentInstruction[1].u.operand;
    int srcVReg = currentInstruction[2].u.operand;
    emitGetVirtualRegister(srcVReg, regT0);
    
    addSlowCase(emitJumpIfNotNumber(regT0));

    emitValueProfilingSite();
    if (srcVReg != dstVReg)
        emitPutVirtualRegister(dstVReg);
}

void JIT::emit_op_to_string(Instruction* currentInstruction)
{
    int srcVReg = currentInstruction[2].u.operand;
    emitGetVirtualRegister(srcVReg, regT0);

    addSlowCase(emitJumpIfNotJSCell(regT0));
    addSlowCase(branch8(NotEqual, Address(regT0, JSCell::typeInfoTypeOffset()), TrustedImm32(StringType)));

    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emit_op_catch(Instruction* currentInstruction)
{
    restoreCalleeSavesFromVMEntryFrameCalleeSavesBuffer();

    move(TrustedImmPtr(m_vm), regT3);
    load64(Address(regT3, VM::callFrameForCatchOffset()), callFrameRegister);
    storePtr(TrustedImmPtr(nullptr), Address(regT3, VM::callFrameForCatchOffset()));

    addPtr(TrustedImm32(stackPointerOffsetFor(codeBlock()) * sizeof(Register)), callFrameRegister, stackPointerRegister);

    callOperationNoExceptionCheck(operationCheckIfExceptionIsUncatchableAndNotifyProfiler);
    Jump isCatchableException = branchTest32(Zero, returnValueGPR);
    jumpToExceptionHandler();
    isCatchableException.link(this);

    move(TrustedImmPtr(m_vm), regT3);
    load64(Address(regT3, VM::exceptionOffset()), regT0);
    store64(TrustedImm64(JSValue::encode(JSValue())), Address(regT3, VM::exceptionOffset()));
    emitPutVirtualRegister(currentInstruction[1].u.operand);

    load64(Address(regT0, Exception::valueOffset()), regT0);
    emitPutVirtualRegister(currentInstruction[2].u.operand);
}

void JIT::emit_op_assert(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_assert);
    slowPathCall.call();
}

void JIT::emit_op_create_lexical_environment(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_create_lexical_environment);
    slowPathCall.call();
}

void JIT::emit_op_get_parent_scope(Instruction* currentInstruction)
{
    int currentScope = currentInstruction[2].u.operand;
    emitGetVirtualRegister(currentScope, regT0);
    loadPtr(Address(regT0, JSScope::offsetOfNext()), regT0);
    emitStoreCell(currentInstruction[1].u.operand, regT0);
}

void JIT::emit_op_switch_imm(Instruction* currentInstruction)
{
    size_t tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    SimpleJumpTable* jumpTable = &m_codeBlock->switchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeOffset, defaultOffset, SwitchRecord::Immediate));
    jumpTable->ensureCTITable();

    emitGetVirtualRegister(scrutinee, regT0);
    callOperation(operationSwitchImmWithUnknownKeyType, regT0, tableIndex);
    jump(returnValueGPR);
}

void JIT::emit_op_switch_char(Instruction* currentInstruction)
{
    size_t tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    SimpleJumpTable* jumpTable = &m_codeBlock->switchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeOffset, defaultOffset, SwitchRecord::Character));
    jumpTable->ensureCTITable();

    emitGetVirtualRegister(scrutinee, regT0);
    callOperation(operationSwitchCharWithUnknownKeyType, regT0, tableIndex);
    jump(returnValueGPR);
}

void JIT::emit_op_switch_string(Instruction* currentInstruction)
{
    size_t tableIndex = currentInstruction[1].u.operand;
    unsigned defaultOffset = currentInstruction[2].u.operand;
    unsigned scrutinee = currentInstruction[3].u.operand;

    // create jump table for switch destinations, track this switch statement.
    StringJumpTable* jumpTable = &m_codeBlock->stringSwitchJumpTable(tableIndex);
    m_switches.append(SwitchRecord(jumpTable, m_bytecodeOffset, defaultOffset));

    emitGetVirtualRegister(scrutinee, regT0);
    callOperation(operationSwitchStringWithUnknownKeyType, regT0, tableIndex);
    jump(returnValueGPR);
}

void JIT::emit_op_debug(Instruction* currentInstruction)
{
    load32(codeBlock()->debuggerRequestsAddress(), regT0);
    Jump noDebuggerRequests = branchTest32(Zero, regT0);
    callOperation(operationDebug, currentInstruction[1].u.operand);
    noDebuggerRequests.link(this);
}

void JIT::emit_op_eq_null(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src1 = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src1, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    Jump isMasqueradesAsUndefined = branchTest8(NonZero, Address(regT0, JSCell::typeInfoFlagsOffset()), TrustedImm32(MasqueradesAsUndefined));
    move(TrustedImm32(0), regT0);
    Jump wasNotMasqueradesAsUndefined = jump();

    isMasqueradesAsUndefined.link(this);
    emitLoadStructure(regT0, regT2, regT1);
    move(TrustedImmPtr(m_codeBlock->globalObject()), regT0);
    loadPtr(Address(regT2, Structure::globalObjectOffset()), regT2);
    comparePtr(Equal, regT0, regT2, regT0);
    Jump wasNotImmediate = jump();

    isImmediate.link(this);

    and64(TrustedImm32(~TagBitUndefined), regT0);
    compare64(Equal, regT0, TrustedImm32(ValueNull), regT0);

    wasNotImmediate.link(this);
    wasNotMasqueradesAsUndefined.link(this);

    emitTagBool(regT0);
    emitPutVirtualRegister(dst);

}

void JIT::emit_op_neq_null(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int src1 = currentInstruction[2].u.operand;

    emitGetVirtualRegister(src1, regT0);
    Jump isImmediate = emitJumpIfNotJSCell(regT0);

    Jump isMasqueradesAsUndefined = branchTest8(NonZero, Address(regT0, JSCell::typeInfoFlagsOffset()), TrustedImm32(MasqueradesAsUndefined));
    move(TrustedImm32(1), regT0);
    Jump wasNotMasqueradesAsUndefined = jump();

    isMasqueradesAsUndefined.link(this);
    emitLoadStructure(regT0, regT2, regT1);
    move(TrustedImmPtr(m_codeBlock->globalObject()), regT0);
    loadPtr(Address(regT2, Structure::globalObjectOffset()), regT2);
    comparePtr(NotEqual, regT0, regT2, regT0);
    Jump wasNotImmediate = jump();

    isImmediate.link(this);

    and64(TrustedImm32(~TagBitUndefined), regT0);
    compare64(NotEqual, regT0, TrustedImm32(ValueNull), regT0);

    wasNotImmediate.link(this);
    wasNotMasqueradesAsUndefined.link(this);

    emitTagBool(regT0);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_enter(Instruction*)
{
    // Even though CTI doesn't use them, we initialize our constant
    // registers to zap stale pointers, to avoid unnecessarily prolonging
    // object lifetime and increasing GC pressure.
    size_t count = m_codeBlock->m_numVars;
    for (size_t j = CodeBlock::llintBaselineCalleeSaveSpaceAsVirtualRegisters(); j < count; ++j)
        emitInitRegister(virtualRegisterForLocal(j).offset());

    emitWriteBarrier(m_codeBlock);

    emitEnterOptimizationCheck();
}

void JIT::emit_op_get_scope(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    emitGetFromCallFrameHeaderPtr(CallFrameSlot::callee, regT0);
    loadPtr(Address(regT0, JSFunction::offsetOfScopeChain()), regT0);
    emitStoreCell(dst, regT0);
}

void JIT::emit_op_to_this(Instruction* currentInstruction)
{
    WriteBarrierBase<Structure>* cachedStructure = &currentInstruction[2].u.structure;
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT1);

    emitJumpSlowCaseIfNotJSCell(regT1);

    addSlowCase(branch8(NotEqual, Address(regT1, JSCell::typeInfoTypeOffset()), TrustedImm32(FinalObjectType)));
    loadPtr(cachedStructure, regT2);
    addSlowCase(branchTestPtr(Zero, regT2));
    load32(Address(regT2, Structure::structureIDOffset()), regT2);
    addSlowCase(branch32(NotEqual, Address(regT1, JSCell::structureIDOffset()), regT2));
}

void JIT::emit_op_create_this(Instruction* currentInstruction)
{
    int callee = currentInstruction[2].u.operand;
    WriteBarrierBase<JSCell>* cachedFunction = &currentInstruction[4].u.jsCell;
    RegisterID calleeReg = regT0;
    RegisterID rareDataReg = regT4;
    RegisterID resultReg = regT0;
    RegisterID allocatorReg = regT1;
    RegisterID structureReg = regT2;
    RegisterID cachedFunctionReg = regT4;
    RegisterID scratchReg = regT3;

    emitGetVirtualRegister(callee, calleeReg);
    addSlowCase(branch8(NotEqual, Address(calleeReg, JSCell::typeInfoTypeOffset()), TrustedImm32(JSFunctionType)));
    loadPtr(Address(calleeReg, JSFunction::offsetOfRareData()), rareDataReg);
    addSlowCase(branchTestPtr(Zero, rareDataReg));
    loadPtr(Address(rareDataReg, FunctionRareData::offsetOfObjectAllocationProfile() + ObjectAllocationProfile::offsetOfAllocator()), allocatorReg);
    loadPtr(Address(rareDataReg, FunctionRareData::offsetOfObjectAllocationProfile() + ObjectAllocationProfile::offsetOfStructure()), structureReg);
    addSlowCase(branchTestPtr(Zero, allocatorReg));

    loadPtr(cachedFunction, cachedFunctionReg);
    Jump hasSeenMultipleCallees = branchPtr(Equal, cachedFunctionReg, TrustedImmPtr(JSCell::seenMultipleCalleeObjects()));
    addSlowCase(branchPtr(NotEqual, calleeReg, cachedFunctionReg));
    hasSeenMultipleCallees.link(this);

    JumpList slowCases;
    emitAllocateJSObject(resultReg, nullptr, allocatorReg, structureReg, TrustedImmPtr(0), scratchReg, slowCases);
    emitGetVirtualRegister(callee, scratchReg);
    loadPtr(Address(scratchReg, JSFunction::offsetOfRareData()), scratchReg);
    load32(Address(scratchReg, FunctionRareData::offsetOfObjectAllocationProfile() + ObjectAllocationProfile::offsetOfInlineCapacity()), scratchReg);
    emitInitializeInlineStorage(resultReg, scratchReg);
    addSlowCase(slowCases);
    emitPutVirtualRegister(currentInstruction[1].u.operand);
}

void JIT::emitSlow_op_create_this(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter); // Callee::m_type != JSFunctionType.
    linkSlowCase(iter); // doesn't have rare data
    linkSlowCase(iter); // doesn't have an allocation profile
    linkSlowCase(iter); // allocation failed (no allocator)
    linkSlowCase(iter); // allocation failed (allocator empty)
    linkSlowCase(iter); // cached function didn't match

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_create_this);
    slowPathCall.call();
}

void JIT::emit_op_check_tdz(Instruction* currentInstruction)
{
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT0);
    addSlowCase(branchTest64(Zero, regT0));
}

void JIT::emitSlow_op_check_tdz(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_throw_tdz_error);
    slowPathCall.call();
}


// Slow cases

void JIT::emitSlow_op_to_this(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_to_this);
    slowPathCall.call();
}

void JIT::emitSlow_op_to_primitive(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_to_primitive);
    slowPathCall.call();
}

void JIT::emitSlow_op_not(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_not);
    slowPathCall.call();
}

void JIT::emitSlow_op_eq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    callOperation(operationCompareEq, regT0, regT1);
    emitTagBool(returnValueGPR);
    emitPutVirtualRegister(currentInstruction[1].u.operand, returnValueGPR);
}

void JIT::emitSlow_op_neq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    callOperation(operationCompareEq, regT0, regT1);
    xor32(TrustedImm32(0x1), regT0);
    emitTagBool(returnValueGPR);
    emitPutVirtualRegister(currentInstruction[1].u.operand, returnValueGPR);
}

void JIT::emitSlow_op_stricteq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_stricteq);
    slowPathCall.call();
}

void JIT::emitSlow_op_nstricteq(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);
    linkSlowCase(iter);
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_nstricteq);
    slowPathCall.call();
}

void JIT::emitSlow_op_instanceof(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;
    int proto = currentInstruction[3].u.operand;

    linkSlowCaseIfNotJSCell(iter, value);
    linkSlowCaseIfNotJSCell(iter, proto);
    linkSlowCase(iter);
    linkSlowCase(iter);
    emitGetVirtualRegister(value, regT0);
    emitGetVirtualRegister(proto, regT1);
    callOperation(operationInstanceOf, dst, regT0, regT1);
}

void JIT::emitSlow_op_instanceof_custom(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int dst = currentInstruction[1].u.operand;
    int value = currentInstruction[2].u.operand;
    int constructor = currentInstruction[3].u.operand;
    int hasInstanceValue = currentInstruction[4].u.operand;

    linkSlowCase(iter);
    emitGetVirtualRegister(value, regT0);
    emitGetVirtualRegister(constructor, regT1);
    emitGetVirtualRegister(hasInstanceValue, regT2);
    callOperation(operationInstanceOfCustom, regT0, regT1, regT2);
    emitTagBool(returnValueGPR);
    emitPutVirtualRegister(dst, returnValueGPR);
}

void JIT::emitSlow_op_to_number(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_to_number);
    slowPathCall.call();
}

void JIT::emitSlow_op_to_string(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter); // Not JSCell.
    linkSlowCase(iter); // Not JSString.

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_to_string);
    slowPathCall.call();
}

#endif // USE(JSVALUE64)

void JIT::emit_op_loop_hint(Instruction*)
{
    // Emit the JIT optimization check: 
    if (canBeOptimized()) {
        addSlowCase(branchAdd32(PositiveOrZero, TrustedImm32(Options::executionCounterIncrementForLoop()),
            AbsoluteAddress(m_codeBlock->addressOfJITExecuteCounter())));
    }
}

void JIT::emitSlow_op_loop_hint(Instruction*, Vector<SlowCaseEntry>::iterator& iter)
{
#if ENABLE(DFG_JIT)
    // Emit the slow path for the JIT optimization check:
    if (canBeOptimized()) {
        linkSlowCase(iter);

        copyCalleeSavesFromFrameOrRegisterToVMEntryFrameCalleeSavesBuffer();

        callOperation(operationOptimize, m_bytecodeOffset);
        Jump noOptimizedEntry = branchTestPtr(Zero, returnValueGPR);
        if (!ASSERT_DISABLED) {
            Jump ok = branchPtr(MacroAssembler::Above, returnValueGPR, TrustedImmPtr(bitwise_cast<void*>(static_cast<intptr_t>(1000))));
            abortWithReason(JITUnreasonableLoopHintJumpTarget);
            ok.link(this);
        }
        jump(returnValueGPR);
        noOptimizedEntry.link(this);

        emitJumpSlowToHot(jump(), OPCODE_LENGTH(op_loop_hint));
    }
#else
    UNUSED_PARAM(iter);
#endif
}

void JIT::emit_op_throw_static_error(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_throw_static_error);
    slowPathCall.call();
}

void JIT::emit_op_check_traps(Instruction*)
{
    addSlowCase(branchTest8(NonZero, AbsoluteAddress(m_vm->needTrapHandlingAddress())));
}

void JIT::emitSlow_op_check_traps(Instruction*, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    callOperation(operationHandleTraps);
}

void JIT::emit_op_new_regexp(Instruction* currentInstruction)
{
    callOperation(operationNewRegexp, currentInstruction[1].u.operand, m_codeBlock->regexp(currentInstruction[2].u.operand));
}

void JIT::emitNewFuncCommon(Instruction* currentInstruction)
{
    Jump lazyJump;
    int dst = currentInstruction[1].u.operand;

#if USE(JSVALUE64)
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT0);
#else
    emitLoadPayload(currentInstruction[2].u.operand, regT0);
#endif
    FunctionExecutable* funcExec = m_codeBlock->functionDecl(currentInstruction[3].u.operand);

    OpcodeID opcodeID = m_vm->interpreter->getOpcodeID(currentInstruction->u.opcode);
    if (opcodeID == op_new_func)
        callOperation(operationNewFunction, dst, regT0, funcExec);
    else if (opcodeID == op_new_generator_func)
        callOperation(operationNewGeneratorFunction, dst, regT0, funcExec);
    else {
        ASSERT(opcodeID == op_new_async_func);
        callOperation(operationNewAsyncFunction, dst, regT0, funcExec);
    }
}

void JIT::emit_op_new_func(Instruction* currentInstruction)
{
    emitNewFuncCommon(currentInstruction);
}

void JIT::emit_op_new_generator_func(Instruction* currentInstruction)
{
    emitNewFuncCommon(currentInstruction);
}

void JIT::emit_op_new_async_func(Instruction* currentInstruction)
{
    emitNewFuncCommon(currentInstruction);
}

void JIT::emitNewFuncExprCommon(Instruction* currentInstruction)
{
    Jump notUndefinedScope;
    int dst = currentInstruction[1].u.operand;
#if USE(JSVALUE64)
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT0);
    notUndefinedScope = branch64(NotEqual, regT0, TrustedImm64(JSValue::encode(jsUndefined())));
    store64(TrustedImm64(JSValue::encode(jsUndefined())), Address(callFrameRegister, sizeof(Register) * dst));
#else
    emitLoadPayload(currentInstruction[2].u.operand, regT0);
    notUndefinedScope = branch32(NotEqual, tagFor(currentInstruction[2].u.operand), TrustedImm32(JSValue::UndefinedTag));
    emitStore(dst, jsUndefined());
#endif
    Jump done = jump();
    notUndefinedScope.link(this);
        
    FunctionExecutable* function = m_codeBlock->functionExpr(currentInstruction[3].u.operand);
    OpcodeID opcodeID = m_vm->interpreter->getOpcodeID(currentInstruction->u.opcode);

    if (opcodeID == op_new_func_exp)
        callOperation(operationNewFunction, dst, regT0, function);
    else if (opcodeID == op_new_generator_func_exp)
        callOperation(operationNewGeneratorFunction, dst, regT0, function);
    else {
        ASSERT(opcodeID == op_new_async_func_exp);
        callOperation(operationNewAsyncFunction, dst, regT0, function);
    }

    done.link(this);
}

void JIT::emit_op_new_func_exp(Instruction* currentInstruction)
{
    emitNewFuncExprCommon(currentInstruction);
}

void JIT::emit_op_new_generator_func_exp(Instruction* currentInstruction)
{
    emitNewFuncExprCommon(currentInstruction);
}

void JIT::emit_op_new_async_func_exp(Instruction* currentInstruction)
{
    emitNewFuncExprCommon(currentInstruction);
}

void JIT::emit_op_new_array(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int valuesIndex = currentInstruction[2].u.operand;
    int size = currentInstruction[3].u.operand;
    addPtr(TrustedImm32(valuesIndex * sizeof(Register)), callFrameRegister, regT0);
    callOperation(operationNewArrayWithProfile, dst,
        currentInstruction[4].u.arrayAllocationProfile, regT0, size);
}

void JIT::emit_op_new_array_with_size(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int sizeIndex = currentInstruction[2].u.operand;
#if USE(JSVALUE64)
    emitGetVirtualRegister(sizeIndex, regT0);
    callOperation(operationNewArrayWithSizeAndProfile, dst,
        currentInstruction[3].u.arrayAllocationProfile, regT0);
#else
    emitLoad(sizeIndex, regT1, regT0);
    callOperation(operationNewArrayWithSizeAndProfile, dst,
        currentInstruction[3].u.arrayAllocationProfile, regT1, regT0);
#endif
}

void JIT::emit_op_new_array_buffer(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int valuesIndex = currentInstruction[2].u.operand;
    int size = currentInstruction[3].u.operand;
    const JSValue* values = codeBlock()->constantBuffer(valuesIndex);
    callOperation(operationNewArrayBufferWithProfile, dst, currentInstruction[4].u.arrayAllocationProfile, values, size);
}

void JIT::emit_op_new_array_with_spread(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_new_array_with_spread);
    slowPathCall.call();
}

void JIT::emit_op_spread(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_spread);
    slowPathCall.call();
}

#if USE(JSVALUE64)
void JIT::emit_op_has_structure_property(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int enumerator = currentInstruction[4].u.operand;

    emitGetVirtualRegister(base, regT0);
    emitGetVirtualRegister(enumerator, regT1);
    emitJumpSlowCaseIfNotJSCell(regT0, base);

    load32(Address(regT0, JSCell::structureIDOffset()), regT0);
    addSlowCase(branch32(NotEqual, regT0, Address(regT1, JSPropertyNameEnumerator::cachedStructureIDOffset())));
    
    move(TrustedImm64(JSValue::encode(jsBoolean(true))), regT0);
    emitPutVirtualRegister(dst);
}

void JIT::privateCompileHasIndexedProperty(ByValInfo* byValInfo, ReturnAddressPtr returnAddress, JITArrayMode arrayMode)
{
    Instruction* currentInstruction = m_codeBlock->instructions().begin() + byValInfo->bytecodeIndex;
    
    PatchableJump badType;
    
    // FIXME: Add support for other types like TypedArrays and Arguments.
    // See https://bugs.webkit.org/show_bug.cgi?id=135033 and https://bugs.webkit.org/show_bug.cgi?id=135034.
    JumpList slowCases = emitLoadForArrayMode(currentInstruction, arrayMode, badType);
    move(TrustedImm64(JSValue::encode(jsBoolean(true))), regT0);
    Jump done = jump();

    LinkBuffer patchBuffer(*m_vm, *this, m_codeBlock);
    
    patchBuffer.link(badType, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    patchBuffer.link(slowCases, CodeLocationLabel(MacroAssemblerCodePtr::createFromExecutableAddress(returnAddress.value())).labelAtOffset(byValInfo->returnAddressToSlowPath));
    
    patchBuffer.link(done, byValInfo->badTypeJump.labelAtOffset(byValInfo->badTypeJumpToDone));
    
    byValInfo->stubRoutine = FINALIZE_CODE_FOR_STUB(
        m_codeBlock, patchBuffer,
        ("Baseline has_indexed_property stub for %s, return point %p", toCString(*m_codeBlock).data(), returnAddress.value()));
    
    MacroAssembler::repatchJump(byValInfo->badTypeJump, CodeLocationLabel(byValInfo->stubRoutine->code().code()));
    MacroAssembler::repatchCall(CodeLocationCall(MacroAssemblerCodePtr(returnAddress)), FunctionPtr(operationHasIndexedPropertyGeneric));
}

void JIT::emit_op_has_indexed_property(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int property = currentInstruction[3].u.operand;
    ArrayProfile* profile = currentInstruction[4].u.arrayProfile;
    ByValInfo* byValInfo = m_codeBlock->addByValInfo();
    
    emitGetVirtualRegisters(base, regT0, property, regT1);

    // This is technically incorrect - we're zero-extending an int32. On the hot path this doesn't matter.
    // We check the value as if it was a uint32 against the m_vectorLength - which will always fail if
    // number was signed since m_vectorLength is always less than intmax (since the total allocation
    // size is always less than 4Gb). As such zero extending will have been correct (and extending the value
    // to 64-bits is necessary since it's used in the address calculation. We zero extend rather than sign
    // extending since it makes it easier to re-tag the value in the slow case.
    zeroExtend32ToPtr(regT1, regT1);

    emitJumpSlowCaseIfNotJSCell(regT0, base);
    emitArrayProfilingSiteWithCell(regT0, regT2, profile);
    and32(TrustedImm32(IndexingShapeMask), regT2);

    JITArrayMode mode = chooseArrayMode(profile);
    PatchableJump badType;

    // FIXME: Add support for other types like TypedArrays and Arguments.
    // See https://bugs.webkit.org/show_bug.cgi?id=135033 and https://bugs.webkit.org/show_bug.cgi?id=135034.
    JumpList slowCases = emitLoadForArrayMode(currentInstruction, mode, badType);
    
    move(TrustedImm64(JSValue::encode(jsBoolean(true))), regT0);

    addSlowCase(badType);
    addSlowCase(slowCases);
    
    Label done = label();
    
    emitPutVirtualRegister(dst);

    Label nextHotPath = label();
    
    m_byValCompilationInfo.append(ByValCompilationInfo(byValInfo, m_bytecodeOffset, PatchableJump(), badType, mode, profile, done, nextHotPath));
}

void JIT::emitSlow_op_has_indexed_property(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int property = currentInstruction[3].u.operand;
    ByValInfo* byValInfo = m_byValCompilationInfo[m_byValInstructionIndex].byValInfo;
    
    linkSlowCaseIfNotJSCell(iter, base); // base cell check
    linkSlowCase(iter); // base array check
    linkSlowCase(iter); // vector length check
    linkSlowCase(iter); // empty value
    
    Label slowPath = label();
    
    emitGetVirtualRegister(base, regT0);
    emitGetVirtualRegister(property, regT1);
    Call call = callOperation(operationHasIndexedPropertyDefault, dst, regT0, regT1, byValInfo);

    m_byValCompilationInfo[m_byValInstructionIndex].slowPathTarget = slowPath;
    m_byValCompilationInfo[m_byValInstructionIndex].returnAddress = call;
    m_byValInstructionIndex++;
}

void JIT::emit_op_get_direct_pname(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int base = currentInstruction[2].u.operand;
    int index = currentInstruction[4].u.operand;
    int enumerator = currentInstruction[5].u.operand;

    // Check that base is a cell
    emitGetVirtualRegister(base, regT0);
    emitJumpSlowCaseIfNotJSCell(regT0, base);

    // Check the structure
    emitGetVirtualRegister(enumerator, regT2);
    load32(Address(regT0, JSCell::structureIDOffset()), regT1);
    addSlowCase(branch32(NotEqual, regT1, Address(regT2, JSPropertyNameEnumerator::cachedStructureIDOffset())));

    // Compute the offset
    emitGetVirtualRegister(index, regT1);
    // If index is less than the enumerator's cached inline storage, then it's an inline access
    Jump outOfLineAccess = branch32(AboveOrEqual, regT1, Address(regT2, JSPropertyNameEnumerator::cachedInlineCapacityOffset()));
    addPtr(TrustedImm32(JSObject::offsetOfInlineStorage()), regT0);
    signExtend32ToPtr(regT1, regT1);
    load64(BaseIndex(regT0, regT1, TimesEight), regT0);
    
    Jump done = jump();

    // Otherwise it's out of line
    outOfLineAccess.link(this);
    loadPtr(Address(regT0, JSObject::butterflyOffset()), regT0);
    sub32(Address(regT2, JSPropertyNameEnumerator::cachedInlineCapacityOffset()), regT1);
    neg32(regT1);
    signExtend32ToPtr(regT1, regT1);
    int32_t offsetOfFirstProperty = static_cast<int32_t>(offsetInButterfly(firstOutOfLineOffset)) * sizeof(EncodedJSValue);
    load64(BaseIndex(regT0, regT1, TimesEight, offsetOfFirstProperty), regT0);
    
    done.link(this);
    emitValueProfilingSite();
    emitPutVirtualRegister(dst, regT0);
}

void JIT::emitSlow_op_get_direct_pname(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    int base = currentInstruction[2].u.operand;
    linkSlowCaseIfNotJSCell(iter, base);
    linkSlowCase(iter);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_get_direct_pname);
    slowPathCall.call();
}

void JIT::emit_op_enumerator_structure_pname(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int enumerator = currentInstruction[2].u.operand;
    int index = currentInstruction[3].u.operand;

    emitGetVirtualRegister(index, regT0);
    emitGetVirtualRegister(enumerator, regT1);
    Jump inBounds = branch32(Below, regT0, Address(regT1, JSPropertyNameEnumerator::endStructurePropertyIndexOffset()));

    move(TrustedImm64(JSValue::encode(jsNull())), regT0);

    Jump done = jump();
    inBounds.link(this);

    loadPtr(Address(regT1, JSPropertyNameEnumerator::cachedPropertyNamesVectorOffset()), regT1);
    signExtend32ToPtr(regT0, regT0);
    load64(BaseIndex(regT1, regT0, TimesEight), regT0);

    done.link(this);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_enumerator_generic_pname(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int enumerator = currentInstruction[2].u.operand;
    int index = currentInstruction[3].u.operand;

    emitGetVirtualRegister(index, regT0);
    emitGetVirtualRegister(enumerator, regT1);
    Jump inBounds = branch32(Below, regT0, Address(regT1, JSPropertyNameEnumerator::endGenericPropertyIndexOffset()));

    move(TrustedImm64(JSValue::encode(jsNull())), regT0);

    Jump done = jump();
    inBounds.link(this);

    loadPtr(Address(regT1, JSPropertyNameEnumerator::cachedPropertyNamesVectorOffset()), regT1);
    signExtend32ToPtr(regT0, regT0);
    load64(BaseIndex(regT1, regT0, TimesEight), regT0);
    
    done.link(this);
    emitPutVirtualRegister(dst);
}

void JIT::emit_op_profile_type(Instruction* currentInstruction)
{
    TypeLocation* cachedTypeLocation = currentInstruction[2].u.location;
    int valueToProfile = currentInstruction[1].u.operand;

    emitGetVirtualRegister(valueToProfile, regT0);

    JumpList jumpToEnd;

    jumpToEnd.append(branchTest64(Zero, regT0));

    // Compile in a predictive type check, if possible, to see if we can skip writing to the log.
    // These typechecks are inlined to match those of the 64-bit JSValue type checks.
    if (cachedTypeLocation->m_lastSeenType == TypeUndefined)
        jumpToEnd.append(branch64(Equal, regT0, TrustedImm64(JSValue::encode(jsUndefined()))));
    else if (cachedTypeLocation->m_lastSeenType == TypeNull)
        jumpToEnd.append(branch64(Equal, regT0, TrustedImm64(JSValue::encode(jsNull()))));
    else if (cachedTypeLocation->m_lastSeenType == TypeBoolean) {
        move(regT0, regT1);
        and64(TrustedImm32(~1), regT1);
        jumpToEnd.append(branch64(Equal, regT1, TrustedImm64(ValueFalse)));
    } else if (cachedTypeLocation->m_lastSeenType == TypeAnyInt)
        jumpToEnd.append(emitJumpIfInt(regT0));
    else if (cachedTypeLocation->m_lastSeenType == TypeNumber)
        jumpToEnd.append(emitJumpIfNumber(regT0));
    else if (cachedTypeLocation->m_lastSeenType == TypeString) {
        Jump isNotCell = emitJumpIfNotJSCell(regT0);
        jumpToEnd.append(branch8(Equal, Address(regT0, JSCell::typeInfoTypeOffset()), TrustedImm32(StringType)));
        isNotCell.link(this);
    }

    // Load the type profiling log into T2.
    TypeProfilerLog* cachedTypeProfilerLog = m_vm->typeProfilerLog();
    move(TrustedImmPtr(cachedTypeProfilerLog), regT2);
    // Load the next log entry into T1.
    loadPtr(Address(regT2, TypeProfilerLog::currentLogEntryOffset()), regT1);

    // Store the JSValue onto the log entry.
    store64(regT0, Address(regT1, TypeProfilerLog::LogEntry::valueOffset()));

    // Store the structureID of the cell if T0 is a cell, otherwise, store 0 on the log entry.
    Jump notCell = emitJumpIfNotJSCell(regT0);
    load32(Address(regT0, JSCell::structureIDOffset()), regT0);
    store32(regT0, Address(regT1, TypeProfilerLog::LogEntry::structureIDOffset()));
    Jump skipIsCell = jump();
    notCell.link(this);
    store32(TrustedImm32(0), Address(regT1, TypeProfilerLog::LogEntry::structureIDOffset()));
    skipIsCell.link(this);

    // Store the typeLocation on the log entry.
    move(TrustedImmPtr(cachedTypeLocation), regT0);
    store64(regT0, Address(regT1, TypeProfilerLog::LogEntry::locationOffset()));

    // Increment the current log entry.
    addPtr(TrustedImm32(sizeof(TypeProfilerLog::LogEntry)), regT1);
    store64(regT1, Address(regT2, TypeProfilerLog::currentLogEntryOffset()));
    Jump skipClearLog = branchPtr(NotEqual, regT1, TrustedImmPtr(cachedTypeProfilerLog->logEndPtr()));
    // Clear the log if we're at the end of the log.
    callOperation(operationProcessTypeProfilerLog);
    skipClearLog.link(this);

    jumpToEnd.link(this);
}

void JIT::emit_op_log_shadow_chicken_prologue(Instruction* currentInstruction)
{
    updateTopCallFrame();
    static_assert(nonArgGPR0 != regT0 && nonArgGPR0 != regT2, "we will have problems if this is true.");
    GPRReg shadowPacketReg = regT0;
    GPRReg scratch1Reg = nonArgGPR0; // This must be a non-argument register.
    GPRReg scratch2Reg = regT2;
    ensureShadowChickenPacket(shadowPacketReg, scratch1Reg, scratch2Reg);
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT3);
    logShadowChickenProloguePacket(shadowPacketReg, scratch1Reg, regT3);
}

void JIT::emit_op_log_shadow_chicken_tail(Instruction* currentInstruction)
{
    updateTopCallFrame();
    static_assert(nonArgGPR0 != regT0 && nonArgGPR0 != regT2, "we will have problems if this is true.");
    GPRReg shadowPacketReg = regT0;
    GPRReg scratch1Reg = nonArgGPR0; // This must be a non-argument register.
    GPRReg scratch2Reg = regT2;
    ensureShadowChickenPacket(shadowPacketReg, scratch1Reg, scratch2Reg);
    emitGetVirtualRegister(currentInstruction[1].u.operand, regT2);
    emitGetVirtualRegister(currentInstruction[2].u.operand, regT3);
    logShadowChickenTailPacket(shadowPacketReg, JSValueRegs(regT2), regT3, m_codeBlock, CallSiteIndex(m_bytecodeOffset));
}

#endif // USE(JSVALUE64)

void JIT::emit_op_get_enumerable_length(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_get_enumerable_length);
    slowPathCall.call();
}

void JIT::emitSlow_op_has_structure_property(Instruction* currentInstruction, Vector<SlowCaseEntry>::iterator& iter)
{
    linkSlowCase(iter);
    linkSlowCase(iter);

    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_has_structure_property);
    slowPathCall.call();
}

void JIT::emit_op_has_generic_property(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_has_generic_property);
    slowPathCall.call();
}

void JIT::emit_op_get_property_enumerator(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_get_property_enumerator);
    slowPathCall.call();
}

void JIT::emit_op_to_index_string(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_to_index_string);
    slowPathCall.call();
}

void JIT::emit_op_profile_control_flow(Instruction* currentInstruction)
{
    BasicBlockLocation* basicBlockLocation = currentInstruction[1].u.basicBlockLocation;
#if USE(JSVALUE64)
    basicBlockLocation->emitExecuteCode(*this);
#else
    basicBlockLocation->emitExecuteCode(*this, regT0);
#endif
}

void JIT::emit_op_create_direct_arguments(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_create_direct_arguments);
    slowPathCall.call();
}

void JIT::emit_op_create_scoped_arguments(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_create_scoped_arguments);
    slowPathCall.call();
}

void JIT::emit_op_create_cloned_arguments(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_create_cloned_arguments);
    slowPathCall.call();
}

void JIT::emit_op_argument_count(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    load32(payloadFor(CallFrameSlot::argumentCount), regT0);
    sub32(TrustedImm32(1), regT0);
    JSValueRegs result = JSValueRegs::withTwoAvailableRegs(regT0, regT1);
    boxInt32(regT0, result);
    emitPutVirtualRegister(dst, result);
}

void JIT::emit_op_create_rest(Instruction* currentInstruction)
{
    JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_create_rest);
    slowPathCall.call();
}

void JIT::emit_op_get_rest_length(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    unsigned numParamsToSkip = currentInstruction[2].u.unsignedValue;
    load32(payloadFor(CallFrameSlot::argumentCount), regT0);
    sub32(TrustedImm32(1), regT0);
    Jump zeroLength = branch32(LessThanOrEqual, regT0, Imm32(numParamsToSkip));
    sub32(Imm32(numParamsToSkip), regT0);
#if USE(JSVALUE64)
    boxInt32(regT0, JSValueRegs(regT0));
#endif
    Jump done = jump();

    zeroLength.link(this);
#if USE(JSVALUE64)
    move(TrustedImm64(JSValue::encode(jsNumber(0))), regT0);
#else
    move(TrustedImm32(0), regT0);
#endif

    done.link(this);
#if USE(JSVALUE64)
    emitPutVirtualRegister(dst, regT0);
#else
    move(TrustedImm32(JSValue::Int32Tag), regT1);
    emitPutVirtualRegister(dst, JSValueRegs(regT1, regT0));
#endif
}

void JIT::emit_op_get_argument(Instruction* currentInstruction)
{
    int dst = currentInstruction[1].u.operand;
    int index = currentInstruction[2].u.operand;
#if USE(JSVALUE64)
    JSValueRegs resultRegs(regT0);
#else
    JSValueRegs resultRegs(regT1, regT0);
#endif

    load32(payloadFor(CallFrameSlot::argumentCount), regT2);
    Jump argumentOutOfBounds = branch32(LessThanOrEqual, regT2, TrustedImm32(index));
    loadValue(addressFor(CallFrameSlot::thisArgument + index), resultRegs);
    Jump done = jump();

    argumentOutOfBounds.link(this);
    moveValue(jsUndefined(), resultRegs);

    done.link(this);
    emitValueProfilingSite();
    emitPutVirtualRegister(dst, resultRegs);
}

} // namespace JSC

#endif // ENABLE(JIT)
