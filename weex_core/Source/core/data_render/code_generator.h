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
#include "core/data_render/object.h"

namespace weex {
namespace core {
namespace data_render {
class ValueRef;
class FuncState;
class ExecState;
class CodeGenerator : public ASTVisitor {
 public:
  CodeGenerator(ExecState *exec_state)
      : exec_state_(exec_state), block_(nullptr),func_(nullptr),class_(nullptr) {}
  ~CodeGenerator() {}
  void Visit(ChunkStatement *node, void *data) override;
  void Visit(StringConstant *node, void *data) override;
  void Visit(RegexConstant *node, void *data) override;
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
  void Visit(NullConstant *node, void *data) override;
  void Visit(UndefinedConstant *node, void *data) override;
  void Visit(IntegralConstant *node, void *data) override;
  void Visit(MemberExpression *node, void *data) override;
  void Visit(Identifier *node, void *data) override;
  void Visit(PrefixExpression *node, void *data) override;
  void Visit(ObjectConstant *node, void *data) override;
  void Visit(ArrayConstant *node, void *data) override;
  void Visit(ReturnStatement *node, void *data) override;
  void Visit(ExpressionList *node, void *data) override;
  void Visit(JSXNodeExpression *node, void *data) override;
  void Visit(ClassStatement *node, void *data) override;
  void Visit(ClassBody *node, void *data) override;
  void Visit(ArrowFunctionStatement *node, void *data) override;
  void Visit(ThisExpression *node, void *data) override;
  void Visit(NewExpression *node, void *data) override;
  void Visit(PostfixExpression *node, void *data) override;
  void Visit(ContinueStatement *node, void *data) override;
  void Visit(SwitchStatement *node, void *data) override;
  void Visit(TryCatchStatement *node, void *data) override;
  void Visit(BreakStatement *node, void *data) override;
  void Visit(ClassProperty *node, void *data) override;

 private:
  template <class T>
  class Node {
   public:
      Node() : parent_(nullptr), children_{nullptr} {}
    virtual ~Node() {}
    inline T *parent() { return parent_; }
    inline void set_parent(T *t) { parent_ = t; }
    inline T *children() { return children_; }
    inline void set_children(T *t) { children_ = t; }

   private:
    T *parent_;
    T *children_;
  };

  class BlockCnt : public Node<BlockCnt> {
   public:
    BlockCnt() : for_continue_slots_(), for_break_slots_(), reg_refs_(), variables_(), idx_(0), is_loop_(false) {}
    ~BlockCnt() {}

    inline long NextRegisterId() { return idx_++; }

    long FindRegisterId(const std::string &name);
      
    bool FindVariable(const std::string &name);
    void AddVariable(const std::string &name, long reg);

    inline void set_func_state(FuncState *func_state) {
      func_state_ = func_state;
    }
    inline FuncState *func_state() { return func_state_; }
    inline void set_exec_state(ExecState *exec_state) {
      exec_state_ = exec_state;
    }
    
    inline ExecState *exec_state() { return exec_state_; }
    int for_start_index();
    int for_update_index();
    std::vector<size_t>& for_continue_slots();
    std::vector<size_t>& break_slots();
    void set_for_start_index(int index) { for_start_index_ = index; }
    void set_for_update_index(int index) { for_update_index_ = index; }
    inline std::unordered_map<std::string, long> &variables() {
      return variables_;
    }
    inline void set_idx(int idx) { idx_ = idx; }
    inline int idx() { return idx_; }
    inline bool is_loop() { return is_loop_; }
    inline void set_is_loop(bool is_loop) { is_loop_ = is_loop; }
    inline bool is_switch() { return is_switch_; }
    inline void set_is_switch(bool flag) { is_switch_ = flag; }
    inline std::vector<long>& reg_refs() { return reg_refs_; }
    inline bool& is_register_scope() { return is_register_scope_; }
    void reset();
   private:
    bool FindRegisterId(const std::string &name, long &ret);
    ValueRef *FindValueRef(const std::string &name, long &reg_ref);
    std::unordered_map<std::string, long> variables_;
    std::vector<long> reg_refs_;
    int idx_;
    bool is_loop_;
    bool is_switch_;
    bool is_register_scope_;
    FuncState *func_state_{nullptr};
    ExecState *exec_state_{nullptr};
    int for_start_index_{-1};
    int for_update_index_{-1};
    std::vector<size_t> for_break_slots_;
    std::vector<size_t> for_continue_slots_;
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

  class ClassCnt : public Node<ClassCnt> {
    public:
      ClassCnt() {}
      ~ClassCnt() {}
      inline void set_class_value(Value *class_value) {
          class_value_ = class_value;
      }
      inline Value *class_value() { return class_value_; }
      inline std::string &class_name() { return class_name_; }
    private:
      Value *class_value_;
      std::string class_name_;
  };
  class RegisterScope {
   public:
    RegisterScope(BlockCnt *block) : stored_idx_(block->idx()), block_(block) {
        block_->is_register_scope() = true;
    }
      ~RegisterScope();

   private:
    int stored_idx_;
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
  class ClassScope {
    public:
        ClassScope(CodeGenerator *cg, Value *class_value) : cg_(cg) { cg_->EnterClass(class_value); }
        ~ClassScope() { cg_->LeaveClass(); }
    private:
        CodeGenerator *cg_;
        DISALLOW_COPY_AND_ASSIGN(ClassScope);
  };
  void AddCallInstruction(long ret, OPCode code, std::vector<long> orders);
  void EnterClass(Value *class_value);
  void LeaveClass();
  void EnterFunction();
  void LeaveFunction();
  void EnterBlock();
  void LeaveBlock();
  ExecState *exec_state_;
  FuncCnt *func_;
  BlockCnt *block_;
  ClassCnt *class_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_CODE_GENERATOR_H_
