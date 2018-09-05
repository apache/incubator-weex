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
  OP_MOVE,       //	A B	    R(A) = R(B)
  OP_LOADK,      //	A Bx	R(A) = Constant[Bx]
  OP_LOADNULL,   //	A       R(A) = null
  OP_GETGLOBAL,  //	A Bx	R(A) = Global[B]
  OP_GETFUNC,    //	A Bx  	R(A) = Function[B]
  OP_NEW,
  OP_GETCLASS,   // A B C R(A) = B->C
  OP_GETMEMBER,
  OP_GETMEMBERVAR,
  OP_SETMEMBERVAR,
  OP_GETSUPER,   // A B C     R(A) = super[B] C = super func
  
  OP_SETOUTVAR,     // A B C R(A) = B->C
  OP_RESETOUTVAR,
  OP_GETOUTVAR,     // A B C R(A) = B->C
    
  OP_SETTABLE,  //	A B C	R(A)[R(B)] = RK(C)

  OP_SETARRAY,  //    A B C    R(A)[R(B)] = RK(C)
  OP_GETINDEX,  //    A B C    R(A) = R(B)[R(C)]
  OP_GETINDEXVAR,
    
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

  OP_JMP,   //	A Bx	if (!R(A)) pc += Bx
  OP_GOTO,  //   Ax      pc = Ax
  OP_EQ,    //	A B C	R(A) = R(B) == R(C)
  OP_SEQ,
  OP_LT,    //	A B C	R(A) = R(A) <  R(B)
  OP_LTE,
  OP_GT,
  OP_GTE,
  OP_AND,
  OP_OR,
  OP_IN,
  OP_NOT,

  OP_CALL,     //	A B C	R(A) = R(B)(R(B+1), ... ,R(B+C-1))
  OP_RETURN0,  //	Return
  OP_RETURN1,  //	Return R(A)

  OP_PRE_INCR,  // A      if (B >= 0) R(B) = ++R(A) else ++R(A)
  OP_PRE_DECR,  // A      if (B >= 0) R(B) = --R(A) else --R(A)
    
  OP_POST_INCR,  // A      if (B >= 0) R(B) = R(A)++ else R(A)++
  OP_POST_DECR,  // A      if (B >= 0) R(B) = R(A)++ else R(A)--

  OP_INVALID,

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

#define GET_OP_CODE(i) (OpCode)(((i) >> POS_OP) & 0xFF)
#define GET_ARG_A(i) (long)(((i) >> POS_A) & 0xFF)
#define GET_ARG_B(i) (long)(((i) >> POS_B) & 0xFF)
#define GET_ARG_C(i) (long)(((i) >> POS_C) & 0xFF)
#define GET_ARG_Ax(i) (long)(((i) >> POS_Ax) & 0xFFFFFF)
#define GET_ARG_Bx(i) (long)(((i) >> POS_Bx) & 0xFFFF)

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_OP_CODE_H_
