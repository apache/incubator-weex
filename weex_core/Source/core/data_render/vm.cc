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
#include "core/data_render/vm.h"
#include "core/data_render/exec_state.h"

namespace weex {
namespace core {
namespace data_render {

void VM::RunFrame(ExecState* exec_state, Frame frame) {
  Value* a = nullptr;
  Value* b = nullptr;
  Value* c = nullptr;
  auto pc = frame.pc;
  while (pc != frame.end) {
    Instruction instruction = *pc++;
    switch (GET_OP_CODE(instruction)) {
      case OP_MOVE:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        *a = *b;
        break;
      case OP_LOADNULL:
        a = frame.reg + GET_ARG_A(instruction);
        a->type = Value::Type::NIL;
        break;
      case OP_LOADK:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.func->f->GetConstant(GET_ARG_B(instruction));
        *a = *b;
        break;
      case OP_GETGLOBAL:
        a = frame.reg + GET_ARG_A(instruction);
        b = exec_state->global()->Find(GET_ARG_B(instruction));
        *a = *b;
        break;
      case OP_ADD:
        break;
      case OP_SUB:
        break;
      case OP_MUL:
        break;
      case OP_MOD:
        break;
      case OP_POW:
        break;
      case OP_CALL: {
        a = frame.reg + GET_ARG_A(instruction);
        size_t argc = GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        exec_state->CallFunction(c, argc, a);
      } break;
      default:
        break;
    }
  }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex