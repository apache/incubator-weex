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

#ifndef CORE_DATA_RENDER_EXEC_STATE_H
#define CORE_DATA_RENDER_EXEC_STATE_H

#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "core/data_render/object.h"
#include "core/data_render/op_code.h"
#include "core/data_render/class_factory.h"
#include "core/data_render/vnode/vnode.h"
#include "core/data_render/vnode/vnode_render_context.h"

namespace weex {
namespace core {
namespace data_render {
    
#define VM_EXEC_STACK_SIZE               512

class ValueRef {
    friend class ExecState;
public:
    ValueRef(FuncState *func_state, long register_id) : func_state_(func_state), register_id_(register_id), ref_id_(gs_ref_id++), func_index_(-1) { SetNil(&value_); };
    inline int ref_id() { return ref_id_; }
    inline void SetClosure(FuncClosure *closure) { closure_ = closure; }
    inline FuncState *func_state() { return func_state_; }
    inline long register_id() { return register_id_; }
    inline Value &value() { return value_; }
    inline bool &is_closure() { return is_closure_; }
    inline FuncClosure *closure() { return closure_; }
    static void reset() { gs_ref_id = 0; }
    ~ValueRef() {}
private:
    static int gs_ref_id;
    FuncState *func_state_{nullptr};
    int ref_id_;
    long register_id_;
    int func_index_;
    Value value_;
    FuncClosure *closure_{nullptr};
    bool is_closure_{false};
};
    
struct ClassInstance;

class FuncClosure {
public:
    explicit FuncClosure(ValueRef *ref);
    virtual ~FuncClosure() {}
    inline void SetValueRef(ValueRef *ref) { ref_ = ref; value_.ref = ref ? &ref->value() : nullptr;  }
    inline FuncState *func_state() { return func_state_; }
    inline long register_id() { return register_id_; }
    inline Value &value() { return value_; }
private:
    FuncState *func_state_{nullptr};
    long register_id_{-1};
    Value value_;
    ValueRef *ref_;
};

class FuncState {
 public:
  FuncState() : instructions_(), args_(), in_closure_(), out_closure_(), constants_(), children_(), super_index_(-1), in_closure_refs_(), out_closure_refs_() {}
  virtual ~FuncState() {}

  int AddConstant(Value value) {
    for (auto i = 0; i != constants_.size(); ++i) {
      if (constants_[i] == value) {
        return i;
      }
    }
    constants_.push_back(std::move(value));
    return (int)constants_.size() - 1;
  }
  void AddInClosure(ValueRef *ref) {
      if (std::find(in_closure_.begin(), in_closure_.end(), ref) == in_closure_.end()) {
          in_closure_.push_back(ref);
      }
  }
  void AddOutClosure(ValueRef *ref) {
      if (std::find(out_closure_.begin(), out_closure_.end(), ref) == out_closure_.end()) {
          out_closure_.push_back(ref);
      }
  }

  inline Value *GetConstant(int index) { return &constants_[index]; }
  inline size_t GetConstantSize() { return constants_.size();}
  inline bool Inclusive(long arg) { return std::find(args_.begin(), args_.end(), arg) == args_.end() ? false : true; }
  inline size_t AddInstruction(Instruction i) {
    instructions_.push_back(i);
    return instructions_.size() - 1;
  }
  inline size_t ReplaceInstruction(size_t pos, Instruction i) {
    instructions_[pos] = i;
    return pos;
  }
  inline std::vector<Instruction>& instructions() { return instructions_; }
  inline void AddChild(FuncState *func) {
    func->super_func_ = this;
    children_.push_back(std::unique_ptr<FuncState>(func));
  }
  inline const std::vector<std::unique_ptr<FuncState>>& children() {
    return children_;
  }
  inline FuncState *GetChild(size_t pos) { return children_[pos].get(); }
  inline std::vector<Value>& constants() { return constants_; }
  inline FuncState *super_func() { return super_func_; }
  inline void set_super_func(FuncState *func) { super_func_ = func; }
  inline void set_is_class_func(bool is_class_func) { is_class_func_ = is_class_func; }
  inline bool is_class_func() { return is_class_func_; }
  inline ClassInstance * &class_inst() { return class_inst_; }
  inline int &argc() { return argc_; }
  inline std::vector<long> &args() { return args_; }
  std::vector<FuncState *> all_childrens() {
      std::vector<FuncState *> all_childrens;
      for (auto &child : children_) {
          all_childrens.push_back(child.get());
          std::vector<FuncState *> children = child->all_childrens();
          all_childrens.insert(all_childrens.end(), children.begin(), children.end());
      }
      return all_childrens;
  }
  inline std::vector<ValueRef *> &in_closure() { return in_closure_; }
  inline std::vector<ValueRef *> &out_closure() { return out_closure_; }
  inline int super_index() const { return super_index_; }
  inline void set_super_index(int super_index) { super_index_ = super_index; }
  inline std::vector<int32_t>& in_closure_refs() { return in_closure_refs_; }
  inline std::vector<int32_t>& out_closure_refs() { return out_closure_refs_; }
 private:
  std::vector<Instruction> instructions_;
  std::vector<Value> constants_;
  std::vector<std::unique_ptr<FuncState>> children_;
  std::vector<long> args_;
  std::vector<ValueRef *> in_closure_;
  std::vector<ValueRef *> out_closure_;
  std::vector<int32_t> in_closure_refs_;
  std::vector<int32_t> out_closure_refs_;
  FuncState *super_func_{nullptr};
  int super_index_;
  bool is_class_func_{false};
  ClassInstance *class_inst_{nullptr};
  int argc_{0};
};
        
// TODO Each Func should contain a stack whose size is 256
class ExecStack {
 public:
  ExecStack() : stack_(VM_EXEC_STACK_SIZE), top_(&stack_[0]) {}
  Value** top() { return &top_; }
  Value* base() { return &stack_[0]; }
  void reset();
 private:
  std::vector<Value> stack_;
  Value *top_;
};
    
class ExecState {
 public:
  ExecState(VM *vm);
  virtual ~ExecState();
  void Compile(std::string& error);
  void Execute(std::string& error);
  const Value Call(const std::string& func_name, const std::vector<Value>& args);
  const Value Call(Value *func, const std::vector<Value>& args);
  const Value Call(FuncState *func, const std::vector<Value>& args);
  const Value Call(FuncInstance *func, const std::vector<Value>& args);
  size_t GetArgumentCount();
  Value *GetArgument(int index);
  ValueRef *AddRef(FuncState *func_state, long register_id);
  ValueRef *FindRef(int index);
  void Register(const std::string& name, CFunction function);
  void Register(const std::string& name, Value value);
  inline std::vector<ValueRef *> &refs() { return refs_; };
  inline Variables *global() { return global_.get(); }
  inline void reset(FuncState *func_state) { func_state_.reset(func_state); }
  inline FuncState *func_state() { return func_state_.get(); }
  inline ExecStack *stack() { return stack_.get(); }
  inline StringTable *string_table() { return string_table_.get(); }
  inline VNodeRenderContext *context() { return render_context_.get(); }
  inline ClassFactory *class_factory() { return class_factory_.get(); }
  inline uint32_t global_compile_index() { return global_compile_index_; }
  inline uint32_t class_compile_index() { return class_compile_index_; }
  inline uint32_t string_compile_index() { return string_compile_index_; }
  inline std::unordered_map<std::string, long>& global_variables() { return global_variables_; }
 private:
  friend class VM;
  friend class CodeGenerator;
  void resetArguments(Value *func, size_t argc);
  void CallFunction(Value *func, size_t argc, Value *ret);
  int findSuperIndex(const std::vector<FuncState*>& func_states);
  VM *vm_;
  std::unique_ptr<ClassFactory> class_factory_;
  std::vector<Frame> frames_;
  std::vector<ValueRef *> refs_;
  std::unique_ptr<Variables> global_;
  std::unique_ptr<ExecStack> stack_;
  std::unique_ptr<FuncState> func_state_;
  std::unique_ptr<StringTable> string_table_;
  std::unique_ptr<VNodeRenderContext> render_context_;
  std::unordered_map<std::string, long> global_variables_;
  std::unordered_map<int, json11::Json> styles_;
  uint32_t global_compile_index_{0};
  uint32_t class_compile_index_{0};
  uint32_t string_compile_index_{0};
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_EXEC_STATE_H
