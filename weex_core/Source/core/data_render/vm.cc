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

#define LOGE(...)

#define INT_OP(op, v1, v2) CAST_U2S((CAST_S2U(v1)) op CAST_S2U(v2))
#define NUM_OP(op, d1, d2) ((d1) op (d2))
#define MATH_OP(op) op
#define CAST_S2U(o) ((unsigned)(o))
#define CAST_U2S(o) ((signed)(o))

namespace weex {
namespace core {
namespace data_render {

inline bool IsInt(Value *o) { return Value::Type::INT == o->type; }

inline int IntMod(int a, int b) {
    if (CAST_S2U(b) + 1u <= 1u) {
        if (b == 0) {
            LOGE("Error ValueMod Values[", a, b);
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

inline bool IsNumber(Value *o) { return Value::Type::NUMBER == o->type; }

inline int64_t IntValue(Value *o) { return o->i; }

inline double NumValue(Value *o) { return o->n; }

inline int ToNumber_(Value *value, double *ret) {
    if (IsInt(value)) {
        *ret = IntValue(value);
        return 1;
    } else if (IsNumber(value)) {
        *ret = NumValue(value);
        return 1;
    } else {
        return -1;
    }
}

inline int ToNum(Value *o, double *n) {
    return IsNumber(o) ? (*n = NumValue(o), 1) : ToNumber_(o, n);
}

inline void SetIValue(Value *o, int iv) {
    o->type = Value::Type::INT;
    o->i = iv;
}

inline void SetDValue(Value *o, double d) {
    o->type = Value::Type::NUMBER;
    o->n = d;
}

inline double NumPow(double d1, double d2) {
    return MATH_OP(pow)(d1, d2);
}

inline double NumIDiv(double d1, double d2) {
    return MATH_OP(floor)(NUM_OP(/, d1, d2));
}

inline double NumMod(double d1, double d2) {
    double ret = MATH_OP(fmod)(d1, d2);
    if (ret * d2 < 0) ret += d2;
    return ret;
}

void VM::RunFrame(ExecState *exec_state, Frame frame) {
    Value *a = nullptr;
    Value *b = nullptr;
    Value *c = nullptr;
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
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, INT_OP(+, IntValue(b), IntValue(c)));
                } else if (ToNum(b, &d1) && ToNum(c, &d2)) {
                    SetDValue(a, NUM_OP(+, d1, d2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_ADD]", b->type, c->type);
                }
                break;

            case OP_SUB:
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, INT_OP(-, IntValue(b), IntValue(c)));
                } else if (ToNum(b, &d1) && ToNum(c, &d2)) {
                    SetDValue(a, NUM_OP(-, d1, d2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_SUB]", b->type, c->type);
                }
                break;

            case OP_MUL:
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, INT_OP(*, IntValue(b), IntValue(c)));
                } else if (ToNum(b, &d1) && ToNum(c, &d2)) {
                    SetDValue(a, NUM_OP(*, d1, d2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_MUL]", b->type, c->type);
                }
                break;

            case OP_DIV:
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, NUM_OP(/, IntValue(b), IntValue(c)));
                } else if (ToNum(b, &d1) && ToNum(c, &d2)) {
                    SetDValue(a, NUM_OP(/, IntValue(b), IntValue(c)));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_DIV]", b->type, c->type);
                }
                break;

            case OP_IDIV:
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, INT_OP(/, IntValue(b), IntValue(c)));
                } else if (ToNum(b, &d1) && ToNum(c, &d2)) {
                    SetDValue(a, NumIDiv(d1, d2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_IDIV]", b->type, c->type);
                }
                break;

            case OP_MOD:
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, IntMod(IntValue(b), IntValue(c)));
                } else if (ToNum(b, &d1) && ToNum(c, &d2)) {
                    SetDValue(a, NumMod(d1, d2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_MOD]", b->type, c->type);
                }
                break;

            case OP_POW:
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, NumPow(IntValue(b), IntValue(c)));
                } else if (ToNum(b, &d1) && ToNum(c, &d2)) {
                    SetDValue(a, NumPow(d1, d2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_POW]", b->type, c->type);
                }
                break;

            case OP_CALL: {
                a = frame.reg + GET_ARG_A(instruction);
                size_t argc = GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                exec_state->CallFunction(c, argc, a);
            }
                break;
            default:
                break;
        }
    }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex