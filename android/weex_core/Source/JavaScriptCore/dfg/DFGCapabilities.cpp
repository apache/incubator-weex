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
#include "DFGCapabilities.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "DFGCommon.h"
#include "Interpreter.h"
#include "JSCInlines.h"
#include "Options.h"

namespace JSC { namespace DFG {

bool isSupported()
{
    return Options::useDFGJIT()
        && MacroAssembler::supportsFloatingPoint();
}

bool isSupportedForInlining(CodeBlock* codeBlock)
{
    return codeBlock->ownerScriptExecutable()->isInliningCandidate();
}

bool mightCompileEval(CodeBlock* codeBlock)
{
    return isSupported()
        && codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount()
        && codeBlock->ownerScriptExecutable()->isOkToOptimize();
}
bool mightCompileProgram(CodeBlock* codeBlock)
{
    return isSupported()
        && codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount()
        && codeBlock->ownerScriptExecutable()->isOkToOptimize();
}
bool mightCompileFunctionForCall(CodeBlock* codeBlock)
{
    return isSupported()
        && codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount()
        && codeBlock->ownerScriptExecutable()->isOkToOptimize();
}
bool mightCompileFunctionForConstruct(CodeBlock* codeBlock)
{
    return isSupported()
        && codeBlock->instructionCount() <= Options::maximumOptimizationCandidateInstructionCount()
        && codeBlock->ownerScriptExecutable()->isOkToOptimize();
}

bool mightInlineFunctionForCall(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumFunctionForCallInlineCandidateInstructionCount()
        && isSupportedForInlining(codeBlock);
}
bool mightInlineFunctionForClosureCall(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumFunctionForClosureCallInlineCandidateInstructionCount()
        && isSupportedForInlining(codeBlock);
}
bool mightInlineFunctionForConstruct(CodeBlock* codeBlock)
{
    return codeBlock->instructionCount() <= Options::maximumFunctionForConstructInlineCandidateInstructionCount()
        && isSupportedForInlining(codeBlock);
}
bool canUseOSRExitFuzzing(CodeBlock* codeBlock)
{
    return codeBlock->ownerScriptExecutable()->canUseOSRExitFuzzing();
}

inline void debugFail(CodeBlock* codeBlock, OpcodeID opcodeID, CapabilityLevel result)
{
    if (Options::verboseCompilation() && !canCompile(result))
        dataLog("Cannot compile code block ", *codeBlock, " because of opcode ", opcodeNames[opcodeID], "\n");
}

CapabilityLevel capabilityLevel(OpcodeID opcodeID, CodeBlock* codeBlock, Instruction* pc)
{
    UNUSED_PARAM(codeBlock); // This function does some bytecode parsing. Ordinarily bytecode parsing requires the owning CodeBlock. It's sort of strange that we don't use it here right now.
    UNUSED_PARAM(pc);
    
    switch (opcodeID) {
    case op_enter:
    case op_to_this:
    case op_argument_count:
    case op_check_tdz:
    case op_create_this:
    case op_bitand:
    case op_bitor:
    case op_bitxor:
    case op_rshift:
    case op_lshift:
    case op_urshift:
    case op_unsigned:
    case op_inc:
    case op_dec:
    case op_add:
    case op_sub:
    case op_negate:
    case op_mul:
    case op_mod:
    case op_pow:
    case op_div:
    case op_debug:
    case op_profile_type:
    case op_profile_control_flow:
    case op_mov:
    case op_overrides_has_instance:
    case op_instanceof:
    case op_instanceof_custom:
    case op_is_empty:
    case op_is_undefined:
    case op_is_boolean:
    case op_is_number:
    case op_is_object:
    case op_is_object_or_null:
    case op_is_cell_with_type:
    case op_is_function:
    case op_not:
    case op_less:
    case op_lesseq:
    case op_greater:
    case op_greatereq:
    case op_eq:
    case op_eq_null:
    case op_stricteq:
    case op_neq:
    case op_neq_null:
    case op_nstricteq:
    case op_get_by_val:
    case op_put_by_val:
    case op_put_by_val_direct:
    case op_try_get_by_id:
    case op_get_by_id:
    case op_get_by_id_proto_load:
    case op_get_by_id_unset:
    case op_get_by_id_with_this:
    case op_get_by_val_with_this:
    case op_get_array_length:
    case op_put_by_id:
    case op_put_by_id_with_this:
    case op_put_by_val_with_this:
    case op_put_getter_by_id:
    case op_put_setter_by_id:
    case op_put_getter_setter_by_id:
    case op_put_getter_by_val:
    case op_put_setter_by_val:
    case op_define_data_property:
    case op_define_accessor_property:
    case op_del_by_id:
    case op_del_by_val:
    case op_jmp:
    case op_jtrue:
    case op_jfalse:
    case op_jeq_null:
    case op_jneq_null:
    case op_jless:
    case op_jlesseq:
    case op_jgreater:
    case op_jgreatereq:
    case op_jnless:
    case op_jnlesseq:
    case op_jngreater:
    case op_jngreatereq:
    case op_loop_hint:
    case op_check_traps:
    case op_ret:
    case op_end:
    case op_new_object:
    case op_new_array:
    case op_new_array_with_size:
    case op_new_array_buffer:
    case op_new_array_with_spread:
    case op_spread:
    case op_strcat:
    case op_to_primitive:
    case op_throw:
    case op_throw_static_error:
    case op_call:
    case op_tail_call:
    case op_construct:
    case op_call_varargs:
    case op_tail_call_varargs:
    case op_tail_call_forward_arguments:
    case op_construct_varargs:
    case op_create_direct_arguments:
    case op_create_scoped_arguments:
    case op_create_cloned_arguments:
    case op_get_from_arguments:
    case op_put_to_arguments:
    case op_get_argument:
    case op_jneq_ptr:
    case op_typeof:
    case op_to_number:
    case op_to_string:
    case op_switch_imm:
    case op_switch_char:
    case op_in:
    case op_get_scope:
    case op_get_from_scope:
    case op_get_enumerable_length:
    case op_has_generic_property:
    case op_has_structure_property:
    case op_has_indexed_property:
    case op_get_direct_pname:
    case op_get_property_enumerator:
    case op_enumerator_structure_pname:
    case op_enumerator_generic_pname:
    case op_to_index_string:
    case op_new_func:
    case op_new_func_exp:
    case op_new_generator_func:
    case op_new_generator_func_exp:
    case op_new_async_func:
    case op_new_async_func_exp:
    case op_set_function_name:
    case op_create_lexical_environment:
    case op_get_parent_scope:
    case op_catch:
    case op_create_rest:
    case op_get_rest_length:
    case op_log_shadow_chicken_prologue:
    case op_log_shadow_chicken_tail:
    case op_put_to_scope:
    case op_resolve_scope:
    case op_new_regexp:
        return CanCompileAndInline;

    case op_switch_string: // Don't inline because we don't want to copy string tables in the concurrent JIT.
    case op_call_eval:
        return CanCompile;

    default:
        return CannotCompile;
    }
}

CapabilityLevel capabilityLevel(CodeBlock* codeBlock)
{
    Interpreter* interpreter = codeBlock->vm()->interpreter;
    Instruction* instructionsBegin = codeBlock->instructions().begin();
    unsigned instructionCount = codeBlock->instructions().size();
    CapabilityLevel result = CanCompileAndInline;
    
    for (unsigned bytecodeOffset = 0; bytecodeOffset < instructionCount; ) {
        switch (interpreter->getOpcodeID(instructionsBegin[bytecodeOffset].u.opcode)) {
#define DEFINE_OP(opcode, length) \
        case opcode: { \
            CapabilityLevel newResult = leastUpperBound(result, capabilityLevel(opcode, codeBlock, instructionsBegin + bytecodeOffset)); \
            if (newResult != result) { \
                debugFail(codeBlock, opcode, newResult); \
                result = newResult; \
            } \
            bytecodeOffset += length; \
            break; \
        }
            FOR_EACH_OPCODE_ID(DEFINE_OP)
#undef DEFINE_OP
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
    }
    
    return result;
}

} } // namespace JSC::DFG

#endif
