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
#include "base/common.h"
#include "core/data_render/ast_visitor.h"
#include "core/data_render/ast.h"
#include "core/data_render/op_code.h"

namespace weex {
namespace core {
namespace data_render {
class FuncState;
class ExecState;
class CodeGenerator : public ASTVisitor {
 public:
  CodeGenerator(ExecState *exec_state)
      : exec_state_(exec_state), cur_block_(nullptr) {}
  ~CodeGenerator() {}
  void Visit(ChunkStatement *node) override;
  void Visit(StringLiteral *node) override;
//  void Visit(UnaryExpressionNode* node, void* data) override;
//  void Visit(BinaryExpressionNode* node, void* data) override;
//  void Visit(VariableNode* node, void* data) override;
//  void Visit(AssignmentNode* node, void* data) override;
//  void Visit(VarDeclareNode* node, void* data) override;
//  void Visit(DotAccessorNode* node, void* data) override;
//  void Visit(ExpressionListNode* node, void* data) override;
//  void Visit(FunctionCallNode* node, void* data) override;
//  void Visit(ForNode* node, void* data) override;
//  void Visit(IfElseNode* node, void* data) override;
//  void Visit(FunctionNode* node, void* data) override;
//  void Visit(BlockNode* node, void* data) override;

 private:
  template <class T>
  class Node {
   public:
    Node() : parent_(nullptr) {}
    virtual ~Node() {}
    inline T* parent() { return parent_; }
    inline void set_parent(T* t) { parent_ = t; }

   private:
    T* parent_;
  };

  class FuncCnt : public Node<FuncCnt> {
   public:
    FuncCnt() {}
    ~FuncCnt() {}
    inline std::unordered_map<std::string, long>& upvalue() { return upvalue_; }
    inline void set_func_state(FuncState* func_state) {
      func_state_ = func_state;
    }
    inline FuncState* func_state() { return func_state_; }

   private:
    FuncState* func_state_;
    std::unordered_map<std::string, long> upvalue_;
  };

  class BlockCnt : public Node<BlockCnt> {
   public:
    BlockCnt() : variables_(), idx_(0), is_loop_(false) {}
    ~BlockCnt() {}

    inline long NextRegisterId() { return idx_++; }

    inline long FindRegisterId(const std::string& name) {
      auto iter = variables_.find(name);
      if (iter != variables_.end()) {
        return iter->second;
      }
      if (parent() != nullptr) {
        return parent()->FindRegisterId(name);
      }
      return -1;
    }

    inline std::unordered_map<std::string, long>& variables() {
      return variables_;
    }
    inline void set_idx(int idx) { idx_ = idx; }
    inline int idx() { return idx_; }
    inline bool is_loop() { return is_loop_; }

   private:
    std::unordered_map<std::string, long> variables_;
    int idx_;
    bool is_loop_;
  };

  class RegisterScope {
   public:
    RegisterScope(BlockCnt* block) : stored_idx_(block->idx()), block_(block) {}
    ~RegisterScope() { block_->set_idx(stored_idx_); }

   private:
    long stored_idx_;
    BlockCnt* block_;
    DISALLOW_COPY_AND_ASSIGN(RegisterScope);
  };

  class BlockScope {
   public:
    BlockScope(CodeGenerator* cg) : cg_(cg) { cg_->EnterBlock(); }
    ~BlockScope() { cg_->LeaveBlock(); }

   private:
    CodeGenerator* cg_;
    DISALLOW_COPY_AND_ASSIGN(BlockScope);
  };

  class FuncScope {
   public:
    FuncScope(CodeGenerator* cg) : cg_(cg) { cg_->EnterFunction(); }
    ~FuncScope() { cg_->LeaveFunction(); }

   private:
    CodeGenerator* cg_;
    DISALLOW_COPY_AND_ASSIGN(FuncScope);
  };

  void EnterFunction();
  void LeaveFunction();
  void EnterBlock();
  void LeaveBlock();
  ExecState* exec_state_;
  std::unique_ptr<FuncCnt> cur_func_;
  std::unique_ptr<BlockCnt> cur_block_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_CODE_GENERATOR_H_
