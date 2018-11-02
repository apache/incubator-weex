/*
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

#ifndef CORE_DATA_RENDER_OP_CODE_H_
#define CORE_DATA_RENDER_OP_CODE_H_

#include <assert.h>
#include <cstdint>

namespace weex {
namespace core {
namespace data_render {

#define OP_CODE_LIST(T)                                        \
T(OP_MOVE,           "OP_MOVE",           2)                                                    \
T(OP_SETTABLE,       "OP_SETTABLE",       3)                                                    \
T(OP_SETOUTVAR,      "OP_SETOUTVAR",      2)                                                    \
T(OP_LOADK,          "OP_LOADK",          2)                                                    \
T(OP_CALL,           "OP_CALL",           3)                                                    \
T(OP_GETMEMBER,      "OP_GETMEMBER",      3)                                                    \
T(OP_LOADNULL,       "OP_LOADNULL",       1)                                                    \
T(OP_GETGLOBAL,      "OP_GETGLOBAL",      2)                                                    \
T(OP_GETFUNC,        "OP_GETFUNC",        2)                                                    \
T(OP_NEW,            "OP_NEW",            3)                                                    \
T(OP_CONSTRUCTOR,    "OP_CONSTRUCTOR",    3)                                                    \
T(OP_GETCLASS,       "OP_GETCLASS",       3)                                                    \
T(OP_GETMEMBERVAR,   "OP_GETMEMBERVAR",   3)                                                    \
T(OP_SETMEMBERVAR,   "OP_SETMEMBERVAR",   2)                                                    \
T(OP_GETSUPER,       "OP_GETSUPER",       3)                                                    \
T(OP_RESETOUTVAR,    "OP_RESETOUTVAR",    1)                                                    \
T(OP_GETOUTVAR,      "OP_GETOUTVAR",      2)                                                    \
T(OP_OUT_CLOSURE,    "OP_OUT_CLOSURE",    2)                                                    \
T(OP_IN_CLOSURE,     "OP_IN_CLOSURE",     2)                                                    \
T(OP_REMOVE_CLOSURE, "OP_REMOVE_CLOSURE", 1)                                                    \
T(OP_SETARRAY,       "OP_SETARRAY",       3)                                                    \
T(OP_GETINDEX,       "OP_GETINDEX",       3)                                                    \
T(OP_GETINDEXVAR,    "OP_GETINDEXVAR",    3)                                                    \
T(OP_TYPEOF,         "OP_TYPEOF",         2)                                                    \
T(OP_ADD,            "OP_ADD",            3)                                                    \
T(OP_SUB,            "OP_SUB",            3)                                                    \
T(OP_MUL,            "OP_MUL",            3)                                                    \
T(OP_MOD,            "OP_MOD",            3)                                                    \
T(OP_DIV,            "OP_DIV",            3)                                                    \
T(OP_JMP,            "OP_JMP",            2)                                                    \
T(OP_TRUE_JMP,       "OP_TRUE_JMP",       2)                                                    \
T(OP_JMPTO,          "OP_JMPTO",          2)                                                    \
T(OP_TRUE_JMPTO,     "OP_TRUE_JMPTO",     2)                                                    \
T(OP_GOTO,           "OP_GOTO",           1)                                                    \
T(OP_EQ,             "OP_EQ",             3)                                                    \
T(OP_SEQ,            "OP_SEQ",            3)                                                    \
T(OP_LT,             "OP_LT",             3)                                                    \
T(OP_LTE,            "OP_LTE",            3)                                                    \
T(OP_GT,             "OP_GT",             3)                                                    \
T(OP_GTE,            "OP_GTE",            3)                                                    \
T(OP_AND,            "OP_AND",            3)                                                    \
T(OP_OR,             "OP_OR",             3)                                                    \
T(OP_IN,             "OP_IN",             3)                                                    \
T(OP_NOT,            "OP_NOT",            2)                                                    \
T(OP_RETURN0,        "OP_RETURN0",        0)                                                    \
T(OP_RETURN1,        "OP_RETURN1",        1)                                                    \
T(OP_PREV_INCR,      "OP_PREV_INCR",      2)                                                    \
T(OP_PREV_DECR,      "OP_PREV_DECR",      2)                                                    \
T(OP_POST_INCR,      "OP_POST_INCR",      2)                                                    \
T(OP_POST_DECR,      "OP_POST_DECR",      2)                                                    \
T(OP_IDIV,           "OP_IDIV",           3)                                                    \
T(OP_POW,            "OP_POW",            3)                                                    \
T(OP_BAND,           "OP_BAND",           3)                                                    \
T(OP_BOR,            "OP_BOR",            3)                                                    \
T(OP_BXOR,           "OP_BXOR",           3)                                                    \
T(OP_SHL,            "OP_SHL",            3)                                                    \
T(OP_SHR,            "OP_SHR",            3)                                                    \
T(OP_UNM,            "OP_UNM",            3)                                                    \
T(OP_BNOT,           "OP_BNOT",           3)                                                    \
T(OP_INVALID,        "OP_INVALID",        0)                                                    \

#define T(name, string, ops) name,
    enum OPCode {
        OP_CODE_LIST(T) NUM_OPCODES
    };
#undef T

class OPUtil {
public:
    static const char *name(OPCode code) {
        assert(code < NUM_OPCODES);
        return s_name_[code];
    }
    static int ops(OPCode code) {
        assert(code < NUM_OPCODES);  // token is unsigned.
        return s_ops_[code];
    }
private:
    static const char *const s_name_[NUM_OPCODES];
    static const int8_t s_ops_[NUM_OPCODES];
};

typedef unsigned long Instruction;

#define SIZE_OP 8
#define SIZE_C 8
#define SIZE_B 8
#define SIZE_A 8
#define SIZE_Ax (SIZE_A + SIZE_B + SIZE_C)
#define SIZE_Bx (SIZE_B + SIZE_C)

#define POS_OP 0
#define POS_A (POS_OP + SIZE_OP)
#define POS_Ax POS_A
#define POS_B (POS_A + SIZE_A)
#define POS_Bx (POS_B)
#define POS_C (POS_B + SIZE_B)

#define CREATE_ABC(op_code, a, b, c)                                     \
  a < 0 || b < 0 || c < 0                                                \
? ((Instruction)OP_INVALID << POS_OP)                              \
      : ((Instruction)op_code << POS_OP) | ((Instruction)(a) << POS_A) | \
            ((Instruction)(b) << POS_B) | ((Instruction)(c) << POS_C)

#define CREATE_Ax(op_code, ax)                 \
ax < 0 ? ((Instruction)OP_INVALID << POS_OP) \
         : ((Instruction)op_code << POS_OP) | ((Instruction)(ax) << POS_Ax)

#define CREATE_ABx(op_code, a, bx)                                          \
a < 0 || bx < 0 ? ((Instruction)OP_INVALID << POS_OP)                     \
        : ((Instruction)op_code << POS_OP) | ((Instruction)(a) << POS_A)    \
             | ((Instruction)(bx) << POS_Bx)

#define GET_OP_CODE(i) (OPCode)(((i) >> POS_OP) & 0xFF)
#define GET_ARG_A(i) (long)(((i) >> POS_A) & 0xFF)
#define GET_ARG_B(i) (long)(((i) >> POS_B) & 0xFF)
#define GET_ARG_C(i) (long)(((i) >> POS_C) & 0xFF)
#define GET_ARG_Ax(i) (long)(((i) >> POS_Ax) & 0xFFFFFF)
#define GET_ARG_Bx(i) (long)(((i) >> POS_Bx) & 0xFFFF)

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_OP_CODE_H_
