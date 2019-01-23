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

#if ENABLE(JIT)

#include "JSCInlines.h"

namespace JSC {

#if USE(JSVALUE64)
inline MacroAssembler::JumpList JIT::emitDoubleGetByVal(Instruction* instruction, PatchableJump& badType)
{
    JumpList slowCases = emitDoubleLoad(instruction, badType);
    moveDoubleTo64(fpRegT0, regT0);
    sub64(tagTypeNumberRegister, regT0);
    return slowCases;
}
#else
inline MacroAssembler::JumpList JIT::emitDoubleGetByVal(Instruction* instruction, PatchableJump& badType)
{
    JumpList slowCases = emitDoubleLoad(instruction, badType);
    moveDoubleToInts(fpRegT0, regT0, regT1);
    return slowCases;
}
#endif // USE(JSVALUE64)

ALWAYS_INLINE MacroAssembler::JumpList JIT::emitLoadForArrayMode(Instruction* currentInstruction, JITArrayMode arrayMode, PatchableJump& badType)
{
    switch (arrayMode) {
    case JITInt32:
        return emitInt32Load(currentInstruction, badType);
    case JITDouble:
        return emitDoubleLoad(currentInstruction, badType);
    case JITContiguous:
        return emitContiguousLoad(currentInstruction, badType);
    case JITArrayStorage:
        return emitArrayStorageLoad(currentInstruction, badType);
    default:
        break;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return MacroAssembler::JumpList();
}

inline MacroAssembler::JumpList JIT::emitContiguousGetByVal(Instruction* instruction, PatchableJump& badType, IndexingType expectedShape)
{
    return emitContiguousLoad(instruction, badType, expectedShape);
}

inline MacroAssembler::JumpList JIT::emitArrayStorageGetByVal(Instruction* instruction, PatchableJump& badType)
{
    return emitArrayStorageLoad(instruction, badType);
}

ALWAYS_INLINE bool JIT::isOperandConstantDouble(int src)
{
    return m_codeBlock->isConstantRegisterIndex(src) && getConstantOperand(src).isDouble();
}

ALWAYS_INLINE JSValue JIT::getConstantOperand(int src)
{
    ASSERT(m_codeBlock->isConstantRegisterIndex(src));
    return m_codeBlock->getConstant(src);
}

ALWAYS_INLINE void JIT::emitPutIntToCallFrameHeader(RegisterID from, int entry)
{
#if USE(JSVALUE32_64)
    store32(TrustedImm32(Int32Tag), intTagFor(entry, callFrameRegister));
    store32(from, intPayloadFor(entry, callFrameRegister));
#else
    store64(from, addressFor(entry, callFrameRegister));
#endif
}

ALWAYS_INLINE void JIT::emitLoadCharacterString(RegisterID src, RegisterID dst, JumpList& failures)
{
    failures.append(branchStructure(NotEqual, Address(src, JSCell::structureIDOffset()), m_vm->stringStructure.get()));
    failures.append(branch32(NotEqual, MacroAssembler::Address(src, ThunkHelpers::jsStringLengthOffset()), TrustedImm32(1)));
    loadPtr(MacroAssembler::Address(src, ThunkHelpers::jsStringValueOffset()), dst);
    failures.append(branchTest32(Zero, dst));
    loadPtr(MacroAssembler::Address(dst, StringImpl::flagsOffset()), regT1);
    loadPtr(MacroAssembler::Address(dst, StringImpl::dataOffset()), dst);

    JumpList is16Bit;
    JumpList cont8Bit;
    is16Bit.append(branchTest32(Zero, regT1, TrustedImm32(StringImpl::flagIs8Bit())));
    load8(MacroAssembler::Address(dst, 0), dst);
    cont8Bit.append(jump());
    is16Bit.link(this);
    load16(MacroAssembler::Address(dst, 0), dst);
    cont8Bit.link(this);
}

ALWAYS_INLINE JIT::Call JIT::emitNakedCall(CodePtr function)
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.
    Call nakedCall = nearCall();
    m_calls.append(CallRecord(nakedCall, m_bytecodeOffset, function.executableAddress()));
    return nakedCall;
}

ALWAYS_INLINE JIT::Call JIT::emitNakedTailCall(CodePtr function)
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.
    Call nakedCall = nearTailCall();
    m_calls.append(CallRecord(nakedCall, m_bytecodeOffset, function.executableAddress()));
    return nakedCall;
}

ALWAYS_INLINE void JIT::updateTopCallFrame()
{
    ASSERT(static_cast<int>(m_bytecodeOffset) >= 0);
#if USE(JSVALUE32_64)
    Instruction* instruction = m_codeBlock->instructions().begin() + m_bytecodeOffset; 
    uint32_t locationBits = CallSiteIndex(instruction).bits();
#else
    uint32_t locationBits = CallSiteIndex(m_bytecodeOffset).bits();
#endif
    store32(TrustedImm32(locationBits), intTagFor(CallFrameSlot::argumentCount));
    
    // FIXME: It's not clear that this is needed. JITOperations tend to update the top call frame on
    // the C++ side.
    // https://bugs.webkit.org/show_bug.cgi?id=155693
    storePtr(callFrameRegister, &m_vm->topCallFrame);
}

ALWAYS_INLINE MacroAssembler::Call JIT::appendCallWithExceptionCheck(const FunctionPtr& function)
{
    updateTopCallFrame();
    MacroAssembler::Call call = appendCall(function);
    exceptionCheck();
    return call;
}

#if OS(WINDOWS) && CPU(X86_64)
ALWAYS_INLINE MacroAssembler::Call JIT::appendCallWithExceptionCheckAndSlowPathReturnType(const FunctionPtr& function)
{
    updateTopCallFrame();
    MacroAssembler::Call call = appendCallWithSlowPathReturnType(function);
    exceptionCheck();
    return call;
}
#endif

ALWAYS_INLINE MacroAssembler::Call JIT::appendCallWithCallFrameRollbackOnException(const FunctionPtr& function)
{
    updateTopCallFrame(); // The callee is responsible for setting topCallFrame to their caller
    MacroAssembler::Call call = appendCall(function);
    exceptionCheckWithCallFrameRollback();
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::appendCallWithExceptionCheckSetJSValueResult(const FunctionPtr& function, int dst)
{
    MacroAssembler::Call call = appendCallWithExceptionCheck(function);
#if USE(JSVALUE64)
    emitPutVirtualRegister(dst, returnValueGPR);
#else
    emitStore(dst, returnValueGPR2, returnValueGPR);
#endif
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::appendCallWithExceptionCheckSetJSValueResultWithProfile(const FunctionPtr& function, int dst)
{
    MacroAssembler::Call call = appendCallWithExceptionCheck(function);
    emitValueProfilingSite();
#if USE(JSVALUE64)
    emitPutVirtualRegister(dst, returnValueGPR);
#else
    emitStore(dst, returnValueGPR2, returnValueGPR);
#endif
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(P_JITOperation_E operation)
{
    setupArgumentsExecState();
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperationNoExceptionCheck(Z_JITOperation_E operation)
{
    setupArgumentsExecState();
    updateTopCallFrame();
    return appendCall(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_E operation)
{
    setupArgumentsExecState();
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_EJsc operation, GPRReg arg1)
{
    setupArgumentsWithExecState(arg1);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_EJscZ operation, GPRReg arg1, int32_t arg2)
{
    setupArgumentsWithExecState(arg1, TrustedImm32(arg2));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_EL operation, GPRReg arg1)
{
    setupArgumentsWithExecState(arg1);
    return appendCallWithExceptionCheck(operation);
}
    
ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_EL operation, TrustedImmPtr arg1)
{
    setupArgumentsWithExecState(arg1);
    return appendCallWithExceptionCheck(operation);
}
    
ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_EO operation, GPRReg arg)
{
    setupArgumentsWithExecState(arg);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_ESt operation, Structure* structure)
{
    setupArgumentsWithExecState(TrustedImmPtr(structure));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(C_JITOperation_EZ operation, int32_t arg)
{
    setupArgumentsWithExecState(TrustedImm32(arg));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_E operation, int dst)
{
    setupArgumentsExecState();
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EAapJcpZ operation, int dst, ArrayAllocationProfile* arg1, GPRReg arg2, int32_t arg3)
{
    setupArgumentsWithExecState(TrustedImmPtr(arg1), arg2, TrustedImm32(arg3));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EAapJcpZ operation, int dst, ArrayAllocationProfile* arg1, const JSValue* arg2, int32_t arg3)
{
    setupArgumentsWithExecState(TrustedImmPtr(arg1), TrustedImmPtr(arg2), TrustedImm32(arg3));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EC operation, int dst, JSCell* cell)
{
    setupArgumentsWithExecState(TrustedImmPtr(cell));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EC operation, JSCell* cell)
{
    setupArgumentsWithExecState(TrustedImmPtr(cell));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJscC operation, int dst, GPRReg arg1, JSCell* cell)
{
    setupArgumentsWithExecState(arg1, TrustedImmPtr(cell));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJscCJ operation, int dst, GPRReg arg1, JSCell* cell, GPRReg arg2)
{
    setupArgumentsWithExecState(arg1, TrustedImmPtr(cell), arg2);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}
    
ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EP operation, int dst, void* pointer)
{
    setupArgumentsWithExecState(TrustedImmPtr(pointer));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(WithProfileTag, J_JITOperation_EPc operation, int dst, Instruction* bytecodePC)
{
    setupArgumentsWithExecState(TrustedImmPtr(bytecodePC));
    return appendCallWithExceptionCheckSetJSValueResultWithProfile(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EPc operation, int dst, Instruction* bytecodePC)
{
    setupArgumentsWithExecState(TrustedImmPtr(bytecodePC));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EZ operation, int dst, int32_t arg)
{
    setupArgumentsWithExecState(TrustedImm32(arg));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EZZ operation, int dst, int32_t arg1, int32_t arg2)
{
    setupArgumentsWithExecState(TrustedImm32(arg1), TrustedImm32(arg2));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_ECC operation, RegisterID regOp1, RegisterID regOp2)
{
    setupArgumentsWithExecState(regOp1, regOp2);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_EOJss operation, RegisterID regOp1, RegisterID regOp2)
{
    setupArgumentsWithExecState(regOp1, regOp2);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(Sprt_JITOperation_EZ operation, int32_t op)
{
#if OS(WINDOWS) && CPU(X86_64)
    setupArgumentsWithExecStateForCallWithSlowPathReturnType(TrustedImm32(op));
    return appendCallWithExceptionCheckAndSlowPathReturnType(operation);
#else
    setupArgumentsWithExecState(TrustedImm32(op));
    return appendCallWithExceptionCheck(operation);
#endif
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_E operation)
{
    setupArgumentsExecState();
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EC operation, RegisterID regOp)
{
    setupArgumentsWithExecState(regOp);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECC operation, RegisterID regOp1, RegisterID regOp2)
{
    setupArgumentsWithExecState(regOp1, regOp2);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EE operation, RegisterID regOp)
{
    setupArgumentsWithExecState(regOp);
    updateTopCallFrame();
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EPc operation, Instruction* bytecodePC)
{
    setupArgumentsWithExecState(TrustedImmPtr(bytecodePC));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EZ operation, int32_t op)
{
    setupArgumentsWithExecState(TrustedImm32(op));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperationWithCallFrameRollbackOnException(J_JITOperation_E operation)
{
    setupArgumentsExecState();
    return appendCallWithCallFrameRollbackOnException(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperationWithCallFrameRollbackOnException(V_JITOperation_ECb operation, CodeBlock* pointer)
{
    setupArgumentsWithExecState(TrustedImmPtr(pointer));
    return appendCallWithCallFrameRollbackOnException(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperationWithCallFrameRollbackOnException(Z_JITOperation_E operation)
{
    setupArgumentsExecState();
    return appendCallWithCallFrameRollbackOnException(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECIZC operation, RegisterID regOp1, UniquedStringImpl* identOp2, int32_t op3, RegisterID regOp4)
{
    setupArgumentsWithExecState(regOp1, TrustedImmPtr(identOp2), TrustedImm32(op3), regOp4);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJ operation, JSValueRegs result, JSValueRegs arg)
{
    setupArgumentsWithExecState(arg);
    Call call = appendCallWithExceptionCheck(operation);
    setupResults(result);
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJ operation, JSValueRegs result, JSValueRegs arg1, JSValueRegs arg2)
{
    setupArgumentsWithExecState(arg1, arg2);
    Call call = appendCallWithExceptionCheck(operation);
    setupResults(result);
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJArp operation, JSValueRegs result, JSValueRegs operand, ArithProfile* arithProfile)
{
    setupArgumentsWithExecState(operand, TrustedImmPtr(arithProfile));
    Call call = appendCallWithExceptionCheck(operation);
    setupResults(result);
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJArp operation, JSValueRegs result, JSValueRegs arg1, JSValueRegs arg2, ArithProfile* arithProfile)
{
    setupArgumentsWithExecState(arg1, arg2, TrustedImmPtr(arithProfile));
    Call call = appendCallWithExceptionCheck(operation);
    setupResults(result);
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJMic operation, JSValueRegs result, JSValueRegs arg, TrustedImmPtr mathIC)
{
    setupArgumentsWithExecState(arg, mathIC);
    Call call = appendCallWithExceptionCheck(operation);
    setupResults(result);
    return call;
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJMic operation, JSValueRegs result, JSValueRegs arg1, JSValueRegs arg2, TrustedImmPtr mathIC)
{
    setupArgumentsWithExecState(arg1, arg2, mathIC);
    Call call = appendCallWithExceptionCheck(operation);
    setupResults(result);
    return call;
}

#if USE(JSVALUE64)
ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(Z_JITOperation_EJZZ operation, GPRReg arg1, int32_t arg2, int32_t arg3)
{
    setupArgumentsWithExecState(arg1, TrustedImm32(arg2), TrustedImm32(arg3));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(F_JITOperation_EFJZZ operation, GPRReg arg1, GPRReg arg2, int32_t arg3, GPRReg arg4)
{
    setupArgumentsWithExecState(arg1, arg2, TrustedImm32(arg3), arg4);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ESsiJJI operation, StructureStubInfo* stubInfo, RegisterID regOp1, RegisterID regOp2, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), regOp1, regOp2, TrustedImmPtr(uid));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJJJ operation, RegisterID regOp1, RegisterID regOp2, RegisterID regOp3)
{
    setupArgumentsWithExecState(regOp1, regOp2, regOp3);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJJJAp operation, RegisterID regOp1, RegisterID regOp2, RegisterID regOp3, ArrayProfile* arrayProfile)
{
    setupArgumentsWithExecState(regOp1, regOp2, regOp3, TrustedImmPtr(arrayProfile));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJJJBy operation, RegisterID regOp1, RegisterID regOp2, RegisterID regOp3, ByValInfo* byValInfo)
{
    setupArgumentsWithExecState(regOp1, regOp2, regOp3, TrustedImmPtr(byValInfo));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EZJ operation, int dst, GPRReg arg)
{
    setupArgumentsWithExecState(TrustedImm32(dst), arg);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_ESsiJI operation, int dst, StructureStubInfo* stubInfo, GPRReg arg1, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), arg1, TrustedImmPtr(uid));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(JIT::WithProfileTag, J_JITOperation_ESsiJI operation, int dst, StructureStubInfo* stubInfo, GPRReg arg1, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), arg1, TrustedImmPtr(uid));
    return appendCallWithExceptionCheckSetJSValueResultWithProfile(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(JIT::WithProfileTag, J_JITOperation_ESsiJJI operation, int dst, StructureStubInfo* stubInfo, GPRReg arg1, GPRReg arg2, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), arg1, arg2, TrustedImmPtr(uid));
    return appendCallWithExceptionCheckSetJSValueResultWithProfile(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(JIT::WithProfileTag, J_JITOperation_EJJ operation, int dst, GPRReg arg1, GPRReg arg2)
{
    setupArgumentsWithExecState(arg1, arg2);
    return appendCallWithExceptionCheckSetJSValueResultWithProfile(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EAapJ operation, int dst, ArrayAllocationProfile* arg1, GPRReg arg2)
{
    setupArgumentsWithExecState(TrustedImmPtr(arg1), arg2);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJ operation, int dst, GPRReg arg1)
{
    setupArgumentsWithExecState(arg1);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJI operation, int dst, GPRReg arg1, UniquedStringImpl* arg2)
{
    setupArgumentsWithExecState(arg1, TrustedImmPtr(arg2));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJ operation, int dst, GPRReg arg1, GPRReg arg2)
{
    setupArgumentsWithExecState(arg1, arg2);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJAp operation, int dst, GPRReg arg1, GPRReg arg2, ArrayProfile* arrayProfile)
{
    setupArgumentsWithExecState(arg1, arg2, TrustedImmPtr(arrayProfile));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJBy operation, int dst, GPRReg arg1, GPRReg arg2, ByValInfo* byValInfo)
{
    setupArgumentsWithExecState(arg1, arg2, TrustedImmPtr(byValInfo));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(Z_JITOperation_EJOJ operation, GPRReg arg1, GPRReg arg2, GPRReg arg3)
{
    setupArgumentsWithExecState(arg1, arg2, arg3);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperationNoExceptionCheck(V_JITOperation_EJ operation, GPRReg arg1)
{
    setupArgumentsWithExecState(arg1);
    updateTopCallFrame();
    return appendCall(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(P_JITOperation_EJS operation, GPRReg arg1, size_t arg2)
{
    setupArgumentsWithExecState(arg1, TrustedImmPtr(arg2));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_EJ operation, RegisterID regOp)
{
    setupArgumentsWithExecState(regOp);
    return appendCallWithExceptionCheck(operation);
}


ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_EJI operation, GPRReg arg1, UniquedStringImpl* arg2)
{
    setupArgumentsWithExecState(arg1, TrustedImmPtr(arg2));
    return appendCallWithExceptionCheck(operation);
}


ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_EJJ operation, RegisterID regOp1, RegisterID regOp2)
{
    setupArgumentsWithExecState(regOp1, regOp2);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EZSymtabJ operation, int op1, SymbolTable* symbolTable, RegisterID regOp3)
{
    setupArgumentsWithExecState(TrustedImm32(op1), TrustedImmPtr(symbolTable), regOp3);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EZSymtabJ operation, int op1, SymbolTable* symbolTable, RegisterID regOp3)
{
    setupArgumentsWithExecState(TrustedImm32(op1), TrustedImmPtr(symbolTable), regOp3);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJ operation, RegisterID regOp)
{
    setupArgumentsWithExecState(regOp);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECIZJJ operation, RegisterID regOp1, UniquedStringImpl* identOp2, int32_t op3, RegisterID regOp4, RegisterID regOp5)
{
    setupArgumentsWithExecState(regOp1, TrustedImmPtr(identOp2), TrustedImm32(op3), regOp4, regOp5);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECJ operation, RegisterID arg1, RegisterID arg2)
{
    setupArgumentsWithExecState(arg1, arg2);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECJZC operation, RegisterID regOp1, RegisterID regOp2, int32_t op3, RegisterID regOp4)
{
    setupArgumentsWithExecState(regOp1, regOp2, TrustedImm32(op3), regOp4);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJZ operation, RegisterID regOp1, int32_t op2)
{
    setupArgumentsWithExecState(regOp1, TrustedImm32(op2));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJZJ operation, RegisterID regOp1, int32_t op2, RegisterID regOp3)
{
    setupArgumentsWithExecState(regOp1, TrustedImm32(op2), regOp3);
    return appendCallWithExceptionCheck(operation);
}

#else // USE(JSVALUE32_64)

ALWAYS_INLINE MacroAssembler::Call JIT::callOperationNoExceptionCheck(V_JITOperation_EJ operation, GPRReg arg1Tag, GPRReg arg1Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag);
    updateTopCallFrame();
    return appendCall(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(Z_JITOperation_EJOJ operation, GPRReg arg1Tag, GPRReg arg1Payload, GPRReg arg2, GPRReg arg3Tag, GPRReg arg3Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, arg2, EABI_32BIT_DUMMY_ARG arg3Payload, arg3Tag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(Z_JITOperation_EJZZ operation, GPRReg arg1Tag, GPRReg arg1Payload, int32_t arg2, int32_t arg3)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, TrustedImm32(arg2), TrustedImm32(arg3));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(F_JITOperation_EFJZZ operation, GPRReg arg1, GPRReg arg2Tag, GPRReg arg2Payload, int32_t arg3, GPRReg arg4)
{
    setupArgumentsWithExecState(arg1, arg2Payload, arg2Tag, TrustedImm32(arg3), arg4);
    return appendCallWithExceptionCheck(operation);
}
    
ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EAapJ operation, int dst, ArrayAllocationProfile* arg1, GPRReg arg2Tag, GPRReg arg2Payload)
{
    setupArgumentsWithExecState(TrustedImmPtr(arg1), arg2Payload, arg2Tag);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJ operation, int dst, GPRReg arg1Tag, GPRReg arg1Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_ESsiJI operation, int dst, StructureStubInfo* stubInfo, GPRReg arg1Tag, GPRReg arg1Payload, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), arg1Payload, arg1Tag, TrustedImmPtr(uid));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(JIT::WithProfileTag, J_JITOperation_ESsiJI operation, int dst, StructureStubInfo* stubInfo, GPRReg arg1Tag, GPRReg arg1Payload, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), arg1Payload, arg1Tag, TrustedImmPtr(uid));
    return appendCallWithExceptionCheckSetJSValueResultWithProfile(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJI operation, int dst, GPRReg arg1Tag, GPRReg arg1Payload, UniquedStringImpl* arg2)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, TrustedImmPtr(arg2));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(JIT::WithProfileTag, J_JITOperation_ESsiJJI operation, int dst, StructureStubInfo* stubInfo, GPRReg arg1Tag, GPRReg arg1Payload, GPRReg arg2Tag, GPRReg arg2Payload, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), arg1Payload, arg1Tag, arg2Payload, arg2Tag, TrustedImmPtr(uid));
    return appendCallWithExceptionCheckSetJSValueResultWithProfile(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJ operation, int dst, GPRReg arg1Tag, GPRReg arg1Payload, GPRReg arg2Tag, GPRReg arg2Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, arg2Payload, arg2Tag);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJAp operation, int dst, GPRReg arg1Tag, GPRReg arg1Payload, GPRReg arg2Tag, GPRReg arg2Payload, ArrayProfile* arrayProfile)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, arg2Payload, arg2Tag, TrustedImmPtr(arrayProfile));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJJBy operation, int dst, GPRReg arg1Tag, GPRReg arg1Payload, GPRReg arg2Tag, GPRReg arg2Payload, ByValInfo* byValInfo)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, arg2Payload, arg2Tag, TrustedImmPtr(byValInfo));
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(JIT::WithProfileTag, J_JITOperation_EJJ operation, int dst, GPRReg arg1Tag, GPRReg arg1Payload, GPRReg arg2Tag, GPRReg arg2Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, arg2Payload, arg2Tag);
    return appendCallWithExceptionCheckSetJSValueResultWithProfile(operation, dst);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(P_JITOperation_EJS operation, GPRReg arg1Tag, GPRReg arg1Payload, size_t arg2)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, TrustedImmPtr(arg2));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_EJ operation, RegisterID argTag, RegisterID argPayload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG argPayload, argTag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_EJI operation, GPRReg arg1Tag, GPRReg arg1Payload, UniquedStringImpl* arg2)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, TrustedImmPtr(arg2));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(S_JITOperation_EJJ operation, RegisterID arg1Tag, RegisterID arg1Payload, RegisterID arg2Tag, RegisterID arg2Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG arg1Payload, arg1Tag, arg2Payload, arg2Tag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECIZCC operation, RegisterID regOp1, UniquedStringImpl* identOp2, int32_t op3, RegisterID regOp4, RegisterID regOp5)
{
    setupArgumentsWithExecState(regOp1, TrustedImmPtr(identOp2), TrustedImm32(op3), regOp4, regOp5);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECJ operation, RegisterID arg1, RegisterID arg2Tag, RegisterID arg2Payload)
{
    setupArgumentsWithExecState(arg1, arg2Payload, arg2Tag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ECJZC operation, RegisterID arg1, RegisterID arg2Tag, RegisterID arg2Payload, int32_t arg3, RegisterID arg4)
{
    setupArgumentsWithExecState(arg1, arg2Payload, arg2Tag, TrustedImm32(arg3), arg4);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJ operation, RegisterID regOp1Tag, RegisterID regOp1Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG regOp1Payload, regOp1Tag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EZSymtabJ operation, int32_t op1, SymbolTable* symbolTable, RegisterID regOp3Tag, RegisterID regOp3Payload)
{
    setupArgumentsWithExecState(TrustedImm32(op1), TrustedImmPtr(symbolTable), EABI_32BIT_DUMMY_ARG regOp3Payload, regOp3Tag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_ESsiJJI operation, StructureStubInfo* stubInfo, RegisterID regOp1Tag, RegisterID regOp1Payload, RegisterID regOp2Tag, RegisterID regOp2Payload, UniquedStringImpl* uid)
{
    setupArgumentsWithExecState(TrustedImmPtr(stubInfo), regOp1Payload, regOp1Tag, regOp2Payload, regOp2Tag, TrustedImmPtr(uid));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJJJ operation, RegisterID regOp1Tag, RegisterID regOp1Payload, RegisterID regOp2Tag, RegisterID regOp2Payload, RegisterID regOp3Tag, RegisterID regOp3Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG regOp1Payload, regOp1Tag, regOp2Payload, regOp2Tag, regOp3Payload, regOp3Tag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJJJAp operation, RegisterID regOp1Tag, RegisterID regOp1Payload, RegisterID regOp2Tag, RegisterID regOp2Payload, RegisterID regOp3Tag, RegisterID regOp3Payload, ArrayProfile* arrayProfile)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG regOp1Payload, regOp1Tag, regOp2Payload, regOp2Tag, regOp3Payload, regOp3Tag, TrustedImmPtr(arrayProfile));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJJJBy operation, RegisterID regOp1Tag, RegisterID regOp1Payload, RegisterID regOp2Tag, RegisterID regOp2Payload, RegisterID regOp3Tag, RegisterID regOp3Payload, ByValInfo* byValInfo)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG regOp1Payload, regOp1Tag, regOp2Payload, regOp2Tag, regOp3Payload, regOp3Tag, TrustedImmPtr(byValInfo));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EZJ operation, int dst, RegisterID regOp1Tag, RegisterID regOp1Payload)
{
    setupArgumentsWithExecState(TrustedImm32(dst), regOp1Payload, regOp1Tag);
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJZ operation, RegisterID regOp1Tag, RegisterID regOp1Payload, int32_t op2)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG regOp1Payload, regOp1Tag, TrustedImm32(op2));
    return appendCallWithExceptionCheck(operation);
}

ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(V_JITOperation_EJZJ operation, RegisterID regOp1Tag, RegisterID regOp1Payload, int32_t op2, RegisterID regOp3Tag, RegisterID regOp3Payload)
{
    setupArgumentsWithExecState(EABI_32BIT_DUMMY_ARG regOp1Payload, regOp1Tag, TrustedImm32(op2), EABI_32BIT_DUMMY_ARG regOp3Payload, regOp3Tag);
    return appendCallWithExceptionCheck(operation);
}
    
ALWAYS_INLINE MacroAssembler::Call JIT::callOperation(J_JITOperation_EJscCJ operation, int dst, GPRReg arg1, JSCell* cell, GPRReg arg2Tag, GPRReg arg2Payload)
{
    setupArgumentsWithExecState(arg1, TrustedImmPtr(cell), EABI_32BIT_DUMMY_ARG arg2Payload, arg2Tag);
    return appendCallWithExceptionCheckSetJSValueResult(operation, dst);
}

#endif // USE(JSVALUE32_64)

ALWAYS_INLINE JIT::Jump JIT::checkStructure(RegisterID reg, Structure* structure)
{
    return branchStructure(NotEqual, Address(reg, JSCell::structureIDOffset()), structure);
}

ALWAYS_INLINE void JIT::linkSlowCaseIfNotJSCell(Vector<SlowCaseEntry>::iterator& iter, int vReg)
{
    if (!m_codeBlock->isKnownNotImmediate(vReg))
        linkSlowCase(iter);
}

ALWAYS_INLINE void JIT::linkAllSlowCasesForBytecodeOffset(Vector<SlowCaseEntry>& slowCases, Vector<SlowCaseEntry>::iterator& iter, unsigned bytecodeOffset)
{
    while (iter != slowCases.end() && iter->to == bytecodeOffset) {
        iter->from.link(this);
        ++iter;
    }
}

ALWAYS_INLINE void JIT::addSlowCase(Jump jump)
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.

    m_slowCases.append(SlowCaseEntry(jump, m_bytecodeOffset));
}

ALWAYS_INLINE void JIT::addSlowCase(const JumpList& jumpList)
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.

    for (const Jump& jump : jumpList.jumps())
        m_slowCases.append(SlowCaseEntry(jump, m_bytecodeOffset));
}

ALWAYS_INLINE void JIT::addSlowCase()
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.
    
    Jump emptyJump; // Doing it this way to make Windows happy.
    m_slowCases.append(SlowCaseEntry(emptyJump, m_bytecodeOffset));
}

ALWAYS_INLINE void JIT::addJump(Jump jump, int relativeOffset)
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.

    m_jmpTable.append(JumpTable(jump, m_bytecodeOffset + relativeOffset));
}

ALWAYS_INLINE void JIT::emitJumpSlowToHot(Jump jump, int relativeOffset)
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.

    jump.linkTo(m_labels[m_bytecodeOffset + relativeOffset], this);
}

ALWAYS_INLINE JIT::Jump JIT::emitJumpIfCellObject(RegisterID cellReg)
{
    return branch8(AboveOrEqual, Address(cellReg, JSCell::typeInfoTypeOffset()), TrustedImm32(ObjectType));
}

