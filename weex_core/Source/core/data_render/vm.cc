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

#define LOGE(...) ((void)0)

#define INT_OP(op, v1, v2) CAST_U2S((CAST_S2U(v1)) op CAST_S2U(v2))
#define NUM_OP(op, d1, d2) ((d1) op (d2))
#define MATH_OP(op) op
#define CAST_S2U(o) ((unsigned)(o))
#define CAST_U2S(o) ((signed)(o))
#define CAST(t, exp) ((t)(exp))
#define CAST_INT(i) CAST(int64_t, (i))
#define NUM_BITS CAST_INT(sizeof(int64_t) * CHAR_BIT)

namespace weex {
namespace core {
namespace data_render {

inline bool IsInt(const Value *o) { return Value::Type::INT == o->type; }

inline int IntMod(const int &a, const int &b) {
    if (CAST_S2U(b) + 1u <= 1u) {
        if (b == 0) {
            LOGE("Error ValueMod Values[%d, %d]", a, b);
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

inline bool IsNumber(const Value *o) { return Value::Type::NUMBER == o->type; }

inline bool IsBool(const Value *o) { return Value::Type::BOOL == o->type;}

inline int64_t IntValue(const Value *o) { return o->i; }

inline double NumValue(const Value *o) { return o->n; }

inline bool BoolValue(const Value *o) { return (Value::Type::BOOL == o->type) ? o->b : false;}

inline int ToNumber_(const Value *value, double &ret) {
    if (IsInt(value)) {
        ret = IntValue(value);
        return 1;
    } else if (IsNumber(value)) {
        ret = NumValue(value);
        return 1;
    } else {
        return 0;
    }
}

inline int ToNum(const Value *o, double &n) {
    return IsNumber(o) ? (n = NumValue(o), 1) : ToNumber_(o, n);
}

int ToBool(const Value *o, bool &b) {
    double d1;
    if (Value::Type::BOOL == o->type) {
        b = BoolValue(o);
    } else if (Value::Type::INT == o->type) {
        b = IntValue(o);
    } else if (Value::Type::NUMBER == o->type) {
        b = NumValue(o);
    } else if (ToNum(o, d1)) {
        b = d1;
    } else {
        b = false;
        return 0;
    }
    return 1;
}

inline void SetIValue(Value *o, int iv) {
    o->type = Value::Type::INT;
    o->i = iv;
}

inline void SetDValue(Value *o, double d) {
    o->type = Value::Type::NUMBER;
    o->n = d;
}

inline void SetBValue(Value *o, bool b) {
    o->type = Value::Type::BOOL;
    o->b = b;
}

inline double NumPow(const double &d1, const double &d2) {
    return MATH_OP(pow)(d1, d2);
}

inline double NumIDiv(const double &d1, const double &d2) {
    return MATH_OP(floor)(NUM_OP(/, d1, d2));
}

inline double NumMod(const double &d1, const double &d2) {
    double ret = MATH_OP(fmod)(d1, d2);
    if (ret * d2 < 0) ret += d2;
    return ret;
}

inline bool NumEq(const double &d1, const double &d2) {
    return d1 == d2;
}

inline bool NumLT(const double &d1, const double &d2) {
    return d1 < d2;
}

inline int Number2Int(const double &n, int64_t &p) {
    if (n >= MININTEGER && n < -MININTEGER) {
        p = n;
        return 1;
    }
    return 0;
}

/*
** try to convert a value to an integer, rounding according to 'mode':
** mode == 0: accepts only integral values
** mode == 1: takes the floor of the number
** mode == 2: takes the ceil of the number
*/
int ToInteger(const Value *o, const int &mode, int64_t &v) {

    Value tmp;
    double d;
    label:
    if (IsNumber(o)) {
        double n = NumValue(o);
        double f = MATH_OP(floor)(n);
        if (n != f) {
            if (mode == 0) {
                return 0;
            } else if (mode > 1) {
                f += 1;
            }
        }
        return Number2Int(f, v);
    } else if (IsInt(o)) {
        v = IntValue(o);
        return 1;
    } else if (ToNum(o, d)) {
        SetDValue(&tmp, d);
        o = &tmp;
        goto label;
    } else {
        return 0;
    }
}

bool ValueEqulas(const Value *a, const Value *b) {
    if (a->type != b->type) {
        return false;
    }
    double d1, d2;
    if (IsNumber(a)) {
        return NumEq(NumValue(a), NumValue(b));
    } else if (IsInt(a)) {
        return IntValue(a) == IntValue(b);
    } else if (IsBool(a)) {
        return BoolValue(a) == BoolValue(b);
    }
    else if (ToNum(a, d1) && ToNum(b, d2)){
        return NumEq(d1, d2);
    } else {
        return false;
    }
}

bool ValueLE(const Value *a, const Value *b) {
    if (a->type != b->type) {
        return false;
    }
    double d1, d2;
    d1 = NumValue(a);
    d2 = NumValue(b);
    label:
    if (IsNumber(a)) {
        return NumLT(d1, d2) || NumEq(d1, d2);
    } else if (IsInt(a)) {
        return IntValue(a) <= IntValue(b);
    } else if (ToNum(a, d1) && ToNum(b, d2)){
        goto label;
    } else {
        return false;
    }
}

bool ValueLT(const Value *a, const Value *b) {
    if (a->type != b->type) {
        return false;
    }
    double d1, d2;
    if (IsNumber(a)) {
        return NumLT(NumValue(a), NumValue(b));
    } else if (IsInt(a)) {
        return IntValue(a) < IntValue(b);
    } else if (ToNum(a, d1) && ToNum(b, d2)){
        return NumLT(d1, d2);
    } else {
        return false;
    }
}

inline double NumUnm(const double &d) {
    return -d;
}

inline int64_t ShiftLeft(const int64_t &a, const int64_t &b) {
    if (b < 0) {
        if (b <= -NUM_BITS) {
            return 0;
        } else {
            return INT_OP(>>, a, b);
        }
    } else {
        if (b >= NUM_BITS) {
            return 0;
        } else {
            return INT_OP(<<, a, b);
        }
    }
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
            case OP_GETFUNC: {
                a = frame.reg + GET_ARG_A(instruction);
                a->type = Value::Type::FUNC;
                a->f = frame.func->f->GetChild(GET_ARG_B(instruction));
                break;
            }

            case OP_ADD:
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                if (IsInt(b) && IsInt(c)) {
                    SetIValue(a, INT_OP(+, IntValue(b), IntValue(c)));
                } else if (ToNum(b, d1) && ToNum(c, d2)) {
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
                } else if (ToNum(b, d1) && ToNum(c, d2)) {
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
                } else if (ToNum(b, d1) && ToNum(c, d2)) {
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
                } else if (ToNum(b, d1) && ToNum(c, d2)) {
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
                } else if (ToNum(b, d1) && ToNum(c, d2)) {
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
                } else if (ToNum(b, d1) && ToNum(c, d2)) {
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
                } else if (ToNum(b, d1) && ToNum(c, d2)) {
                    SetDValue(a, NumPow(d1, d2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_POW]", b->type, c->type);
                }
                break;

            case OP_BAND: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                int64_t i1, i2;
                if (ToInteger(b, 0, i1) && ToInteger(c, 0, i2)) {
                    SetIValue(a, INT_OP(&, i1, i2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_BAND]", b->type, c->type);
                }
            }
                break;

            case OP_CALL: {
                a = frame.reg + GET_ARG_A(instruction);
                size_t argc = GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                exec_state->CallFunction(c, argc, a);
            }
                break;

            case OP_JMP: {
                a = frame.reg + GET_ARG_A(instruction);
                int true_pc_jump = GET_ARG_B(instruction);
                int false_pc_jump = GET_ARG_C(instruction);
                bool con = false;
                if (!ToBool(a, con)) {
                    LOGE("Unspport Type[%s] with OP_CODE[OP_JMP]", a->type);
                    return;
                }
                if (con) {
                    pc += true_pc_jump - 1;
                } else {
                    pc += false_pc_jump - 1;
                }
            }
                break;

            case OP_GOTO: {
                pc = frame.pc + GET_ARG_A(instruction);
            }
                break;

            case OP_EQ: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                SetBValue(a, ValueEqulas(b, c));
            }
                break;

            case OP_LT: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                SetBValue(a, ValueLT(b, c));
            }
                break;

            case OP_LE: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                SetBValue(a, ValueLE(b, c));
            }
                break;

            case OP_UNM: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                if (IsInt(b)) {
                    SetIValue(a, INT_OP(-, 0, IntValue(b)));
                } else if (IsNumber(b)) {
                    SetDValue(a, NUM_OP(-, 0, NumValue(b)));
                } else {
                    LOGE("Unspport Type[%s] with OP_CODE[OP_UNM]", b->type);
                }
            }
                break;

            case OP_BNOT: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                int64_t i;
                if (ToInteger(b, 0, i)) {
                    SetIValue(a, INT_OP(^, ~CAST_S2U(0), i));
                } else {
                    LOGE("Unspport Type[%s] with OP_CODE[OP_BNOT]", b->type);
                }
            }
                break;

