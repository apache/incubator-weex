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

#include "CodeBlock.h"
#include "Interpreter.h"

namespace JSC {

template<typename Block, typename Functor, typename Instruction>
void computeUsesForBytecodeOffset(Block* codeBlock, OpcodeID opcodeID, Instruction* instruction, const Functor& functor)
{
    if (opcodeID != op_enter && codeBlock->wasCompiledWithDebuggingOpcodes() && codeBlock->scopeRegister().isValid())
        functor(codeBlock, instruction, opcodeID, codeBlock->scopeRegister().offset());

    switch (opcodeID) {
    // No uses.
    case op_new_regexp:
    case op_new_array_buffer:
    case op_throw_static_error:
    case op_debug:
    case op_jneq_ptr:
    case op_loop_hint:
    case op_jmp:
    case op_new_object:
    case op_enter:
    case op_argument_count:
    case op_catch:
    case op_profile_control_flow:
    case op_create_direct_arguments:
    case op_create_cloned_arguments:
    case op_get_rest_length:
    case op_check_traps:
    case op_get_argument:
        return;
    case op_assert:
    case op_get_scope:
    case op_to_this:
    case op_check_tdz:
    case op_profile_type:
    case op_throw:
    case op_end:
    case op_ret:
    case op_jtrue:
    case op_jfalse:
    case op_jeq_null:
    case op_jneq_null:
    case op_dec:
    case op_inc:
    case op_log_shadow_chicken_prologue: {
        ASSERT(opcodeLengths[opcodeID] > 1);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        return;
    }
    case op_jlesseq:
    case op_jgreater:
    case op_jgreatereq:
    case op_jnless:
    case op_jnlesseq:
    case op_jngreater:
    case op_jngreatereq:
    case op_jless:
    case op_set_function_name:
    case op_log_shadow_chicken_tail: {
        ASSERT(opcodeLengths[opcodeID] > 2);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        return;
    }
    case op_put_by_val_direct:
    case op_put_by_val: {
        ASSERT(opcodeLengths[opcodeID] > 3);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        return;
    }
    case op_put_by_index:
    case op_put_by_id:
    case op_put_to_scope:
    case op_put_to_arguments: {
        ASSERT(opcodeLengths[opcodeID] > 3);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        return;
    }
    case op_put_by_id_with_this: {
        ASSERT(opcodeLengths[opcodeID] > 4);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        return;
    }
    case op_put_by_val_with_this: {
        ASSERT(opcodeLengths[opcodeID] > 4);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        return;
    }
    case op_put_getter_by_id:
    case op_put_setter_by_id: {
        ASSERT(opcodeLengths[opcodeID] > 4);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        return;
    }
    case op_put_getter_setter_by_id: {
        ASSERT(opcodeLengths[opcodeID] > 5);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[5].u.operand);
        return;
    }
    case op_put_getter_by_val:
    case op_put_setter_by_val: {
        ASSERT(opcodeLengths[opcodeID] > 4);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        return;
    }
    case op_define_data_property: {
        ASSERT(opcodeLengths[opcodeID] > 4);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        return;
    }
    case op_define_accessor_property: {
        ASSERT(opcodeLengths[opcodeID] > 5);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[5].u.operand);
        return;
    }
    case op_spread:
    case op_get_property_enumerator:
    case op_get_enumerable_length:
    case op_new_func_exp:
    case op_new_generator_func_exp:
    case op_new_async_func_exp:
    case op_to_index_string:
    case op_create_lexical_environment:
    case op_resolve_scope:
    case op_get_from_scope:
    case op_to_primitive:
    case op_try_get_by_id:
    case op_get_by_id:
    case op_get_by_id_proto_load:
    case op_get_by_id_unset:
    case op_get_array_length:
    case op_typeof:
    case op_is_empty:
    case op_is_undefined:
    case op_is_boolean:
    case op_is_number:
    case op_is_object:
    case op_is_object_or_null:
    case op_is_cell_with_type:
    case op_is_function:
    case op_to_number:
    case op_to_string:
    case op_negate:
    case op_neq_null:
    case op_eq_null:
    case op_not:
    case op_mov:
    case op_new_array_with_size:
    case op_create_this:
    case op_del_by_id:
    case op_unsigned:
    case op_new_func:
    case op_new_generator_func:
    case op_new_async_func:
    case op_get_parent_scope:
    case op_create_scoped_arguments:
    case op_create_rest:
    case op_get_from_arguments: {
        ASSERT(opcodeLengths[opcodeID] > 2);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        return;
    }
    case op_has_generic_property:
    case op_has_indexed_property:
    case op_enumerator_structure_pname:
    case op_enumerator_generic_pname:
    case op_get_by_val:
    case op_in:
    case op_overrides_has_instance:
    case op_instanceof:
    case op_add:
    case op_mul:
    case op_div:
    case op_mod:
    case op_sub:
    case op_pow:
    case op_lshift:
    case op_rshift:
    case op_urshift:
    case op_bitand:
    case op_bitxor:
    case op_bitor:
    case op_less:
    case op_lesseq:
    case op_greater:
    case op_greatereq:
    case op_nstricteq:
    case op_stricteq:
    case op_neq:
    case op_eq:
    case op_push_with_scope:
    case op_get_by_id_with_this:
    case op_del_by_val:
    case op_tail_call_forward_arguments: {
        ASSERT(opcodeLengths[opcodeID] > 3);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        return;
    }
    case op_get_by_val_with_this: {
        ASSERT(opcodeLengths[opcodeID] > 4);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        return;
    }
    case op_instanceof_custom:
    case op_has_structure_property:
    case op_construct_varargs:
    case op_call_varargs:
    case op_tail_call_varargs: {
        ASSERT(opcodeLengths[opcodeID] > 4);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        return;
    }
    case op_get_direct_pname: {
        ASSERT(opcodeLengths[opcodeID] > 5);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[4].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[5].u.operand);
        return;
    }
    case op_switch_string:
    case op_switch_char:
    case op_switch_imm: {
        ASSERT(opcodeLengths[opcodeID] > 3);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        return;
    }
    case op_new_array_with_spread:
    case op_new_array:
    case op_strcat: {
        int base = instruction[2].u.operand;
        int count = instruction[3].u.operand;
        for (int i = 0; i < count; i++)
            functor(codeBlock, instruction, opcodeID, base - i);
        return;
    }
    case op_construct:
    case op_call_eval:
    case op_call:
    case op_tail_call: {
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        int argCount = instruction[3].u.operand;
        int registerOffset = -instruction[4].u.operand;
        int lastArg = registerOffset + CallFrame::thisArgumentOffset();
        for (int i = 0; i < argCount; i++)
            functor(codeBlock, instruction, opcodeID, lastArg + i);
        if (opcodeID == op_call_eval)
            functor(codeBlock, instruction, opcodeID, codeBlock->scopeRegister().offset());
        return;
    }
    case op_yield: {
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[3].u.operand);
        return;
    }
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }
}

template<typename Block, typename Instruction, typename Functor>
void computeDefsForBytecodeOffset(Block* codeBlock, OpcodeID opcodeID, Instruction* instruction, const Functor& functor)
{
    switch (opcodeID) {
    // These don't define anything.
    case op_put_to_scope:
    case op_end:
    case op_throw:
    case op_throw_static_error:
    case op_assert:
    case op_debug:
    case op_ret:
    case op_jmp:
    case op_jtrue:
    case op_jfalse:
    case op_jeq_null:
    case op_jneq_null:
    case op_jneq_ptr:
    case op_jless:
    case op_jlesseq:
    case op_jgreater:
    case op_jgreatereq:
    case op_jnless:
    case op_jnlesseq:
    case op_jngreater:
    case op_jngreatereq:
    case op_loop_hint:
    case op_switch_imm:
    case op_switch_char:
    case op_switch_string:
    case op_put_by_id:
    case op_put_by_id_with_this:
    case op_put_by_val_with_this:
    case op_put_getter_by_id:
    case op_put_setter_by_id:
    case op_put_getter_setter_by_id:
    case op_put_getter_by_val:
    case op_put_setter_by_val:
    case op_put_by_val:
    case op_put_by_val_direct:
    case op_put_by_index:
    case op_define_data_property:
    case op_define_accessor_property:
    case op_profile_type:
    case op_profile_control_flow:
    case op_put_to_arguments:
    case op_set_function_name:
    case op_check_traps:
    case op_log_shadow_chicken_prologue:
    case op_log_shadow_chicken_tail:
    case op_yield:
#define LLINT_HELPER_OPCODES(opcode, length) case opcode:
        FOR_EACH_LLINT_OPCODE_EXTENSION(LLINT_HELPER_OPCODES);
#undef LLINT_HELPER_OPCODES
        return;
    // These all have a single destination for the first argument.
    case op_argument_count:
    case op_to_index_string:
    case op_get_enumerable_length:
    case op_has_indexed_property:
    case op_has_structure_property:
    case op_has_generic_property:
    case op_get_direct_pname:
    case op_get_property_enumerator:
    case op_enumerator_structure_pname:
    case op_enumerator_generic_pname:
    case op_get_parent_scope:
    case op_push_with_scope:
    case op_create_lexical_environment:
    case op_resolve_scope:
    case op_strcat:
    case op_to_primitive:
    case op_create_this:
    case op_new_array:
    case op_new_array_with_spread:
    case op_spread:
    case op_new_array_buffer:
    case op_new_array_with_size:
    case op_new_regexp:
    case op_new_func:
    case op_new_func_exp:
    case op_new_generator_func:
    case op_new_generator_func_exp:
    case op_new_async_func:
    case op_new_async_func_exp:
    case op_call_varargs:
    case op_tail_call_varargs:
    case op_tail_call_forward_arguments:
    case op_construct_varargs:
    case op_get_from_scope:
    case op_call:
    case op_tail_call:
    case op_call_eval:
    case op_construct:
    case op_try_get_by_id:
    case op_get_by_id:
    case op_get_by_id_proto_load:
    case op_get_by_id_unset:
    case op_get_by_id_with_this:
    case op_get_by_val_with_this:
    case op_get_array_length:
    case op_overrides_has_instance:
    case op_instanceof:
    case op_instanceof_custom:
    case op_get_by_val:
    case op_typeof:
    case op_is_empty:
    case op_is_undefined:
    case op_is_boolean:
    case op_is_number:
    case op_is_object:
    case op_is_object_or_null:
    case op_is_cell_with_type:
    case op_is_function:
    case op_in:
    case op_to_number:
    case op_to_string:
    case op_negate:
    case op_add:
    case op_mul:
    case op_div:
    case op_mod:
    case op_sub:
    case op_pow:
    case op_lshift:
    case op_rshift:
    case op_urshift:
    case op_bitand:
    case op_bitxor:
    case op_bitor:
    case op_inc:
    case op_dec:
    case op_eq:
    case op_neq:
    case op_stricteq:
    case op_nstricteq:
    case op_less:
    case op_lesseq:
    case op_greater:
    case op_greatereq:
    case op_neq_null:
    case op_eq_null:
    case op_not:
    case op_mov:
    case op_new_object:
    case op_to_this:
    case op_check_tdz:
    case op_get_scope:
    case op_create_direct_arguments:
    case op_create_scoped_arguments:
    case op_create_cloned_arguments:
    case op_del_by_id:
    case op_del_by_val:
    case op_unsigned:
    case op_get_from_arguments: 
    case op_get_argument:
    case op_create_rest:
    case op_get_rest_length: {
        ASSERT(opcodeLengths[opcodeID] > 1);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        return;
    }
    case op_catch: {
        ASSERT(opcodeLengths[opcodeID] > 2);
        functor(codeBlock, instruction, opcodeID, instruction[1].u.operand);
        functor(codeBlock, instruction, opcodeID, instruction[2].u.operand);
        return;
    }
    case op_enter: {
        for (unsigned i = codeBlock->m_numVars; i--;)
            functor(codeBlock, instruction, opcodeID, virtualRegisterForLocal(i).offset());
        return;
    }
    }
}

} // namespace JSC
