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
    
void ExecStack::reset() {
    size_t size = VM_EXEC_STACK_SIZE - (top_ - base());
    for (int i = 0; i < size; i++) {
        top_[i].ref = NULL;
        SetNil(&top_[i]);
    }
}

ExecState::ExecState(VM *vm)
    : vm_(vm),
      frames_(),
      refs_(),
      global_(new Variables),
      stack_(new ExecStack),
      func_state_(nullptr),
      string_table_(new StringTable),
      render_context_(new VNodeRenderContext),
      factory_(new TableFactory()),
      class_factory_(new ClassFactory()),
      global_variables_() {}

void ExecState::Compile(std::string& err) {

#if DEBUG
  TimeCost tc("Compile");
#endif
  err.clear();
  CodeGenerator generator(this);
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
      catch (std::exception &e) {
          auto error = static_cast<Error *>(&e);
          if (error) {
              err = error->what();
              std::cerr << error->what() << std::endl;
          }
          return;
      }
      if (expr->IsChunkStatement()) {
          try {
              generator.Visit(expr->AsChunkStatement().get(), nullptr);
          }
          catch (std::exception &e) {
              auto error = static_cast<Error *>(&e);
              if (error) {
                  err = error->what();
                  std::cerr << error->what() << std::endl;
              }
              return;
          }
      }
  }
}

void ExecState::Execute(std::string& err) {
#if DEBUG
  TimeCost tc("Execute");
#endif
  err.clear();
  Value chunk;
  chunk.type = Value::Type::FUNC;
  chunk.f = func_state_.get();
  *stack_->base() = chunk;
  try {
      CallFunction(stack_->base(), 0, nullptr);
  } catch (std::exception &e) {
      auto error = static_cast<Error *>(&e);
      if (error) {
          err = error->what();
          std::cerr << error->what() << std::endl;
      }
      return;
  }
}

const Value ExecState::Call(const std::string& func_name,
                             const std::vector<Value>& params) {
  Value ret;
  auto it = global_variables_.find(func_name);
  if (it != global_variables_.end()) {
    long reg = it->second;
    Value* function = *stack_->top() + 1;
    *function = *(stack_->base() + reg);
    for (int i = 0; i < params.size(); ++i) {
      *(function + i + 1) = params[i];
    }
    CallFunction(function, params.size(), &ret);
  }
  return ret;
}
    
const Value ExecState::Call(Value *func, const std::vector<Value>& params) {
    Value ret;
    do {
        // 首先检查函数是否属于堆栈函数
        long reg = func - stack_->base();
        if (reg >= VM_EXEC_STACK_SIZE) {
            throw VMExecError("call function out of stack");
            break;
        }
        for (int i = 0; i < params.size(); i++) {
            *(func + i + 1) = params[i];
        }
        CallFunction(func, params.size(), &ret);
        
    } while (0);
    
    return ret;
}

void ExecState::CallFunction(Value *func, size_t argc, Value *ret) {
    *stack_->top() = func + argc;
    if (func->type == Value::Type::CFUNC) {
        Frame frame;
        frame.reg = func;
        frames_.push_back(frame);
        auto result = reinterpret_cast<CFunction>(func->cf)(this);
        if (ret != nullptr && !IsNil(&result)) {
            *ret = result;
        }
        frames_.pop_back();
    }
    else {
        Frame frame;
        frame.func = func;
        frame.reg = func;
        frame.pc = &(*func->f->instructions().begin());
        frame.end = &(*func->f->instructions().end());
        frames_.push_back(frame);
        vm_->RunFrame(this, frame, ret);
        stack_->reset();
        frames_.pop_back();
    }
}
    
ValueRef* ExecState::AddRef(FuncState *func_state, long register_id) {
    ValueRef *ref = nullptr;
    for (int i = 0; i < refs_.size(); i++) {
        ValueRef *ref_cur = refs_[i];
        if (ref_cur->func_state_ == func_state && ref_cur->register_id_ == register_id) {
            ref = ref_cur;
            break;
        }
    }
    if (!ref) {
        ref = new ValueRef(func_state, register_id);
        refs_.push_back(ref);
    }
    return ref;
}
    
ValueRef *ExecState::FindRef(int index) {
    ValueRef *ref = nullptr;
    for (int i = 0; i < refs_.size(); i++) {
        ValueRef *ref_cur = refs_[i];
        if (ref_cur->ref_id_ == index) {
            ref = ref_cur;
            break;
        }
    }
    return ref;
}

size_t ExecState::GetArgumentCount() {
  return *stack_->top() - frames_.back().reg;
}

Value* ExecState::GetArgument(int index) {
  return frames_.back().reg + index + 1;
}
    
int ValueRef::s_ref_id = 0;

}  // namespace data_render
}  // namespace core
}  // namespace weex
