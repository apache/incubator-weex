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

#ifndef CORE_DATA_RENDER_CODE_GENERATOR_H_
#define CORE_DATA_RENDER_CODE_GENERATOR_H_

#include <unordered_map>
#include "core/data_render/ast.h"
#include "core/data_render/op_code.h"

namespace weex {
namespace core {
namespace data_render {
class BlockState;
class ExecState;
class CodeGenerator : public Visitor {
 public:
  CodeGenerator() : exec_state_(nullptr), current_block_(nullptr) {}
  ~CodeGenerator() {}
  void Visit(ConstantNode* node, void* data) override;
  void Visit(UnaryExpressionNode* node, void* data) override;
  void Visit(BinaryExpressionNode* node, void* data) override;
  void Visit(VariableNode* node, void* data) override;
  void Visit(AssignmentNode* node, void* data) override;
  void Visit(DotAccessorNode* node, void* data) override;
  void Visit(ExpressionListNode* node, void* data) override;
  void Visit(FunctionCallNode* node, void* data) override;
  void Visit(ForNode* node, void* data) override;
  void Visit(IfElseNode* node, void* data) override;
  void Visit(BlockNode* node, void* data) override;
  void Visit(ChunkNode* node, void* data) override;

 private:
  class BlockCnt {
   public:
    BlockCnt()
        : block_state_(nullptr), upvalue_(), pre_(), idx_(0), is_loop_(false) {}

    inline long NextRegisterId() { return idx_++; }

    inline long FindRegisterId(const std::string& name) {
      auto iter = upvalue_.find(name);
      if (iter != upvalue_.end()) {
        return iter->second;
      }
      if (pre_ != nullptr) {
        return pre_->FindRegisterId(name);
      }
      return -1;
    }

    inline void set_block_state(BlockState* block_state) {
      block_state_ = block_state;
    }
    inline BlockState* block_state() { return block_state_; }
    inline std::unordered_map<std::string, long>& upvalue() { return upvalue_; }
    inline BlockCnt* pre() { return pre_; }
    inline void set_pre(BlockCnt* pre) { pre_ = pre; }
    inline void set_idx(int idx) { idx_ = idx; }
    inline int idx() { return idx_; }
    inline bool is_loop() { return is_loop_; }

   private:
    BlockState* block_state_;
    std::unordered_map<std::string, long> upvalue_;
    BlockCnt* pre_;
    int idx_;
    bool is_loop_;
  };
  class RegisterIdScope {
   public:
    RegisterIdScope(BlockCnt* block) : block_(block), save_(block->idx()) {}

    ~RegisterIdScope() { block_->set_idx(save_); }

   private:
    BlockCnt* block_;
    int save_;
  };
  class BlockScope {
   public:
    BlockScope(CodeGenerator* cg) : cg_(cg) { cg_->EnterBlock(); }

    ~BlockScope() { cg_->LeaveBlock(); }

   private:
    CodeGenerator* cg_;
  };
  void EnterBlock();
  void LeaveBlock();
  ExecState* exec_state_;
  std::unique_ptr<BlockCnt> current_block_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_CODE_GENERATOR_H_
