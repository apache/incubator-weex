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

#ifndef CORE_DATA_RENDER_CONTEXT_H_
#define CORE_DATA_RENDER_CONTEXT_H_

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
    ValueRef(FuncState *func_state, long register_id) : func_state_(func_state), register_id_(register_id), ref_id_(gs_ref_id++) { SetNil(&value_); };
    inline int ref_id() { return ref_id_; }
    inline FuncState *func_state() { return func_state_; }
    inline long register_id() { return register_id_; }
    inline Value &value() { return value_; }
    ~ValueRef() {}
private:
    static int gs_ref_id;
    FuncState *func_state_{nullptr};
    int ref_id_;
    long register_id_;
    Value value_;
};

class FuncState {
 public:
  FuncState() : instructions_(), constants_(), children_() {}
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

  inline Value* GetConstant(int index) { return &constants_[index]; }
  inline size_t AddInstruction(Instruction i) {
    instructions_.push_back(i);
    return instructions_.size() - 1;
  }
  inline size_t ReplaceInstruction(size_t pos, Instruction i) {
    instructions_[pos] = i;
    return pos;
  }
  inline std::vector<Instruction>& instructions() { return instructions_; }
  inline void AddChild(FuncState* func) {
    children_.push_back(std::unique_ptr<FuncState>(func));
  }
  inline std::vector<std::unique_ptr<FuncState>>& children() {
    return children_;
  }
  inline FuncState* GetChild(size_t pos) { return children_[pos].get(); }

 private:
  std::vector<Instruction> instructions_;
  std::vector<Value> constants_;
  std::vector<std::unique_ptr<FuncState>> children_;
};
    
// TODO Each Func should contain a stack whose size is 256
class ExecStack {
 public:
  ExecStack() : stack_(VM_EXEC_STACK_SIZE) {}
  Value** top() { return &top_; }
  Value* base() { return &stack_[0]; }
  void reset();
 private:
  std::vector<Value> stack_;
  Value* top_;
};

class ExecState {
 public:
  ExecState(VM* vm);
  virtual ~ExecState() {}
  void Compile(std::string& error);
  void Execute(std::string& error);
  const Value Call(const std::string& func_name, const std::vector<Value>& params);
  const Value Call(Value *func, const std::vector<Value>& params);
  size_t GetArgumentCount();
  Value* GetArgument(int index);
  ValueRef *AddRef(FuncState *func_state, long register_id);
  ValueRef *FindRef(int index);
  std::vector<ValueRef *> &refs() { return refs_; };
  inline Variables* global() { return global_.get(); }
  inline ExecStack* stack() { return stack_.get(); }
  inline StringTable* string_table() { return string_table_.get(); }
  inline VNodeRenderContext* context() { return render_context_.get(); }
  inline ClassFactory *class_factory() { return class_factory_.get(); }

 private:
  friend class VM;
  friend class CodeGenerator;
  void CallFunction(Value *func, size_t argc, Value *ret);

  VM* vm_;

  std::unique_ptr<ClassFactory> class_factory_;


  std::vector<Frame> frames_;
  std::vector<ValueRef *> refs_;
  std::unique_ptr<Variables> global_;
  std::unique_ptr<ExecStack> stack_;
  std::unique_ptr<FuncState> func_state_;
  std::unique_ptr<StringTable> string_table_;
  std::unique_ptr<VNodeRenderContext> render_context_;
  std::unordered_map<std::string, long> global_variables_;
};
    
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_CONTEXT_H_
