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

// We've run into some problems where changing the size of the class JIT leads to
// performance fluctuations.  Try forcing alignment in an attempt to stabalize this.
#if COMPILER(GCC_OR_CLANG)
#define JIT_CLASS_ALIGNMENT __attribute__ ((aligned (32)))
#else
#define JIT_CLASS_ALIGNMENT
#endif

#define ASSERT_JIT_OFFSET(actual, expected) ASSERT_WITH_MESSAGE(actual == expected, "JIT Offset \"%s\" should be %d, not %d.\n", #expected, static_cast<int>(expected), static_cast<int>(actual));

#include "CodeBlock.h"
#include "CompactJITCodeMap.h"
#include "JITDisassembler.h"
#include "JITInlineCacheGenerator.h"
#include "JITMathIC.h"
#include "JSInterfaceJIT.h"
#include "PCToCodeOriginMap.h"
#include "UnusedPointer.h"

namespace JSC {

    enum OpcodeID : unsigned;

    class ArrayAllocationProfile;
    class CallLinkInfo;
    class CodeBlock;
    class FunctionExecutable;
    class JIT;
    class Identifier;
    class Interpreter;
    class MarkedAllocator;
    class Register;
    class StructureChain;
    class StructureStubInfo;

    struct Instruction;
    struct OperandTypes;
    struct SimpleJumpTable;
    struct StringJumpTable;

    struct CallRecord {
        MacroAssembler::Call from;
        unsigned bytecodeOffset;
        void* to;

        CallRecord()
        {
        }

        CallRecord(MacroAssembler::Call from, unsigned bytecodeOffset, void* to = 0)
            : from(from)
            , bytecodeOffset(bytecodeOffset)
            , to(to)
        {
        }
    };

    struct JumpTable {
        MacroAssembler::Jump from;
        unsigned toBytecodeOffset;

        JumpTable(MacroAssembler::Jump f, unsigned t)
            : from(f)
            , toBytecodeOffset(t)
        {
        }
    };

    struct SlowCaseEntry {
        MacroAssembler::Jump from;
        unsigned to;
        
        SlowCaseEntry(MacroAssembler::Jump f, unsigned t)
            : from(f)
            , to(t)
        {
        }
    };

    struct SwitchRecord {
        enum Type {
            Immediate,
            Character,
            String
        };

        Type type;

        union {
            SimpleJumpTable* simpleJumpTable;
            StringJumpTable* stringJumpTable;
        } jumpTable;

        unsigned bytecodeOffset;
        unsigned defaultOffset;

        SwitchRecord(SimpleJumpTable* jumpTable, unsigned bytecodeOffset, unsigned defaultOffset, Type type)
            : type(type)
            , bytecodeOffset(bytecodeOffset)
            , defaultOffset(defaultOffset)
        {
            this->jumpTable.simpleJumpTable = jumpTable;
        }

        SwitchRecord(StringJumpTable* jumpTable, unsigned bytecodeOffset, unsigned defaultOffset)
            : type(String)
            , bytecodeOffset(bytecodeOffset)
            , defaultOffset(defaultOffset)
        {
            this->jumpTable.stringJumpTable = jumpTable;
        }
    };

    struct ByValCompilationInfo {
        ByValCompilationInfo() { }
        
        ByValCompilationInfo(ByValInfo* byValInfo, unsigned bytecodeIndex, MacroAssembler::PatchableJump notIndexJump, MacroAssembler::PatchableJump badTypeJump, JITArrayMode arrayMode, ArrayProfile* arrayProfile, MacroAssembler::Label doneTarget, MacroAssembler::Label nextHotPathTarget)
            : byValInfo(byValInfo)
            , bytecodeIndex(bytecodeIndex)
            , notIndexJump(notIndexJump)
            , badTypeJump(badTypeJump)
            , arrayMode(arrayMode)
            , arrayProfile(arrayProfile)
            , doneTarget(doneTarget)
            , nextHotPathTarget(nextHotPathTarget)
        {
        }

        ByValInfo* byValInfo;
        unsigned bytecodeIndex;
        MacroAssembler::PatchableJump notIndexJump;
        MacroAssembler::PatchableJump badTypeJump;
        JITArrayMode arrayMode;
        ArrayProfile* arrayProfile;
        MacroAssembler::Label doneTarget;
        MacroAssembler::Label nextHotPathTarget;
        MacroAssembler::Label slowPathTarget;
        MacroAssembler::Call returnAddress;
    };

    struct CallCompilationInfo {
        MacroAssembler::DataLabelPtr hotPathBegin;
        MacroAssembler::Call hotPathOther;
        MacroAssembler::Call callReturnLocation;
        CallLinkInfo* callLinkInfo;
    };

    void ctiPatchCallByReturnAddress(ReturnAddressPtr, FunctionPtr newCalleeFunction);

    class JIT : private JSInterfaceJIT {
        friend class JITSlowPathCall;
        friend class JITStubCall;

        using MacroAssembler::Jump;
        using MacroAssembler::JumpList;
        using MacroAssembler::Label;

        static const uintptr_t patchGetByIdDefaultStructure = unusedPointer;
        static const int patchGetByIdDefaultOffset = 0;
        // Magic number - initial offset cannot be representable as a signed 8bit value, or the X86Assembler
        // will compress the displacement, and we may not be able to fit a patched offset.
        static const int patchPutByIdDefaultOffset = 256;

    public:
        JIT(VM*, CodeBlock* = 0, unsigned loopOSREntryBytecodeOffset = 0);
        ~JIT();

        void compileWithoutLinking(JITCompilationEffort);
        CompilationResult link();

        void doMainThreadPreparationBeforeCompile();
        
        static CompilationResult compile(VM* vm, CodeBlock* codeBlock, JITCompilationEffort effort, unsigned bytecodeOffset = 0)
        {
            return JIT(vm, codeBlock, bytecodeOffset).privateCompile(effort);
        }
        
        static void compileGetByVal(VM* vm, CodeBlock* codeBlock, ByValInfo* byValInfo, ReturnAddressPtr returnAddress, JITArrayMode arrayMode)
        {
            JIT jit(vm, codeBlock);
            jit.m_bytecodeOffset = byValInfo->bytecodeIndex;
            jit.privateCompileGetByVal(byValInfo, returnAddress, arrayMode);
        }

