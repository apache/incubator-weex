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
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "core/data_render/op_code.h"

namespace weex {
namespace core {
namespace data_render {

class VM;
class Frame;
class Value;
class ExecState;
class FuncState;
class String;
class StringTable;

typedef Value (*CFunction)(ExecState*);

struct Value {
  union {
    int64_t i;
    double n;
    bool b;
    FuncState* f;
    void* cf;
    String* str;
  };

  enum Type { NIL, INT, NUMBER, BOOL, FUNC, CFUNC, STRING };

  Type type;

  Value() : type(NIL) {}
  Value(int64_t value) : i(value), type(INT) {}
  Value(double value) : n(value), type(NUMBER) {}
  Value(bool value) : b(value), type(BOOL) {}
  Value(String* value) : str(value), type(STRING) {}
  Value(const Value& value) {
    type = value.type;
    switch (type) {
      case NUMBER:
        n = value.n;
        break;
      case BOOL:
        b = value.b;
        break;
      case STRING:
        str = value.str;
        break;
      case FUNC:
        f = value.f;
        break;
      case CFUNC:
        cf = value.cf;
        break;
      default:
        break;
    }
  }

  friend bool operator==(const Value& left, const Value& right) {
    if (left.type != right.type) return false;
    switch (left.type) {
      case NIL:
        return true;
      case INT:
        return left.i == right.i;
      case NUMBER:
        return fabs(left.n - right.n) < 0.000001;
      case BOOL:
        return left.b == right.b;
      case STRING:
        return left.str == right.str;
      case FUNC:
        return left.f == right.f;
      case CFUNC:
        return left.cf == right.cf;
      default:
        break;
    }
    return false;
  }
};

class FuncState {
 public:
  FuncState() : instructions_(), constants_(), children_() {}

  int AddConstant(Value value) {
    for (auto i = 0; i != constants_.size(); ++i) {
      if (constants_[i] == value) {
        return i;
      }
    }
    constants_.push_back(std::move(value));
    return constants_.size() - 1;
  }

  inline Value* GetConstant(int index) { return &constants_[index]; }
  inline size_t AddInstruction(Instruction i) {
    instructions_.push_back(i);
    return instructions_.size() - 1;
  }
  inline std::vector<Instruction>& instructions() { return instructions_; }
  inline void AddChild(FuncState* func) {
    children_.push_back(std::unique_ptr<FuncState>(func));
  }
  inline std::vector<std::unique_ptr<FuncState>>& children() {
    return children_;
  }

 private:
  std::vector<Instruction> instructions_;
  std::vector<Value> constants_;
  std::vector<std::unique_ptr<FuncState>> children_;
};

class ExecStack {
 public:
  ExecStack() : stack_(512) {}
  Value** top() { return &top_; }
  Value* base() { return &stack_[0]; }

 private:
  std::vector<Value> stack_;
  Value* top_;
};

class Global {
 public:
  Value* Find(int index);
  int IndexOf(const std::string& name);
  int Add(const std::string& name, Value value);

 private:
  std::map<std::string, int> map_;
  std::vector<Value> values_;
};

class ExecState {
 public:
  ExecState(VM* vm);
  ~ExecState();
  void Compile(const std::string& source);
  void Execute();
  void CallFunction(Value* func, size_t argc, Value* ret);

  size_t GetArgumentCount();
  Value* GetArgument(int index);

  inline Global* global() { return global_.get(); }
  inline ExecStack* stack() { return stack_.get(); }
  inline StringTable* string_table() { return string_table_.get(); }

 private:
  std::vector<Frame> frames_;
  std::unique_ptr<ExecStack> stack_;
  friend class CodeGenerator;
  std::unique_ptr<FuncState> func_state_;
  std::unique_ptr<Global> global_;
  std::unique_ptr<StringTable> string_table_;
  VM* vm_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // WEEX_PROJECT_CONTEXT_H
