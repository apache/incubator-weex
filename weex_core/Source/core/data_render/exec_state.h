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

#ifndef WEEX_PROJECT_CONTEXT_H
#define WEEX_PROJECT_CONTEXT_H

#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include "core/data_render/op_code.h"

namespace weex {
namespace core {
namespace data_render {

class VM;

struct Value {
  union {
    int i;
    double n;
    bool b;
  };

  enum Type { Nil, Integer, Number, Boolean };

  Type type;

  Value(int value) : i(value), type(Integer) {}
  Value(double value) : n(value), type(Number) {}
  Value(bool value) : b(value), type(Boolean) {}

  friend bool operator==(const Value& left, const Value& right) {
    if (left.type != right.type) return false;
    switch (left.type) {
      case Nil:
        return true;
      case Integer:
        return left.i == right.i;
      case Number:
        return fabs(left.n - right.n) < 0.000001;
      case Boolean:
        return left.b == right.b;
      default:
        break;
    }
    return false;
  }
};

struct UpvalDesc {
  std::string name_;
  long register_id_;
  bool in_parent_stack_;

  UpvalDesc(const std::string& name, long register_id, bool in_parent_stack)
      : name_(name),
        register_id_(register_id),
        in_parent_stack_(in_parent_stack) {}
};

class BlockState {
 public:
  BlockState() : instructions_(), constants_(), next_(nullptr), pre_(nullptr) {}
  ~BlockState() {}

  void AddInstruction(Instruction i) { instructions_.push_back(i); }

  int AddConstant(Value value) {
    for (auto i = 0; i != constants_.size(); ++i) {
      if (constants_[i] == value) {
        return i;
      }
    }
    constants_.push_back(std::move(value));
    return constants_.size() - 1;
  }

  void set_next(BlockState* block) { next_ = block; }

  BlockState* next() { return next_.get(); }

 private:
  std::vector<Instruction> instructions_;
  std::vector<Value> constants_;
  std::unique_ptr<BlockState> next_;
  BlockState* pre_;
};

// TODO
class FuncState {
 public:
  FuncState() : upvalues_() {}

 private:
  std::vector<UpvalDesc> upvalues_;
};

class ExecStack {
 public:
  ExecStack() : stack_() {}
  Value& top() { return stack_[0]; }
  Value& base() { return stack_[0]; }

 private:
  std::vector<Value> stack_;
};

class Global {
 public:
  int IndexOf(const std::string&) { return 0; }
};

class ExecState {
 public:
  void Compile(const std::string& source);
  void Execute();

  Global* global() { return global_.get(); }
  ExecStack* stack() { return stack_.get(); }

 private:
  std::unique_ptr<ExecStack> stack_;
  friend class CodeGenerator;
  std::unique_ptr<BlockState> block_state_;
  std::unique_ptr<Global> global_;
  VM* vm_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // WEEX_PROJECT_CONTEXT_H
