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

#include "core/data_render/exec_state.h"
#include "core/data_render/code_generator.h"
#include "core/data_render/string_table.h"
#include "core/data_render/vm.h"

namespace weex {
namespace core {
namespace data_render {

Value* Global::Find(int index) {
  if (index >= values_.size() || index < 0) {
    return nullptr;
  }
  return &values_[index];
}

int Global::IndexOf(const std::string& name) {
  auto iter = map_.find(name);
  if (iter != map_.end()) {
    return iter->second;
  }
  return -1;
}

int Global::Add(const std::string& name, Value value) {
  auto iter = map_.find(name);
  if (iter != map_.end()) {
    return iter->second;
  }
  values_.push_back(value);
  int index = values_.size() - 1;
  map_.insert(std::make_pair(name, index));
  return index;
}

ExecState::ExecState(VM* vm)
    : frames_(),
      stack_(new ExecStack),
      func_state_(nullptr),
      global_(new Global),
      string_table_(new StringTable),
      vm_(vm) {}

ExecState::~ExecState() {}

void ExecState::Compile(const std::string& source) {
  CodeGenerator generator(this);

  Json* json = new Json();
  Handle<ExpressionList> stmts = new ExpressionList();

  ChunkStatement* chunk =
      new ChunkStatement(*json, stmts);  // new ChunkStatement(NULL, stmts);

  Handle<IntegralLiteral> intLiteral1 = new IntegralLiteral(*json, 1);
  Handle<IntegralLiteral> intLiteral2 = new IntegralLiteral(*json, 2);

  Declaration* declaration = new Declaration(*json, "aa", intLiteral1);
  Declaration* declaration1 = new Declaration(*json, "bb", intLiteral2);

  std::vector<Handle<Expression>> args;

  Handle<Expression> lhs = nullptr;
  Handle<Expression> rhs = nullptr;

  BinaryExpression* addOperation = new BinaryExpression(
      *json, BinaryOperation::kAddition, declaration, declaration1);

  chunk->PushExpression(addOperation);

  generator.Visit(chunk, nullptr);

  //    Handle<StringLiteral> stringLiteral = new StringLiteral(*json, "string
  //    1");
  //
  //    Declaration *declaration = new Declaration(*json, "aa", stringLiteral);
  //
  //    Handle<Expression> callee = new Identifier(*json, "log");
  //
  //    std::vector<Handle<Expression>> args;
  //
  //    Handle<Expression> arg1 = new Identifier(*json, "aa");
  //
  //    args.push_back(Handle<Expression>(arg1));
  //
  //    CallExpression *callExpression = new CallExpression(*json, callee,
  //    args);
  //
  //    chunk->PushExpression(declaration);
  //    chunk->PushExpression(callExpression);
  //
  //    generator.Visit(chunk, nullptr);
}

void ExecState::Execute() {
  Value chunk;
  chunk.type = Value::Type::FUNC;
  chunk.f = func_state_.get();
  *stack_->base() = chunk;
  CallFunction(stack_->base(), 0, nullptr);
}

void ExecState::CallFunction(Value* func, size_t argc, Value* ret) {
  if (func->type == Value::Type::CFUNC) {
    Frame frame;
    *stack_->top() = func + argc;
    frame.reg = func;
    frames_.push_back(frame);
    auto result = reinterpret_cast<CFunction>(func->cf)(this);
    if (ret != nullptr) {
      *ret = result;
    }
    frames_.pop_back();
  } else {
    Frame frame;
    frame.func = func;
    frame.reg = func;
    frame.pc = &(*func->f->instructions().begin());
    frame.end = &(*func->f->instructions().end());
    frames_.push_back(frame);
    vm_->RunFrame(this, frame);
    frames_.pop_back();
  }
}

size_t ExecState::GetArgumentCount() {
  return *stack_->top() - frames_.back().reg;
}

Value* ExecState::GetArgument(int index) {
  return frames_.back().reg + index + 1;
}
void ExecState::setVNodeRoot(VNode* v_node) {
  root_.reset(v_node);
}
VNode* ExecState::find_node(const std::string& ref) {
  auto it = node_map_.find(ref);
  if (it == node_map_.end()){
    return nullptr;
  }
  return it->second;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
