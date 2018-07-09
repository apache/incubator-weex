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

void CodeGenerator::EnterBlock() {
    BlockCnt* pre_block = current_block_.release();
    current_block_.reset(new BlockCnt);
    current_block_->set_pre(pre_block);
    current_block_->set_block_state(new BlockState);
    if (pre_block != nullptr) {
        pre_block->block_state()->set_next(current_block_->block_state());
    }
}

void CodeGenerator::LeaveBlock() {
    if (current_block_->pre() == nullptr) {
        exec_state_->block_state_.reset(current_block_->block_state());
    }
    current_block_.reset(current_block_->pre());
}

void CodeGenerator::Visit(ConstantNode* node, void* data) {
    long reg = data == nullptr ? -1 : *static_cast<long*>(data);
    BlockState* state = current_block_->block_state();
    if (node->IsBool()) {
        int index = state->AddConstant(static_cast<BoolNode *>(node)->value());
        Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
        state->AddInstruction(i);
    } else if (node->IsInteger()) {
        int index = state->AddConstant(static_cast<IntegerNode *>(node)->value());
        Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
        state->AddInstruction(i);
    } else if (node->IsNumber()) {
        int index = state->AddConstant(static_cast<NumberNode *>(node)->value());
        Instruction i = CREATE_ABC(OpCode::OP_LOADK, reg, index, 0);
        state->AddInstruction(i);
    } else if (node->IsUndefined() || node->IsNull()) {
        Instruction i = CREATE_ABC(OpCode::OP_LOADNULL, reg, 0, 0);
        state->AddInstruction(i);
    }
}

void CodeGenerator::Visit(UnaryExpressionNode* node, void* data) {}
void CodeGenerator::Visit(BinaryExpressionNode* node, void* data) {}

void CodeGenerator::Visit(VariableNode* node, void* data) {
    long reg_a = data == nullptr ? -1 : *static_cast<long*>(data);
    BlockState* state = current_block_->block_state();
    current_block_->upvalue()[node->identifier()] = reg_a;

    int reg_b = current_block_->FindRegisterId(node->identifier());
    if (reg_b >= 0) {
        state->AddInstruction(CREATE_ABC(OP_MOVE, reg_a, reg_b, 0));
    }

    int index = exec_state_->global()->IndexOf(node->identifier());
    if (index >= 0) {
        state->AddInstruction(CREATE_ABC(OP_GETGLOBAL, reg_a, index, 0));
    }

}

void CodeGenerator::Visit(AssignmentNode* node, void* data) {
    RegisterIdScope scope(current_block_.get());
    long reg = current_block_->NextRegisterId();
    current_block_->upvalue()[node->identifier()] = reg;
    if (node->expression() != nullptr) {
        node->expression()->Accept(this, &reg);
    } else {
        BlockState* state = current_block_->block_state();
        state->AddInstruction(CREATE_ABC(OP_LOADNULL, reg, 0, 0));
    }
}

void CodeGenerator::Visit(DotAccessorNode* node, void* data) {}
void CodeGenerator::Visit(ExpressionListNode* node, void* data) {}
void CodeGenerator::Visit(FunctionCallNode* node, void* data) {}
void CodeGenerator::Visit(ForNode* node, void* data) {}
void CodeGenerator::Visit(IfElseNode* node, void* data) {}

void CodeGenerator::Visit(BlockNode* node, void* data) {
  BlockScope block_scoped(this);
  for (auto it = node->statements().begin(); it != node->statements().end();
       ++it) {
    (*it)->Accept(this, nullptr);
  }
}

void CodeGenerator::Visit(ChunkNode* node, void* data) {
  node->block()->Accept(this, nullptr);
}

}  // namespace data_render
}  // namespace core
}  // namespace weex