        static void compileGetByValWithCachedId(VM* vm, CodeBlock* codeBlock, ByValInfo* byValInfo, ReturnAddressPtr returnAddress, const Identifier& propertyName)
        {
            JIT jit(vm, codeBlock);
            jit.m_bytecodeOffset = byValInfo->bytecodeIndex;
            jit.privateCompileGetByValWithCachedId(byValInfo, returnAddress, propertyName);
        }

        static void compilePutByVal(VM* vm, CodeBlock* codeBlock, ByValInfo* byValInfo, ReturnAddressPtr returnAddress, JITArrayMode arrayMode)
        {
            JIT jit(vm, codeBlock);
            jit.m_bytecodeOffset = byValInfo->bytecodeIndex;
            jit.privateCompilePutByVal(byValInfo, returnAddress, arrayMode);
        }
        
        static void compileDirectPutByVal(VM* vm, CodeBlock* codeBlock, ByValInfo* byValInfo, ReturnAddressPtr returnAddress, JITArrayMode arrayMode)
        {
            JIT jit(vm, codeBlock);
            jit.m_bytecodeOffset = byValInfo->bytecodeIndex;
            jit.privateCompilePutByVal(byValInfo, returnAddress, arrayMode);
        }

        static void compilePutByValWithCachedId(VM* vm, CodeBlock* codeBlock, ByValInfo* byValInfo, ReturnAddressPtr returnAddress, PutKind putKind, const Identifier& propertyName)
        {
            JIT jit(vm, codeBlock);
            jit.m_bytecodeOffset = byValInfo->bytecodeIndex;
            jit.privateCompilePutByValWithCachedId(byValInfo, returnAddress, putKind, propertyName);
        }

        static void compileHasIndexedProperty(VM* vm, CodeBlock* codeBlock, ByValInfo* byValInfo, ReturnAddressPtr returnAddress, JITArrayMode arrayMode)
        {
            JIT jit(vm, codeBlock);
            jit.m_bytecodeOffset = byValInfo->bytecodeIndex;
            jit.privateCompileHasIndexedProperty(byValInfo, returnAddress, arrayMode);
        }

        static CodeRef compileCTINativeCall(VM*, NativeFunction);

        static unsigned frameRegisterCountFor(CodeBlock*);
        static int stackPointerOffsetFor(CodeBlock*);

        JS_EXPORT_PRIVATE static HashMap<CString, double> compileTimeStats();

    private:
        void privateCompileMainPass();
        void privateCompileLinkPass();
        void privateCompileSlowCases();
        CompilationResult privateCompile(JITCompilationEffort);
        
        void privateCompileGetByVal(ByValInfo*, ReturnAddressPtr, JITArrayMode);
        void privateCompileGetByValWithCachedId(ByValInfo*, ReturnAddressPtr, const Identifier&);
        void privateCompilePutByVal(ByValInfo*, ReturnAddressPtr, JITArrayMode);
        void privateCompilePutByValWithCachedId(ByValInfo*, ReturnAddressPtr, PutKind, const Identifier&);

        void privateCompileHasIndexedProperty(ByValInfo*, ReturnAddressPtr, JITArrayMode);

        Label privateCompileCTINativeCall(VM*, bool isConstruct = false);
        CodeRef privateCompileCTINativeCall(VM*, NativeFunction);
        void privateCompilePatchGetArrayLength(ReturnAddressPtr returnAddress);

        // Add a call out from JIT code, without an exception check.
        Call appendCall(const FunctionPtr& function)
        {
            Call functionCall = call();
            m_calls.append(CallRecord(functionCall, m_bytecodeOffset, function.value()));
            return functionCall;
        }

#if OS(WINDOWS) && CPU(X86_64)
        Call appendCallWithSlowPathReturnType(const FunctionPtr& function)
        {
            Call functionCall = callWithSlowPathReturnType();
            m_calls.append(CallRecord(functionCall, m_bytecodeOffset, function.value()));
            return functionCall;
        }
#endif

        void exceptionCheck(Jump jumpToHandler)
        {
            m_exceptionChecks.append(jumpToHandler);
        }

        void exceptionCheck()
        {
            m_exceptionChecks.append(emitExceptionCheck());
        }

        void exceptionCheckWithCallFrameRollback()
        {
            m_exceptionChecksWithCallFrameRollback.append(emitExceptionCheck());
        }

        void privateCompileExceptionHandlers();

        void addSlowCase(Jump);
        void addSlowCase(const JumpList&);
        void addSlowCase();
        void addJump(Jump, int);
        void emitJumpSlowToHot(Jump, int);

