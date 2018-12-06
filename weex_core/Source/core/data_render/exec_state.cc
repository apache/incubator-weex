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
#include "core/data_render/class_array.h"
#include "core/data_render/vnode/vnode_exec_env.h"
#if DEBUG
#include "core/data_render/monitor/vm_monitor.h"
#endif

namespace weex {
namespace core {
namespace data_render {
    
int ValueRef::gs_ref_id = 0;
       
FuncClosure::FuncClosure(ValueRef *ref) {
    if (ref) {
        func_state_ = ref->func_state();
        register_id_ = ref->register_id();
        value_ = ref->value();
        value_.ref = &ref->value();
    }
    else {
        SetNil(&value_);
        value_.ref = nullptr;
    }
    ref_ = ref;
}

void ExecStack::reset() {
    size_t size = (VM_EXEC_STACK_SIZE - (top_ - base()) - 1) * sizeof(Value);
    //LOGD("reset:%i=>\n", (int)(top_ - base()));
    memset(top_, 0, size);
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
      class_factory_(new ClassFactory()),
      global_variables_() {}
    
ExecState::~ExecState() {
    for (ValueRef *ref : refs_) {
        if (ref) {
            delete ref;
        }
    }
}
    
void ExecState::Compile(std::string& err) {
#if DEBUG
  TimeCost tc("Compile");
#endif
  ValueRef::reset();
  class_compile_index_ = static_cast<uint32_t>(class_factory_->descs().size());
  global_compile_index_ = static_cast<uint32_t>(global()->size());
  string_compile_index_ = static_cast<uint32_t>(string_table_->store().size());
  ValueRef::gs_ref_id = 0;
  err.clear();
  CodeGenerator generator(this);
  if (!context()->raw_json().is_null()) {
      VNodeExecEnv::ParseData(this);
      VNodeExecEnv::ParseStyle((this));
      VNodeExecEnv::ParseScript(this);
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
  // reset stack top pointer when main call
  *stack_->top() = stack_->base();
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

const Value ExecState::Call(const std::string& func_name, const std::vector<Value>& args) {
    Value ret;
    do {
        int index = global()->IndexOf(func_name);
        long caller = -1;
        if (index >= 0) {
            **stack_->top() = *(global()->Find(index));
        }
        else {
            auto iter = global_variables_.find(func_name);
            if (iter == global_variables_.end()) {
                break;
            }
            caller = iter->second;
            if (caller < 0) {
                break;
            }
            **stack_->top() = *(stack_->base() + caller);
        }
        for (int i = 0; i < args.size(); ++i) {
            *(*stack_->top() + i + 1) = args[i];
        }
        CallFunction(*stack_->top(), args.size(), &ret);

    } while (0);
    
    return ret;
}
    
const Value ExecState::Call(FuncInstance *func, const std::vector<Value>& args) {
    Value ret;
    do {
        Value functor;
        SetGCFuncValue(&functor, reinterpret_cast<GCObject *>(func));
        **stack_->top() = functor;
        int index = 0;
        for (int i = 0; i < args.size(); i++) {
            *(*stack_->top() + i + 1 + index) = args[i];
        }
        CallFunction(*stack_->top(), args.size(), &ret);
        
    } while (0);
    
    return ret;
}
    
const Value ExecState::Call(FuncState *func_state, const std::vector<Value>& args) {
    Value ret;
    do {
        Value func(func_state);
        **stack_->top() = func;
        int index = 0;
        for (int i = 0; i < args.size(); i++) {
            *(*stack_->top() + i + 1 + index) = args[i];
        }
        CallFunction(*stack_->top(), args.size(), &ret);
        
    } while (0);
    
    return ret;
}
    
const Value ExecState::Call(Value *func, const std::vector<Value>& args) {
    Value ret;
    do {
        // 首先检查函数是否属于堆栈函数
        long reg = func - stack_->base();
        if (reg >= VM_EXEC_STACK_SIZE) {
            throw VMExecError("call function out of stack");
            break;
        }
        for (int i = 0; i < args.size(); i++) {
            *(func + i + 1) = args[i];
        }
        CallFunction(func, args.size(), &ret);
        
    } while (0);
    
    return ret;
}
    
void ExecState::resetArguments(Value *func, size_t argc) {
    do {
        auto iter = global_variables_.find(JS_GLOBAL_ARGUMENTS);
        if (iter == global_variables_.end()) {
            break;
        }
        long caller = iter->second;
        if (caller < 0) {
            break;
        }
        Value *arguments = stack_->base() + caller;
        if (!IsArray(arguments)) {
            break;
        }
        ClearArray(ValueTo<Array>(arguments));
        for (int i = 0; i < argc; i++) {
            PushArray(ValueTo<Array>(arguments), *(func + i + 1));
        }
        
    } while (0);
}
    
void ExecState::Register(const std::string& name, CFunction func) {
    Value funcVal;
    funcVal.type = Value::Type::CFUNC;
    funcVal.cf = reinterpret_cast<void *>(func);
    Register(name, funcVal);
}
    
void ExecState::Register(const std::string& name, Value value) {
    global()->Add(name, value);
}

void ExecState::CallFunction(Value *func, size_t argc, Value *ret) {
    if (func->type == Value::Type::CFUNC) {
        *stack_->top() = func + argc;
        Frame frame;
        frame.reg = func;
        frames_.push_back(frame);
        auto result = reinterpret_cast<CFunction>(func->cf)(this);
        if (ret) {
            *ret = result;
        }
        stack_->reset();
        frames_.pop_back();
    }
    else {
        FuncState *func_state = func->type == Value::Type::FUNC ? func->f : ValueTo<FuncInstance>(func)->func_;
        if (argc < func_state->argc()) {
            size_t size = (func_state->argc() - argc) * sizeof(Value);
            memset(func + argc + 1, 0, size);
        }
        *stack_->top() = func + argc;
        Frame frame;
        frame.func = func_state;
        frame.reg = func;
        frame.pc = &(*func_state->instructions().begin());
        frame.end = &(*func_state->instructions().end());
        frames_.push_back(frame);
        resetArguments(func, argc);
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
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