ALWAYS_INLINE JIT::Jump JIT::emitJumpIfCellNotObject(RegisterID cellReg)
{
    return branch8(Below, Address(cellReg, JSCell::typeInfoTypeOffset()), TrustedImm32(ObjectType));
}

#if ENABLE(SAMPLING_FLAGS)
ALWAYS_INLINE void JIT::setSamplingFlag(int32_t flag)
{
    ASSERT(flag >= 1);
    ASSERT(flag <= 32);
    or32(TrustedImm32(1u << (flag - 1)), AbsoluteAddress(SamplingFlags::addressOfFlags()));
}

ALWAYS_INLINE void JIT::clearSamplingFlag(int32_t flag)
{
    ASSERT(flag >= 1);
    ASSERT(flag <= 32);
    and32(TrustedImm32(~(1u << (flag - 1))), AbsoluteAddress(SamplingFlags::addressOfFlags()));
}
#endif

#if ENABLE(SAMPLING_COUNTERS)
ALWAYS_INLINE void JIT::emitCount(AbstractSamplingCounter& counter, int32_t count)
{
    add64(TrustedImm32(count), AbsoluteAddress(counter.addressOfCounter()));
}
#endif

#if ENABLE(OPCODE_SAMPLING)
#if CPU(X86_64)
ALWAYS_INLINE void JIT::sampleInstruction(Instruction* instruction, bool inHostFunction)
{
    move(TrustedImmPtr(m_interpreter->sampler()->sampleSlot()), X86Registers::ecx);
    storePtr(TrustedImmPtr(m_interpreter->sampler()->encodeSample(instruction, inHostFunction)), X86Registers::ecx);
}
#else
ALWAYS_INLINE void JIT::sampleInstruction(Instruction* instruction, bool inHostFunction)
{
    storePtr(TrustedImmPtr(m_interpreter->sampler()->encodeSample(instruction, inHostFunction)), m_interpreter->sampler()->sampleSlot());
}
#endif
#endif