        void compileOpCall(OpcodeID, Instruction*, unsigned callLinkInfoIndex);
        void compileOpCallSlowCase(OpcodeID, Instruction*, Vector<SlowCaseEntry>::iterator&, unsigned callLinkInfoIndex);
        void compileSetupVarargsFrame(OpcodeID, Instruction*, CallLinkInfo*);
        void compileCallEval(Instruction*);
        void compileCallEvalSlowCase(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitPutCallResult(Instruction*);

        enum CompileOpStrictEqType { OpStrictEq, OpNStrictEq };
        void compileOpStrictEq(Instruction* instruction, CompileOpStrictEqType type);
        bool isOperandConstantDouble(int src);
        
        void emitLoadDouble(int index, FPRegisterID value);
        void emitLoadInt32ToDouble(int index, FPRegisterID value);
        Jump emitJumpIfCellObject(RegisterID cellReg);
        Jump emitJumpIfCellNotObject(RegisterID cellReg);

        enum WriteBarrierMode { UnconditionalWriteBarrier, ShouldFilterBase, ShouldFilterValue, ShouldFilterBaseAndValue };
        // value register in write barrier is used before any scratch registers
        // so may safely be the same as either of the scratch registers.
        void emitWriteBarrier(unsigned owner, unsigned value, WriteBarrierMode);
        void emitWriteBarrier(JSCell* owner, unsigned value, WriteBarrierMode);
        void emitWriteBarrier(JSCell* owner);

        // This assumes that the value to profile is in regT0 and that regT3 is available for
        // scratch.
        void emitValueProfilingSite(ValueProfile*);
        void emitValueProfilingSite(unsigned bytecodeOffset);
        void emitValueProfilingSite();
        void emitArrayProfilingSiteWithCell(RegisterID cell, RegisterID indexingType, ArrayProfile*);
        void emitArrayProfilingSiteForBytecodeIndexWithCell(RegisterID cell, RegisterID indexingType, unsigned bytecodeIndex);
        void emitArrayProfileStoreToHoleSpecialCase(ArrayProfile*);
        void emitArrayProfileOutOfBoundsSpecialCase(ArrayProfile*);
        
        JITArrayMode chooseArrayMode(ArrayProfile*);
        
        // Property is in regT1, base is in regT0. regT2 contains indexing type.
        // Property is int-checked and zero extended. Base is cell checked.
        // Structure is already profiled. Returns the slow cases. Fall-through
        // case contains result in regT0, and it is not yet profiled.
        JumpList emitInt32Load(Instruction* instruction, PatchableJump& badType) { return emitContiguousLoad(instruction, badType, Int32Shape); }
        JumpList emitDoubleLoad(Instruction*, PatchableJump& badType);
        JumpList emitContiguousLoad(Instruction*, PatchableJump& badType, IndexingType expectedShape = ContiguousShape);
        JumpList emitArrayStorageLoad(Instruction*, PatchableJump& badType);
        JumpList emitLoadForArrayMode(Instruction*, JITArrayMode, PatchableJump& badType);

        JumpList emitInt32GetByVal(Instruction* instruction, PatchableJump& badType) { return emitContiguousGetByVal(instruction, badType, Int32Shape); }
        JumpList emitDoubleGetByVal(Instruction*, PatchableJump& badType);
        JumpList emitContiguousGetByVal(Instruction*, PatchableJump& badType, IndexingType expectedShape = ContiguousShape);
        JumpList emitArrayStorageGetByVal(Instruction*, PatchableJump& badType);
        JumpList emitDirectArgumentsGetByVal(Instruction*, PatchableJump& badType);
        JumpList emitScopedArgumentsGetByVal(Instruction*, PatchableJump& badType);
        JumpList emitIntTypedArrayGetByVal(Instruction*, PatchableJump& badType, TypedArrayType);
        JumpList emitFloatTypedArrayGetByVal(Instruction*, PatchableJump& badType, TypedArrayType);
        
        // Property is in regT1, base is in regT0. regT2 contains indecing type.
        // The value to store is not yet loaded. Property is int-checked and
        // zero-extended. Base is cell checked. Structure is already profiled.
        // returns the slow cases.
        JumpList emitInt32PutByVal(Instruction* currentInstruction, PatchableJump& badType)
        {
            return emitGenericContiguousPutByVal(currentInstruction, badType, Int32Shape);
        }
        JumpList emitDoublePutByVal(Instruction* currentInstruction, PatchableJump& badType)
        {
            return emitGenericContiguousPutByVal(currentInstruction, badType, DoubleShape);
        }
        JumpList emitContiguousPutByVal(Instruction* currentInstruction, PatchableJump& badType)
        {
            return emitGenericContiguousPutByVal(currentInstruction, badType);
        }
        JumpList emitGenericContiguousPutByVal(Instruction*, PatchableJump& badType, IndexingType indexingShape = ContiguousShape);
        JumpList emitArrayStoragePutByVal(Instruction*, PatchableJump& badType);
        JumpList emitIntTypedArrayPutByVal(Instruction*, PatchableJump& badType, TypedArrayType);
        JumpList emitFloatTypedArrayPutByVal(Instruction*, PatchableJump& badType, TypedArrayType);

        // Identifier check helper for GetByVal and PutByVal.
        void emitByValIdentifierCheck(ByValInfo*, RegisterID cell, RegisterID scratch, const Identifier&, JumpList& slowCases);

        JITGetByIdGenerator emitGetByValWithCachedId(ByValInfo*, Instruction*, const Identifier&, Jump& fastDoneCase, Jump& slowDoneCase, JumpList& slowCases);
        JITPutByIdGenerator emitPutByValWithCachedId(ByValInfo*, Instruction*, PutKind, const Identifier&, JumpList& doneCases, JumpList& slowCases);

        enum FinalObjectMode { MayBeFinal, KnownNotFinal };

        void emitGetVirtualRegister(int src, JSValueRegs dst);
        void emitPutVirtualRegister(int dst, JSValueRegs src);

        int32_t getOperandConstantInt(int src);
        double getOperandConstantDouble(int src);

#if USE(JSVALUE32_64)
        bool getOperandConstantInt(int op1, int op2, int& op, int32_t& constant);

        void emitLoadTag(int index, RegisterID tag);
        void emitLoadPayload(int index, RegisterID payload);

        void emitLoad(const JSValue& v, RegisterID tag, RegisterID payload);
        void emitLoad(int index, RegisterID tag, RegisterID payload, RegisterID base = callFrameRegister);
        void emitLoad2(int index1, RegisterID tag1, RegisterID payload1, int index2, RegisterID tag2, RegisterID payload2);

        void emitStore(int index, RegisterID tag, RegisterID payload, RegisterID base = callFrameRegister);
        void emitStore(int index, const JSValue constant, RegisterID base = callFrameRegister);
        void emitStoreInt32(int index, RegisterID payload, bool indexIsInt32 = false);
        void emitStoreInt32(int index, TrustedImm32 payload, bool indexIsInt32 = false);
        void emitStoreCell(int index, RegisterID payload, bool indexIsCell = false);
        void emitStoreBool(int index, RegisterID payload, bool indexIsBool = false);
        void emitStoreDouble(int index, FPRegisterID value);

        void emitJumpSlowCaseIfNotJSCell(int virtualRegisterIndex);
        void emitJumpSlowCaseIfNotJSCell(int virtualRegisterIndex, RegisterID tag);

        void compileGetByIdHotPath(const Identifier*);

        // Arithmetic opcode helpers
        void emitBinaryDoubleOp(OpcodeID, int dst, int op1, int op2, OperandTypes, JumpList& notInt32Op1, JumpList& notInt32Op2, bool op1IsInRegisters = true, bool op2IsInRegisters = true);

#else // USE(JSVALUE32_64)
        void emitGetVirtualRegister(int src, RegisterID dst);
        void emitGetVirtualRegister(VirtualRegister src, RegisterID dst);
        void emitGetVirtualRegisters(int src1, RegisterID dst1, int src2, RegisterID dst2);
        void emitGetVirtualRegisters(VirtualRegister src1, RegisterID dst1, VirtualRegister src2, RegisterID dst2);
        void emitPutVirtualRegister(int dst, RegisterID from = regT0);
        void emitPutVirtualRegister(VirtualRegister dst, RegisterID from = regT0);
        void emitStoreCell(int dst, RegisterID payload, bool /* only used in JSValue32_64 */ = false)
        {
            emitPutVirtualRegister(dst, payload);
        }
        void emitStoreCell(VirtualRegister dst, RegisterID payload)
        {
            emitPutVirtualRegister(dst, payload);
        }

        Jump emitJumpIfJSCell(RegisterID);
        Jump emitJumpIfBothJSCells(RegisterID, RegisterID, RegisterID);
        void emitJumpSlowCaseIfJSCell(RegisterID);
        void emitJumpSlowCaseIfNotJSCell(RegisterID);
        void emitJumpSlowCaseIfNotJSCell(RegisterID, int VReg);
        Jump emitJumpIfInt(RegisterID);
        Jump emitJumpIfNotInt(RegisterID);
        Jump emitJumpIfNotInt(RegisterID, RegisterID, RegisterID scratch);
        PatchableJump emitPatchableJumpIfNotInt(RegisterID);
        void emitJumpSlowCaseIfNotInt(RegisterID);
        void emitJumpSlowCaseIfNotNumber(RegisterID);
        void emitJumpSlowCaseIfNotInt(RegisterID, RegisterID, RegisterID scratch);

        void emitTagBool(RegisterID);

        void compileGetByIdHotPath(int baseVReg, const Identifier*);

#endif // USE(JSVALUE32_64)

        void emit_compareAndJump(OpcodeID, int op1, int op2, unsigned target, RelationalCondition);
        void emit_compareAndJumpSlow(int op1, int op2, unsigned target, DoubleCondition, size_t (JIT_OPERATION *operation)(ExecState*, EncodedJSValue, EncodedJSValue), bool invert, Vector<SlowCaseEntry>::iterator&);
        
        void assertStackPointerOffset();

        void emit_op_add(Instruction*);
        void emit_op_bitand(Instruction*);
        void emit_op_bitor(Instruction*);
        void emit_op_bitxor(Instruction*);
        void emit_op_call(Instruction*);
        void emit_op_tail_call(Instruction*);
        void emit_op_call_eval(Instruction*);
        void emit_op_call_varargs(Instruction*);
        void emit_op_tail_call_varargs(Instruction*);
        void emit_op_tail_call_forward_arguments(Instruction*);
        void emit_op_construct_varargs(Instruction*);
        void emit_op_catch(Instruction*);
        void emit_op_construct(Instruction*);
        void emit_op_create_this(Instruction*);
        void emit_op_to_this(Instruction*);
        void emit_op_create_direct_arguments(Instruction*);
        void emit_op_create_scoped_arguments(Instruction*);
        void emit_op_create_cloned_arguments(Instruction*);
        void emit_op_get_argument(Instruction*);
        void emit_op_argument_count(Instruction*);
        void emit_op_create_rest(Instruction*);
        void emit_op_get_rest_length(Instruction*);
        void emit_op_check_tdz(Instruction*);
        void emit_op_assert(Instruction*);
        void emit_op_debug(Instruction*);
        void emit_op_del_by_id(Instruction*);
        void emit_op_del_by_val(Instruction*);
        void emit_op_div(Instruction*);
        void emit_op_end(Instruction*);
        void emit_op_enter(Instruction*);
        void emit_op_get_scope(Instruction*);
        void emit_op_eq(Instruction*);
        void emit_op_eq_null(Instruction*);
        void emit_op_try_get_by_id(Instruction*);
        void emit_op_get_by_id(Instruction*);
        void emit_op_get_by_id_with_this(Instruction*);
        void emit_op_get_by_val_with_this(Instruction*);
        void emit_op_get_arguments_length(Instruction*);
        void emit_op_get_by_val(Instruction*);
        void emit_op_get_argument_by_val(Instruction*);
        void emit_op_init_lazy_reg(Instruction*);
        void emit_op_overrides_has_instance(Instruction*);
        void emit_op_instanceof(Instruction*);
        void emit_op_instanceof_custom(Instruction*);
        void emit_op_is_empty(Instruction*);
        void emit_op_is_undefined(Instruction*);
        void emit_op_is_boolean(Instruction*);
        void emit_op_is_number(Instruction*);
        void emit_op_is_object(Instruction*);
        void emit_op_is_cell_with_type(Instruction*);
        void emit_op_jeq_null(Instruction*);
        void emit_op_jfalse(Instruction*);
        void emit_op_jmp(Instruction*);
        void emit_op_jneq_null(Instruction*);
        void emit_op_jneq_ptr(Instruction*);
        void emit_op_jless(Instruction*);
        void emit_op_jlesseq(Instruction*);
        void emit_op_jgreater(Instruction*);
        void emit_op_jgreatereq(Instruction*);
        void emit_op_jnless(Instruction*);
        void emit_op_jnlesseq(Instruction*);
        void emit_op_jngreater(Instruction*);
        void emit_op_jngreatereq(Instruction*);
        void emit_op_jtrue(Instruction*);
        void emit_op_loop_hint(Instruction*);
        void emit_op_check_traps(Instruction*);
        void emit_op_lshift(Instruction*);
        void emit_op_mod(Instruction*);
        void emit_op_mov(Instruction*);
        void emit_op_mul(Instruction*);
        void emit_op_negate(Instruction*);
        void emit_op_neq(Instruction*);
        void emit_op_neq_null(Instruction*);
        void emit_op_new_array(Instruction*);
        void emit_op_new_array_with_size(Instruction*);
        void emit_op_new_array_buffer(Instruction*);
        void emit_op_new_array_with_spread(Instruction*);
        void emit_op_spread(Instruction*);
        void emit_op_new_func(Instruction*);
        void emit_op_new_func_exp(Instruction*);
        void emit_op_new_generator_func(Instruction*);
        void emit_op_new_generator_func_exp(Instruction*);
        void emit_op_new_async_func(Instruction*);
        void emit_op_new_async_func_exp(Instruction*);
        void emit_op_new_object(Instruction*);
        void emit_op_new_regexp(Instruction*);
        void emit_op_not(Instruction*);
        void emit_op_nstricteq(Instruction*);
        void emit_op_dec(Instruction*);
        void emit_op_inc(Instruction*);
        void emit_op_pow(Instruction*);
        void emit_op_profile_type(Instruction*);
        void emit_op_profile_control_flow(Instruction*);
        void emit_op_push_with_scope(Instruction*);
        void emit_op_create_lexical_environment(Instruction*);
        void emit_op_get_parent_scope(Instruction*);
        void emit_op_put_by_id(Instruction*);
        void emit_op_put_by_id_with_this(Instruction*);
        void emit_op_put_by_index(Instruction*);
        void emit_op_put_by_val(Instruction*);
        void emit_op_put_by_val_with_this(Instruction*);
        void emit_op_put_getter_by_id(Instruction*);
        void emit_op_put_setter_by_id(Instruction*);
        void emit_op_put_getter_setter_by_id(Instruction*);
        void emit_op_put_getter_by_val(Instruction*);
        void emit_op_put_setter_by_val(Instruction*);
        void emit_op_define_data_property(Instruction*);
        void emit_op_define_accessor_property(Instruction*);
        void emit_op_ret(Instruction*);
        void emit_op_rshift(Instruction*);
        void emit_op_set_function_name(Instruction*);
        void emit_op_strcat(Instruction*);
        void emit_op_stricteq(Instruction*);
        void emit_op_sub(Instruction*);
        void emit_op_switch_char(Instruction*);
        void emit_op_switch_imm(Instruction*);
        void emit_op_switch_string(Instruction*);
        void emit_op_tear_off_arguments(Instruction*);
        void emit_op_throw(Instruction*);
        void emit_op_throw_static_error(Instruction*);
        void emit_op_to_number(Instruction*);
        void emit_op_to_string(Instruction*);
        void emit_op_to_primitive(Instruction*);
        void emit_op_unexpected_load(Instruction*);
        void emit_op_unsigned(Instruction*);
        void emit_op_urshift(Instruction*);
        void emit_op_get_enumerable_length(Instruction*);
        void emit_op_has_generic_property(Instruction*);
        void emit_op_has_structure_property(Instruction*);
        void emit_op_has_indexed_property(Instruction*);
        void emit_op_get_direct_pname(Instruction*);
        void emit_op_get_property_enumerator(Instruction*);
        void emit_op_enumerator_structure_pname(Instruction*);
        void emit_op_enumerator_generic_pname(Instruction*);
        void emit_op_to_index_string(Instruction*);
        void emit_op_log_shadow_chicken_prologue(Instruction*);
        void emit_op_log_shadow_chicken_tail(Instruction*);

        void emitSlow_op_add(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_bitand(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_bitor(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_bitxor(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_call(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_tail_call(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_call_eval(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_call_varargs(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_tail_call_varargs(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_tail_call_forward_arguments(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_construct_varargs(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_construct(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_to_this(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_create_this(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_check_tdz(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_div(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_eq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_callee(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_try_get_by_id(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_by_id(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_by_id_with_this(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_arguments_length(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_by_val(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_argument_by_val(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_instanceof(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_instanceof_custom(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jless(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jlesseq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jgreater(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jgreatereq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jnless(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jnlesseq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jngreater(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jngreatereq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_jtrue(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_loop_hint(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_check_traps(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_lshift(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_mod(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_mul(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_negate(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_neq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_new_object(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_not(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_nstricteq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_dec(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_inc(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_put_by_id(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_put_by_val(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_rshift(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_stricteq(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_sub(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_to_number(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_to_string(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_to_primitive(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_unsigned(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_urshift(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_has_indexed_property(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_has_structure_property(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_direct_pname(Instruction*, Vector<SlowCaseEntry>::iterator&);

        void emit_op_resolve_scope(Instruction*);
        void emit_op_get_from_scope(Instruction*);
        void emit_op_put_to_scope(Instruction*);
        void emit_op_get_from_arguments(Instruction*);
        void emit_op_put_to_arguments(Instruction*);
        void emitSlow_op_resolve_scope(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_get_from_scope(Instruction*, Vector<SlowCaseEntry>::iterator&);
        void emitSlow_op_put_to_scope(Instruction*, Vector<SlowCaseEntry>::iterator&);

        void emitRightShift(Instruction*, bool isUnsigned);
        void emitRightShiftSlowCase(Instruction*, Vector<SlowCaseEntry>::iterator&, bool isUnsigned);

        void emitNewFuncCommon(Instruction*);
        void emitNewFuncExprCommon(Instruction*);
        void emitVarInjectionCheck(bool needsVarInjectionChecks);
        void emitResolveClosure(int dst, int scope, bool needsVarInjectionChecks, unsigned depth);
        void emitLoadWithStructureCheck(int scope, Structure** structureSlot);
#if USE(JSVALUE64)
        void emitGetVarFromPointer(JSValue* operand, GPRReg);
        void emitGetVarFromIndirectPointer(JSValue** operand, GPRReg);
#else
        void emitGetVarFromIndirectPointer(JSValue** operand, GPRReg tag, GPRReg payload);
        void emitGetVarFromPointer(JSValue* operand, GPRReg tag, GPRReg payload);
#endif
        void emitGetClosureVar(int scope, uintptr_t operand);
        void emitNotifyWrite(WatchpointSet*);
        void emitNotifyWrite(GPRReg pointerToSet);
        void emitPutGlobalVariable(JSValue* operand, int value, WatchpointSet*);
        void emitPutGlobalVariableIndirect(JSValue** addressOfOperand, int value, WatchpointSet**);
        void emitPutClosureVar(int scope, uintptr_t operand, int value, WatchpointSet*);

        void emitInitRegister(int dst);

        void emitPutIntToCallFrameHeader(RegisterID from, int entry);

        JSValue getConstantOperand(int src);
        bool isOperandConstantInt(int src);
        bool isOperandConstantChar(int src);

        template <typename Generator, typename ProfiledFunction, typename NonProfiledFunction>
        void emitMathICFast(JITUnaryMathIC<Generator>*, Instruction*, ProfiledFunction, NonProfiledFunction);
        template <typename Generator, typename ProfiledFunction, typename NonProfiledFunction>
        void emitMathICFast(JITBinaryMathIC<Generator>*, Instruction*, ProfiledFunction, NonProfiledFunction);

        template <typename Generator, typename ProfiledRepatchFunction, typename ProfiledFunction, typename RepatchFunction>
        void emitMathICSlow(JITBinaryMathIC<Generator>*, Instruction*, ProfiledRepatchFunction, ProfiledFunction, RepatchFunction);
        template <typename Generator, typename ProfiledRepatchFunction, typename ProfiledFunction, typename RepatchFunction>
        void emitMathICSlow(JITUnaryMathIC<Generator>*, Instruction*, ProfiledRepatchFunction, ProfiledFunction, RepatchFunction);

        Jump getSlowCase(Vector<SlowCaseEntry>::iterator& iter)
        {
            return iter++->from;
        }
        void linkSlowCase(Vector<SlowCaseEntry>::iterator& iter)
        {
            if (iter->from.isSet())
                iter->from.link(this);
            ++iter;
        }
        void linkDummySlowCase(Vector<SlowCaseEntry>::iterator& iter)
        {
            ASSERT(!iter->from.isSet());
            ++iter;
        }
        void linkSlowCaseIfNotJSCell(Vector<SlowCaseEntry>::iterator&, int virtualRegisterIndex);
        void linkAllSlowCasesForBytecodeOffset(Vector<SlowCaseEntry>& slowCases,
            Vector<SlowCaseEntry>::iterator&, unsigned bytecodeOffset);

        MacroAssembler::Call appendCallWithExceptionCheck(const FunctionPtr&);
#if OS(WINDOWS) && CPU(X86_64)
        MacroAssembler::Call appendCallWithExceptionCheckAndSlowPathReturnType(const FunctionPtr&);
#endif
        MacroAssembler::Call appendCallWithCallFrameRollbackOnException(const FunctionPtr&);
        MacroAssembler::Call appendCallWithExceptionCheckSetJSValueResult(const FunctionPtr&, int);
        MacroAssembler::Call appendCallWithExceptionCheckSetJSValueResultWithProfile(const FunctionPtr&, int);
        
        enum WithProfileTag { WithProfile };
        
        MacroAssembler::Call callOperation(C_JITOperation_E);
        MacroAssembler::Call callOperation(C_JITOperation_EO, GPRReg);
        MacroAssembler::Call callOperation(C_JITOperation_EL, GPRReg);
        MacroAssembler::Call callOperation(C_JITOperation_EL, TrustedImmPtr);
        MacroAssembler::Call callOperation(C_JITOperation_ESt, Structure*);
        MacroAssembler::Call callOperation(C_JITOperation_EZ, int32_t);
        MacroAssembler::Call callOperation(Z_JITOperation_EJZZ, GPRReg, int32_t, int32_t);
        MacroAssembler::Call callOperation(J_JITOperation_E, int);
        MacroAssembler::Call callOperation(J_JITOperation_EAapJ, int, ArrayAllocationProfile*, GPRReg);
        MacroAssembler::Call callOperation(J_JITOperation_EAapJcpZ, int, ArrayAllocationProfile*, GPRReg, int32_t);
        MacroAssembler::Call callOperation(J_JITOperation_EAapJcpZ, int, ArrayAllocationProfile*, const JSValue*, int32_t);
        MacroAssembler::Call callOperation(J_JITOperation_EC, int, JSCell*);
        MacroAssembler::Call callOperation(V_JITOperation_EC, JSCell*);
        MacroAssembler::Call callOperation(J_JITOperation_EJ, int, GPRReg);
        MacroAssembler::Call callOperation(J_JITOperation_EJ, JSValueRegs, JSValueRegs);
#if USE(JSVALUE64)
        MacroAssembler::Call callOperation(J_JITOperation_ESsiJI, int, StructureStubInfo*, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(WithProfileTag, J_JITOperation_ESsiJI, int, StructureStubInfo*, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(WithProfileTag, J_JITOperation_ESsiJJI, int, StructureStubInfo*, GPRReg, GPRReg, UniquedStringImpl*);
#else
        MacroAssembler::Call callOperation(J_JITOperation_ESsiJI, int, StructureStubInfo*, GPRReg, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(WithProfileTag, J_JITOperation_ESsiJI, int, StructureStubInfo*, GPRReg, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(WithProfileTag, J_JITOperation_ESsiJJI, int, StructureStubInfo*, GPRReg, GPRReg, GPRReg, GPRReg, UniquedStringImpl*);
#endif
        MacroAssembler::Call callOperation(J_JITOperation_EJI, int, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(J_JITOperation_EJJ, int, GPRReg, GPRReg);
        MacroAssembler::Call callOperation(J_JITOperation_EJArp, JSValueRegs, JSValueRegs, ArithProfile*);
        MacroAssembler::Call callOperation(J_JITOperation_EJJArp, JSValueRegs, JSValueRegs, JSValueRegs, ArithProfile*);
        MacroAssembler::Call callOperation(J_JITOperation_EJJ, JSValueRegs, JSValueRegs, JSValueRegs);
        MacroAssembler::Call callOperation(J_JITOperation_EJMic, JSValueRegs, JSValueRegs, TrustedImmPtr);
        MacroAssembler::Call callOperation(J_JITOperation_EJJMic, JSValueRegs, JSValueRegs, JSValueRegs, TrustedImmPtr);
        MacroAssembler::Call callOperation(J_JITOperation_EJJAp, int, GPRReg, GPRReg, ArrayProfile*);
        MacroAssembler::Call callOperation(J_JITOperation_EJJBy, int, GPRReg, GPRReg, ByValInfo*);
        MacroAssembler::Call callOperation(Z_JITOperation_EJOJ, GPRReg, GPRReg, GPRReg);
        MacroAssembler::Call callOperation(C_JITOperation_EJsc, GPRReg);
        MacroAssembler::Call callOperation(J_JITOperation_EJscC, int, GPRReg, JSCell*);
        MacroAssembler::Call callOperation(J_JITOperation_EJscCJ, int, GPRReg, JSCell*, GPRReg);
        MacroAssembler::Call callOperation(C_JITOperation_EJscZ, GPRReg, int32_t);
        MacroAssembler::Call callOperation(C_JITOperation_EJscZ, int, GPRReg, int32_t);
#if USE(JSVALUE64)
        MacroAssembler::Call callOperation(WithProfileTag, J_JITOperation_EJJ, int, GPRReg, GPRReg);
#else
        MacroAssembler::Call callOperation(WithProfileTag, J_JITOperation_EJJ, int, GPRReg, GPRReg, GPRReg, GPRReg);
#endif
        MacroAssembler::Call callOperation(J_JITOperation_EP, int, void*);
        MacroAssembler::Call callOperation(WithProfileTag, J_JITOperation_EPc, int, Instruction*);
        MacroAssembler::Call callOperation(J_JITOperation_EPc, int, Instruction*);
        MacroAssembler::Call callOperation(J_JITOperation_EZ, int, int32_t);
        MacroAssembler::Call callOperation(J_JITOperation_EZZ, int, int32_t, int32_t);
        MacroAssembler::Call callOperation(P_JITOperation_E);
        MacroAssembler::Call callOperation(P_JITOperation_EJS, GPRReg, size_t);
        MacroAssembler::Call callOperation(S_JITOperation_ECC, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(S_JITOperation_EJ, RegisterID);
        MacroAssembler::Call callOperation(S_JITOperation_EJI, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(S_JITOperation_EJJ, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(S_JITOperation_EOJss, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(Sprt_JITOperation_EZ, int32_t);
        MacroAssembler::Call callOperation(V_JITOperation_E);
        MacroAssembler::Call callOperation(V_JITOperation_EC, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_ECC, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_ECIZC, RegisterID, UniquedStringImpl*, int32_t, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_ECIZCC, RegisterID, UniquedStringImpl*, int32_t, RegisterID, RegisterID);
#if USE(JSVALUE64)
        MacroAssembler::Call callOperation(V_JITOperation_ECJZC, RegisterID, RegisterID, int32_t, RegisterID);
#else
        MacroAssembler::Call callOperation(V_JITOperation_ECJZC, RegisterID, RegisterID, RegisterID, int32_t, RegisterID);
#endif
        MacroAssembler::Call callOperation(J_JITOperation_EE, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EZSymtabJ, int, SymbolTable*, RegisterID);
        MacroAssembler::Call callOperation(J_JITOperation_EZSymtabJ, int, SymbolTable*, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EJ, RegisterID);
        MacroAssembler::Call callOperationNoExceptionCheck(Z_JITOperation_E);
#if USE(JSVALUE64)
        MacroAssembler::Call callOperationNoExceptionCheck(V_JITOperation_EJ, RegisterID);
#else
        MacroAssembler::Call callOperationNoExceptionCheck(V_JITOperation_EJ, RegisterID, RegisterID);
#endif
#if USE(JSVALUE64)
        MacroAssembler::Call callOperation(F_JITOperation_EFJZZ, RegisterID, RegisterID, int32_t, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_ESsiJJI, StructureStubInfo*, RegisterID, RegisterID, UniquedStringImpl*);
        MacroAssembler::Call callOperation(V_JITOperation_ECIZJJ, RegisterID, UniquedStringImpl*, int32_t, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_ECJ, RegisterID, RegisterID);
#else
        MacroAssembler::Call callOperation(V_JITOperation_ESsiJJI, StructureStubInfo*, RegisterID, RegisterID, RegisterID, RegisterID, UniquedStringImpl*);
        MacroAssembler::Call callOperation(V_JITOperation_ECJ, RegisterID, RegisterID, RegisterID);
#endif
        MacroAssembler::Call callOperation(V_JITOperation_EJJJ, RegisterID, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EJJJAp, RegisterID, RegisterID, RegisterID, ArrayProfile*);
        MacroAssembler::Call callOperation(V_JITOperation_EJJJBy, RegisterID, RegisterID, RegisterID, ByValInfo*);
        MacroAssembler::Call callOperation(V_JITOperation_EJZJ, RegisterID, int32_t, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EJZ, RegisterID, int32_t);
        MacroAssembler::Call callOperation(V_JITOperation_EPc, Instruction*);
        MacroAssembler::Call callOperation(V_JITOperation_EZ, int32_t);
        MacroAssembler::Call callOperation(V_JITOperation_EZJ, int, GPRReg);
        MacroAssembler::Call callOperationWithCallFrameRollbackOnException(J_JITOperation_E);
        MacroAssembler::Call callOperationWithCallFrameRollbackOnException(V_JITOperation_ECb, CodeBlock*);
        MacroAssembler::Call callOperationWithCallFrameRollbackOnException(Z_JITOperation_E);
#if USE(JSVALUE32_64)
        MacroAssembler::Call callOperation(F_JITOperation_EFJZZ, RegisterID, RegisterID, RegisterID, int32_t, RegisterID);
        MacroAssembler::Call callOperation(Z_JITOperation_EJZZ, GPRReg, GPRReg, int32_t, int32_t);
        MacroAssembler::Call callOperation(J_JITOperation_EAapJ, int, ArrayAllocationProfile*, GPRReg, GPRReg);
        MacroAssembler::Call callOperation(J_JITOperation_EJ, int, GPRReg, GPRReg);
        MacroAssembler::Call callOperation(J_JITOperation_EJI, int, GPRReg, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(J_JITOperation_EJJ, int, GPRReg, GPRReg, GPRReg, GPRReg);
        MacroAssembler::Call callOperation(Z_JITOperation_EJOJ, GPRReg, GPRReg, GPRReg, GPRReg, GPRReg);
        MacroAssembler::Call callOperation(J_JITOperation_EJJAp, int, GPRReg, GPRReg, GPRReg, GPRReg, ArrayProfile*);
        MacroAssembler::Call callOperation(J_JITOperation_EJJBy, int, GPRReg, GPRReg, GPRReg, GPRReg, ByValInfo*);
        MacroAssembler::Call callOperation(P_JITOperation_EJS, GPRReg, GPRReg, size_t);
        MacroAssembler::Call callOperation(S_JITOperation_EJ, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(S_JITOperation_EJI, GPRReg, GPRReg, UniquedStringImpl*);
        MacroAssembler::Call callOperation(S_JITOperation_EJJ, RegisterID, RegisterID, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EZSymtabJ, int, SymbolTable*, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EJ, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EJJJ, RegisterID, RegisterID, RegisterID, RegisterID, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EJJJAp, RegisterID, RegisterID, RegisterID, RegisterID, RegisterID, RegisterID, ArrayProfile*);
        MacroAssembler::Call callOperation(V_JITOperation_EJJJBy, RegisterID, RegisterID, RegisterID, RegisterID, RegisterID, RegisterID, ByValInfo*);
        MacroAssembler::Call callOperation(V_JITOperation_EJZ, RegisterID, RegisterID, int32_t);
        MacroAssembler::Call callOperation(V_JITOperation_EJZJ, RegisterID, RegisterID, int32_t, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(V_JITOperation_EZJ, int32_t, RegisterID, RegisterID);
        MacroAssembler::Call callOperation(J_JITOperation_EJscCJ, int, GPRReg, JSCell*, GPRReg, GPRReg);
#endif

        template<typename SnippetGenerator>
        void emitBitBinaryOpFastPath(Instruction* currentInstruction);

        void emitRightShiftFastPath(Instruction* currentInstruction, OpcodeID);

        Jump checkStructure(RegisterID reg, Structure* structure);

        void updateTopCallFrame();

        Call emitNakedCall(CodePtr function = CodePtr());
        Call emitNakedTailCall(CodePtr function = CodePtr());

        // Loads the character value of a single character string into dst.
        void emitLoadCharacterString(RegisterID src, RegisterID dst, JumpList& failures);
        
#if ENABLE(DFG_JIT)
        void emitEnterOptimizationCheck();
#else
        void emitEnterOptimizationCheck() { }
#endif

#ifndef NDEBUG
        void printBytecodeOperandTypes(int src1, int src2);
#endif

#if ENABLE(SAMPLING_FLAGS)
        void setSamplingFlag(int32_t);
        void clearSamplingFlag(int32_t);
#endif

#if ENABLE(SAMPLING_COUNTERS)
        void emitCount(AbstractSamplingCounter&, int32_t = 1);
#endif

#if ENABLE(OPCODE_SAMPLING)
        void sampleInstruction(Instruction*, bool = false);
#endif

#if ENABLE(CODEBLOCK_SAMPLING)
        void sampleCodeBlock(CodeBlock*);
#else
        void sampleCodeBlock(CodeBlock*) {}
#endif

#if ENABLE(DFG_JIT)
        bool canBeOptimized() { return m_canBeOptimized; }
        bool canBeOptimizedOrInlined() { return m_canBeOptimizedOrInlined; }
        bool shouldEmitProfiling() { return m_shouldEmitProfiling; }
#else
        bool canBeOptimized() { return false; }
        bool canBeOptimizedOrInlined() { return false; }
        // Enables use of value profiler with tiered compilation turned off,
        // in which case all code gets profiled.
        bool shouldEmitProfiling() { return false; }
#endif

        static bool reportCompileTimes();
        static bool computeCompileTimes();
        
        // If you need to check the value of an instruction multiple times and the instruction is
        // part of a LLInt inline cache, then you want to use this. It will give you the value of
        // the instruction at the start of JITing.
        Instruction* copiedInstruction(Instruction*);

        Interpreter* m_interpreter;
        
        RefCountedArray<Instruction> m_instructions;

        Vector<CallRecord> m_calls;
        Vector<Label> m_labels;
        Vector<JITGetByIdGenerator> m_getByIds;
        Vector<JITGetByIdWithThisGenerator> m_getByIdsWithThis;
        Vector<JITPutByIdGenerator> m_putByIds;
        Vector<ByValCompilationInfo> m_byValCompilationInfo;
        Vector<CallCompilationInfo> m_callCompilationInfo;
        Vector<JumpTable> m_jmpTable;

        unsigned m_bytecodeOffset;
        Vector<SlowCaseEntry> m_slowCases;
        Vector<SwitchRecord> m_switches;

        JumpList m_exceptionChecks;
        JumpList m_exceptionChecksWithCallFrameRollback;
        Label m_exceptionHandler;

        unsigned m_getByIdIndex;
        unsigned m_getByIdWithThisIndex;
        unsigned m_putByIdIndex;
        unsigned m_byValInstructionIndex;
        unsigned m_callLinkInfoIndex;
        
        Label m_arityCheck;
        std::unique_ptr<LinkBuffer> m_linkBuffer;

        std::unique_ptr<JITDisassembler> m_disassembler;
        RefPtr<Profiler::Compilation> m_compilation;
        WeakRandom m_randomGenerator;
        static CodeRef stringGetByValStubGenerator(VM*);

        PCToCodeOriginMapBuilder m_pcToCodeOriginMapBuilder;

        HashMap<Instruction*, void*> m_instructionToMathIC;
        HashMap<Instruction*, MathICGenerationState> m_instructionToMathICGenerationState;

        bool m_canBeOptimized;
        bool m_canBeOptimizedOrInlined;
        bool m_shouldEmitProfiling;
        unsigned m_loopOSREntryBytecodeOffset { 0 };
    } JIT_CLASS_ALIGNMENT;

} // namespace JSC

#endif // ENABLE(JIT)
