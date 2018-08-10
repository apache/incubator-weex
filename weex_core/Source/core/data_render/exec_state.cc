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
#include "core/data_render/parser.h"
#include "core/data_render/rax_parser_builder.h"
#include "core/data_render/rax_parser.h"
#include "core/data_render/rax_parser_context.h"
#include "core/data_render/common_error.h"

#if DEBUG
#include "core/data_render/monitor/vm_monitor.h"
#endif

namespace weex {
namespace core {
namespace data_render {

ExecState::ExecState(VM* vm)
    : vm_(vm),
      frames_(),
      global_(new Variables),
      stack_(new ExecStack),
      func_state_(nullptr),
      string_table_(new StringTable),
      render_context_(new VNodeRenderContext),
      factory_(new TableFactory()),
      class_factory_(new ClassFactory()),
      global_variables_() {}

void ExecState::Compile() {

#if DEBUG
  TimeCost tc("Compile");
#endif
  CodeGenerator generator(this);
  std::string err;
  if (!context()->raw_json().is_null()) {
      ParseResult result = Parser::Parse(context()->raw_json(),err);
      generator.Visit(result.expr().get(), nullptr);
  }
  else if (context()->raw_source().length() > 0) {
      weex::core::data_render::RAXParserBuilder builder(context()->raw_source());
      weex::core::data_render::RAXParser *parser = builder.parser();
      Handle<Expression> expr = nullptr;
      try {
          expr = ParseProgram(parser);
      }
      catch (std::exception &) {
          std::cout << "Parsed Error" << std::endl;
      }
      std::cout << "Parsed correctly" << std::endl;
      if (expr->IsChunkStatement()) {
          generator.Visit(expr->AsChunkStatement().get(), nullptr);
      }
  }
}

void ExecState::Execute() {
#if DEBUG
  TimeCost tc("ExecuteMain");
#endif
  Value chunk;
  chunk.type = Value::Type::FUNC;
  chunk.f = func_state_.get();
  *stack_->base() = chunk;
  try {
      CallFunction(stack_->base(), 0, nullptr);
  } catch (std::exception &e) {
      auto error = dynamic_cast<VMExecError *>(&e);
      if (error) {
          std::cerr << error->what() << std::endl;
      }
      return;
  }
}

const Value& ExecState::Call(const std::string& func_name,
                             const std::vector<Value>& params) {
  Value ret;
  auto it = global_variables_.find(func_name);
  if (it != global_variables_.end()) {
    long reg = it->second;
    Value* function = *stack_->top() + 1;
    *function = *(stack_->base() + reg);
    for (int i = 0; i < params.size(); ++i) {
      *(function + i) = params[i];
    }
    CallFunction(function, params.size(), &ret);
  }
  return ret;
}

void ExecState::CallFunction(Value* func, size_t argc, Value* ret) {
  *stack_->top() = func + argc;
  if (func->type == Value::Type::CFUNC) {
    Frame frame;
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
    vm_->RunFrame(this, frame, ret);
    frames_.pop_back();
  }
}

size_t ExecState::GetArgumentCount() {
  return *stack_->top() - frames_.back().reg;
}

Value* ExecState::GetArgument(int index) {
  return frames_.back().reg + index + 1;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
