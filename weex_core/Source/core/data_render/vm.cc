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
#include "core/data_render/object.h"
#include "core/data_render/table.h"
#include "common_error.h"

#if DEBUG
#include "core/data_render/monitor/vm_monitor.h"
#endif

namespace weex {
namespace core {
namespace data_render {

void VM::RunFrame(ExecState* exec_state, Frame frame, Value* ret) {

#if DEBUG
  TimeCost tc;
#endif

  Value* a = nullptr;
  Value* b = nullptr;
  Value* c = nullptr;
  auto pc = frame.pc;
  while (pc != frame.end) {
    Instruction instruction = *pc++;
    double d1, d2;
    OpCode op(GET_OP_CODE(instruction));

#if DEBUG
    tc.op_start(op);
#endif

    switch (op) {
      case OP_MOVE:
        LOGD("OP_MOVE A:%ld B:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction));
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        if (IsValueRef(a)) {
            *a->var = *b;
        }
        else {
            *a = *b;
        }
        break;
      case OP_LOADNULL:
        a = frame.reg + GET_ARG_A(instruction);
        a->type = Value::Type::NIL;
        break;
      case OP_LOADK:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.func->f->GetConstant((int)GET_ARG_Bx(instruction));
        *a = *b;
        break;
      case OP_GETGLOBAL:
        a = frame.reg + GET_ARG_A(instruction);
        b = exec_state->global()->Find((int)GET_ARG_Bx(instruction));
        *a = *b;
        break;
      case OP_GETFUNC: {
        a = frame.reg + GET_ARG_A(instruction);
        a->type = Value::Type::FUNC;
        a->f = frame.func->f->GetChild(GET_ARG_Bx(instruction));
        break;
      }

      case OP_ADD:
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (IsString(b) || IsString(c)) {
          SetSValue(a, StringAdd(exec_state->string_table(), b, c));
        } else if (IsInt(b) && IsInt(c)) {
          SetIValue(a, INT_OP(+, IntValue(b), IntValue(c)));
        } else if (ToNum(b, d1) && ToNum(c, d2)) {
          SetDValue(a, NUM_OP(+, d1, d2));
        } else {
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_ADD]", b->type, c->type);
            throw VMExecError("Unspport Type with OP_CODE[OP_ADD]");
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_SUB]", b->type, c->type);
          throw VMExecError("Unspport Type with OP_CODE[OP_SUB]");
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_MUL]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_DIV]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_IDIV]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_MOD]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_POW]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_BAND]", b->type, c->type);
        }
      }
        break;

      case OP_CALL: {
        LOGD("OP_CALL A:%ld B:%ld C:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction), GET_ARG_C(instruction));
        a = frame.reg + GET_ARG_A(instruction);
        size_t argc = GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        exec_state->CallFunction(c, argc, a);
          
      }
        break;

      case OP_JMP: {
          a = frame.reg + GET_ARG_A(instruction);
          bool con = false;
          if (!ToBool(a, con)) {
              throw VMExecError("Unspport Type With OP_CODE [OP_JMP]");
          }
          if (!con) {
              pc += GET_ARG_Bx(instruction);
          }
      }
        break;

      case OP_GOTO: {
        pc = frame.pc + GET_ARG_Ax(instruction);
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
          LOGE("Unspport Type[%d] with OP_CODE[OP_UNM]", b->type);
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
          LOGE("Unspport Type[%d] with OP_CODE[OP_BNOT]", b->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_BOR]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_BXOR]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_SHL]", b->type, c->type);
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
          LOGE("Unspport Type[%d,%d] with OP_CODE[OP_SHR]", b->type, c->type);
        }
      }
        break;

      case OP_PRE_INCR: {
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        if (IsInt(a)) {
          SetIValue(a, (int)IntValue(a) + 1);
          if (NULL != b) {
            SetIValue(b, (int)IntValue(a));
          }
        } else if (IsNumber(a)) {
          SetDValue(a, NumValue(a) + 1);
          if (NULL != b) {
            SetDValue(b, NumValue(a));
          }
        } else {
          LOGE("Unspport Type[%d] with OP_CODE[OP_PRE_INCR]", a->type);
        }
      }
        break;

      case OP_PRE_DECR: {
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        if (IsInt(a)) {
          SetIValue(a, IntValue(a) - 1);
          if (GET_ARG_B(instruction) != 0) {
            SetIValue(b, IntValue(a));
          }
        } else if (IsNumber(a)) {
          SetDValue(a, NumValue(a) - 1);
          if (GET_ARG_B(instruction) != 0) {
            SetDValue(b, NumValue(a));
          }
        } else {
          throw VMExecError("Unspport Type with OP_CODE [OP_PRE_DECR]");
        }
      }
        break;
      case OP_NEWCLASS: {
          LOGD("OP_NEWCLASS A:%ld B:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction));
          a = frame.reg + GET_ARG_A(instruction);
          b = exec_state->global()->Find((int)(GET_ARG_Bx(instruction)));
          if (!IsClass(b)) {
              throw VMExecError("Unspport Find Desc with OP_CODE [OP_NEWCLASS]");
          }
          *a = exec_state->class_factory()->CreateClassInstance(ObjectValue<ClassDescriptor>(b));
          break;
      }
        case OP_GETSUPER:
        {
            LOGD("OP_GETSUPER A:%ld B:%ld C:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction), GET_ARG_C(instruction));
            a = frame.reg + GET_ARG_A(instruction);
            b = frame.reg + GET_ARG_B(instruction);
            c = frame.reg + GET_ARG_C(instruction);
            if (!IsClassInstance(b)) {
                throw VMExecError("Type Error For Class Instance with OP_CODE [OP_GETSUPER]");
            }
            ClassInstance *inst = ObjectValue<ClassInstance>(b);
            ClassInstance *inst_super = inst->p_super_;
            if (!inst_super) {
                throw VMExecError("Instance Can't Find Super With OP_CODE [OP_GETSUPER]");
            }
            int index = inst_super->p_desc_->funcs_->IndexOf("constructor");
            if (index < 0) {
                throw VMExecError("Can't Find Super Constructor With OP_CODE [OP_GETSUPER]");
            }
            SetCIValue(a, reinterpret_cast<GCObject *>(inst->p_super_));
            *c = *inst_super->p_desc_->funcs_->Find(index);
            break;
        }
        case OP_GETCLASS:
        {
            LOGD("OP_GETCLASS A:%ld B:%ld C:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction), GET_ARG_C(instruction));
            a = frame.reg + GET_ARG_A(instruction);
            b = frame.reg + GET_ARG_B(instruction);
            c = frame.reg + GET_ARG_C(instruction);
            if (!IsClassInstance(b)) {
                throw VMExecError("Type Error For Class Instance with OP_CODE [OP_GETCLASS]");
            }
            if (!IsString(c)) {
                throw VMExecError("Type Error For Member with OP_CODE [OP_GETCLASS]");
            }
            int index = ObjectValue<ClassInstance>(b)->p_desc_->funcs_->IndexOf(StringValue(c)->c_str());
            if (index < 0) {
                throw VMExecError("Can't Find " + std::string(StringValue(c)->c_str()) + " With OP_CODE [OP_GETCLASS]");
            }
            *a = *ObjectValue<ClassInstance>(b)->p_desc_->funcs_->Find(index);
            break;
        }
        case OP_GETCLASSVAR:
        {
            LOGD("OP_GETCLASSVAR A:%ld B:%ld C:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction), GET_ARG_C(instruction));
            a = frame.reg + GET_ARG_A(instruction);
            b = frame.reg + GET_ARG_B(instruction);
            c = frame.reg + GET_ARG_C(instruction);
            if (!IsClassInstance(b)) {
                throw VMExecError("Type Error For Class Instance with OP_CODE [OP_GETCLASSVAR]");
            }
            if (!IsString(c)) {
                throw VMExecError("Type Error For Member with OP_CODE [OP_GETCLASSVAR]");
            }
            Variables *vars = ObjectValue<ClassInstance>(b)->vars_.get();
            std::string var_name = StringValue(c)->c_str();
            int index = vars->IndexOf(var_name);
            if (index < 0) {
                Value var;
                SetNil(&var);
                index = vars->Add(var_name, var);
            }
            Value *ref = vars->Find(index);
            SetValueRef(a, ref);
            break;
        }
        case OP_SETVALUE:
        {
            LOGD("OP_SETVALUE A:%ld B:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction));
            a = frame.reg + GET_ARG_A(instruction);
            int index = (int)GET_ARG_Bx(instruction);
            break;
        }
        case OP_GETVALUE:
        {
            LOGD("OP_GETVALUE A:%ld B:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction));
            a = frame.reg + GET_ARG_A(instruction);
            int index = (int)GET_ARG_Bx(instruction);
            break;
        }
      case OP_NEWTABLE: {
        a = frame.reg + GET_ARG_A(instruction);
        Value t = exec_state->table_factory()->CreateTable();
        *a = t;
      }
        break;

      case OP_GETTABLE: {
        LOGD("OP_GETTABLE A:%ld B:%ld C:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction), GET_ARG_C(instruction));
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (!IsTable(b)) {
          // TODO error
            throw VMExecError("Unspport Type with OP_CODE [OP_GETTABLE]");
        }
        Value* ret = GetTabValue(reinterpret_cast<Table *>(b->gc), *c);
        if (!IsNil(ret)) {
          *a = *ret;
        } else {
          SetNil(a);
        }
      }
        break;

      case OP_SETTABLE: {
        LOGD("OP_SETTABLE A:%ld B:%ld C:%ld\n", GET_ARG_A(instruction), GET_ARG_B(instruction), GET_ARG_C(instruction));
        a = frame.reg + GET_ARG_A(instruction);
        b = frame.reg + GET_ARG_B(instruction);
        c = frame.reg + GET_ARG_C(instruction);
        if (!IsTable(a)) {
          // TODO error
            throw VMExecError("Table Type Error With OP_CODE [OP_SETTABLE]");
        }
        int ret = SetTabValue(reinterpret_cast<Table*>(a->gc), b, *c);
        if (!ret) {
          // TODO set faile
            throw VMExecError("Set Table Error With OP_CODE [OP_SETTABLE]");
        }
      }
        break;
      case OP_RETURN0: {
        return;
      }

      case OP_RETURN1: {
        if (ret == nullptr) {
          return;
        } else {
          a = frame.reg + GET_ARG_A(instruction);
          *ret = *a;
          return;
        }
      }
      case OP_INVALID: {
          throw VMExecError("Error With OP_CODE [OP_INVALID]");
      }
        break;

      default:
        break;
    }

#if DEBUG
    tc.op_end();
#endif

  }
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
