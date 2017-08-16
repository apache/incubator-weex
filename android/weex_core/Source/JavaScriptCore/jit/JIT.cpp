/*
 * Copyright (C) 2008-2017 Apple Inc. All rights reserved.
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

#include "BytecodeGraph.h"
#include "CodeBlock.h"
#include "CodeBlockWithJITType.h"
#include "DFGCapabilities.h"
#include "Interpreter.h"
#include "JITInlines.h"
#include "JITOperations.h"
#include "JSArray.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "LinkBuffer.h"
#include "MaxFrameExtentForSlowPathCall.h"
#include "PCToCodeOriginMap.h"
#include "ProfilerDatabase.h"
#include "ProgramCodeBlock.h"
#include "ResultType.h"
#include "SlowPathCall.h"
#include "StackAlignment.h"
#include "TypeProfilerLog.h"
#include "Trace.h"
#include <wtf/CryptographicallyRandomNumber.h>
#include <wtf/GraphNodeWorklist.h>
#include <wtf/SimpleStats.h>

using namespace std;

namespace JSC {

double totalBaselineCompileTime;
double totalDFGCompileTime;
double totalFTLCompileTime;
double totalFTLDFGCompileTime;
double totalFTLB3CompileTime;

void ctiPatchCallByReturnAddress(ReturnAddressPtr returnAddress, FunctionPtr newCalleeFunction)
{
    MacroAssembler::repatchCall(
        CodeLocationCall(MacroAssemblerCodePtr(returnAddress)),
        newCalleeFunction);
}

JIT::CodeRef JIT::compileCTINativeCall(VM* vm, NativeFunction func)
{
    if (!vm->canUseJIT())
        return CodeRef::createLLIntCodeRef(llint_native_call_trampoline);
    JIT jit(vm, 0);
    return jit.privateCompileCTINativeCall(vm, func);
}

JIT::JIT(VM* vm, CodeBlock* codeBlock, unsigned loopOSREntryBytecodeOffset)
    : JSInterfaceJIT(vm, codeBlock)
    , m_interpreter(vm->interpreter)
    , m_labels(codeBlock ? codeBlock->numberOfInstructions() : 0)
    , m_bytecodeOffset(std::numeric_limits<unsigned>::max())
    , m_getByIdIndex(UINT_MAX)
    , m_getByIdWithThisIndex(UINT_MAX)
    , m_putByIdIndex(UINT_MAX)
    , m_byValInstructionIndex(UINT_MAX)
    , m_callLinkInfoIndex(UINT_MAX)
    , m_randomGenerator(cryptographicallyRandomNumber())
    , m_pcToCodeOriginMapBuilder(*vm)
    , m_canBeOptimized(false)
    , m_shouldEmitProfiling(false)
    , m_loopOSREntryBytecodeOffset(loopOSREntryBytecodeOffset)
{
}

JIT::~JIT()
{
}

#if ENABLE(DFG_JIT)
void JIT::emitEnterOptimizationCheck()
{
    if (!canBeOptimized())
        return;

    JumpList skipOptimize;
    
    skipOptimize.append(branchAdd32(Signed, TrustedImm32(Options::executionCounterIncrementForEntry()), AbsoluteAddress(m_codeBlock->addressOfJITExecuteCounter())));
    ASSERT(!m_bytecodeOffset);

    copyCalleeSavesFromFrameOrRegisterToVMEntryFrameCalleeSavesBuffer();

    callOperation(operationOptimize, m_bytecodeOffset);
    skipOptimize.append(branchTestPtr(Zero, returnValueGPR));
    move(returnValueGPR2, stackPointerRegister);
    jump(returnValueGPR);
    skipOptimize.link(this);
}
#endif

void JIT::emitNotifyWrite(WatchpointSet* set)
{
    if (!set || set->state() == IsInvalidated) {
        addSlowCase(Jump());
        return;
    }
    
    addSlowCase(branch8(NotEqual, AbsoluteAddress(set->addressOfState()), TrustedImm32(IsInvalidated)));
}

void JIT::emitNotifyWrite(GPRReg pointerToSet)
{
    addSlowCase(branch8(NotEqual, Address(pointerToSet, WatchpointSet::offsetOfState()), TrustedImm32(IsInvalidated)));
}

void JIT::assertStackPointerOffset()
{
    if (ASSERT_DISABLED)
        return;
    
    addPtr(TrustedImm32(stackPointerOffsetFor(m_codeBlock) * sizeof(Register)), callFrameRegister, regT0);
    Jump ok = branchPtr(Equal, regT0, stackPointerRegister);
    breakpoint();
    ok.link(this);
}

#define NEXT_OPCODE(name) \
    m_bytecodeOffset += OPCODE_LENGTH(name); \
    break;

#define DEFINE_SLOW_OP(name) \
    case op_##name: { \
        if (m_bytecodeOffset >= startBytecodeOffset) { \
            JITSlowPathCall slowPathCall(this, currentInstruction, slow_path_##name); \
            slowPathCall.call(); \
        } \
        NEXT_OPCODE(op_##name); \
    }

#define DEFINE_OP(name) \
    case name: { \
        if (m_bytecodeOffset >= startBytecodeOffset) { \
            emit_##name(currentInstruction); \
        } \
        NEXT_OPCODE(name); \
    }

#define DEFINE_SLOWCASE_OP(name) \
    case name: { \
        emitSlow_##name(currentInstruction, iter); \
        NEXT_OPCODE(name); \
    }

void JIT::privateCompileMainPass()
{
    if (false)
        dataLog("Compiling ", *m_codeBlock, "\n");
    
    jitAssertTagsInPlace();
    jitAssertArgumentCountSane();
    
    Instruction* instructionsBegin = m_codeBlock->instructions().begin();
    unsigned instructionCount = m_instructions.size();

    m_callLinkInfoIndex = 0;

    unsigned startBytecodeOffset = 0;
    if (m_loopOSREntryBytecodeOffset && m_codeBlock->inherits(*m_codeBlock->vm(), ProgramCodeBlock::info())) {
        // We can only do this optimization because we execute ProgramCodeBlock's exactly once.
        // This optimization would be invalid otherwise. When the LLInt determines it wants to
        // do OSR entry into the baseline JIT in a loop, it will pass in the bytecode offset it
        // was executing at when it kicked off our compilation. We only need to compile code for
        // anything reachable from that bytecode offset.

        // We only bother building the bytecode graph if it could save time and executable
        // memory. We pick an arbitrary offset where we deem this is profitable.
        if (m_loopOSREntryBytecodeOffset >= 200) {
            // As a simplification, we don't find all bytecode ranges that are unreachable.
            // Instead, we just find the minimum bytecode offset that is reachable, and
            // compile code from that bytecode offset onwards.

            BytecodeGraph<CodeBlock> graph(m_codeBlock, m_instructions);
            BytecodeBasicBlock* block = graph.findBasicBlockForBytecodeOffset(m_loopOSREntryBytecodeOffset);
            RELEASE_ASSERT(block);

            GraphNodeWorklist<BytecodeBasicBlock*> worklist;
            startBytecodeOffset = UINT_MAX;
            worklist.push(block);
            while (BytecodeBasicBlock* block = worklist.pop()) {
                startBytecodeOffset = std::min(startBytecodeOffset, block->leaderOffset());
                worklist.pushAll(block->successors());
            }
        }
    }

    for (m_bytecodeOffset = 0; m_bytecodeOffset < instructionCount; ) {
        if (m_bytecodeOffset == startBytecodeOffset && startBytecodeOffset > 0) {
            // We've proven all bytecode instructions up until here are unreachable.
            // Let's ensure that by crashing if it's ever hit.
            breakpoint();
        }

        if (m_disassembler)
            m_disassembler->setForBytecodeMainPath(m_bytecodeOffset, label());
        Instruction* currentInstruction = instructionsBegin + m_bytecodeOffset;
        ASSERT_WITH_MESSAGE(m_interpreter->isOpcode(currentInstruction->u.opcode), "privateCompileMainPass gone bad @ %d", m_bytecodeOffset);

        m_pcToCodeOriginMapBuilder.appendItem(label(), CodeOrigin(m_bytecodeOffset));

#if ENABLE(OPCODE_SAMPLING)
        if (m_bytecodeOffset > 0) // Avoid the overhead of sampling op_enter twice.
            sampleInstruction(currentInstruction);
#endif

        m_labels[m_bytecodeOffset] = label();

#if ENABLE(JIT_VERBOSE)
        dataLogF("Old JIT emitting code for bc#%u at offset 0x%lx.\n", m_bytecodeOffset, (long)debugOffset());
#endif
        
        OpcodeID opcodeID = m_interpreter->getOpcodeID(currentInstruction->u.opcode);

        if (m_compilation) {
            add64(
                TrustedImm32(1),
                AbsoluteAddress(m_compilation->executionCounterFor(Profiler::OriginStack(Profiler::Origin(
                    m_compilation->bytecodes(), m_bytecodeOffset)))->address()));
        }
        
        if (Options::eagerlyUpdateTopCallFrame())
            updateTopCallFrame();

        unsigned bytecodeOffset = m_bytecodeOffset;

        switch (opcodeID) {
        DEFINE_SLOW_OP(in)
        DEFINE_SLOW_OP(less)
        DEFINE_SLOW_OP(lesseq)
        DEFINE_SLOW_OP(greater)
        DEFINE_SLOW_OP(greatereq)
        DEFINE_SLOW_OP(is_function)
        DEFINE_SLOW_OP(is_object_or_null)
        DEFINE_SLOW_OP(typeof)

        DEFINE_OP(op_add)
        DEFINE_OP(op_bitand)
        DEFINE_OP(op_bitor)
        DEFINE_OP(op_bitxor)
        DEFINE_OP(op_call)
        DEFINE_OP(op_tail_call)
        DEFINE_OP(op_call_eval)
        DEFINE_OP(op_call_varargs)
        DEFINE_OP(op_tail_call_varargs)
        DEFINE_OP(op_tail_call_forward_arguments)
        DEFINE_OP(op_construct_varargs)
        DEFINE_OP(op_catch)
        DEFINE_OP(op_construct)
        DEFINE_OP(op_create_this)
        DEFINE_OP(op_to_this)
        DEFINE_OP(op_create_direct_arguments)
        DEFINE_OP(op_create_scoped_arguments)
        DEFINE_OP(op_create_cloned_arguments)
        DEFINE_OP(op_get_argument)
        DEFINE_OP(op_argument_count)
        DEFINE_OP(op_create_rest)
        DEFINE_OP(op_get_rest_length)
        DEFINE_OP(op_check_tdz)
        DEFINE_OP(op_assert)
        DEFINE_OP(op_debug)
        DEFINE_OP(op_del_by_id)
        DEFINE_OP(op_del_by_val)
        DEFINE_OP(op_div)
        DEFINE_OP(op_end)
        DEFINE_OP(op_enter)
        DEFINE_OP(op_get_scope)
        DEFINE_OP(op_eq)
        DEFINE_OP(op_eq_null)
        DEFINE_OP(op_try_get_by_id)
        case op_get_array_length:
        case op_get_by_id_proto_load:
        case op_get_by_id_unset:
        DEFINE_OP(op_get_by_id)
        DEFINE_OP(op_get_by_id_with_this)
        DEFINE_OP(op_get_by_val)
        DEFINE_OP(op_get_by_val_with_this)
        DEFINE_OP(op_overrides_has_instance)
        DEFINE_OP(op_instanceof)
        DEFINE_OP(op_instanceof_custom)
        DEFINE_OP(op_is_empty)
        DEFINE_OP(op_is_undefined)
        DEFINE_OP(op_is_boolean)
        DEFINE_OP(op_is_number)
        DEFINE_OP(op_is_object)
        DEFINE_OP(op_is_cell_with_type)
        DEFINE_OP(op_jeq_null)
        DEFINE_OP(op_jfalse)
        DEFINE_OP(op_jmp)
        DEFINE_OP(op_jneq_null)
        DEFINE_OP(op_jneq_ptr)
        DEFINE_OP(op_jless)
        DEFINE_OP(op_jlesseq)
        DEFINE_OP(op_jgreater)
        DEFINE_OP(op_jgreatereq)
        DEFINE_OP(op_jnless)
        DEFINE_OP(op_jnlesseq)
        DEFINE_OP(op_jngreater)
        DEFINE_OP(op_jngreatereq)
        DEFINE_OP(op_jtrue)
        DEFINE_OP(op_loop_hint)
        DEFINE_OP(op_check_traps)
        DEFINE_OP(op_lshift)
        DEFINE_OP(op_mod)
        DEFINE_OP(op_mov)
        DEFINE_OP(op_mul)
        DEFINE_OP(op_negate)
        DEFINE_OP(op_neq)
        DEFINE_OP(op_neq_null)
        DEFINE_OP(op_new_array)
        DEFINE_OP(op_new_array_with_size)
        DEFINE_OP(op_new_array_buffer)
        DEFINE_OP(op_new_array_with_spread)
        DEFINE_OP(op_spread)
        DEFINE_OP(op_new_func)
        DEFINE_OP(op_new_func_exp)
        DEFINE_OP(op_new_generator_func)
        DEFINE_OP(op_new_generator_func_exp)
        DEFINE_OP(op_new_async_func)
        DEFINE_OP(op_new_async_func_exp)
        DEFINE_OP(op_new_object)
        DEFINE_OP(op_new_regexp)
        DEFINE_OP(op_not)
        DEFINE_OP(op_nstricteq)
        DEFINE_OP(op_dec)
        DEFINE_OP(op_inc)
        DEFINE_OP(op_pow)
        DEFINE_OP(op_profile_type)
        DEFINE_OP(op_profile_control_flow)
        DEFINE_OP(op_push_with_scope)
        DEFINE_OP(op_create_lexical_environment)
        DEFINE_OP(op_get_parent_scope)
        DEFINE_OP(op_put_by_id)
        DEFINE_OP(op_put_by_id_with_this)
        DEFINE_OP(op_put_by_index)
        case op_put_by_val_direct:
        DEFINE_OP(op_put_by_val)
        DEFINE_OP(op_put_by_val_with_this)
        DEFINE_OP(op_put_getter_by_id)
        DEFINE_OP(op_put_setter_by_id)
        DEFINE_OP(op_put_getter_setter_by_id)
        DEFINE_OP(op_put_getter_by_val)
        DEFINE_OP(op_put_setter_by_val)
        DEFINE_OP(op_define_data_property)
        DEFINE_OP(op_define_accessor_property)

        DEFINE_OP(op_ret)
        DEFINE_OP(op_rshift)
        DEFINE_OP(op_unsigned)
        DEFINE_OP(op_urshift)
        DEFINE_OP(op_set_function_name)
        DEFINE_OP(op_strcat)
        DEFINE_OP(op_stricteq)
        DEFINE_OP(op_sub)
        DEFINE_OP(op_switch_char)
        DEFINE_OP(op_switch_imm)
        DEFINE_OP(op_switch_string)
        DEFINE_OP(op_throw)
        DEFINE_OP(op_throw_static_error)
        DEFINE_OP(op_to_number)
        DEFINE_OP(op_to_string)
        DEFINE_OP(op_to_primitive)

        DEFINE_OP(op_resolve_scope)
        DEFINE_OP(op_get_from_scope)
        DEFINE_OP(op_put_to_scope)
        DEFINE_OP(op_get_from_arguments)
        DEFINE_OP(op_put_to_arguments)

        DEFINE_OP(op_get_enumerable_length)
        DEFINE_OP(op_has_generic_property)
        DEFINE_OP(op_has_structure_property)
        DEFINE_OP(op_has_indexed_property)
        DEFINE_OP(op_get_direct_pname)
        DEFINE_OP(op_get_property_enumerator)
        DEFINE_OP(op_enumerator_structure_pname)
        DEFINE_OP(op_enumerator_generic_pname)
        DEFINE_OP(op_to_index_string)
            
        DEFINE_OP(op_log_shadow_chicken_prologue)
        DEFINE_OP(op_log_shadow_chicken_tail)
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }

        if (false)
            dataLog("At ", bytecodeOffset, ": ", m_slowCases.size(), "\n");
    }

    RELEASE_ASSERT(m_callLinkInfoIndex == m_callCompilationInfo.size());

#ifndef NDEBUG
    // Reset this, in order to guard its use with ASSERTs.
    m_bytecodeOffset = std::numeric_limits<unsigned>::max();
#endif
}

void JIT::privateCompileLinkPass()
{
    unsigned jmpTableCount = m_jmpTable.size();
    for (unsigned i = 0; i < jmpTableCount; ++i)
        m_jmpTable[i].from.linkTo(m_labels[m_jmpTable[i].toBytecodeOffset], this);
    m_jmpTable.clear();
}

void JIT::privateCompileSlowCases()
{
    Instruction* instructionsBegin = m_codeBlock->instructions().begin();

    m_getByIdIndex = 0;
    m_getByIdWithThisIndex = 0;
    m_putByIdIndex = 0;
    m_byValInstructionIndex = 0;
    m_callLinkInfoIndex = 0;
    
    // Use this to assert that slow-path code associates new profiling sites with existing
    // ValueProfiles rather than creating new ones. This ensures that for a given instruction
    // (say, get_by_id) we get combined statistics for both the fast-path executions of that
    // instructions and the slow-path executions. Furthermore, if the slow-path code created
    // new ValueProfiles then the ValueProfiles would no longer be sorted by bytecode offset,
    // which would break the invariant necessary to use CodeBlock::valueProfileForBytecodeOffset().
    unsigned numberOfValueProfiles = m_codeBlock->numberOfValueProfiles();

    for (Vector<SlowCaseEntry>::iterator iter = m_slowCases.begin(); iter != m_slowCases.end();) {
        m_bytecodeOffset = iter->to;

        m_pcToCodeOriginMapBuilder.appendItem(label(), CodeOrigin(m_bytecodeOffset));

        unsigned firstTo = m_bytecodeOffset;

        Instruction* currentInstruction = instructionsBegin + m_bytecodeOffset;
        
        RareCaseProfile* rareCaseProfile = 0;
        if (shouldEmitProfiling())
            rareCaseProfile = m_codeBlock->addRareCaseProfile(m_bytecodeOffset);

#if ENABLE(JIT_VERBOSE)
        dataLogF("Old JIT emitting slow code for bc#%u at offset 0x%lx.\n", m_bytecodeOffset, (long)debugOffset());
#endif
        
        if (m_disassembler)
            m_disassembler->setForBytecodeSlowPath(m_bytecodeOffset, label());

        switch (m_interpreter->getOpcodeID(currentInstruction->u.opcode)) {
        DEFINE_SLOWCASE_OP(op_add)
        DEFINE_SLOWCASE_OP(op_bitand)
        DEFINE_SLOWCASE_OP(op_bitor)
        DEFINE_SLOWCASE_OP(op_bitxor)
        DEFINE_SLOWCASE_OP(op_call)
        DEFINE_SLOWCASE_OP(op_tail_call)
        DEFINE_SLOWCASE_OP(op_call_eval)
        DEFINE_SLOWCASE_OP(op_call_varargs)
        DEFINE_SLOWCASE_OP(op_tail_call_varargs)
        DEFINE_SLOWCASE_OP(op_tail_call_forward_arguments)
        DEFINE_SLOWCASE_OP(op_construct_varargs)
        DEFINE_SLOWCASE_OP(op_construct)
        DEFINE_SLOWCASE_OP(op_to_this)
        DEFINE_SLOWCASE_OP(op_check_tdz)
        DEFINE_SLOWCASE_OP(op_create_this)
        DEFINE_SLOWCASE_OP(op_div)
        DEFINE_SLOWCASE_OP(op_eq)
        DEFINE_SLOWCASE_OP(op_try_get_by_id)
        case op_get_array_length:
        case op_get_by_id_proto_load:
        case op_get_by_id_unset:
        DEFINE_SLOWCASE_OP(op_get_by_id)
        DEFINE_SLOWCASE_OP(op_get_by_id_with_this)
        DEFINE_SLOWCASE_OP(op_get_by_val)
        DEFINE_SLOWCASE_OP(op_instanceof)
        DEFINE_SLOWCASE_OP(op_instanceof_custom)
        DEFINE_SLOWCASE_OP(op_jless)
        DEFINE_SLOWCASE_OP(op_jlesseq)
        DEFINE_SLOWCASE_OP(op_jgreater)
        DEFINE_SLOWCASE_OP(op_jgreatereq)
        DEFINE_SLOWCASE_OP(op_jnless)
        DEFINE_SLOWCASE_OP(op_jnlesseq)
        DEFINE_SLOWCASE_OP(op_jngreater)
        DEFINE_SLOWCASE_OP(op_jngreatereq)
        DEFINE_SLOWCASE_OP(op_loop_hint)
        DEFINE_SLOWCASE_OP(op_check_traps)
        DEFINE_SLOWCASE_OP(op_lshift)
        DEFINE_SLOWCASE_OP(op_mod)
        DEFINE_SLOWCASE_OP(op_mul)
        DEFINE_SLOWCASE_OP(op_negate)
        DEFINE_SLOWCASE_OP(op_neq)
        DEFINE_SLOWCASE_OP(op_new_object)
        DEFINE_SLOWCASE_OP(op_not)
        DEFINE_SLOWCASE_OP(op_nstricteq)
        DEFINE_SLOWCASE_OP(op_dec)
        DEFINE_SLOWCASE_OP(op_inc)
        DEFINE_SLOWCASE_OP(op_put_by_id)
        case op_put_by_val_direct:
        DEFINE_SLOWCASE_OP(op_put_by_val)
        DEFINE_SLOWCASE_OP(op_rshift)
        DEFINE_SLOWCASE_OP(op_unsigned)
        DEFINE_SLOWCASE_OP(op_urshift)
        DEFINE_SLOWCASE_OP(op_stricteq)
        DEFINE_SLOWCASE_OP(op_sub)
        DEFINE_SLOWCASE_OP(op_to_number)
        DEFINE_SLOWCASE_OP(op_to_string)
        DEFINE_SLOWCASE_OP(op_to_primitive)
        DEFINE_SLOWCASE_OP(op_has_indexed_property)
        DEFINE_SLOWCASE_OP(op_has_structure_property)
        DEFINE_SLOWCASE_OP(op_get_direct_pname)

        DEFINE_SLOWCASE_OP(op_resolve_scope)
        DEFINE_SLOWCASE_OP(op_get_from_scope)
        DEFINE_SLOWCASE_OP(op_put_to_scope)

        default:
            RELEASE_ASSERT_NOT_REACHED();
        }

        if (false)
            dataLog("At ", firstTo, " slow: ", iter - m_slowCases.begin(), "\n");

        RELEASE_ASSERT_WITH_MESSAGE(iter == m_slowCases.end() || firstTo != iter->to, "Not enough jumps linked in slow case codegen.");
        RELEASE_ASSERT_WITH_MESSAGE(firstTo == (iter - 1)->to, "Too many jumps linked in slow case codegen.");
        
        if (shouldEmitProfiling())
            add32(TrustedImm32(1), AbsoluteAddress(&rareCaseProfile->m_counter));

        emitJumpSlowToHot(jump(), 0);
    }

    RELEASE_ASSERT(m_getByIdIndex == m_getByIds.size());
    RELEASE_ASSERT(m_getByIdWithThisIndex == m_getByIdsWithThis.size());
    RELEASE_ASSERT(m_putByIdIndex == m_putByIds.size());
    RELEASE_ASSERT(m_callLinkInfoIndex == m_callCompilationInfo.size());
    RELEASE_ASSERT(numberOfValueProfiles == m_codeBlock->numberOfValueProfiles());

#ifndef NDEBUG
    // Reset this, in order to guard its use with ASSERTs.
    m_bytecodeOffset = std::numeric_limits<unsigned>::max();
#endif
}

void JIT::compileWithoutLinking(JITCompilationEffort effort)
{
    double before = 0;
    if (UNLIKELY(computeCompileTimes()))
        before = monotonicallyIncreasingTimeMS();
    
    {
        ConcurrentJSLocker locker(m_codeBlock->m_lock);
        m_instructions = m_codeBlock->instructions().clone();
    }

    DFG::CapabilityLevel level = m_codeBlock->capabilityLevel();
    switch (level) {
    case DFG::CannotCompile:
        m_canBeOptimized = false;
        m_canBeOptimizedOrInlined = false;
        m_shouldEmitProfiling = false;
        break;
    case DFG::CanCompile:
    case DFG::CanCompileAndInline:
        m_canBeOptimized = true;
        m_canBeOptimizedOrInlined = true;
        m_shouldEmitProfiling = true;
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
    
    switch (m_codeBlock->codeType()) {
    case GlobalCode:
    case ModuleCode:
    case EvalCode:
        m_codeBlock->m_shouldAlwaysBeInlined = false;
        break;
    case FunctionCode:
        // We could have already set it to false because we detected an uninlineable call.
        // Don't override that observation.
        m_codeBlock->m_shouldAlwaysBeInlined &= canInline(level) && DFG::mightInlineFunction(m_codeBlock);
        break;
    }

    if (Options::dumpDisassembly() || (m_vm->m_perBytecodeProfiler && Options::disassembleBaselineForProfiler()))
        m_disassembler = std::make_unique<JITDisassembler>(m_codeBlock);
    if (m_vm->m_perBytecodeProfiler) {
        m_compilation = adoptRef(
            new Profiler::Compilation(
                m_vm->m_perBytecodeProfiler->ensureBytecodesFor(m_codeBlock),
                Profiler::Baseline));
        m_compilation->addProfiledBytecodes(*m_vm->m_perBytecodeProfiler, m_codeBlock);
    }
    
    m_pcToCodeOriginMapBuilder.appendItem(label(), CodeOrigin(0, nullptr));

    if (m_disassembler)
        m_disassembler->setStartOfCode(label());

    // Just add a little bit of randomness to the codegen
    if (m_randomGenerator.getUint32() & 1)
        nop();

    emitFunctionPrologue();
    emitPutToCallFrameHeader(m_codeBlock, CallFrameSlot::codeBlock);

    Label beginLabel(this);

    sampleCodeBlock(m_codeBlock);
#if ENABLE(OPCODE_SAMPLING)
    sampleInstruction(m_codeBlock->instructions().begin());
#endif

    if (m_codeBlock->codeType() == FunctionCode) {
        ASSERT(m_bytecodeOffset == std::numeric_limits<unsigned>::max());
        if (shouldEmitProfiling()) {
            for (int argument = 0; argument < m_codeBlock->numParameters(); ++argument) {
                // If this is a constructor, then we want to put in a dummy profiling site (to
                // keep things consistent) but we don't actually want to record the dummy value.
                if (m_codeBlock->m_isConstructor && !argument)
                    continue;
                int offset = CallFrame::argumentOffsetIncludingThis(argument) * static_cast<int>(sizeof(Register));
#if USE(JSVALUE64)
                load64(Address(callFrameRegister, offset), regT0);
#elif USE(JSVALUE32_64)
                load32(Address(callFrameRegister, offset + OBJECT_OFFSETOF(JSValue, u.asBits.payload)), regT0);
                load32(Address(callFrameRegister, offset + OBJECT_OFFSETOF(JSValue, u.asBits.tag)), regT1);
#endif
                emitValueProfilingSite(m_codeBlock->valueProfileForArgument(argument));
            }
        }
    }

    addPtr(TrustedImm32(stackPointerOffsetFor(m_codeBlock) * sizeof(Register)), callFrameRegister, regT1);
    Jump stackOverflow = branchPtr(Above, AbsoluteAddress(m_vm->addressOfSoftStackLimit()), regT1);

    move(regT1, stackPointerRegister);
    checkStackPointerAlignment();

    emitSaveCalleeSaves();
    emitMaterializeTagCheckRegisters();
    
    RELEASE_ASSERT(!JITCode::isJIT(m_codeBlock->jitType()));

    privateCompileMainPass();
    privateCompileLinkPass();
    privateCompileSlowCases();
    
    if (m_disassembler)
        m_disassembler->setEndOfSlowPath(label());
    m_pcToCodeOriginMapBuilder.appendItem(label(), PCToCodeOriginMapBuilder::defaultCodeOrigin());

    stackOverflow.link(this);
    m_bytecodeOffset = 0;
    if (maxFrameExtentForSlowPathCall)
        addPtr(TrustedImm32(-maxFrameExtentForSlowPathCall), stackPointerRegister);
    callOperationWithCallFrameRollbackOnException(operationThrowStackOverflowError, m_codeBlock);

    if (m_codeBlock->codeType() == FunctionCode) {
        m_arityCheck = label();
        store8(TrustedImm32(0), &m_codeBlock->m_shouldAlwaysBeInlined);
        emitFunctionPrologue();
        emitPutToCallFrameHeader(m_codeBlock, CallFrameSlot::codeBlock);

        load32(payloadFor(CallFrameSlot::argumentCount), regT1);
        branch32(AboveOrEqual, regT1, TrustedImm32(m_codeBlock->m_numParameters)).linkTo(beginLabel, this);

        m_bytecodeOffset = 0;

        if (maxFrameExtentForSlowPathCall)
            addPtr(TrustedImm32(-maxFrameExtentForSlowPathCall), stackPointerRegister);
        callOperationWithCallFrameRollbackOnException(m_codeBlock->m_isConstructor ? operationConstructArityCheck : operationCallArityCheck);
        if (maxFrameExtentForSlowPathCall)
            addPtr(TrustedImm32(maxFrameExtentForSlowPathCall), stackPointerRegister);
        branchTest32(Zero, returnValueGPR).linkTo(beginLabel, this);
        move(returnValueGPR, GPRInfo::argumentGPR0);
        emitNakedCall(m_vm->getCTIStub(arityFixupGenerator).code());

#if !ASSERT_DISABLED
        m_bytecodeOffset = std::numeric_limits<unsigned>::max(); // Reset this, in order to guard its use with ASSERTs.
#endif

        jump(beginLabel);
    }

    ASSERT(m_jmpTable.isEmpty());
    
    privateCompileExceptionHandlers();
    
    if (m_disassembler)
        m_disassembler->setEndOfCode(label());
    m_pcToCodeOriginMapBuilder.appendItem(label(), PCToCodeOriginMapBuilder::defaultCodeOrigin());

    m_linkBuffer = std::unique_ptr<LinkBuffer>(new LinkBuffer(*m_vm, *this, m_codeBlock, effort));

    double after = 0;
    if (UNLIKELY(computeCompileTimes())) {
        after = monotonicallyIncreasingTimeMS();

        if (Options::reportTotalCompileTimes())
            totalBaselineCompileTime += after - before;
    }
    if (UNLIKELY(reportCompileTimes())) {
        CString codeBlockName = toCString(*m_codeBlock);
        
        dataLog("Optimized ", codeBlockName, " with Baseline JIT into ", m_linkBuffer->size(), " bytes in ", after - before, " ms.\n");
    }
}

CompilationResult JIT::link()
{
    LinkBuffer& patchBuffer = *m_linkBuffer;
    
    if (patchBuffer.didFailToAllocate())
        return CompilationFailed;

    // Translate vPC offsets into addresses in JIT generated code, for switch tables.
    for (auto& record : m_switches) {
        unsigned bytecodeOffset = record.bytecodeOffset;

        if (record.type != SwitchRecord::String) {
            ASSERT(record.type == SwitchRecord::Immediate || record.type == SwitchRecord::Character); 
            ASSERT(record.jumpTable.simpleJumpTable->branchOffsets.size() == record.jumpTable.simpleJumpTable->ctiOffsets.size());

            record.jumpTable.simpleJumpTable->ctiDefault = patchBuffer.locationOf(m_labels[bytecodeOffset + record.defaultOffset]);

            for (unsigned j = 0; j < record.jumpTable.simpleJumpTable->branchOffsets.size(); ++j) {
                unsigned offset = record.jumpTable.simpleJumpTable->branchOffsets[j];
                record.jumpTable.simpleJumpTable->ctiOffsets[j] = offset ? patchBuffer.locationOf(m_labels[bytecodeOffset + offset]) : record.jumpTable.simpleJumpTable->ctiDefault;
            }
        } else {
            ASSERT(record.type == SwitchRecord::String);

            record.jumpTable.stringJumpTable->ctiDefault = patchBuffer.locationOf(m_labels[bytecodeOffset + record.defaultOffset]);

            for (auto& location : record.jumpTable.stringJumpTable->offsetTable.values()) {
                unsigned offset = location.branchOffset;
                location.ctiOffset = offset ? patchBuffer.locationOf(m_labels[bytecodeOffset + offset]) : record.jumpTable.stringJumpTable->ctiDefault;
            }
        }
    }

    for (size_t i = 0; i < m_codeBlock->numberOfExceptionHandlers(); ++i) {
        HandlerInfo& handler = m_codeBlock->exceptionHandler(i);
        handler.nativeCode = patchBuffer.locationOf(m_labels[handler.target]);
    }

    for (auto& record : m_calls) {
        if (record.to)
            patchBuffer.link(record.from, FunctionPtr(record.to));
    }

    for (unsigned i = m_getByIds.size(); i--;)
        m_getByIds[i].finalize(patchBuffer);
    for (unsigned i = m_getByIdsWithThis.size(); i--;)
        m_getByIdsWithThis[i].finalize(patchBuffer);
    for (unsigned i = m_putByIds.size(); i--;)
        m_putByIds[i].finalize(patchBuffer);

    if (m_byValCompilationInfo.size()) {
        CodeLocationLabel exceptionHandler = patchBuffer.locationOf(m_exceptionHandler);

        for (const auto& byValCompilationInfo : m_byValCompilationInfo) {
            PatchableJump patchableNotIndexJump = byValCompilationInfo.notIndexJump;
            CodeLocationJump notIndexJump = CodeLocationJump();
            if (Jump(patchableNotIndexJump).isSet())
                notIndexJump = CodeLocationJump(patchBuffer.locationOf(patchableNotIndexJump));
            CodeLocationJump badTypeJump = CodeLocationJump(patchBuffer.locationOf(byValCompilationInfo.badTypeJump));
            CodeLocationLabel doneTarget = patchBuffer.locationOf(byValCompilationInfo.doneTarget);
            CodeLocationLabel nextHotPathTarget = patchBuffer.locationOf(byValCompilationInfo.nextHotPathTarget);
            CodeLocationLabel slowPathTarget = patchBuffer.locationOf(byValCompilationInfo.slowPathTarget);
            CodeLocationCall returnAddress = patchBuffer.locationOf(byValCompilationInfo.returnAddress);

            *byValCompilationInfo.byValInfo = ByValInfo(
                byValCompilationInfo.bytecodeIndex,
                notIndexJump,
                badTypeJump,
                exceptionHandler,
                byValCompilationInfo.arrayMode,
                byValCompilationInfo.arrayProfile,
                differenceBetweenCodePtr(badTypeJump, doneTarget),
                differenceBetweenCodePtr(badTypeJump, nextHotPathTarget),
                differenceBetweenCodePtr(returnAddress, slowPathTarget));
        }
    }

    for (auto& compilationInfo : m_callCompilationInfo) {
        CallLinkInfo& info = *compilationInfo.callLinkInfo;
        info.setCallLocations(
            CodeLocationLabel(patchBuffer.locationOfNearCall(compilationInfo.callReturnLocation)),
            CodeLocationLabel(patchBuffer.locationOf(compilationInfo.hotPathBegin)),
            patchBuffer.locationOfNearCall(compilationInfo.hotPathOther));
    }

    CompactJITCodeMap::Encoder jitCodeMapEncoder;
    for (unsigned bytecodeOffset = 0; bytecodeOffset < m_labels.size(); ++bytecodeOffset) {
        if (m_labels[bytecodeOffset].isSet())
            jitCodeMapEncoder.append(bytecodeOffset, patchBuffer.offsetOf(m_labels[bytecodeOffset]));
    }
    m_codeBlock->setJITCodeMap(jitCodeMapEncoder.finish());

    MacroAssemblerCodePtr withArityCheck;
    if (m_codeBlock->codeType() == FunctionCode)
        withArityCheck = patchBuffer.locationOf(m_arityCheck);

    if (Options::dumpDisassembly()) {
        m_disassembler->dump(patchBuffer);
        patchBuffer.didAlreadyDisassemble();
    }
    if (m_compilation) {
        if (Options::disassembleBaselineForProfiler())
            m_disassembler->reportToProfiler(m_compilation.get(), patchBuffer);
        m_vm->m_perBytecodeProfiler->addCompilation(m_codeBlock, *m_compilation);
    }

    if (m_pcToCodeOriginMapBuilder.didBuildMapping())
        m_codeBlock->setPCToCodeOriginMap(std::make_unique<PCToCodeOriginMap>(WTFMove(m_pcToCodeOriginMapBuilder), patchBuffer));
    
    CodeRef result = FINALIZE_CODE(
        patchBuffer,
        ("Baseline JIT code for %s", toCString(CodeBlockWithJITType(m_codeBlock, JITCode::BaselineJIT)).data()));
    
    m_vm->machineCodeBytesPerBytecodeWordForBaselineJIT->add(
        static_cast<double>(result.size()) /
        static_cast<double>(m_instructions.size()));

    m_codeBlock->shrinkToFit(CodeBlock::LateShrink);
    m_codeBlock->setJITCode(
        adoptRef(*new DirectJITCode(result, withArityCheck, JITCode::BaselineJIT)));

#if ENABLE(JIT_VERBOSE)
    dataLogF("JIT generated code for %p at [%p, %p).\n", m_codeBlock, result.executableMemory()->start(), result.executableMemory()->end());
#endif
    
    return CompilationSuccessful;
}

CompilationResult JIT::privateCompile(JITCompilationEffort effort)
{
    base::debug::TraceScope traceScope("jsc", "JIT::privateCompile");
    doMainThreadPreparationBeforeCompile();
    compileWithoutLinking(effort);
    return link();
}

void JIT::privateCompileExceptionHandlers()
{
    if (!m_exceptionChecksWithCallFrameRollback.empty()) {
        m_exceptionChecksWithCallFrameRollback.link(this);

        copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();

        // lookupExceptionHandlerFromCallerFrame is passed two arguments, the VM and the exec (the CallFrame*).

        move(TrustedImmPtr(vm()), GPRInfo::argumentGPR0);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR1);

#if CPU(X86)
        // FIXME: should use the call abstraction, but this is currently in the SpeculativeJIT layer!
        poke(GPRInfo::argumentGPR0);
        poke(GPRInfo::argumentGPR1, 1);
#endif
        m_calls.append(CallRecord(call(), std::numeric_limits<unsigned>::max(), FunctionPtr(lookupExceptionHandlerFromCallerFrame).value()));
        jumpToExceptionHandler();
    }

    if (!m_exceptionChecks.empty() || m_byValCompilationInfo.size()) {
        m_exceptionHandler = label();
        m_exceptionChecks.link(this);

        copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();

        // lookupExceptionHandler is passed two arguments, the VM and the exec (the CallFrame*).
        move(TrustedImmPtr(vm()), GPRInfo::argumentGPR0);
        move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR1);

#if CPU(X86)
        // FIXME: should use the call abstraction, but this is currently in the SpeculativeJIT layer!
        poke(GPRInfo::argumentGPR0);
        poke(GPRInfo::argumentGPR1, 1);
#endif
        m_calls.append(CallRecord(call(), std::numeric_limits<unsigned>::max(), FunctionPtr(lookupExceptionHandler).value()));
        jumpToExceptionHandler();
    }
}

void JIT::doMainThreadPreparationBeforeCompile()
{
    // This ensures that we have the most up to date type information when performing typecheck optimizations for op_profile_type.
    if (m_vm->typeProfiler())
        m_vm->typeProfilerLog()->processLogEntries(ASCIILiteral("Preparing for JIT compilation."));
}

unsigned JIT::frameRegisterCountFor(CodeBlock* codeBlock)
{
    ASSERT(static_cast<unsigned>(codeBlock->m_numCalleeLocals) == WTF::roundUpToMultipleOf(stackAlignmentRegisters(), static_cast<unsigned>(codeBlock->m_numCalleeLocals)));

    return roundLocalRegisterCountForFramePointerOffset(codeBlock->m_numCalleeLocals + maxFrameExtentForSlowPathCallInRegisters);
}

int JIT::stackPointerOffsetFor(CodeBlock* codeBlock)
{
    return virtualRegisterForLocal(frameRegisterCountFor(codeBlock) - 1).offset();
}

bool JIT::reportCompileTimes()
{
    return Options::reportCompileTimes() || Options::reportBaselineCompileTimes();
}

bool JIT::computeCompileTimes()
{
    return reportCompileTimes() || Options::reportTotalCompileTimes();
}

HashMap<CString, double> JIT::compileTimeStats()
{
    HashMap<CString, double> result;
    if (Options::reportTotalCompileTimes()) {
        result.add("Total Compile Time", totalBaselineCompileTime + totalDFGCompileTime + totalFTLCompileTime);
        result.add("Baseline Compile Time", totalBaselineCompileTime);
#if ENABLE(DFG_JIT)
        result.add("DFG Compile Time", totalDFGCompileTime);
#if ENABLE(FTL_JIT)
        result.add("FTL Compile Time", totalFTLCompileTime);
        result.add("FTL (DFG) Compile Time", totalFTLDFGCompileTime);
        result.add("FTL (B3) Compile Time", totalFTLB3CompileTime);
#endif // ENABLE(FTL_JIT)
#endif // ENABLE(DFG_JIT)
    }
    return result;
}

} // namespace JSC

#endif // ENABLE(JIT)
