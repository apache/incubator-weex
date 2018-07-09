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
namespace weex {
namespace core {
namespace data_render {

enum OpCode {
  OP_MOVE,      //	A B	    R(A) = R(B)
  OP_LOADK,      //	A sBx	R(A) = sBx
  OP_LOADNULL,  //	A       R(A) = null
  OP_GETGLOBAL,  //	A B	    R(A) = Global[B]
  OP_GETUPVAL,  //	A B	    R(A) = UpValue[B]
  OP_SETUPVAL,  //	A B	    UpValue[B] = R(A)
  OP_GETTABLE,  //	A B C	R(A) = R(B)[R(C)]
  OP_SETTABLE,  //	A B C	R(A)[R(B)] = RK(C)

  OP_ADD,   //	A B C	R(A) = R(B) + R(C)
  OP_SUB,   //	A B C	R(A) = R(B) - R(C)
  OP_MUL,   //	A B C	R(A) = R(B) * R(C)
  OP_MOD,   //	A B C	R(A) = R(B) % R(C)
  OP_POW,   //	A B C	R(A) = R(B) ^ R(C)
  OP_DIV,   //	A B C	R(A) = R(B) / R(C)
  OP_IDIV,  //	A B C	R(A) = R(B) // R(C)
  OP_BAND,  //	A B C	R(A) = R(B) & R(C)
  OP_BOR,   //	A B C	R(A) = R(B) | R(C)
  OP_BXOR,  //	A B C	R(A) = R(B) ~ R(C)
  OP_SHL,   //	A B C	R(A) = R(B) << R(C)
  OP_SHR,   //	A B C	R(A) = R(B) >> R(C)
  OP_UNM,   //	A B	    R(A) = -R(B)
  OP_BNOT,  //	A B	    R(A) = ~R(B)

  OP_JMP,   //	A B C	if (R(A)) pc += R(B) else pc += R(C)
  OP_EQ,    //	A B C	R(A) = R(B) == R(C)
  OP_LT,    //	A B C	R(A) = R(A) <  R(B)
  OP_LE,    //	A B C	R(A) = R(A) <= R(B)

  OP_CALL,  //	A B C	R(C) = R(A)(R(A+1), ... ,R(A+B-1))

};

typedef unsigned long Instruction;

#define SIZE_OP 8
#define SIZE_C 8
#define SIZE_B 8
#define SIZE_A 8

#define POS_OP 0
#define POS_A (POS_OP + SIZE_OP)
#define POS_B (POS_A + SIZE_A)
#define POS_C (POS_B + SIZE_B)

#define CREATE_ABC(op_code, a, b, c)                             \
  ((Instruction)op_code << POS_OP) | ((Instruction)a << POS_A) | \
   ((Instruction)b << POS_B) | ((Instruction)c << POS_C)

#define GET_OP_CODE(i) (OpCode)(((i) >> POS_OP) & SIZE_OP)
#define GET_ARG_A(i) (OpCode)(((i) >> POS_A) & SIZE_A)
#define GET_ARG_B(i) (OpCode)(((i) >> POS_B) & SIZE_B)
#define GET_ARG_C(i) (OpCode)(((i) >> POS_C) & SIZE_C)

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_OP_CODE_H_
