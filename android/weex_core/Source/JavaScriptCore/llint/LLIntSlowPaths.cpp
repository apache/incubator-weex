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
#include "LLIntSlowPaths.h"

#include "ArrayConstructor.h"
#include "CallFrame.h"
#include "CommonSlowPaths.h"
#include "CommonSlowPathsExceptions.h"
#include "Error.h"
#include "ErrorHandlingScope.h"
#include "EvalCodeBlock.h"
#include "Exception.h"
#include "ExceptionFuzz.h"
#include "FunctionCodeBlock.h"
#include "FunctionWhitelist.h"
#include "GetterSetter.h"
#include "HostCallReturnValue.h"
#include "Interpreter.h"
#include "IteratorOperations.h"
#include "JIT.h"
#include "JITExceptions.h"
#include "JITWorklist.h"
#include "JSAsyncFunction.h"
#include "JSCInlines.h"
#include "JSCJSValue.h"
#include "JSFixedArray.h"
#include "JSGeneratorFunction.h"
#include "JSGlobalObjectFunctions.h"
#include "JSLexicalEnvironment.h"
#include "JSString.h"
#include "JSWithScope.h"
#include "LLIntCommon.h"
#include "LLIntData.h"
#include "LLIntExceptions.h"
#include "LowLevelInterpreter.h"
#include "ModuleProgramCodeBlock.h"
#include "ObjectConstructor.h"
#include "ObjectPropertyConditionSet.h"
#include "ProgramCodeBlock.h"
#include "ProtoCallFrame.h"
#include "RegExpObject.h"
#include "ShadowChicken.h"
#include "StructureRareDataInlines.h"
#include "VMInlines.h"
#include <wtf/NeverDestroyed.h>
#include <wtf/StringPrintStream.h>

namespace JSC { namespace LLInt {

#define LLINT_BEGIN_NO_SET_PC() \
    VM& vm = exec->vm();      \
    NativeCallFrameTracer tracer(&vm, exec); \
    auto throwScope = DECLARE_THROW_SCOPE(vm)

#ifndef NDEBUG
#define LLINT_SET_PC_FOR_STUBS() do { \
        exec->codeBlock()->bytecodeOffset(pc); \
        exec->setCurrentVPC(pc); \
    } while (false)
#else
#define LLINT_SET_PC_FOR_STUBS() do { \
        exec->setCurrentVPC(pc); \
    } while (false)
#endif

#define LLINT_BEGIN()                           \
    LLINT_BEGIN_NO_SET_PC();                    \
    LLINT_SET_PC_FOR_STUBS()

#define LLINT_OP(index) (exec->uncheckedR(pc[index].u.operand))
#define LLINT_OP_C(index) (exec->r(pc[index].u.operand))

#define LLINT_RETURN_TWO(first, second) do {       \
        return encodeResult(first, second);        \
    } while (false)

#define LLINT_END_IMPL() LLINT_RETURN_TWO(pc, 0)

#define LLINT_THROW(exceptionToThrow) do {                        \
        throwException(exec, throwScope, exceptionToThrow);       \
        pc = returnToThrow(exec);                                 \
        LLINT_END_IMPL();                                         \
    } while (false)

#define LLINT_CHECK_EXCEPTION() do {                    \
        doExceptionFuzzingIfEnabled(exec, throwScope, "LLIntSlowPaths", pc);    \
        if (UNLIKELY(throwScope.exception())) {         \
            pc = returnToThrow(exec);                   \
            LLINT_END_IMPL();                           \
        }                                               \
    } while (false)

#define LLINT_END() do {                        \
        LLINT_CHECK_EXCEPTION();                \
        LLINT_END_IMPL();                       \
    } while (false)

#define LLINT_BRANCH(opcode, condition) do {                      \
        bool __b_condition = (condition);                         \
        LLINT_CHECK_EXCEPTION();                                  \
        if (__b_condition)                                        \
            pc += pc[OPCODE_LENGTH(opcode) - 1].u.operand;        \
        else                                                      \
            pc += OPCODE_LENGTH(opcode);                          \
        LLINT_END_IMPL();                                         \
    } while (false)

#define LLINT_RETURN(value) do {                \
        JSValue __r_returnValue = (value);      \
        LLINT_CHECK_EXCEPTION();                \
        LLINT_OP(1) = __r_returnValue;          \
        LLINT_END_IMPL();                       \
    } while (false)

#define LLINT_RETURN_WITH_PC_ADJUSTMENT(value, pcAdjustment) do { \
        JSValue __r_returnValue = (value);      \
        LLINT_CHECK_EXCEPTION();                \
        LLINT_OP(1) = __r_returnValue;          \
        pc += (pcAdjustment);                   \
        LLINT_END_IMPL();                       \
    } while (false)

#define LLINT_RETURN_PROFILED(opcode, value) do {               \
        JSValue __rp_returnValue = (value);                     \
        LLINT_CHECK_EXCEPTION();                                \
        LLINT_OP(1) = __rp_returnValue;                         \
        LLINT_PROFILE_VALUE(opcode, __rp_returnValue);          \
        LLINT_END_IMPL();                                       \
    } while (false)

#if 1
#define LLINT_PROFILE_VALUE(opcode, value) do { \
    __atomic_store_n(&pc[OPCODE_LENGTH(opcode) - 1].u.profile->m_buckets[0], \
        JSValue::encode(value), __ATOMIC_SEQ_CST); \
    } while (false)
#else
#define LLINT_PROFILE_VALUE(opcode, value) do { \
        pc[OPCODE_LENGTH(opcode) - 1].u.profile->m_buckets[0] = \
        JSValue::encode(value);                  \
    } while (false)
#endif // WTF_ARM_ARCH_VERSION == 7

#define LLINT_CALL_END_IMPL(exec, callTarget) LLINT_RETURN_TWO((callTarget), (exec))

#define LLINT_CALL_THROW(exec, exceptionToThrow) do {                   \
        ExecState* __ct_exec = (exec);                                  \
        throwException(__ct_exec, throwScope, exceptionToThrow);        \
        LLINT_CALL_END_IMPL(0, callToThrow(__ct_exec));                 \
    } while (false)

#define LLINT_CALL_CHECK_EXCEPTION(exec, execCallee) do {               \
        ExecState* __cce_exec = (exec);                                 \
        ExecState* __cce_execCallee = (execCallee);                     \
        doExceptionFuzzingIfEnabled(__cce_exec, throwScope, "LLIntSlowPaths/call", nullptr); \
        if (UNLIKELY(throwScope.exception()))                           \
            LLINT_CALL_END_IMPL(0, callToThrow(__cce_execCallee));      \
    } while (false)

#define LLINT_CALL_RETURN(exec, execCallee, callTarget) do {            \
        ExecState* __cr_exec = (exec);                                  \
        ExecState* __cr_execCallee = (execCallee);                      \
        void* __cr_callTarget = (callTarget);                           \
        LLINT_CALL_CHECK_EXCEPTION(__cr_exec, __cr_execCallee);         \
        LLINT_CALL_END_IMPL(__cr_execCallee, __cr_callTarget);          \
    } while (false)

#define LLINT_RETURN_CALLEE_FRAME(execCallee) do {                      \
        ExecState* __rcf_exec = (execCallee);                           \
        LLINT_RETURN_TWO(pc, __rcf_exec);                               \
    } while (false)
    
extern "C" SlowPathReturnType llint_trace_operand(ExecState* exec, Instruction* pc, int fromWhere, int operand)
{
    LLINT_BEGIN();
    dataLogF("%p / %p: executing bc#%zu, op#%u: Trace(%d): %d: %d\n",
            exec->codeBlock(),
            exec,
            static_cast<intptr_t>(pc - exec->codeBlock()->instructions().begin()),
            exec->vm().interpreter->getOpcodeID(pc[0].u.opcode),
            fromWhere,
            operand,
            pc[operand].u.operand);
    LLINT_END();
}

extern "C" SlowPathReturnType llint_trace_value(ExecState* exec, Instruction* pc, int fromWhere, int operand)
{
    JSValue value = LLINT_OP_C(operand).jsValue();
    union {
        struct {
            uint32_t tag;
            uint32_t payload;
        } bits;
        EncodedJSValue asValue;
    } u;
    u.asValue = JSValue::encode(value);
    dataLogF(
        "%p / %p: executing bc#%zu, op#%u: Trace(%d): %d: %d: %08x:%08x: %s\n",
        exec->codeBlock(),
        exec,
        static_cast<intptr_t>(pc - exec->codeBlock()->instructions().begin()),
        exec->vm().interpreter->getOpcodeID(pc[0].u.opcode),
        fromWhere,
        operand,
        pc[operand].u.operand,
        u.bits.tag,
        u.bits.payload,
        toCString(value).data());
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(trace_prologue)
{
    dataLogF("%p / %p: in prologue of ", exec->codeBlock(), exec);
    dataLog(*exec->codeBlock(), "\n");
    LLINT_END_IMPL();
}

static void traceFunctionPrologue(ExecState* exec, const char* comment, CodeSpecializationKind kind)
{
    JSFunction* callee = jsCast<JSFunction*>(exec->jsCallee());
    FunctionExecutable* executable = callee->jsExecutable();
    CodeBlock* codeBlock = executable->codeBlockFor(kind);
    dataLogF("%p / %p: in %s of ", codeBlock, exec, comment);
    dataLog(*codeBlock);
    dataLogF(" function %p, executable %p; numVars = %u, numParameters = %u, numCalleeLocals = %u, caller = %p.\n",
        callee, executable, codeBlock->m_numVars, codeBlock->numParameters(), codeBlock->m_numCalleeLocals, exec->callerFrame());
}

LLINT_SLOW_PATH_DECL(trace_prologue_function_for_call)
{
    traceFunctionPrologue(exec, "call prologue", CodeForCall);
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(trace_prologue_function_for_construct)
{
    traceFunctionPrologue(exec, "construct prologue", CodeForConstruct);
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(trace_arityCheck_for_call)
{
    traceFunctionPrologue(exec, "call arity check", CodeForCall);
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(trace_arityCheck_for_construct)
{
    traceFunctionPrologue(exec, "construct arity check", CodeForConstruct);
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(trace)
{
    dataLogF("%p / %p: executing bc#%zu, %s, pc = %p\n",
            exec->codeBlock(),
            exec,
            static_cast<intptr_t>(pc - exec->codeBlock()->instructions().begin()),
            opcodeNames[exec->vm().interpreter->getOpcodeID(pc[0].u.opcode)], pc);
    if (exec->vm().interpreter->getOpcodeID(pc[0].u.opcode) == op_enter) {
        dataLogF("Frame will eventually return to %p\n", exec->returnPC().value());
        *bitwise_cast<volatile char*>(exec->returnPC().value());
    }
    if (exec->vm().interpreter->getOpcodeID(pc[0].u.opcode) == op_ret) {
        dataLogF("Will be returning to %p\n", exec->returnPC().value());
        dataLogF("The new cfr will be %p\n", exec->callerFrame());
    }
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(special_trace)
{
    dataLogF("%p / %p: executing special case bc#%zu, op#%u, return PC is %p\n",
            exec->codeBlock(),
            exec,
            static_cast<intptr_t>(pc - exec->codeBlock()->instructions().begin()),
            exec->vm().interpreter->getOpcodeID(pc[0].u.opcode),
            exec->returnPC().value());
    LLINT_END_IMPL();
}

enum EntryKind { Prologue, ArityCheck };

#if ENABLE(JIT)
static FunctionWhitelist& ensureGlobalJITWhitelist()
{
    static LazyNeverDestroyed<FunctionWhitelist> baselineWhitelist;
    static std::once_flag initializeWhitelistFlag;
    std::call_once(initializeWhitelistFlag, [] {
        const char* functionWhitelistFile = Options::jitWhitelist();
        baselineWhitelist.construct(functionWhitelistFile);
    });
    return baselineWhitelist;
}

inline bool shouldJIT(ExecState* exec, CodeBlock* codeBlock)
{
    if (!Options::bytecodeRangeToJITCompile().isInRange(codeBlock->instructionCount())
        || !ensureGlobalJITWhitelist().contains(codeBlock))
        return false;

    // You can modify this to turn off JITting without rebuilding the world.
    return exec->vm().canUseJIT();
}

// Returns true if we should try to OSR.
inline bool jitCompileAndSetHeuristics(CodeBlock* codeBlock, ExecState* exec, unsigned loopOSREntryBytecodeOffset = 0)
{
    VM& vm = exec->vm();
    DeferGCForAWhile deferGC(vm.heap); // My callers don't set top callframe, so we don't want to GC here at all.
    
    codeBlock->updateAllValueProfilePredictions();

    if (!codeBlock->checkIfJITThresholdReached()) {
        CODEBLOCK_LOG_EVENT(codeBlock, "delayJITCompile", ("threshold not reached, counter = ", codeBlock->llintExecuteCounter()));
        if (Options::verboseOSR())
            dataLogF("    JIT threshold should be lifted.\n");
        return false;
    }
    
    JITWorklist::instance()->poll(vm);
    
    switch (codeBlock->jitType()) {
    case JITCode::BaselineJIT: {
        if (Options::verboseOSR())
            dataLogF("    Code was already compiled.\n");
        codeBlock->jitSoon();
        return true;
    }
    case JITCode::InterpreterThunk: {
        JITWorklist::instance()->compileLater(codeBlock, loopOSREntryBytecodeOffset);
        return codeBlock->jitType() == JITCode::BaselineJIT;
    }
    default:
        dataLog("Unexpected code block in LLInt: ", *codeBlock, "\n");
        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
}

static SlowPathReturnType entryOSR(ExecState* exec, Instruction*, CodeBlock* codeBlock, const char *name, EntryKind kind)
{
    if (Options::verboseOSR()) {
        dataLog(
            *codeBlock, ": Entered ", name, " with executeCounter = ",
            codeBlock->llintExecuteCounter(), "\n");
    }
    
    if (!shouldJIT(exec, codeBlock)) {
        codeBlock->dontJITAnytimeSoon();
        LLINT_RETURN_TWO(0, 0);
    }
    if (!jitCompileAndSetHeuristics(codeBlock, exec))
        LLINT_RETURN_TWO(0, 0);
    
    CODEBLOCK_LOG_EVENT(codeBlock, "OSR entry", ("in prologue"));
    
    if (kind == Prologue)
        LLINT_RETURN_TWO(codeBlock->jitCode()->executableAddress(), 0);
    ASSERT(kind == ArityCheck);
    LLINT_RETURN_TWO(codeBlock->jitCode()->addressForCall(MustCheckArity).executableAddress(), 0);
}
#else // ENABLE(JIT)
static SlowPathReturnType entryOSR(ExecState* exec, Instruction*, CodeBlock* codeBlock, const char*, EntryKind)
{
    codeBlock->dontJITAnytimeSoon();
    LLINT_RETURN_TWO(0, exec);
}
#endif // ENABLE(JIT)

LLINT_SLOW_PATH_DECL(entry_osr)
{
    return entryOSR(exec, pc, exec->codeBlock(), "entry_osr", Prologue);
}

LLINT_SLOW_PATH_DECL(entry_osr_function_for_call)
{
    return entryOSR(exec, pc, jsCast<JSFunction*>(exec->jsCallee())->jsExecutable()->codeBlockForCall(), "entry_osr_function_for_call", Prologue);
}

LLINT_SLOW_PATH_DECL(entry_osr_function_for_construct)
{
    return entryOSR(exec, pc, jsCast<JSFunction*>(exec->jsCallee())->jsExecutable()->codeBlockForConstruct(), "entry_osr_function_for_construct", Prologue);
}

LLINT_SLOW_PATH_DECL(entry_osr_function_for_call_arityCheck)
{
    return entryOSR(exec, pc, jsCast<JSFunction*>(exec->jsCallee())->jsExecutable()->codeBlockForCall(), "entry_osr_function_for_call_arityCheck", ArityCheck);
}

LLINT_SLOW_PATH_DECL(entry_osr_function_for_construct_arityCheck)
{
    return entryOSR(exec, pc, jsCast<JSFunction*>(exec->jsCallee())->jsExecutable()->codeBlockForConstruct(), "entry_osr_function_for_construct_arityCheck", ArityCheck);
}

LLINT_SLOW_PATH_DECL(loop_osr)
{
    CodeBlock* codeBlock = exec->codeBlock();

#if ENABLE(JIT)
    if (Options::verboseOSR()) {
        dataLog(
            *codeBlock, ": Entered loop_osr with executeCounter = ",
            codeBlock->llintExecuteCounter(), "\n");
    }
    
    unsigned loopOSREntryBytecodeOffset = pc - codeBlock->instructions().begin();

    if (!shouldJIT(exec, codeBlock)) {
        codeBlock->dontJITAnytimeSoon();
        LLINT_RETURN_TWO(0, 0);
    }
    
    if (!jitCompileAndSetHeuristics(codeBlock, exec, loopOSREntryBytecodeOffset))
        LLINT_RETURN_TWO(0, 0);
    
    CODEBLOCK_LOG_EVENT(codeBlock, "osrEntry", ("at bc#", pc - codeBlock->instructions().begin()));

    ASSERT(codeBlock->jitType() == JITCode::BaselineJIT);
    
    Vector<BytecodeAndMachineOffset> map;
    codeBlock->jitCodeMap()->decode(map);
    BytecodeAndMachineOffset* mapping = binarySearch<BytecodeAndMachineOffset, unsigned>(map, map.size(), pc - codeBlock->instructions().begin(), BytecodeAndMachineOffset::getBytecodeIndex);
    ASSERT(mapping);
    ASSERT(mapping->m_bytecodeIndex == static_cast<unsigned>(pc - codeBlock->instructions().begin()));
    
    void* jumpTarget = codeBlock->jitCode()->executableAddressAtOffset(mapping->m_machineCodeOffset);
    ASSERT(jumpTarget);
    
    LLINT_RETURN_TWO(jumpTarget, exec->topOfFrame());
#else // ENABLE(JIT)
    UNUSED_PARAM(pc);
    codeBlock->dontJITAnytimeSoon();
    LLINT_RETURN_TWO(0, 0);
#endif // ENABLE(JIT)
}

LLINT_SLOW_PATH_DECL(replace)
{
    CodeBlock* codeBlock = exec->codeBlock();

#if ENABLE(JIT)
    if (Options::verboseOSR()) {
        dataLog(
            *codeBlock, ": Entered replace with executeCounter = ",
            codeBlock->llintExecuteCounter(), "\n");
    }
    
    if (shouldJIT(exec, codeBlock))
        jitCompileAndSetHeuristics(codeBlock, exec);
    else
        codeBlock->dontJITAnytimeSoon();
    LLINT_END_IMPL();
#else // ENABLE(JIT)
    codeBlock->dontJITAnytimeSoon();
    LLINT_END_IMPL();
#endif // ENABLE(JIT)
}

LLINT_SLOW_PATH_DECL(stack_check)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    VMEntryFrame* vmEntryFrame = vm.topVMEntryFrame;
    CallFrame* callerFrame = exec->callerFrame(vmEntryFrame);
    if (!callerFrame) {
        callerFrame = exec;
        vmEntryFrame = vm.topVMEntryFrame;
    }
    NativeCallFrameTracerWithRestore tracer(&vm, vmEntryFrame, callerFrame);

    LLINT_SET_PC_FOR_STUBS();

#if LLINT_SLOW_PATH_TRACING
    dataLogF("Checking stack height with exec = %p.\n", exec);
    dataLog("CodeBlock = ", *exec->codeBlock(), "\n");
    dataLogF("Num callee registers = %u.\n", exec->codeBlock()->m_numCalleeLocals);
    dataLogF("Num vars = %u.\n", exec->codeBlock()->m_numVars);

    dataLogF("Current OS stack end is at %p.\n", vm.softStackLimit());
#if !ENABLE(JIT)
    dataLogF("Current C Loop stack end is at %p.\n", vm.cloopStackLimit());
#endif

#endif
    // If the stack check succeeds and we don't need to throw the error, then
    // we'll return 0 instead. The prologue will check for a non-zero value
    // when determining whether to set the callFrame or not.

    // For JIT enabled builds which uses the C stack, the stack is not growable.
    // Hence, if we get here, then we know a stack overflow is imminent. So, just
    // throw the StackOverflowError unconditionally.
#if !ENABLE(JIT)
    ASSERT(!vm.interpreter->cloopStack().containsAddress(exec->topOfFrame()));
    if (LIKELY(vm.ensureStackCapacityFor(exec->topOfFrame())))
        LLINT_RETURN_TWO(pc, 0);
#endif

    ErrorHandlingScope errorScope(vm);
    throwStackOverflowError(callerFrame, throwScope);
    pc = returnToThrow(callerFrame);
    LLINT_RETURN_TWO(pc, exec);
}

LLINT_SLOW_PATH_DECL(slow_path_new_object)
{
    LLINT_BEGIN();
    LLINT_RETURN(constructEmptyObject(exec, pc[3].u.objectAllocationProfile->structure()));
}

LLINT_SLOW_PATH_DECL(slow_path_new_array)
{
    LLINT_BEGIN();
    LLINT_RETURN(constructArrayNegativeIndexed(exec, pc[4].u.arrayAllocationProfile, bitwise_cast<JSValue*>(&LLINT_OP(2)), pc[3].u.operand));
}

LLINT_SLOW_PATH_DECL(slow_path_new_array_with_size)
{
    LLINT_BEGIN();
    LLINT_RETURN(constructArrayWithSizeQuirk(exec, pc[3].u.arrayAllocationProfile, exec->lexicalGlobalObject(), LLINT_OP_C(2).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_new_array_buffer)
{
    LLINT_BEGIN();
    LLINT_RETURN(constructArray(exec, pc[4].u.arrayAllocationProfile, exec->codeBlock()->constantBuffer(pc[2].u.operand), pc[3].u.operand));
}

LLINT_SLOW_PATH_DECL(slow_path_new_regexp)
{
    LLINT_BEGIN();
    RegExp* regExp = exec->codeBlock()->regexp(pc[2].u.operand);
    if (!regExp->isValid())
        LLINT_THROW(createSyntaxError(exec, regExp->errorMessage()));
    LLINT_RETURN(RegExpObject::create(vm, exec->lexicalGlobalObject()->regExpStructure(), regExp));
}

LLINT_SLOW_PATH_DECL(slow_path_instanceof)
{
    LLINT_BEGIN();
    JSValue value = LLINT_OP_C(2).jsValue();
    JSValue proto = LLINT_OP_C(3).jsValue();
    LLINT_RETURN(jsBoolean(JSObject::defaultHasInstance(exec, value, proto)));
}

LLINT_SLOW_PATH_DECL(slow_path_instanceof_custom)
{
    LLINT_BEGIN();

    JSValue value = LLINT_OP_C(2).jsValue();
    JSValue constructor = LLINT_OP_C(3).jsValue();
    JSValue hasInstanceValue = LLINT_OP_C(4).jsValue();

    ASSERT(constructor.isObject());
    ASSERT(hasInstanceValue != exec->lexicalGlobalObject()->functionProtoHasInstanceSymbolFunction() || !constructor.getObject()->structure()->typeInfo().implementsDefaultHasInstance());

    JSValue result = jsBoolean(constructor.getObject()->hasInstance(exec, value, hasInstanceValue));
    LLINT_RETURN(result);
}

LLINT_SLOW_PATH_DECL(slow_path_try_get_by_id)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    const Identifier& ident = codeBlock->identifier(pc[3].u.operand);
    JSValue baseValue = LLINT_OP_C(2).jsValue();
    PropertySlot slot(baseValue, PropertySlot::PropertySlot::InternalMethodType::VMInquiry);

    baseValue.getPropertySlot(exec, ident, slot);
    JSValue result = slot.getPureResult();

    LLINT_RETURN_PROFILED(op_try_get_by_id, result);
}

static void setupGetByIdPrototypeCache(ExecState* exec, VM& vm, Instruction* pc, JSCell* baseCell, PropertySlot& slot, const Identifier& ident)
{
    CodeBlock* codeBlock = exec->codeBlock();
    Structure* structure = baseCell->structure();

    if (structure->typeInfo().prohibitsPropertyCaching())
        return;
    
    if (structure->needImpurePropertyWatchpoint())
        return;

    if (structure->isDictionary()) {
        if (structure->hasBeenFlattenedBefore())
            return;
        structure->flattenDictionaryStructure(vm, jsCast<JSObject*>(baseCell));
    }

    ObjectPropertyConditionSet conditions;
    if (slot.isUnset())
        conditions = generateConditionsForPropertyMiss(vm, codeBlock, exec, structure, ident.impl());
    else
        conditions = generateConditionsForPrototypePropertyHit(vm, codeBlock, exec, structure, slot.slotBase(), ident.impl());

    if (!conditions.isValid())
        return;

    PropertyOffset offset = invalidOffset;
    CodeBlock::StructureWatchpointMap& watchpointMap = codeBlock->llintGetByIdWatchpointMap();
    auto result = watchpointMap.add(structure, Bag<LLIntPrototypeLoadAdaptiveStructureWatchpoint>());
    for (ObjectPropertyCondition condition : conditions) {
        if (!condition.isWatchable())
            return;
        if (condition.condition().kind() == PropertyCondition::Presence)
            offset = condition.condition().offset();
        result.iterator->value.add(condition, pc)->install();
    }
    ASSERT((offset == invalidOffset) == slot.isUnset());

    ConcurrentJSLocker locker(codeBlock->m_lock);

    if (slot.isUnset()) {
        pc[0].u.opcode = LLInt::getOpcode(op_get_by_id_unset);
        pc[4].u.structureID = structure->id();
        return;
    }
    ASSERT(slot.isValue());

    pc[0].u.opcode = LLInt::getOpcode(op_get_by_id_proto_load);
    pc[4].u.structureID = structure->id();
    pc[5].u.operand = offset;
    // We know that this pointer will remain valid because it will be cleared by either a watchpoint fire or
    // during GC when we clear the LLInt caches.
    pc[6].u.pointer = slot.slotBase();
}


LLINT_SLOW_PATH_DECL(slow_path_get_by_id)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    const Identifier& ident = codeBlock->identifier(pc[3].u.operand);
    JSValue baseValue = LLINT_OP_C(2).jsValue();
    PropertySlot slot(baseValue, PropertySlot::PropertySlot::InternalMethodType::Get);

    JSValue result = baseValue.get(exec, ident, slot);
    LLINT_CHECK_EXCEPTION();
    LLINT_OP(1) = result;
    
    if (!LLINT_ALWAYS_ACCESS_SLOW
        && baseValue.isCell()
        && slot.isCacheable()) {

        JSCell* baseCell = baseValue.asCell();
        Structure* structure = baseCell->structure();
        if (slot.isValue() && slot.slotBase() == baseValue) {
            // Start out by clearing out the old cache.
            pc[0].u.opcode = LLInt::getOpcode(op_get_by_id);
            pc[4].u.pointer = nullptr; // old structure
            pc[5].u.pointer = nullptr; // offset

            // Prevent the prototype cache from ever happening.
            pc[7].u.operand = 0;
        
            if (structure->propertyAccessesAreCacheable()) {
                vm.heap.writeBarrier(codeBlock);
                
                ConcurrentJSLocker locker(codeBlock->m_lock);

                pc[4].u.structureID = structure->id();
                pc[5].u.operand = slot.cachedOffset();
            }
        } else if (UNLIKELY(pc[7].u.operand && (slot.isValue() || slot.isUnset()))) {
            ASSERT(slot.slotBase() != baseValue);

            if (!(--pc[7].u.operand))
                setupGetByIdPrototypeCache(exec, vm, pc, baseCell, slot, ident);
        }
    } else if (!LLINT_ALWAYS_ACCESS_SLOW
        && isJSArray(baseValue)
        && ident == exec->propertyNames().length) {
        pc[0].u.opcode = LLInt::getOpcode(op_get_array_length);
        ArrayProfile* arrayProfile = codeBlock->getOrAddArrayProfile(pc - codeBlock->instructions().begin());
        arrayProfile->observeStructure(baseValue.asCell()->structure());
        pc[4].u.arrayProfile = arrayProfile;

        // Prevent the prototype cache from ever happening.
        pc[7].u.operand = 0;
    }

#if 1
    __atomic_store_n(&pc[OPCODE_LENGTH(op_get_by_id) - 1].u.profile->m_buckets[0],
        JSValue::encode(result), __ATOMIC_SEQ_CST);
#else
    pc[OPCODE_LENGTH(op_get_by_id) - 1].u.profile->m_buckets[0] = JSValue::encode(result);
#endif // WTF_ARM_ARCH_VERSION == 7
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_get_arguments_length)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    const Identifier& ident = codeBlock->identifier(pc[3].u.operand);
    JSValue baseValue = LLINT_OP(2).jsValue();
    PropertySlot slot(baseValue, PropertySlot::InternalMethodType::Get);
    LLINT_RETURN(baseValue.get(exec, ident, slot));
}

LLINT_SLOW_PATH_DECL(slow_path_put_by_id)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    const Identifier& ident = codeBlock->identifier(pc[2].u.operand);
    
    JSValue baseValue = LLINT_OP_C(1).jsValue();
    PutPropertySlot slot(baseValue, codeBlock->isStrictMode(), codeBlock->putByIdContext());
    if (pc[8].u.putByIdFlags & PutByIdIsDirect)
        asObject(baseValue)->putDirect(vm, ident, LLINT_OP_C(3).jsValue(), slot);
    else
        baseValue.putInline(exec, ident, LLINT_OP_C(3).jsValue(), slot);
    LLINT_CHECK_EXCEPTION();
    
    if (!LLINT_ALWAYS_ACCESS_SLOW
        && baseValue.isCell()
        && slot.isCacheablePut()) {

        // Start out by clearing out the old cache.
        pc[4].u.pointer = nullptr; // old structure
        pc[5].u.pointer = nullptr; // offset
        pc[6].u.pointer = nullptr; // new structure
        pc[7].u.pointer = nullptr; // structure chain
        pc[8].u.putByIdFlags =
            static_cast<PutByIdFlags>(pc[8].u.putByIdFlags & PutByIdPersistentFlagsMask);
        
        JSCell* baseCell = baseValue.asCell();
        Structure* structure = baseCell->structure();
        
        if (!structure->isUncacheableDictionary()
            && !structure->typeInfo().prohibitsPropertyCaching()
            && baseCell == slot.base()) {

            vm.heap.writeBarrier(codeBlock);
            
            if (slot.type() == PutPropertySlot::NewProperty) {
                GCSafeConcurrentJSLocker locker(codeBlock->m_lock, vm.heap);
            
                if (!structure->isDictionary() && structure->previousID()->outOfLineCapacity() == structure->outOfLineCapacity()) {
                    ASSERT(structure->previousID()->transitionWatchpointSetHasBeenInvalidated());

                    if (normalizePrototypeChain(exec, structure) != InvalidPrototypeChain) {
                        ASSERT(structure->previousID()->isObject());
                        pc[4].u.structureID = structure->previousID()->id();
                        pc[5].u.operand = slot.cachedOffset();
                        pc[6].u.structureID = structure->id();
                        if (!(pc[8].u.putByIdFlags & PutByIdIsDirect)) {
                            StructureChain* chain = structure->prototypeChain(exec);
                            ASSERT(chain);
                            pc[7].u.structureChain.set(
                                vm, codeBlock, chain);
                        }
                        pc[8].u.putByIdFlags = static_cast<PutByIdFlags>(
                            pc[8].u.putByIdFlags |
                            structure->inferredTypeDescriptorFor(ident.impl()).putByIdFlags());
                    }
                }
            } else {
                structure->didCachePropertyReplacement(vm, slot.cachedOffset());
                pc[4].u.structureID = structure->id();
                pc[5].u.operand = slot.cachedOffset();
                pc[8].u.putByIdFlags = static_cast<PutByIdFlags>(
                    pc[8].u.putByIdFlags |
                    structure->inferredTypeDescriptorFor(ident.impl()).putByIdFlags());
            }
        }
    }
    
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_del_by_id)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    JSObject* baseObject = LLINT_OP_C(2).jsValue().toObject(exec);
    LLINT_CHECK_EXCEPTION();
    bool couldDelete = baseObject->methodTable()->deleteProperty(baseObject, exec, codeBlock->identifier(pc[3].u.operand));
    LLINT_CHECK_EXCEPTION();
    if (!couldDelete && codeBlock->isStrictMode())
        LLINT_THROW(createTypeError(exec, UnableToDeletePropertyError));
    LLINT_RETURN(jsBoolean(couldDelete));
}

static ALWAYS_INLINE JSValue getByVal(VM& vm, ExecState* exec, JSValue baseValue, JSValue subscript)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (LIKELY(baseValue.isCell() && subscript.isString())) {
        Structure& structure = *baseValue.asCell()->structure(vm);
        if (JSCell::canUseFastGetOwnProperty(structure)) {
            if (RefPtr<AtomicStringImpl> existingAtomicString = asString(subscript)->toExistingAtomicString(exec)) {
                if (JSValue result = baseValue.asCell()->fastGetOwnProperty(vm, structure, existingAtomicString.get()))
                    return result;
            }
        }
    }
    
    if (subscript.isUInt32()) {
        uint32_t i = subscript.asUInt32();
        if (isJSString(baseValue) && asString(baseValue)->canGetIndex(i))
            return asString(baseValue)->getIndex(exec, i);
        scope.release();
        return baseValue.get(exec, i);
    }

    baseValue.requireObjectCoercible(exec);
    RETURN_IF_EXCEPTION(scope, JSValue());
    auto property = subscript.toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, JSValue());
    scope.release();
    return baseValue.get(exec, property);
}

LLINT_SLOW_PATH_DECL(slow_path_get_by_val)
{
    LLINT_BEGIN();
    LLINT_RETURN_PROFILED(op_get_by_val, getByVal(vm, exec, LLINT_OP_C(2).jsValue(), LLINT_OP_C(3).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_put_by_val)
{
    LLINT_BEGIN();
    
    JSValue baseValue = LLINT_OP_C(1).jsValue();
    JSValue subscript = LLINT_OP_C(2).jsValue();
    JSValue value = LLINT_OP_C(3).jsValue();
    bool isStrictMode = exec->codeBlock()->isStrictMode();
    
    if (LIKELY(subscript.isUInt32())) {
        uint32_t i = subscript.asUInt32();
        if (baseValue.isObject()) {
            JSObject* object = asObject(baseValue);
            if (object->canSetIndexQuickly(i))
                object->setIndexQuickly(vm, i, value);
            else
                object->methodTable()->putByIndex(object, exec, i, value, isStrictMode);
            LLINT_END();
        }
        baseValue.putByIndex(exec, i, value, isStrictMode);
        LLINT_END();
    }

    auto property = subscript.toPropertyKey(exec);
    LLINT_CHECK_EXCEPTION();
    PutPropertySlot slot(baseValue, isStrictMode);
    baseValue.put(exec, property, value, slot);
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_put_by_val_direct)
{
    LLINT_BEGIN();
    
    JSValue baseValue = LLINT_OP_C(1).jsValue();
    JSValue subscript = LLINT_OP_C(2).jsValue();
    JSValue value = LLINT_OP_C(3).jsValue();
    RELEASE_ASSERT(baseValue.isObject());
    JSObject* baseObject = asObject(baseValue);
    bool isStrictMode = exec->codeBlock()->isStrictMode();
    if (LIKELY(subscript.isUInt32())) {
        // Despite its name, JSValue::isUInt32 will return true only for positive boxed int32_t; all those values are valid array indices.
        ASSERT(isIndex(subscript.asUInt32()));
        baseObject->putDirectIndex(exec, subscript.asUInt32(), value, 0, isStrictMode ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
        LLINT_END();
    }

    if (subscript.isDouble()) {
        double subscriptAsDouble = subscript.asDouble();
        uint32_t subscriptAsUInt32 = static_cast<uint32_t>(subscriptAsDouble);
        if (subscriptAsDouble == subscriptAsUInt32 && isIndex(subscriptAsUInt32)) {
            baseObject->putDirectIndex(exec, subscriptAsUInt32, value, 0, isStrictMode ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
            LLINT_END();
        }
    }

    // Don't put to an object if toString threw an exception.
    auto property = subscript.toPropertyKey(exec);
    if (UNLIKELY(throwScope.exception()))
        LLINT_END();

    if (std::optional<uint32_t> index = parseIndex(property))
        baseObject->putDirectIndex(exec, index.value(), value, 0, isStrictMode ? PutDirectIndexShouldThrow : PutDirectIndexShouldNotThrow);
    else {
        PutPropertySlot slot(baseObject, isStrictMode);
        baseObject->putDirect(exec->vm(), property, value, slot);
    }
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_del_by_val)
{
    LLINT_BEGIN();
    JSValue baseValue = LLINT_OP_C(2).jsValue();
    JSObject* baseObject = baseValue.toObject(exec);
    LLINT_CHECK_EXCEPTION();

    JSValue subscript = LLINT_OP_C(3).jsValue();
    
    bool couldDelete;
    
    uint32_t i;
    if (subscript.getUInt32(i))
        couldDelete = baseObject->methodTable()->deletePropertyByIndex(baseObject, exec, i);
    else {
        LLINT_CHECK_EXCEPTION();
        auto property = subscript.toPropertyKey(exec);
        LLINT_CHECK_EXCEPTION();
        couldDelete = baseObject->methodTable()->deleteProperty(baseObject, exec, property);
    }
    
    if (!couldDelete && exec->codeBlock()->isStrictMode())
        LLINT_THROW(createTypeError(exec, UnableToDeletePropertyError));
    
    LLINT_RETURN(jsBoolean(couldDelete));
}

LLINT_SLOW_PATH_DECL(slow_path_put_by_index)
{
    LLINT_BEGIN();
    JSValue arrayValue = LLINT_OP_C(1).jsValue();
    ASSERT(isJSArray(arrayValue));
    asArray(arrayValue)->putDirectIndex(exec, pc[2].u.operand, LLINT_OP_C(3).jsValue());
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_put_getter_by_id)
{
    LLINT_BEGIN();
    ASSERT(LLINT_OP(1).jsValue().isObject());
    JSObject* baseObj = asObject(LLINT_OP(1).jsValue());

    unsigned options = pc[3].u.operand;

    JSValue getter = LLINT_OP(4).jsValue();
    ASSERT(getter.isObject());

    baseObj->putGetter(exec, exec->codeBlock()->identifier(pc[2].u.operand), asObject(getter), options);
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_put_setter_by_id)
{
    LLINT_BEGIN();
    ASSERT(LLINT_OP(1).jsValue().isObject());
    JSObject* baseObj = asObject(LLINT_OP(1).jsValue());

    unsigned options = pc[3].u.operand;

    JSValue setter = LLINT_OP(4).jsValue();
    ASSERT(setter.isObject());

    baseObj->putSetter(exec, exec->codeBlock()->identifier(pc[2].u.operand), asObject(setter), options);
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_put_getter_setter_by_id)
{
    LLINT_BEGIN();
    ASSERT(LLINT_OP(1).jsValue().isObject());
    JSObject* baseObj = asObject(LLINT_OP(1).jsValue());
    
    GetterSetter* accessor = GetterSetter::create(vm, exec->lexicalGlobalObject());
    LLINT_CHECK_EXCEPTION();

    JSValue getter = LLINT_OP(4).jsValue();
    JSValue setter = LLINT_OP(5).jsValue();
    ASSERT(getter.isObject() || getter.isUndefined());
    ASSERT(setter.isObject() || setter.isUndefined());
    ASSERT(getter.isObject() || setter.isObject());
    
    if (!getter.isUndefined())
        accessor->setGetter(vm, exec->lexicalGlobalObject(), asObject(getter));
    if (!setter.isUndefined())
        accessor->setSetter(vm, exec->lexicalGlobalObject(), asObject(setter));
    baseObj->putDirectAccessor(
        exec,
        exec->codeBlock()->identifier(pc[2].u.operand),
        accessor, pc[3].u.operand);
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_put_getter_by_val)
{
    LLINT_BEGIN();
    ASSERT(LLINT_OP(1).jsValue().isObject());
    JSObject* baseObj = asObject(LLINT_OP(1).jsValue());
    JSValue subscript = LLINT_OP_C(2).jsValue();

    unsigned options = pc[3].u.operand;

    JSValue getter = LLINT_OP(4).jsValue();
    ASSERT(getter.isObject());

    auto property = subscript.toPropertyKey(exec);
    LLINT_CHECK_EXCEPTION();

    baseObj->putGetter(exec, property, asObject(getter), options);
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_put_setter_by_val)
{
    LLINT_BEGIN();
    ASSERT(LLINT_OP(1).jsValue().isObject());
    JSObject* baseObj = asObject(LLINT_OP(1).jsValue());
    JSValue subscript = LLINT_OP_C(2).jsValue();

    unsigned options = pc[3].u.operand;

    JSValue setter = LLINT_OP(4).jsValue();
    ASSERT(setter.isObject());

    auto property = subscript.toPropertyKey(exec);
    LLINT_CHECK_EXCEPTION();

    baseObj->putSetter(exec, property, asObject(setter), options);
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_jtrue)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jtrue, LLINT_OP_C(1).jsValue().toBoolean(exec));
}

LLINT_SLOW_PATH_DECL(slow_path_jfalse)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jfalse, !LLINT_OP_C(1).jsValue().toBoolean(exec));
}

LLINT_SLOW_PATH_DECL(slow_path_jless)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jless, jsLess<true>(exec, LLINT_OP_C(1).jsValue(), LLINT_OP_C(2).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_jnless)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jnless, !jsLess<true>(exec, LLINT_OP_C(1).jsValue(), LLINT_OP_C(2).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_jgreater)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jgreater, jsLess<false>(exec, LLINT_OP_C(2).jsValue(), LLINT_OP_C(1).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_jngreater)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jngreater, !jsLess<false>(exec, LLINT_OP_C(2).jsValue(), LLINT_OP_C(1).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_jlesseq)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jlesseq, jsLessEq<true>(exec, LLINT_OP_C(1).jsValue(), LLINT_OP_C(2).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_jnlesseq)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jnlesseq, !jsLessEq<true>(exec, LLINT_OP_C(1).jsValue(), LLINT_OP_C(2).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_jgreatereq)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jgreatereq, jsLessEq<false>(exec, LLINT_OP_C(2).jsValue(), LLINT_OP_C(1).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_jngreatereq)
{
    LLINT_BEGIN();
    LLINT_BRANCH(op_jngreatereq, !jsLessEq<false>(exec, LLINT_OP_C(2).jsValue(), LLINT_OP_C(1).jsValue()));
}

LLINT_SLOW_PATH_DECL(slow_path_switch_imm)
{
    LLINT_BEGIN();
    JSValue scrutinee = LLINT_OP_C(3).jsValue();
    ASSERT(scrutinee.isDouble());
    double value = scrutinee.asDouble();
    int32_t intValue = static_cast<int32_t>(value);
    int defaultOffset = pc[2].u.operand;
    if (value == intValue) {
        CodeBlock* codeBlock = exec->codeBlock();
        pc += codeBlock->switchJumpTable(pc[1].u.operand).offsetForValue(intValue, defaultOffset);
    } else
        pc += defaultOffset;
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_switch_char)
{
    LLINT_BEGIN();
    JSValue scrutinee = LLINT_OP_C(3).jsValue();
    ASSERT(scrutinee.isString());
    JSString* string = asString(scrutinee);
    ASSERT(string->length() == 1);
    int defaultOffset = pc[2].u.operand;
    StringImpl* impl = string->value(exec).impl();
    CodeBlock* codeBlock = exec->codeBlock();
    pc += codeBlock->switchJumpTable(pc[1].u.operand).offsetForValue((*impl)[0], defaultOffset);
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_switch_string)
{
    LLINT_BEGIN();
    JSValue scrutinee = LLINT_OP_C(3).jsValue();
    int defaultOffset = pc[2].u.operand;
    if (!scrutinee.isString())
        pc += defaultOffset;
    else {
        CodeBlock* codeBlock = exec->codeBlock();
        pc += codeBlock->stringSwitchJumpTable(pc[1].u.operand).offsetForValue(asString(scrutinee)->value(exec).impl(), defaultOffset);
    }
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_new_func)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    JSScope* scope = exec->uncheckedR(pc[2].u.operand).Register::scope();
#if LLINT_SLOW_PATH_TRACING
    dataLogF("Creating function!\n");
#endif
    LLINT_RETURN(JSFunction::create(vm, codeBlock->functionDecl(pc[3].u.operand), scope));
}