#if ENABLE(CODEBLOCK_SAMPLING)
#if CPU(X86_64)
ALWAYS_INLINE void JIT::sampleCodeBlock(CodeBlock* codeBlock)
{
    move(TrustedImmPtr(m_interpreter->sampler()->codeBlockSlot()), X86Registers::ecx);
    storePtr(TrustedImmPtr(codeBlock), X86Registers::ecx);
}
#else
ALWAYS_INLINE void JIT::sampleCodeBlock(CodeBlock* codeBlock)
{
    storePtr(TrustedImmPtr(codeBlock), m_interpreter->sampler()->codeBlockSlot());
}
#endif
#endif

ALWAYS_INLINE bool JIT::isOperandConstantChar(int src)
{
    return m_codeBlock->isConstantRegisterIndex(src) && getConstantOperand(src).isString() && asString(getConstantOperand(src).asCell())->length() == 1;
}

inline void JIT::emitValueProfilingSite(ValueProfile* valueProfile)
{
    ASSERT(shouldEmitProfiling());
    ASSERT(valueProfile);

    const RegisterID value = regT0;
#if USE(JSVALUE32_64)
    const RegisterID valueTag = regT1;
#endif
    
    // We're in a simple configuration: only one bucket, so we can just do a direct
    // store.
#if USE(JSVALUE64)
    storeValueExclusive(value, valueProfile->m_buckets);
#else
    EncodedValueDescriptor* descriptor = bitwise_cast<EncodedValueDescriptor*>(valueProfile->m_buckets);
    store64Exclusive(value, valueTag, TrustedImmPtr(&descriptor->asBits.payload));
#endif
}

inline void JIT::emitValueProfilingSite(unsigned bytecodeOffset)
{
    if (!shouldEmitProfiling())
        return;
    emitValueProfilingSite(m_codeBlock->valueProfileForBytecodeOffset(bytecodeOffset));
}

inline void JIT::emitValueProfilingSite()
{
    emitValueProfilingSite(m_bytecodeOffset);
}

inline void JIT::emitArrayProfilingSiteWithCell(RegisterID cell, RegisterID indexingType, ArrayProfile* arrayProfile)
{
    if (shouldEmitProfiling()) {
        load32(MacroAssembler::Address(cell, JSCell::structureIDOffset()), indexingType);
        store32(indexingType, arrayProfile->addressOfLastSeenStructureID());
    }

    load8(Address(cell, JSCell::indexingTypeAndMiscOffset()), indexingType);
}

inline void JIT::emitArrayProfilingSiteForBytecodeIndexWithCell(RegisterID cell, RegisterID indexingType, unsigned bytecodeIndex)
{
    emitArrayProfilingSiteWithCell(cell, indexingType, m_codeBlock->getOrAddArrayProfile(bytecodeIndex));
}

inline void JIT::emitArrayProfileStoreToHoleSpecialCase(ArrayProfile* arrayProfile)
{
    store8(TrustedImm32(1), arrayProfile->addressOfMayStoreToHole());
}

inline void JIT::emitArrayProfileOutOfBoundsSpecialCase(ArrayProfile* arrayProfile)
{
    store8(TrustedImm32(1), arrayProfile->addressOfOutOfBounds());
}

static inline bool arrayProfileSaw(ArrayModes arrayModes, IndexingType capability)
{
    return arrayModesInclude(arrayModes, capability);
}

inline JITArrayMode JIT::chooseArrayMode(ArrayProfile* profile)
{
    ConcurrentJSLocker locker(m_codeBlock->m_lock);
    profile->computeUpdatedPrediction(locker, m_codeBlock);
    ArrayModes arrayModes = profile->observedArrayModes(locker);
    if (arrayProfileSaw(arrayModes, DoubleShape))
        return JITDouble;
    if (arrayProfileSaw(arrayModes, Int32Shape))
        return JITInt32;
    if (arrayProfileSaw(arrayModes, ArrayStorageShape))
        return JITArrayStorage;
    return JITContiguous;
}

ALWAYS_INLINE int32_t JIT::getOperandConstantInt(int src)
{
    return getConstantOperand(src).asInt32();
}

ALWAYS_INLINE double JIT::getOperandConstantDouble(int src)
{
    return getConstantOperand(src).asDouble();
}

#if USE(JSVALUE32_64)

inline void JIT::emitLoadTag(int index, RegisterID tag)
{
    if (m_codeBlock->isConstantRegisterIndex(index)) {
        move(Imm32(getConstantOperand(index).tag()), tag);
        return;
    }

    load32(tagFor(index), tag);
}

inline void JIT::emitLoadPayload(int index, RegisterID payload)
{
    if (m_codeBlock->isConstantRegisterIndex(index)) {
        move(Imm32(getConstantOperand(index).payload()), payload);
        return;
    }

    load32(payloadFor(index), payload);
}

inline void JIT::emitLoad(const JSValue& v, RegisterID tag, RegisterID payload)
{
    move(Imm32(v.payload()), payload);
    move(Imm32(v.tag()), tag);
}

ALWAYS_INLINE void JIT::emitGetVirtualRegister(int src, JSValueRegs dst)
{
    emitLoad(src, dst.tagGPR(), dst.payloadGPR());
}

ALWAYS_INLINE void JIT::emitPutVirtualRegister(int dst, JSValueRegs from)
{
    emitStore(dst, from.tagGPR(), from.payloadGPR());
}

inline void JIT::emitLoad(int index, RegisterID tag, RegisterID payload, RegisterID base)
{
    RELEASE_ASSERT(tag != payload);

    if (base == callFrameRegister) {
        RELEASE_ASSERT(payload != base);
        emitLoadPayload(index, payload);
        emitLoadTag(index, tag);
        return;
    }

    if (payload == base) { // avoid stomping base
        load32(tagFor(index, base), tag);
        load32(payloadFor(index, base), payload);
        return;
    }

    load32(payloadFor(index, base), payload);
    load32(tagFor(index, base), tag);
}

inline void JIT::emitLoad2(int index1, RegisterID tag1, RegisterID payload1, int index2, RegisterID tag2, RegisterID payload2)
{
    emitLoad(index2, tag2, payload2);
    emitLoad(index1, tag1, payload1);
}

inline void JIT::emitLoadDouble(int index, FPRegisterID value)
{
    if (m_codeBlock->isConstantRegisterIndex(index)) {
        WriteBarrier<Unknown>& inConstantPool = m_codeBlock->constantRegister(index);
        loadDouble(TrustedImmPtr(&inConstantPool), value);
    } else
        loadDouble(addressFor(index), value);
}

inline void JIT::emitLoadInt32ToDouble(int index, FPRegisterID value)
{
    if (m_codeBlock->isConstantRegisterIndex(index)) {
        WriteBarrier<Unknown>& inConstantPool = m_codeBlock->constantRegister(index);
        char* bytePointer = reinterpret_cast<char*>(&inConstantPool);
        convertInt32ToDouble(AbsoluteAddress(bytePointer + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), value);
    } else
        convertInt32ToDouble(payloadFor(index), value);
}

inline void JIT::emitStore(int index, RegisterID tag, RegisterID payload, RegisterID base)
{
    store32(payload, payloadFor(index, base));
    store32(tag, tagFor(index, base));
}

inline void JIT::emitStoreInt32(int index, RegisterID payload, bool indexIsInt32)
{
    store32(payload, payloadFor(index, callFrameRegister));
    if (!indexIsInt32)
        store32(TrustedImm32(JSValue::Int32Tag), tagFor(index, callFrameRegister));
}

inline void JIT::emitStoreInt32(int index, TrustedImm32 payload, bool indexIsInt32)
{
    store32(payload, payloadFor(index, callFrameRegister));
    if (!indexIsInt32)
        store32(TrustedImm32(JSValue::Int32Tag), tagFor(index, callFrameRegister));
}

inline void JIT::emitStoreCell(int index, RegisterID payload, bool indexIsCell)
{
    store32(payload, payloadFor(index, callFrameRegister));
    if (!indexIsCell)
        store32(TrustedImm32(JSValue::CellTag), tagFor(index, callFrameRegister));
}

inline void JIT::emitStoreBool(int index, RegisterID payload, bool indexIsBool)
{
    store32(payload, payloadFor(index, callFrameRegister));
    if (!indexIsBool)
        store32(TrustedImm32(JSValue::BooleanTag), tagFor(index, callFrameRegister));
}

inline void JIT::emitStoreDouble(int index, FPRegisterID value)
{
    storeDouble(value, addressFor(index));
}

inline void JIT::emitStore(int index, const JSValue constant, RegisterID base)
{
    store32(Imm32(constant.payload()), payloadFor(index, base));
    store32(Imm32(constant.tag()), tagFor(index, base));
}

ALWAYS_INLINE void JIT::emitInitRegister(int dst)
{
    emitStore(dst, jsUndefined());
}

inline void JIT::emitJumpSlowCaseIfNotJSCell(int virtualRegisterIndex)
{
    if (!m_codeBlock->isKnownNotImmediate(virtualRegisterIndex)) {
        if (m_codeBlock->isConstantRegisterIndex(virtualRegisterIndex))
            addSlowCase(jump());
        else
            addSlowCase(emitJumpIfNotJSCell(virtualRegisterIndex));
    }
}

inline void JIT::emitJumpSlowCaseIfNotJSCell(int virtualRegisterIndex, RegisterID tag)
{
    if (!m_codeBlock->isKnownNotImmediate(virtualRegisterIndex)) {
        if (m_codeBlock->isConstantRegisterIndex(virtualRegisterIndex))
            addSlowCase(jump());
        else
            addSlowCase(branch32(NotEqual, tag, TrustedImm32(JSValue::CellTag)));
    }
}

ALWAYS_INLINE bool JIT::isOperandConstantInt(int src)
{
    return m_codeBlock->isConstantRegisterIndex(src) && getConstantOperand(src).isInt32();
}

ALWAYS_INLINE bool JIT::getOperandConstantInt(int op1, int op2, int& op, int32_t& constant)
{
    if (isOperandConstantInt(op1)) {
        constant = getConstantOperand(op1).asInt32();
        op = op2;
        return true;
    }

    if (isOperandConstantInt(op2)) {
        constant = getConstantOperand(op2).asInt32();
        op = op1;
        return true;
    }
    
    return false;
}

#else // USE(JSVALUE32_64)