            case OP_BOR: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                int64_t i1, i2;
                if (ToInteger(b, 0, i1) && ToInteger(c, 0, i2)) {
                    SetIValue(a, INT_OP(|, i1, i2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_BOR]", b->type, c->type);
                }
            }
                break;

            case OP_BXOR: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                int64_t i1, i2;
                if (ToInteger(b, 0, i1) && ToInteger(c, 0, i2)) {
                    SetIValue(a, INT_OP(^, i1, i2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_BXOR]", b->type, c->type);
                }

            }
                break;

            case OP_SHL: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                int64_t i1, i2;
                if (ToInteger(b, 0, i1) && ToInteger(c, 0, i2)) {
                    SetIValue(a, ShiftLeft(i1, i2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_SHL]", b->type, c->type);
                }
            }
                break;

            case OP_SHR: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                c = frame.reg + GET_ARG_C(instruction);
                int64_t i1, i2;
                if (ToInteger(b, 0, i1) && ToInteger(c, 0, i2)) {
                    SetIValue(a, ShiftLeft(i1, -i2));
                } else {
                    LOGE("Unspport Type[%s,%s] with OP_CODE[OP_SHR]", b->type, c->type);
                }
            }
                break;

            case OP_PRE_INCR: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                if (IsInt(a)) {
                    SetIValue(a, IntValue(a) + 1);
                    if (NULL != b) {
                        SetIValue(b, IntValue(a));
                    }
                } else if (IsNumber(a)) {
                    SetDValue(a, NumValue(a) + 1);
                    if (NULL != b) {
                        SetDValue(b, NumValue(a));
                    }
                } else {
                    LOGE("Unspport Type[%s] with OP_CODE[OP_PRE_INCR]", a->type);
                }
            }
                break;

            case OP_PRE_DECR: {
                a = frame.reg + GET_ARG_A(instruction);
                b = frame.reg + GET_ARG_B(instruction);
                if (IsInt(a)) {
                    SetIValue(a, IntValue(a) - 1);
                    if (NULL != b) {
                        SetIValue(b, IntValue(a));
                    }
                } else if (IsNumber(a)) {
                    SetDValue(a, NumValue(a) - 1);
                    if (NULL != b) {
                        SetDValue(b, NumValue(a));
                    }
                } else {
                    LOGE("Unspport Type[%s] with OP_CODE[OP_PRE_DECR]", a->type);
                }
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