LLINT_SLOW_PATH_DECL(slow_path_new_generator_func)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    JSScope* scope = exec->uncheckedR(pc[2].u.operand).Register::scope();
#if LLINT_SLOW_PATH_TRACING
    dataLogF("Creating function!\n");
#endif
    LLINT_RETURN(JSGeneratorFunction::create(vm, codeBlock->functionDecl(pc[3].u.operand), scope));
}

LLINT_SLOW_PATH_DECL(slow_path_new_async_func)
{
    LLINT_BEGIN();
    CodeBlock* codeBlock = exec->codeBlock();
    JSScope* scope = exec->uncheckedR(pc[2].u.operand).Register::scope();
#if LLINT_SLOW_PATH_TRACING
    dataLogF("Creating async function!\n");
#endif
    LLINT_RETURN(JSAsyncFunction::create(vm, codeBlock->functionDecl(pc[3].u.operand), scope));
}

LLINT_SLOW_PATH_DECL(slow_path_new_func_exp)
{
    LLINT_BEGIN();
    
    CodeBlock* codeBlock = exec->codeBlock();
    JSScope* scope = exec->uncheckedR(pc[2].u.operand).Register::scope();
    FunctionExecutable* executable = codeBlock->functionExpr(pc[3].u.operand);
    
    LLINT_RETURN(JSFunction::create(vm, executable, scope));
}

LLINT_SLOW_PATH_DECL(slow_path_new_generator_func_exp)
{
    LLINT_BEGIN();

    CodeBlock* codeBlock = exec->codeBlock();
    JSScope* scope = exec->uncheckedR(pc[2].u.operand).Register::scope();
    FunctionExecutable* executable = codeBlock->functionExpr(pc[3].u.operand);

    LLINT_RETURN(JSGeneratorFunction::create(vm, executable, scope));
}

LLINT_SLOW_PATH_DECL(slow_path_new_async_func_exp)
{
    LLINT_BEGIN();
    
    CodeBlock* codeBlock = exec->codeBlock();
    JSScope* scope = exec->uncheckedR(pc[2].u.operand).Register::scope();
    FunctionExecutable* executable = codeBlock->functionExpr(pc[3].u.operand);
    
    LLINT_RETURN(JSAsyncFunction::create(vm, executable, scope));
}

LLINT_SLOW_PATH_DECL(slow_path_set_function_name)
{
    LLINT_BEGIN();
    JSFunction* func = jsCast<JSFunction*>(LLINT_OP(1).Register::unboxedCell());
    JSValue name = LLINT_OP_C(2).Register::jsValue();
    func->setFunctionName(exec, name);
    LLINT_END();
}

static SlowPathReturnType handleHostCall(ExecState* execCallee, Instruction* pc, JSValue callee, CodeSpecializationKind kind)
{
    UNUSED_PARAM(pc);

#if LLINT_SLOW_PATH_TRACING
    dataLog("Performing host call.\n");
#endif
    
    ExecState* exec = execCallee->callerFrame();
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    execCallee->setCodeBlock(0);
    execCallee->clearReturnPC();

    if (kind == CodeForCall) {
        CallData callData;
        CallType callType = getCallData(callee, callData);
    
        ASSERT(callType != CallType::JS);
    
        if (callType == CallType::Host) {
            NativeCallFrameTracer tracer(&vm, execCallee);
            execCallee->setCallee(asObject(callee));
            vm.hostCallReturnValue = JSValue::decode(callData.native.function(execCallee));
            
            LLINT_CALL_RETURN(execCallee, execCallee, LLInt::getCodePtr(getHostCallReturnValue));
        }
        
#if LLINT_SLOW_PATH_TRACING
        dataLog("Call callee is not a function: ", callee, "\n");
#endif

        ASSERT(callType == CallType::None);
        LLINT_CALL_THROW(exec, createNotAFunctionError(exec, callee));
    }

    ASSERT(kind == CodeForConstruct);
    
    ConstructData constructData;
    ConstructType constructType = getConstructData(callee, constructData);
    
    ASSERT(constructType != ConstructType::JS);
    
    if (constructType == ConstructType::Host) {
        NativeCallFrameTracer tracer(&vm, execCallee);
        execCallee->setCallee(asObject(callee));
        vm.hostCallReturnValue = JSValue::decode(constructData.native.function(execCallee));

        LLINT_CALL_RETURN(execCallee, execCallee, LLInt::getCodePtr(getHostCallReturnValue));
    }
    
#if LLINT_SLOW_PATH_TRACING
    dataLog("Constructor callee is not a function: ", callee, "\n");
#endif

    ASSERT(constructType == ConstructType::None);
    LLINT_CALL_THROW(exec, createNotAConstructorError(exec, callee));
}

