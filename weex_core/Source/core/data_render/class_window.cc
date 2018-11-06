//
// Created by Xu Jiacheng on 2018/11/6.
//

#include "core/data_render/class_window.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/class.h"
#include "core/data_render/common_error.h"
#include "third_party/json11/json11.hpp"
#include "vnode/vnode_render_manager.h"

namespace weex {
namespace core {
namespace data_render {

static Value Open(ExecState *exec_state){
  size_t argc = exec_state->GetArgumentCount();
  if (argc!=1){
    throw VMExecError("window.open takes more than one args");
  }
  Value* url_value = exec_state->GetArgument(0);
  if(!IsString(url_value)){
    throw VMExecError("window.open arg not string");
  }
  json11::Json::array args_array;
  json11::Json::object obj{
      {"url",CStringValue(url_value)},
      {"animated", "true"},
  };
  args_array.push_back(obj);

  const std::string& args = json11::Json(args_array).dump();
  weex::core::data_render::VNodeRenderManager::GetInstance()
      ->CallNativeModule(exec_state, "navigator", "push", args, 2);
  
  return Value();
}

ClassDescriptor* NewClassWindow() {
  ClassDescriptor *desc = new ClassDescriptor(nullptr);
  AddClassStaticCFunc(desc, "open", Open);
  return desc;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