// get arg puts an arg from the SF register array into a h/w register
ALWAYS_INLINE void JIT::emitGetVirtualRegister(int src, RegisterID dst)
{
    ASSERT(m_bytecodeOffset != std::numeric_limits<unsigned>::max()); // This method should only be called during hot/cold path generation, so that m_bytecodeOffset is set.

    // TODO: we want to reuse values that are already in registers if we can - add a register allocator!
    if (m_codeBlock->isConstantRegisterIndex(src)) {
        JSValue value = m_codeBlock->getConstant(src);
        if (!value.isNumber())
            move(TrustedImm64(JSValue::encode(value)), dst);
        else
            move(Imm64(JSValue::encode(value)), dst);
        return;
    }

    load64(Address(callFrameRegister, src * sizeof(Register)), dst);
}

ALWAYS_INLINE void JIT::emitGetVirtualRegister(int src, JSValueRegs dst)
{
    emitGetVirtualRegister(src, dst.payloadGPR());
}

ALWAYS_INLINE void JIT::emitGetVirtualRegister(VirtualRegister src, RegisterID dst)
{
    emitGetVirtualRegister(src.offset(), dst);
}

ALWAYS_INLINE void JIT::emitGetVirtualRegisters(int src1, RegisterID dst1, int src2, RegisterID dst2)
{
    emitGetVirtualRegister(src1, dst1);
    emitGetVirtualRegister(src2, dst2);
}

ALWAYS_INLINE void JIT::emitGetVirtualRegisters(VirtualRegister src1, RegisterID dst1, VirtualRegister src2, RegisterID dst2)
{
    emitGetVirtualRegisters(src1.offset(), dst1, src2.offset(), dst2);
}

ALWAYS_INLINE bool JIT::isOperandConstantInt(int src)
{
    return m_codeBlock->isConstantRegisterIndex(src) && getConstantOperand(src).isInt32();
}

ALWAYS_INLINE void JIT::emitPutVirtualRegister(int dst, RegisterID from)
{
    store64(from, Address(callFrameRegister, dst * sizeof(Register)));
}

ALWAYS_INLINE void JIT::emitPutVirtualRegister(int dst, JSValueRegs from)
{
    emitPutVirtualRegister(dst, from.payloadGPR());
}

ALWAYS_INLINE void JIT::emitPutVirtualRegister(VirtualRegister dst, RegisterID from)
{
    emitPutVirtualRegister(dst.offset(), from);
}

ALWAYS_INLINE void JIT::emitInitRegister(int dst)
{
    store64(TrustedImm64(JSValue::encode(jsUndefined())), Address(callFrameRegister, dst * sizeof(Register)));
}

ALWAYS_INLINE JIT::Jump JIT::emitJumpIfJSCell(RegisterID reg)
{
    return branchTest64(Zero, reg, tagMaskRegister);
}

ALWAYS_INLINE JIT::Jump JIT::emitJumpIfBothJSCells(RegisterID reg1, RegisterID reg2, RegisterID scratch)
{
    move(reg1, scratch);
    or64(reg2, scratch);
    return emitJumpIfJSCell(scratch);
}

ALWAYS_INLINE void JIT::emitJumpSlowCaseIfJSCell(RegisterID reg)
{
    addSlowCase(emitJumpIfJSCell(reg));
}

ALWAYS_INLINE void JIT::emitJumpSlowCaseIfNotJSCell(RegisterID reg)
{
    addSlowCase(emitJumpIfNotJSCell(reg));
}

ALWAYS_INLINE void JIT::emitJumpSlowCaseIfNotJSCell(RegisterID reg, int vReg)
{
    if (!m_codeBlock->isKnownNotImmediate(vReg))
        emitJumpSlowCaseIfNotJSCell(reg);
}

inline void JIT::emitLoadDouble(int index, FPRegisterID value)
{
    if (m_codeBlock->isConstantRegisterIndex(index)) {
        WriteBarrier<Unknown>& inConstantPool = m_codeBlock->constantRegister(index);
        loadDouble(TrustedImmPtr(&inConstantPool), value);
    } else
        loadDouble(addressFor(index), value);
}

inline void JIT::emitLoadInt32ToDouble(int index, FPRegisterID value)
{
    if (m_codeBlock->isConstantRegisterIndex(index)) {
        ASSERT(isOperandConstantInt(index));
        convertInt32ToDouble(Imm32(getConstantOperand(index).asInt32()), value);
    } else
        convertInt32ToDouble(addressFor(index), value);
}

ALWAYS_INLINE JIT::Jump JIT::emitJumpIfInt(RegisterID reg)
{
    return branch64(AboveOrEqual, reg, tagTypeNumberRegister);
}

ALWAYS_INLINE JIT::Jump JIT::emitJumpIfNotInt(RegisterID reg)
{
    return branch64(Below, reg, tagTypeNumberRegister);
}

ALWAYS_INLINE JIT::PatchableJump JIT::emitPatchableJumpIfNotInt(RegisterID reg)
{
    return patchableBranch64(Below, reg, tagTypeNumberRegister);
}

ALWAYS_INLINE JIT::Jump JIT::emitJumpIfNotInt(RegisterID reg1, RegisterID reg2, RegisterID scratch)
{
    move(reg1, scratch);
    and64(reg2, scratch);
    return emitJumpIfNotInt(scratch);
}

ALWAYS_INLINE void JIT::emitJumpSlowCaseIfNotInt(RegisterID reg)
{
    addSlowCase(emitJumpIfNotInt(reg));
}

ALWAYS_INLINE void JIT::emitJumpSlowCaseIfNotInt(RegisterID reg1, RegisterID reg2, RegisterID scratch)
{
    addSlowCase(emitJumpIfNotInt(reg1, reg2, scratch));
}

ALWAYS_INLINE void JIT::emitJumpSlowCaseIfNotNumber(RegisterID reg)
{
    addSlowCase(emitJumpIfNotNumber(reg));
}

ALWAYS_INLINE void JIT::emitTagBool(RegisterID reg)
{
    or32(TrustedImm32(static_cast<int32_t>(ValueFalse)), reg);
}

inline Instruction* JIT::copiedInstruction(Instruction* inst)
{
    ASSERT(inst >= m_codeBlock->instructions().begin() && inst < m_codeBlock->instructions().end());
    return m_instructions.begin() + (inst - m_codeBlock->instructions().begin());
}

#endif // USE(JSVALUE32_64)

} // namespace JSC

#endif // ENABLE(JIT)