inline SlowPathReturnType setUpCall(ExecState* execCallee, Instruction* pc, CodeSpecializationKind kind, JSValue calleeAsValue, LLIntCallLinkInfo* callLinkInfo = 0)
{
    ExecState* exec = execCallee->callerFrame();
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

#if LLINT_SLOW_PATH_TRACING
    dataLogF("Performing call with recorded PC = %p\n", exec->currentVPC());
#endif
    
    JSCell* calleeAsFunctionCell = getJSFunction(calleeAsValue);
    if (!calleeAsFunctionCell) {
        throwScope.release();
        return handleHostCall(execCallee, pc, calleeAsValue, kind);
    }
    JSFunction* callee = jsCast<JSFunction*>(calleeAsFunctionCell);
    JSScope* scope = callee->scopeUnchecked();
    ExecutableBase* executable = callee->executable();

    MacroAssemblerCodePtr codePtr;
    CodeBlock* codeBlock = 0;
    if (executable->isHostFunction()) {
        codePtr = executable->entrypointFor(kind, MustCheckArity);
    } else {
        FunctionExecutable* functionExecutable = static_cast<FunctionExecutable*>(executable);

        if (!isCall(kind) && functionExecutable->constructAbility() == ConstructAbility::CannotConstruct)
            LLINT_CALL_THROW(exec, createNotAConstructorError(exec, callee));

        CodeBlock** codeBlockSlot = execCallee->addressOfCodeBlock();
        JSObject* error = functionExecutable->prepareForExecution<FunctionExecutable>(vm, callee, scope, kind, *codeBlockSlot);
        ASSERT(throwScope.exception() == error);
        if (UNLIKELY(error))
            LLINT_CALL_THROW(exec, error);
        codeBlock = *codeBlockSlot;
        ASSERT(codeBlock);
        ArityCheckMode arity;
        if (execCallee->argumentCountIncludingThis() < static_cast<size_t>(codeBlock->numParameters()))
            arity = MustCheckArity;
        else
            arity = ArityCheckNotRequired;
        codePtr = functionExecutable->entrypointFor(kind, arity);
    }

    ASSERT(!!codePtr);
    
    if (!LLINT_ALWAYS_ACCESS_SLOW && callLinkInfo) {
        CodeBlock* callerCodeBlock = exec->codeBlock();

        ConcurrentJSLocker locker(callerCodeBlock->m_lock);
        
        if (callLinkInfo->isOnList())
            callLinkInfo->remove();
        callLinkInfo->callee.set(vm, callerCodeBlock, callee);
        callLinkInfo->lastSeenCallee.set(vm, callerCodeBlock, callee);
        callLinkInfo->machineCodeTarget = codePtr;
        if (codeBlock)
            codeBlock->linkIncomingCall(exec, callLinkInfo);
    }

    LLINT_CALL_RETURN(exec, execCallee, codePtr.executableAddress());
}

