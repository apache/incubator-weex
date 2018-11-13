//
// Created by Xu Jiacheng on 2018/11/6.
//

#include "core/data_render/class_math.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/class.h"
#include "core/data_render/common_error.h"
#include "third_party/json11/json11.hpp"
#include "vnode/vnode_render_manager.h"

namespace weex {
namespace core {
namespace data_render {

static Value Ceil(ExecState* exec_state){
  size_t argc = exec_state->GetArgumentCount();
  if (argc!=1){
    throw VMExecError("math.ceil takes more than one args");
  }
  Value* value = exec_state->GetArgument(0);
  if(!IsNumber(value)&&!IsInt(value)){
    throw VMExecError("math.ceil arg not number");
  }

  if (IsNumber(value)){
    return static_cast<int64_t>(ceil(NumValue(value)));
  } else {
    return static_cast<int64_t>(ceil(IntValue(value)));
  }
}

static Value Floor(ExecState* exec_state){
  size_t argc = exec_state->GetArgumentCount();
  if (argc!=1){
    throw VMExecError("math.ceil takes more than one args");
  }
  Value* value = exec_state->GetArgument(0);
  if(!IsNumber(value)&&!IsInt(value)){
    throw VMExecError("math.ceil arg not number");
  }

  if (IsNumber(value)){
    return static_cast<int64_t>(floor(NumValue(value)));
  } else {
    return static_cast<int64_t>(floor(IntValue(value)));
  }
}


static Value Random(ExecState* exec_state){
  return static_cast <double> (rand()) / static_cast <double>(RAND_MAX);
}

static Value Max(ExecState* exec_state){
  size_t argc = exec_state->GetArgumentCount();
  if (argc <= 1){
    return Value(INT64_MIN);
  }

  double max = std::numeric_limits<double>::lowest();
  int index = -1;

  for (int i = 0; i < argc; ++i) {
    auto value = exec_state->GetArgument(i);
    if (IsInt(value)){
      int64_t value_i = IntValue(value);
      if (value_i>max){
        index = i;
        max = static_cast<double>(value_i);
      }
    } else if (IsNumber(value)){
      double value_d = NumValue(value);
      if (value_d>max){
        index = i;
        max = value_d;
      }
    } else {
      return Value();
    }
  }

  if (index>=0){
    return Value(*exec_state->GetArgument(index));
  } else{
    return Value(INT64_MIN);
  }
}

static Value Min(ExecState* exec_state){
  size_t argc = exec_state->GetArgumentCount();
  if (argc <= 1){
    return Value(INT64_MAX);
  }

  double min = std::numeric_limits<double>::max();
  int index = -1;

  for (int i = 0; i < argc; ++i) {
    auto value = exec_state->GetArgument(i);
    if (IsInt(value)){
      int64_t value_i = IntValue(value);
      if (value_i<min){
        index = i;
        min = static_cast<double>(value_i);
      }
    } else if (IsNumber(value)){
      double value_d = NumValue(value);
      if (value_d<min){
        index = i;
        min = value_d;
      }
    } else {
      return Value();
    }
  }

  if (index>=0){
    return Value(*exec_state->GetArgument(index));
  } else{
    return Value(INT64_MAX);
  }
}

ClassDescriptor* NewClassMath() {
  ClassDescriptor *desc = new ClassDescriptor(nullptr);
  AddClassStaticCFunc(desc, "ceil", Ceil);
  AddClassStaticCFunc(desc, "floor", Floor);
  AddClassStaticCFunc(desc, "random", Random);
  AddClassStaticCFunc(desc, "max", Max);
  AddClassStaticCFunc(desc, "min", Min);
  return desc;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
