//
// Created by Xu Jiacheng on 2018/7/20.
//

#include "core/data_render/vnode/vnode_exec_env.h"
#include <sstream>
#include "core/data_render/object.h"
#include "core/data_render/table.h"
#include "core/data_render/table_factory.h"

namespace weex {
namespace core {
namespace data_render {
static Value Log(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  for (int i = 0; i < length; ++i) {
    Value* a = exec_state->GetArgument(i);
    switch (a->type) {
      case Value::Type::NUMBER:
        std::cout << a->n << "\n";
        break;
      case Value::Type::INT:
        std::cout << a->i << "\n";
        break;
      case Value::Type::STRING:
        std::cout << a->str->c_str() << "\n";
        break;
      default:
        break;
    }
  }
  return Value();
}

static Value GetTableSize(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  if (length > 0) {
    Value* value = exec_state->GetArgument(0);
    if (IsTable(value)) {
      return Value(static_cast<int64_t>(GetTableSize(TableValue(value))));
    }
  }
  return Value(static_cast<int64_t>(-1));
}

// createElement("tag_name", "id");
static Value CreateElement(ExecState* exec_state) {
  VNode* node = new VNode(exec_state->GetArgument(1)->str->c_str(),
                          exec_state->GetArgument(0)->str->c_str());
  exec_state->context()->InsertNode(node);
  Value result;
  result.type = Value::Type::CPTR;
  result.cptr = node;
  return result;
}

// appendChild(parent, node);
static Value AppendChild(ExecState* exec_state) {
  VNode* parent =
      exec_state->GetArgument(0)->type == Value::Type::NIL
          ? nullptr
          : reinterpret_cast<VNode*>(exec_state->GetArgument(0)->cptr);
  VNode* child = reinterpret_cast<VNode*>(exec_state->GetArgument(1)->cptr);

  if (parent == nullptr && exec_state->context()->root() == nullptr) {
    exec_state->context()->set_root(child);
  }
  if (parent == nullptr || child == nullptr) {
    return Value();
  }
  parent->AddChild(child);

  return Value();
}

// setAttr(node, "value");
static Value SetAttr(ExecState* exec_state) {
  VNode* node = reinterpret_cast<VNode*>(exec_state->GetArgument(0)->cptr);
  if (node == nullptr) {
    return Value();
  }

  char* key = exec_state->GetArgument(1)->str->c_str();
  Value* p_value = exec_state->GetArgument(2);
  if (p_value->type == Value::STRING) {
    node->SetAttribute(key, p_value->str->c_str());
  } else if (p_value->type == Value::INT) {  // todo use uniform type
                                             // conversion.
    std::stringstream ss;
    ss << p_value->i;
    std::string str = ss.str();
    node->SetAttribute(key, str);
  }

  return Value();
}

// setClassList(node, "class-name");
static Value SetClassList(ExecState* exec_state) {
  VNode* node = reinterpret_cast<VNode*>(exec_state->GetArgument(0)->cptr);
  char* key = exec_state->GetArgument(1)->str->c_str();

  if (node == nullptr) {
    return Value();
  }

  json11::Json& json = exec_state->context()->raw_json();
  const json11::Json& styles = json["styles"];
  const json11::Json& style = styles[key];
  if (style.is_null()) {
    return Value();
  }
  const json11::Json::object& items = style.object_items();
  for (auto it = items.begin(); it != items.end(); it++) {
    node->SetStyle(it->first, it->second.string_value());
  }
  return Value();
}

void RegisterCFunc(ExecState* state, const std::string& name,
                   CFunction function) {
  Value func;
  func.type = Value::Type::CFUNC;
  func.cf = reinterpret_cast<void*>(function);
  state->global()->Add(name, func);
}

void VNodeExecEnv::InitCFuncEnv(ExecState* state) {
  // log
  RegisterCFunc(state, "log", Log);
  RegisterCFunc(state, "sizeof", GetTableSize);
  RegisterCFunc(state, "createElement", CreateElement);
  RegisterCFunc(state, "appendChild", AppendChild);
  RegisterCFunc(state, "setAttr", SetAttr);
  RegisterCFunc(state, "setClassList", SetClassList);
}

Value ParseJson2Value(ExecState* state, const json11::Json& json) {
  if (json.is_null()) {
    return Value();
  } else if (json.is_bool()) {
    return Value(json.bool_value());
  } else if (json.is_number()) {
    // todo check which is int or double.
    return Value(json.number_value());
  } else if (json.is_string()) {
    String* p_str = state->string_table()->StringFromUTF8(json.string_value());
    return Value(p_str);
  } else if (json.is_array()) {
    Value* value = state->getTableFactory()->CreateTable();
    const json11::Json::array& data_objects = json.array_items();
    int64_t array_size = data_objects.size();
    for (int64_t index = 0; index < array_size; index++) {
      // will be free by table
      Value key(index);
      Value val(ParseJson2Value(state, json[index]));
      SetTabValue(TableValue(value), &key, val);
    }
    return Value(*value);
  } else if (json.is_object()) {
    Value* value = state->getTableFactory()->CreateTable();
    const json11::Json::object& data_objects = json.object_items();
    for (auto it = data_objects.begin(); it != data_objects.end(); it++) {
      // will be free by table
      Value key(state->string_table()->StringFromUTF8(it->first));
      Value val(ParseJson2Value(state, it->second));
      SetTabValue(TableValue(value), &key, val);
    }
    return Value(*value);
  } else {
    return Value();
  }
};

void VNodeExecEnv::InitGlobalValue(ExecState* state) {
  const json11::Json& json = state->context()->raw_json();
  Global* global = state->global();
  const json11::Json& data = json["data"];
  if (data.is_null()) {
    return;
  }
  const json11::Json::object& data_objects = data.object_items();
  for (auto it = data_objects.begin(); it != data_objects.end(); it++) {
    const Value& value = ParseJson2Value(state, it->second);
    global->Add(it->first, value);
  }
}

void VNodeExecEnv::InitInitDataValue(ExecState* state,
                                     const std::string& init_data_str) {
  std::string err;
  const json11::Json& json = json11::Json::parse(init_data_str, err);
  if (!err.empty()) {
    LOGE("error parsing init data");
    return;
  }

  const Value& value = ParseJson2Value(state, json);
  state->global()->Set("_init_data_", value);
}
}  // namespace data_render
}  // namespace core
}  // namespace weex