inline SlowPathReturnType genericCall(ExecState* exec, Instruction* pc, CodeSpecializationKind kind)
{
    // This needs to:
    // - Set up a call frame.
    // - Figure out what to call and compile it if necessary.
    // - If possible, link the call's inline cache.
    // - Return a tuple of machine code address to call and the new call frame.
    
    JSValue calleeAsValue = LLINT_OP_C(2).jsValue();
    
    ExecState* execCallee = exec - pc[4].u.operand;
    
    execCallee->setArgumentCountIncludingThis(pc[3].u.operand);
    execCallee->uncheckedR(CallFrameSlot::callee) = calleeAsValue;
    execCallee->setCallerFrame(exec);
    
    ASSERT(pc[5].u.callLinkInfo);
    return setUpCall(execCallee, pc, kind, calleeAsValue, pc[5].u.callLinkInfo);
}

LLINT_SLOW_PATH_DECL(slow_path_call)
{
    LLINT_BEGIN_NO_SET_PC();
    throwScope.release();
    return genericCall(exec, pc, CodeForCall);
}

LLINT_SLOW_PATH_DECL(slow_path_construct)
{
    LLINT_BEGIN_NO_SET_PC();
    throwScope.release();
    return genericCall(exec, pc, CodeForConstruct);
}

