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
#include "core/data_render/ast.h"
#include "core/data_render/ast_visitor.h"
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
  void Visit(ChunkStatement *node, void *data) override;
  void Visit(StringConstant *node, void *data) override;
  void Visit(CallExpression *node, void *data) override;
  void Visit(ArgumentList *node, void *data) override;
  void Visit(IfStatement *node, void *data) override;
  void Visit(IfElseStatement *node, void *data) override;
  void Visit(ForStatement *node, void *data) override;
  void Visit(BlockStatement *node, void *data) override;
  void Visit(FunctionPrototype *node, void *data) override;
  void Visit(FunctionStatement *node, void *data) override;
  void Visit(BinaryExpression *node, void *data) override;
  void Visit(TernaryExpression *node, void *data) override;
  void Visit(CommaExpression *node, void *data) override;
  void Visit(AssignExpression *node, void *data) override;
  void Visit(Declaration *node, void *data) override;
  void Visit(DeclarationList *node, void *data) override;
  void Visit(BooleanConstant *node, void *data) override;
  void Visit(DoubleConstant *node, void *data) override;
  void Visit(IntegralConstant *node, void *data) override;
  void Visit(MemberExpression *node, void *data) override;
  void Visit(Identifier *node, void *data) override;
  void Visit(PrefixExpression *node, void *data) override;
  void Visit(ObjectConstant *node, void *data) override;
  void Visit(ArrayConstant *node, void *data) override;
  void Visit(ReturnStatement *node, void *data) override;
  void Visit(ExpressionList *node, void *data) override;

 private:
  template <class T>
  class Node {
   public:
    Node() : parent_(nullptr) {}
    virtual ~Node() {}
    inline T *parent() { return parent_; }
    inline void set_parent(T *t) { parent_ = t; }

   private:
    T *parent_;
  };

  class BlockCnt : public Node<BlockCnt> {
   public:
    BlockCnt() : variables_(), idx_(0), is_loop_(false) {}
    ~BlockCnt() {}

    inline long NextRegisterId() { return idx_++; }

    inline long FindRegisterId(const std::string &name) {
      auto iter = variables_.find(name);
      if (iter != variables_.end()) {
        return iter->second;
      }
      if (parent() != nullptr) {
        return parent()->FindRegisterId(name);
      }
      return -1;
    }

    inline std::unordered_map<std::string, long> &variables() {
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

  class FuncCnt : public Node<FuncCnt> {
   public:
    FuncCnt() {}
    ~FuncCnt() {}
    inline void set_func_state(FuncState *func_state) {
      func_state_ = func_state;
    }
    inline void set_root_block(BlockCnt *root_block) {
      root_block_ = root_block;
    }
    inline void set_current_block(BlockCnt *current_block) {
      current_block_ = current_block;
    }
    inline FuncState *func_state() { return func_state_; }
    inline BlockCnt *root_block() { return root_block_; }
    inline BlockCnt *current_block() { return current_block_; }

   private:
    FuncState *func_state_;
    BlockCnt *root_block_;
    BlockCnt *current_block_;
  };

  class RegisterScope {
   public:
    RegisterScope(BlockCnt *block) : stored_idx_(block->idx()), block_(block) {}
    ~RegisterScope() { block_->set_idx(stored_idx_); }

   private:
    long stored_idx_;
    BlockCnt *block_;
    DISALLOW_COPY_AND_ASSIGN(RegisterScope);
  };

  class BlockScope {
   public:
    BlockScope(CodeGenerator *cg) : cg_(cg) { cg_->EnterBlock(); }
    ~BlockScope() { cg_->LeaveBlock(); }

   private:
    CodeGenerator *cg_;
    DISALLOW_COPY_AND_ASSIGN(BlockScope);
  };

  class FuncScope {
   public:
    FuncScope(CodeGenerator *cg) : cg_(cg) { cg_->EnterFunction(); }
    ~FuncScope() { cg_->LeaveFunction(); }

   private:
    CodeGenerator *cg_;
    DISALLOW_COPY_AND_ASSIGN(FuncScope);
  };

  void EnterFunction();
  void LeaveFunction();
  void EnterBlock();
  void LeaveBlock();
  ExecState *exec_state_;
  std::unique_ptr<FuncCnt> cur_func_;
  std::unique_ptr<BlockCnt> cur_block_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_CODE_GENERATOR_H_
