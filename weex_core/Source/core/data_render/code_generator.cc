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

namespace weex {
namespace core {
namespace data_render {

void CodeGenerator::EnterFunction() {
  FuncCnt* parent_func = cur_func_.release();
  cur_func_.reset(new FuncCnt);
  cur_func_->set_parent(parent_func);
  cur_func_->set_func_state(new FuncState);
  if (parent_func != nullptr) {
    parent_func->func_state()->AddChild(cur_func_->func_state());
  }
}

void CodeGenerator::LeaveFunction() { cur_func_.reset(cur_func_->parent()); }

void CodeGenerator::EnterBlock() {
  BlockCnt* parent_block = cur_block_.release();
  cur_block_.reset(new BlockCnt);
  cur_block_->set_parent(parent_block);
  if (parent_block != nullptr) {
    cur_block_->set_idx(parent_block->idx());
  }
}

void CodeGenerator::LeaveBlock() { cur_block_.reset(cur_block_->parent()); }

void CodeGenerator::Visit(StringLiteral *node) {
//  long reg = data == nullptr ? -1 : *static_cast<long*>(data);
//  if (reg >= 0) {
//    FuncState* func_state = cur_func_->func_state();
//    if (node->IsBool()) {
//      int index =
//          func_state->AddConstant(static_cast<BoolNode*>(node)->value());
//      Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
//      func_state->AddInstruction(i);
//    } else if (node->IsInteger()) {
//      int index =
//          func_state->AddConstant(static_cast<IntegerNode*>(node)->value());
//      Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
//      func_state->AddInstruction(i);
//    } else if (node->IsNumber()) {
//      int index =
//          func_state->AddConstant(static_cast<NumberNode*>(node)->value());
//      Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
//      func_state->AddInstruction(i);
//    } else if (node->IsUndefined() || node->IsNull()) {
//      Instruction i = CREATE_ABC(OpCode::OP_LOADNULL, reg, 0, 0);
//      func_state->AddInstruction(i);
//    }
//  }
}

//void CodeGenerator::Visit(UnaryExpressionNode* node, void* data) {}
//void CodeGenerator::Visit(BinaryExpressionNode* node, void* data) {}
//
//void CodeGenerator::Visit(VariableNode* node, void* data) {
//  long reg_a = data == nullptr ? -1 : *static_cast<long*>(data);
//  if (reg_a >= 0) {
//    FuncState* state = cur_func_->func_state();
//
//    int reg_b = cur_block_->FindRegisterId(node->identifier());
//    if (reg_b >= 0) {
//      state->AddInstruction(CREATE_ABC(OP_MOVE, reg_a, reg_b, 0));
//      return;
//    }
//
//    int index = exec_state_->global()->IndexOf(node->identifier());
//    if (index >= 0) {
//      state->AddInstruction(CREATE_ABC(OP_GETGLOBAL, reg_a, index, 0));
//      return;
//    }
//  }
//}
//
//void CodeGenerator::Visit(AssignmentNode* node, void* data) {
//  long reg_a = data == nullptr ? -1 : *static_cast<long*>(data);
//  if (reg_a >= 0) {
//    if (node->expression() != nullptr) {
//      node->expression()->Accept(this, &reg_a);
//    } else {
//      FuncState* state = cur_func_->func_state();
//      state->AddInstruction(CREATE_ABC(OP_LOADNULL, reg_a, 0, 0));
//    }
//  }
//}
//
//void CodeGenerator::Visit(VarDeclareNode* node, void* data) {
//  long reg = cur_block_->NextRegisterId();
//  cur_block_->variables().insert(std::make_pair(node->identifier(), reg));
//  if (node->expression() != nullptr) {
//    node->expression()->Accept(this, &reg);
//  } else {
//    FuncState* state = cur_func_->func_state();
//    state->AddInstruction(CREATE_ABC(OP_LOADNULL, reg, 0, 0));
//  }
//}
//
//void CodeGenerator::Visit(DotAccessorNode* node, void* data) {}
//
//void CodeGenerator::Visit(ExpressionListNode* node, void* data) {
//  RegisterScope scope(cur_block_.get());
//  for (auto it = node->expressions().begin(); it != node->expressions().end();
//       ++it) {
//    long reg = cur_block_->NextRegisterId();
//    (*it).get()->Accept(this, &reg);
//  }
//}
//
//void CodeGenerator::Visit(FunctionCallNode* node, void* data) {
//  RegisterScope scope(cur_block_.get());
//  long ret = data == nullptr ? cur_block_->NextRegisterId()
//                             : *static_cast<long*>(data);
//  long caller = cur_block_->NextRegisterId();
//  size_t argc = 0;
//  if (node->caller() != nullptr) {
//    node->caller()->Accept(this, &caller);
//  }
//  if (node->arguments() != nullptr) {
//    argc = node->arguments()->expressions().size();
//    node->arguments()->Accept(this, nullptr);
//  }
//  FuncState* state = cur_func_->func_state();
//  state->AddInstruction(CREATE_ABC(OP_CALL, ret, argc, caller));
//}
//
//void CodeGenerator::Visit(ForNode* node, void* data) {}
//void CodeGenerator::Visit(IfElseNode* node, void* data) {}
//
//void CodeGenerator::Visit(FunctionNode* node, void* data) {
//  if (node->func() != nullptr) {
//    node->func()->Accept(this, data);
//  }
//}
//
//void CodeGenerator::Visit(BlockNode* node, void* data) {
//  BlockScope block_scoped(this);
//  for (auto it = node->statements().begin(); it != node->statements().end();
//       ++it) {
//    (*it)->Accept(this, nullptr);
//  }
//}

void CodeGenerator::Visit(ChunkStatement *stms) {
  FuncScope scope(this);
  BlockScope block_scoped(this);
  // For root func
  long reg = cur_block_->NextRegisterId();
  for (auto it = stms->statements()->raw_list().begin(); it != stms->statements()->raw_list().end();
       ++it) {
    (*it)->Accept(this);
  }
  exec_state_->func_state_.reset(cur_func_->func_state());
  // global value
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