LLINT_SLOW_PATH_DECL(slow_path_size_frame_for_varargs)
{
    LLINT_BEGIN();
    // This needs to:
    // - Set up a call frame while respecting the variable arguments.
    
    unsigned numUsedStackSlots = -pc[5].u.operand;
    unsigned length = sizeFrameForVarargs(exec, vm,
        LLINT_OP_C(4).jsValue(), numUsedStackSlots, pc[6].u.operand);
    LLINT_CALL_CHECK_EXCEPTION(exec, exec);
    
    ExecState* execCallee = calleeFrameForVarargs(exec, numUsedStackSlots, length + 1);
    vm.varargsLength = length;
    vm.newCallFrameReturnValue = execCallee;

    LLINT_RETURN_CALLEE_FRAME(execCallee);
}

LLINT_SLOW_PATH_DECL(slow_path_size_frame_for_forward_arguments)
{
    LLINT_BEGIN();
    // This needs to:
    // - Set up a call frame with the same arguments as the current frame.

    unsigned numUsedStackSlots = -pc[5].u.operand;

    unsigned arguments = sizeFrameForForwardArguments(exec, vm, numUsedStackSlots);
    LLINT_CALL_CHECK_EXCEPTION(exec, exec);

    ExecState* execCallee = calleeFrameForVarargs(exec, numUsedStackSlots, arguments + 1);

    vm.varargsLength = arguments;
    vm.newCallFrameReturnValue = execCallee;

    LLINT_RETURN_CALLEE_FRAME(execCallee);
}

enum class SetArgumentsWith {
    Object,
    CurrentArguments
};

inline SlowPathReturnType varargsSetup(ExecState* exec, Instruction* pc, CodeSpecializationKind kind, SetArgumentsWith set)
{
    LLINT_BEGIN_NO_SET_PC();
    // This needs to:
    // - Figure out what to call and compile it if necessary.
    // - Return a tuple of machine code address to call and the new call frame.

    JSValue calleeAsValue = LLINT_OP_C(2).jsValue();

    ExecState* execCallee = vm.newCallFrameReturnValue;

    if (set == SetArgumentsWith::Object) {
        setupVarargsFrameAndSetThis(exec, execCallee, LLINT_OP_C(3).jsValue(), LLINT_OP_C(4).jsValue(), pc[6].u.operand, vm.varargsLength);
        LLINT_CALL_CHECK_EXCEPTION(exec, exec);
    } else
        setupForwardArgumentsFrameAndSetThis(exec, execCallee, LLINT_OP_C(3).jsValue(), vm.varargsLength);

    execCallee->setCallerFrame(exec);
    execCallee->uncheckedR(CallFrameSlot::callee) = calleeAsValue;
    exec->setCurrentVPC(pc);

    throwScope.release();
    return setUpCall(execCallee, pc, kind, calleeAsValue);
}

LLINT_SLOW_PATH_DECL(slow_path_call_varargs)
{
    return varargsSetup(exec, pc, CodeForCall, SetArgumentsWith::Object);
}

LLINT_SLOW_PATH_DECL(slow_path_tail_call_forward_arguments)
{
    return varargsSetup(exec, pc, CodeForCall, SetArgumentsWith::CurrentArguments);
}

LLINT_SLOW_PATH_DECL(slow_path_construct_varargs)
{
    return varargsSetup(exec, pc, CodeForConstruct, SetArgumentsWith::Object);
}

    
LLINT_SLOW_PATH_DECL(slow_path_call_eval)
{
    LLINT_BEGIN_NO_SET_PC();
    JSValue calleeAsValue = LLINT_OP(2).jsValue();
    
    ExecState* execCallee = exec - pc[4].u.operand;
    
    execCallee->setArgumentCountIncludingThis(pc[3].u.operand);
    execCallee->setCallerFrame(exec);
    execCallee->uncheckedR(CallFrameSlot::callee) = calleeAsValue;
    execCallee->setReturnPC(LLInt::getCodePtr(llint_generic_return_point));
    execCallee->setCodeBlock(0);
    exec->setCurrentVPC(pc);
    
    if (!isHostFunction(calleeAsValue, globalFuncEval)) {
        throwScope.release();
        return setUpCall(execCallee, pc, CodeForCall, calleeAsValue);
    }
    
    vm.hostCallReturnValue = eval(execCallee);
    LLINT_CALL_RETURN(exec, execCallee, LLInt::getCodePtr(getHostCallReturnValue));
}

LLINT_SLOW_PATH_DECL(slow_path_strcat)
{
    LLINT_BEGIN();
    LLINT_RETURN(jsStringFromRegisterArray(exec, &LLINT_OP(2), pc[3].u.operand));
}

LLINT_SLOW_PATH_DECL(slow_path_to_primitive)
{
    LLINT_BEGIN();
    LLINT_RETURN(LLINT_OP_C(2).jsValue().toPrimitive(exec));
}

LLINT_SLOW_PATH_DECL(slow_path_throw)
{
    LLINT_BEGIN();
    LLINT_THROW(LLINT_OP_C(1).jsValue());
}

LLINT_SLOW_PATH_DECL(slow_path_handle_traps)
{
    LLINT_BEGIN_NO_SET_PC();
    ASSERT(vm.needTrapHandling());
    vm.handleTraps(exec);
    UNUSED_PARAM(pc);
    LLINT_RETURN_TWO(throwScope.exception(), exec);
}

LLINT_SLOW_PATH_DECL(slow_path_debug)
{
    LLINT_BEGIN();
    int debugHookType = pc[1].u.operand;
    vm.interpreter->debug(exec, static_cast<DebugHookType>(debugHookType));
    
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_handle_exception)
{
    LLINT_BEGIN_NO_SET_PC();
    UNUSED_PARAM(throwScope);
    genericUnwind(&vm, exec);
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(slow_path_get_from_scope)
{
    LLINT_BEGIN();
    const Identifier& ident = exec->codeBlock()->identifier(pc[3].u.operand);
    JSObject* scope = jsCast<JSObject*>(LLINT_OP(2).jsValue());
    GetPutInfo getPutInfo(pc[4].u.operand);

    // ModuleVar is always converted to ClosureVar for get_from_scope.
    ASSERT(getPutInfo.resolveType() != ModuleVar);

    LLINT_RETURN(scope->getPropertySlot(exec, ident, [&] (bool found, PropertySlot& slot) -> JSValue {
        if (!found) {
            if (getPutInfo.resolveMode() == ThrowIfNotFound)
                return throwException(exec, throwScope, createUndefinedVariableError(exec, ident));
            return jsUndefined();
        }

        JSValue result = JSValue();
        if (scope->isGlobalLexicalEnvironment()) {
            // When we can't statically prove we need a TDZ check, we must perform the check on the slow path.
            result = slot.getValue(exec, ident);
            if (result == jsTDZValue())
                return throwException(exec, throwScope, createTDZError(exec));
        }

        CommonSlowPaths::tryCacheGetFromScopeGlobal(exec, vm, pc, scope, slot, ident);

        if (!result)
            return slot.getValue(exec, ident);
        return result;
    }));
}

LLINT_SLOW_PATH_DECL(slow_path_put_to_scope)
{
    LLINT_BEGIN();

    CodeBlock* codeBlock = exec->codeBlock();
    const Identifier& ident = codeBlock->identifier(pc[2].u.operand);
    JSObject* scope = jsCast<JSObject*>(LLINT_OP(1).jsValue());
    JSValue value = LLINT_OP_C(3).jsValue();
    GetPutInfo getPutInfo = GetPutInfo(pc[4].u.operand);
    if (getPutInfo.resolveType() == LocalClosureVar) {
        JSLexicalEnvironment* environment = jsCast<JSLexicalEnvironment*>(scope);
        environment->variableAt(ScopeOffset(pc[6].u.operand)).set(vm, environment, value);
        
        // Have to do this *after* the write, because if this puts the set into IsWatched, then we need
        // to have already changed the value of the variable. Otherwise we might watch and constant-fold
        // to the Undefined value from before the assignment.
        if (WatchpointSet* set = pc[5].u.watchpointSet)
            set->touch(vm, "Executed op_put_scope<LocalClosureVar>");
        LLINT_END();
    }

    bool hasProperty = scope->hasProperty(exec, ident);
    LLINT_CHECK_EXCEPTION();
    if (hasProperty
        && scope->isGlobalLexicalEnvironment()
        && !isInitialization(getPutInfo.initializationMode())) {
        // When we can't statically prove we need a TDZ check, we must perform the check on the slow path.
        PropertySlot slot(scope, PropertySlot::InternalMethodType::Get);
        JSGlobalLexicalEnvironment::getOwnPropertySlot(scope, exec, ident, slot);
        if (slot.getValue(exec, ident) == jsTDZValue())
            LLINT_THROW(createTDZError(exec));
    }

    if (getPutInfo.resolveMode() == ThrowIfNotFound && !hasProperty)
        LLINT_THROW(createUndefinedVariableError(exec, ident));

    PutPropertySlot slot(scope, codeBlock->isStrictMode(), PutPropertySlot::UnknownContext, isInitialization(getPutInfo.initializationMode()));
    scope->methodTable()->put(scope, exec, ident, value, slot);
    
    CommonSlowPaths::tryCachePutToScopeGlobal(exec, codeBlock, pc, scope, getPutInfo, slot, ident);

    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_check_if_exception_is_uncatchable_and_notify_profiler)
{
    LLINT_BEGIN();
    RELEASE_ASSERT(!!throwScope.exception());

    if (isTerminatedExecutionException(vm, throwScope.exception()))
        LLINT_RETURN_TWO(pc, bitwise_cast<void*>(static_cast<uintptr_t>(1)));
    LLINT_RETURN_TWO(pc, 0);
}

LLINT_SLOW_PATH_DECL(slow_path_log_shadow_chicken_prologue)
{
    LLINT_BEGIN();
    
    JSScope* scope = exec->uncheckedR(pc[1].u.operand).Register::scope();
    vm.shadowChicken().log(vm, exec, ShadowChicken::Packet::prologue(exec->jsCallee(), exec, exec->callerFrame(), scope));
    
    LLINT_END();
}

LLINT_SLOW_PATH_DECL(slow_path_log_shadow_chicken_tail)
{
    LLINT_BEGIN();

    JSValue thisValue = LLINT_OP(1).jsValue();
    JSScope* scope = exec->uncheckedR(pc[2].u.operand).Register::scope();
    
#if USE(JSVALUE64)
    CallSiteIndex callSiteIndex(exec->codeBlock()->bytecodeOffset(pc));
#else
    CallSiteIndex callSiteIndex(pc);
#endif
    vm.shadowChicken().log(vm, exec, ShadowChicken::Packet::tail(exec, thisValue, scope, exec->codeBlock(), callSiteIndex));
    
    LLINT_END();
}

extern "C" SlowPathReturnType llint_throw_stack_overflow_error(VM* vm, ProtoCallFrame* protoFrame)
{
    ExecState* exec = vm->topCallFrame;
    auto scope = DECLARE_THROW_SCOPE(*vm);

    if (!exec)
        exec = protoFrame->callee()->globalObject()->globalExec();
    throwStackOverflowError(exec, scope);
    return encodeResult(0, 0);
}

#if !ENABLE(JIT)
extern "C" SlowPathReturnType llint_stack_check_at_vm_entry(VM* vm, Register* newTopOfStack)
{
    bool success = vm->ensureStackCapacityFor(newTopOfStack);
    return encodeResult(reinterpret_cast<void*>(success), 0);
}
#endif

extern "C" void llint_write_barrier_slow(ExecState* exec, JSCell* cell)
{
    VM& vm = exec->vm();
    vm.heap.writeBarrier(cell);
}

extern "C" NO_RETURN_DUE_TO_CRASH void llint_crash()
{
    CRASH();
}

#if ENABLE(LLINT_STATS)

LLINT_SLOW_PATH_DECL(count_opcode)
{
    OpcodeID opcodeID = exec->vm().interpreter->getOpcodeID(pc[0].u.opcode);
    Data::opcodeStats(opcodeID).count++;
    LLINT_END_IMPL();
}

LLINT_SLOW_PATH_DECL(count_opcode_slow_path)
{
    OpcodeID opcodeID = exec->vm().interpreter->getOpcodeID(pc[0].u.opcode);
    Data::opcodeStats(opcodeID).slowPathCount++;
    LLINT_END_IMPL();
}

#endif // ENABLE(LLINT_STATS)

} } // namespace JSC::LLInt
