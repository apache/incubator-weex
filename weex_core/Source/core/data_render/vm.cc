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

#define IS_INT(o) (Value::Type::INT == o->type)
#define IS_NUMBER(o) (Value::Type::NUMBER == o->type)
#define INT_VALUE(o) (o->i)
#define NUM_VALUE(o) (o->n)
#define TO_NUM(o, n) (IS_NUMBER(o) ? (*(n) = NUM_VALUE(o), 1) : ToNumber(o, n))
#define SET_I_VALUE(o, iv) {o->type=Value::Type::INT;o->i=iv;}
#define SET_D_VALUE(o, d) {o->type=Value::Type::NUMBER;o->n=d;}
#define INT_OP(op, v1, v2) CAST_U2S((CAST_S2U(v1)) op CAST_S2U(v2))
#define NUM_OP(op, d1, d2) ((d1) op (d2))
#define NUM_POW(d1, d2) (MATH_OP(pow)(d1, d2))
#define NUM_I_DIV(d1, d2) (MATH_OP(floor)(NUM_OP(/, d1, d2)))
#define NUM_MOD(d1, d2, ret) \
    {(ret) = MATH_OP(fmod)(d1, d2); if ((ret) * (d2) < 0) (ret) += (d2);}
#define MATH_OP(op) op
#define CAST_S2U(o) ((unsigned)(o))
#define CAST_U2S(o) ((signed)(o))

namespace weex {
namespace core {
namespace data_render {

int ToNumber(Value*, double*);
int ValueMod(int, int);

void VM::RunFrame(ExecState* exec_state, Frame frame) {
  Value* a = nullptr;
  Value* b = nullptr;
  Value* c = nullptr;
  auto pc = frame.pc;
  while (pc != frame.end) {
    Instruction instruction = *pc++;
    double d1, d2;
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
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (IS_INT(b) && IS_INT(c)) {
          SET_I_VALUE(a, INT_OP(+, INT_VALUE(b), INT_VALUE(c)));
        } else if (TO_NUM(b, &d1) && TO_NUM(c, &d2)) {
          SET_D_VALUE(a, NUM_OP(+, d1, d2));
        } else {
          //TODO error
        }
        break;

      case OP_SUB:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (IS_INT(b) && IS_INT(c)) {
          SET_I_VALUE(a, INT_OP(-, INT_VALUE(b), INT_VALUE(c)));
        } else if (TO_NUM(b, &d1) && TO_NUM(c, &d2)) {
          SET_D_VALUE(a, NUM_OP(-, d1, d2));
        } else {
          //TODO error
        }
        break;

      case OP_MUL:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (IS_INT(b) && IS_INT(c)) {
          SET_I_VALUE(a, INT_OP(*, INT_VALUE(b), INT_VALUE(c)));
        } else if (TO_NUM(b, &d1) && TO_NUM(c, &d2)) {
          SET_D_VALUE(a, NUM_OP(*, d1, d2));
        } else {
          //TODO error
        }
        break;

        case OP_DIV:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (TO_NUM(b, &d1) && TO_NUM(c, &d2)) {
          SET_D_VALUE(a, NUM_OP(/, INT_VALUE(b), INT_VALUE(c)));
        } else {
          //TODO error
        }
        break;

        case OP_IDIV:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (IS_INT(b) && IS_INT(c)) {
            SET_I_VALUE(a, INT_OP(/, INT_VALUE(b), INT_VALUE(c)));
        } else if (TO_NUM(b, &d1) && TO_NUM(c, &d2)) {
            SET_D_VALUE(a, NUM_I_DIV(d1, d2));
        } else {
            //TODO error
        }
        break;

      case OP_MOD:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (IS_INT(b) && IS_INT(c)) {
            SET_I_VALUE(a, ValueMod(INT_VALUE(b), INT_VALUE(c)));
        } else if (TO_NUM(b, &d1) && TO_NUM(c, &d2)) {
            double d3 = 0;
            NUM_MOD(d1, d2, d3);
            SET_D_VALUE(a, d3);
        } else {
            //TODO error
        }
        break;

      case OP_POW:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (TO_NUM(b, &d1) && TO_NUM(c, &d2)) {
          SET_D_VALUE(a, NUM_POW(d1, d2));
        } else {
            //TODO
        }
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

int ToNumber(Value* value, double* ret) {
  if (IS_INT(value)) {
    *ret = INT_VALUE(value);
    return 1;
  } else if (IS_NUMBER(value)) {
    *ret = NUM_VALUE(value);
    return 1;
  } else {
    return -1;
  }
}

int ValueMod(int a, int b) {
    if (CAST_S2U(b) + 1u <= 1u) {
        if (b == 0) {
            //TODO error
        }
        return 0;
    } else {
        int ret = a % b;
        if (ret != 0 && (a ^ b) < 0) {
            ret += b;
        }
        return ret;
    }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex