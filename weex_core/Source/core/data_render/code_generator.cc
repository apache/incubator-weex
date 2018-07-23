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

#include "core/data_render/code_generator.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/string_table.h"

namespace weex {
namespace core {
namespace data_render {

void CodeGenerator::EnterFunction() {
  FuncCnt *parent_func = cur_func_.release();
  cur_func_.reset(new FuncCnt);
  cur_func_->set_parent(parent_func);
  cur_func_->set_func_state(new FuncState);
  if (parent_func != nullptr) {
    parent_func->func_state()->AddChild(cur_func_->func_state());
  }
}

void CodeGenerator::LeaveFunction() { cur_func_.reset(cur_func_->parent()); }

void CodeGenerator::EnterBlock() {
  BlockCnt *parent_block = cur_block_.release();
  cur_block_.reset(new BlockCnt);
  cur_block_->set_parent(parent_block);
  if (parent_block != nullptr) {
    cur_block_->set_idx(parent_block->idx());
  }
}

void CodeGenerator::LeaveBlock() { cur_block_.reset(cur_block_->parent()); }

void CodeGenerator::Visit(StringConstant *node, void *data) {
  long reg = data == nullptr ? -1 : *static_cast<long *>(data);
  if (reg >= 0) {
    FuncState *func_state = cur_func_->func_state();
    auto value = exec_state_->string_table_->StringFromUTF8(node->string());
    int index = func_state->AddConstant(std::move(value));
    Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(ExpressionList *node, void *data) {
  for (auto it = node->raw_list().begin(); it != node->raw_list().end(); ++it) {
    auto temp = (*it).get();
    temp->Accept(this, nullptr);
  }
}

void CodeGenerator::Visit(ChunkStatement *stms, void *data) {
  FuncScope scope(this);
  BlockScope block_scoped(this);
  // For root func
  long reg = cur_block_->NextRegisterId();
  for (int i = 0; i < stms->statements()->raw_list().size(); ++i) {
    auto temp = stms->statements()->raw_list()[i].get();
    if (temp != NULL) {
      temp->Accept(this, data);
    }
  }
  exec_state_->func_state_.reset(cur_func_->func_state());
  // Save global variables
  exec_state_->global_variables_ = cur_block_->variables();
}

void CodeGenerator::Visit(CallExpression *stms, void *data) {
  RegisterScope scope(cur_block_.get());
  long ret = data == nullptr ? cur_block_->NextRegisterId()
                             : *static_cast<long *>(data);
  long caller = cur_block_->NextRegisterId();
  size_t argc = 0;
  if (stms->callee().get() != NULL) {
    stms->callee()->Accept(this, &caller);
  }

  argc = stms->args().size();
  for (auto it = stms->args().begin(); it != stms->args().end(); ++it) {
    auto temp = (*it).get();
    long arg = cur_block_->NextRegisterId();
    temp->Accept(this, &arg);
  }

  FuncState *state = cur_func_->func_state();
  state->AddInstruction(CREATE_ABC(OP_CALL, ret, argc, caller));
}

void CodeGenerator::Visit(ArgumentList *node, void *data) {
  Handle<ExpressionList> exprList = node->args();
  exprList->Accept(this, data);
}

void CodeGenerator::Visit(IfStatement *node, void *data) {
  RegisterScope scope(cur_block_.get());

  long condition = cur_block_->NextRegisterId();
  if (node->condition().get() != NULL) {
    node->condition()->Accept(this, &condition);
  }
  auto slot = cur_func_->func_state()->AddInstruction(0);
  int tb_start_index = cur_func_->func_state()->instructions().size() - 1;
  if (node->body().get() != NULL) {
    node->body()->Accept(this, nullptr);
  }
  int tb_end_index = cur_func_->func_state()->instructions().size() - 1;

  FuncState *state = cur_func_->func_state();
  state->ReplaceInstruction(
      slot,
      CREATE_ABC(OP_JMP, condition, 1, tb_end_index - tb_start_index + 1));
}

void CodeGenerator::Visit(IfElseStatement *node, void *data) {
  long ifcondition = cur_block_->NextRegisterId();
  if (node->condition().get() != NULL) {
    node->condition()->Accept(this, &ifcondition);
  }
  auto slot = cur_func_->func_state()->AddInstruction(0);
  int tb_start_index = cur_func_->func_state()->instructions().size() - 1;
  if (node->body().get() != NULL) {
    node->body()->Accept(this, nullptr);
  }
  int tb_end_index = cur_func_->func_state()->instructions().size() - 1;
  auto else_slot = cur_func_->func_state()->AddInstruction(0);

  long else_condition = cur_block_->NextRegisterId();
  if (node->els().get() != NULL) {
    node->els()->Accept(this, &else_condition);
  }

  FuncState *state = cur_func_->func_state();
  state->ReplaceInstruction(
      slot,
      CREATE_ABC(OP_JMP, ifcondition, 1, tb_end_index - tb_start_index + 1));
}

void CodeGenerator::Visit(ForStatement *node, void *data) {
  RegisterScope scope(cur_block_.get());

  long init = cur_block_->NextRegisterId();
  if (node->init().get() != NULL) {
    node->init()->Accept(this, &init);
  }

  int condition_start_index =
      cur_func_->func_state()->instructions().size() - 1;

  long condition = cur_block_->NextRegisterId();
  if (node->condition().get() != NULL) {
    node->condition()->Accept(this, &condition);
  }

  auto slot = cur_func_->func_state()->AddInstruction(0);

  if (node->body().get() != NULL) {
    node->body()->Accept(this, nullptr);
  }

  long update = cur_block_->NextRegisterId();
  if (node->update().get() != NULL) {
    node->update()->Accept(this, &update);
  }

  FuncState *state = cur_func_->func_state();
  state->AddInstruction(CREATE_ABC(OP_GOTO, condition_start_index, 0, 0));

  int for_end_index = cur_func_->func_state()->instructions().size() - 1;

  state->ReplaceInstruction(
      slot, (CREATE_ABC(OP_JMP, condition, 1, for_end_index - slot + 1)));
}

void CodeGenerator::Visit(BlockStatement *node, void *data) {
  for (int i = 0; i < node->statements()->raw_list().size(); ++i) {
    auto temp = node->statements()->raw_list()[i].get();
    if (temp != NULL) {
      temp->Accept(this, data);
    }
  }
}

void CodeGenerator::Visit(FunctionPrototype *node, void *data) {}

void CodeGenerator::Visit(FunctionStatement *node, void *data) {
  RegisterScope register_scope(cur_block_.get());
  long reg = data == nullptr ? cur_block_->NextRegisterId()
                             : *static_cast<long *>(data);
  // Slot
  auto slot = cur_func_->func_state()->AddInstruction(0);
  {
    FuncScope scope(this);
    node->body()->Accept(this, nullptr);
  }
  int index = cur_func_->func_state()->children().size() - 1;
  Instruction i = CREATE_ABC(OP_GETFUNC, reg, index, 0);
  cur_func_->func_state()->ReplaceInstruction(slot, i);
}

void CodeGenerator::Visit(BinaryExpression *node, void *data) {
  RegisterScope scope(cur_block_.get());
  long ret = data == nullptr ? cur_block_->NextRegisterId()
                             : *static_cast<long *>(data);

  long left = cur_block_->NextRegisterId();
  if (node->lhs().get() != NULL) {
    node->lhs()->Accept(this, &left);
  }

  long right = cur_block_->NextRegisterId();
  if (node->rhs().get() != NULL) {
    node->rhs()->Accept(this, &right);
  }

  BinaryOperation opeartion = node->op();

  FuncState *state = cur_func_->func_state();

  // a + b
  if (opeartion == BinaryOperation::kAddition) {
    state->AddInstruction(CREATE_ABC(OP_ADD, ret, left, right));
  }

  // a < b
  if (opeartion == BinaryOperation::kLessThan) {
    state->AddInstruction(CREATE_ABC(OP_LT, ret, left, right));
  }

  // a > b
  if (opeartion == BinaryOperation::kGreaterThan) {
    state->AddInstruction(CREATE_ABC(OP_LT, ret, right, left));
  }

  // a <= b
  if (opeartion == BinaryOperation::kLessThanEqual) {
    state->AddInstruction(CREATE_ABC(OP_LE, ret, left, right));
  }

  // a >= b
  if (opeartion == BinaryOperation::kGreaterThanEqual) {
    state->AddInstruction(CREATE_ABC(OP_LE, ret, right, left));
  }
}

void CodeGenerator::Visit(AssignExpression *node, void *data) {
  RegisterScope scope(cur_block_.get());

  long left = cur_block_->NextRegisterId();
  if (node->lhs().get() != NULL) {
    node->lhs()->Accept(this, &left);
  }

  long right = cur_block_->NextRegisterId();
  if (node->rhs().get() != NULL) {
    node->rhs()->Accept(this, &right);
  }

  // a = b
  cur_func_->func_state()->AddInstruction(CREATE_ABC(OP_MOVE, left, right, 0));
}

void CodeGenerator::Visit(ChildStatement *node, void *data) {
  Handle<ExpressionList> stms = node->statements();
  for (auto it = stms->raw_list().begin(); it != stms->raw_list().end(); ++it) {
    auto temp = (*it).get();
    temp->Accept(this, nullptr);
  }
}

void CodeGenerator::Visit(Declaration *node, void *data) {
  long reg = cur_block_->NextRegisterId();
  cur_block_->variables().insert(std::make_pair(node->name(), reg));
  if (node->expr().get() != nullptr) {
    node->expr()->Accept(this, &reg);
  } else {
    FuncState *state = cur_func_->func_state();
    state->AddInstruction(CREATE_ABC(OP_LOADNULL, reg, 0, 0));
  }
}

void CodeGenerator::Visit(DeclarationList *node, void *data) {
  for (auto it = node->exprs().begin(); it != node->exprs().end(); ++it) {
    auto temp = (*it).get();
    temp->Accept(this, nullptr);
  }
}

void CodeGenerator::Visit(IntegralConstant *node, void *data) {
  long reg = data == nullptr ? -1 : *static_cast<long *>(data);
  if (reg >= 0) {
    FuncState *func_state = cur_func_->func_state();
    int value = node->value();
    int index = func_state->AddConstant(static_cast<int64_t>(value));
    Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(BooleanConstant *node, void *data) {
  long reg = data == nullptr ? -1 : *static_cast<long *>(data);
  if (reg >= 0) {
    FuncState *func_state = cur_func_->func_state();
    bool value = node->pred();
    int index = func_state->AddConstant(static_cast<bool>(value));
    Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(DoubleConstant *node, void *data) {
  long reg = data == nullptr ? -1 : *static_cast<long *>(data);
  if (reg >= 0) {
    FuncState *func_state = cur_func_->func_state();
    double value = node->value();
    int index = func_state->AddConstant(static_cast<double>(value));
    Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
    func_state->AddInstruction(i);
  }
}

void CodeGenerator::Visit(ObjectConstant *node, void *data) {
  long ret = data == nullptr ? -1 : *static_cast<long *>(data);

  FuncState *func_state = cur_func_->func_state();

  // new table
  Value *table = TableFactory::Instance()->CreateTable();
  if (ret >= 0) {
    int tableIndex = func_state->AddConstant(*table);
    Instruction i = CREATE_ABC(OpCode::OP_LOADK, ret, tableIndex, 0);
    func_state->AddInstruction(i);

    // expr
    for (auto it = node->proxy().begin(); it != node->proxy().end(); it++) {
      long item = cur_block_->NextRegisterId();

      auto ktemp = (*it).second;
      ktemp->Accept(this, &item);

      Instruction i = CREATE_ABC(OpCode::OP_SETTABLE, ret, 0, item);
      func_state->AddInstruction(i);
    }
  }
}

void CodeGenerator::Visit(ArrayConstant *node, void *data) {
  long reg = data == nullptr ? -1 : *static_cast<long *>(data);

  FuncState *func_state = cur_func_->func_state();

  // new table
  Value *table = TableFactory::Instance()->CreateTable();
  if (reg >= 0) {
    int tableIndex = func_state->AddConstant(*table);
    Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, tableIndex, 0);
    func_state->AddInstruction(i);

    // expr
    for (auto it = node->exprs().begin(); it != node->exprs().end(); it++) {
      long item = cur_block_->NextRegisterId();

      auto temp = (*it).get();
      temp->Accept(this, &item);

      Instruction i = CREATE_ABC(OpCode::OP_SETTABLE, reg, 0, item);
      func_state->AddInstruction(i);
    }
  }
}

void CodeGenerator::Visit(MemberExpression *node, void *data) {
  long ret = data == nullptr ? cur_block_->NextRegisterId()
                             : *static_cast<long *>(data);

  Handle<Identifier> table = node->expr();
  long tindex = exec_state_->global()->IndexOf(table->GetName());

  if (node->kind() == MemberAccessKind::kIndex) {
    long mindex = cur_block_->NextRegisterId();
    if (node->member().get() != NULL) {
      node->member()->Accept(this, &mindex);
    }

    FuncState *funcState = cur_func_->func_state();
    funcState->AddInstruction(CREATE_ABC(OP_GETTABLE, ret, tindex, mindex));

  } else if (node->kind() == MemberAccessKind::kDot) {
  } else if (node->kind() == MemberAccessKind::kCall) {
  }
}

void CodeGenerator::Visit(Identifier *node, void *data) {
  long reg_a = data == nullptr ? cur_block_->NextRegisterId()
                               : *static_cast<long *>(data);
  if (reg_a >= 0) {
    FuncState *state = cur_func_->func_state();

    int reg_b = cur_block_->FindRegisterId(node->GetName());
    if (reg_b >= 0) {
      state->AddInstruction(CREATE_ABC(OP_MOVE, reg_a, reg_b, 0));
      return;
    }

    int index = exec_state_->global()->IndexOf(node->GetName());
    if (index >= 0) {
      state->AddInstruction(CREATE_ABC(OP_GETGLOBAL, reg_a, index, 0));
      return;
    }
  }
}

void CodeGenerator::Visit(PrefixExpression *node, void *data) {
  RegisterScope scope(cur_block_.get());

  long ret = data == nullptr ? cur_block_->NextRegisterId()
                             : *static_cast<long *>(data);

  Handle<Identifier> iden = node->expr();
  long reg = cur_block_->FindRegisterId(iden->GetName());

  PrefixOperation operation = node->op();
  // ++i
  if (operation == PrefixOperation::kIncrement) {
    cur_func_->func_state()->AddInstruction(
        CREATE_ABC(OP_PRE_INCR, reg, ret, 0));
    // --i
  } else if (operation == PrefixOperation::kDecrement) {
    cur_func_->func_state()->AddInstruction(
        CREATE_ABC(OP_PRE_DECR, reg, ret, 0));
  }
}

void CodeGenerator::Visit(ReturnStatement *node, void *data) {}

}  // namespace data_render
}  // namespace core
}  // namespace weex
