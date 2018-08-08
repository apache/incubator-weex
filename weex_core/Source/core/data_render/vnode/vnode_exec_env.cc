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

#include "core/data_render/vnode/vnode_exec_env.h"
#include <sstream>
#include "core/data_render/object.h"
#include "core/data_render/table.h"
#include "core/data_render/table_factory.h"

namespace weex {
namespace core {
namespace data_render {
json11::Json ParseValue2Json(const Value& value);

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

static Value Merge(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  if (length != 2) {
    return Value();
  }
  Value* lvalue = exec_state->GetArgument(0);
  Value* rvalue = exec_state->GetArgument(1);
  if (!IsTable(lvalue) || !IsTable(rvalue)) {
    return Value();
  }
  Table* l_table = TableValue(lvalue);
  Table* r_table = TableValue(rvalue);
  Value* new_value = exec_state->getTableFactory()->CreateTable();
  Table* new_table = TableValue(new_value);
  new_table->map->insert(r_table->map->begin(), r_table->map->end());
  new_table->map->insert(l_table->map->begin(), l_table->map->end());
  return Value(*new_value);
}

static Value ToString(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  if (length != 1) {
    return Value();
  }
  Value* table = exec_state->GetArgument(0);
  if (!IsTable(table)) {
    return Value();
  }

  const json11::Json& json = ParseValue2Json(*table);
  std::string s;
  json.dump(s);
  String* new_value = exec_state->string_table()->StringFromUTF8(s);
  return Value(new_value);
}

static Value Slice(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  if (length != 3) {
    return Value();
  }
  Value* table = exec_state->GetArgument(0);
  Value* start = exec_state->GetArgument(1);
  Value* end = exec_state->GetArgument(2);
  if (!IsTable(table) || !IsInt(start) || !IsInt(end)) {
    return Value();
  }
  Table* p_table = TableValue(table);
  unsigned int v_start = static_cast<unsigned int>(IntValue(start));
  unsigned int v_end = static_cast<unsigned int>(IntValue(end));
  if (v_end > p_table->array->size()) {
    v_end = p_table->array->size();
  }
  if (v_start > v_end) {
    v_start = v_end;
  }
  Value* new_value = exec_state->getTableFactory()->CreateTable();
  Table* new_table = TableValue(new_value);
  new_table->array->insert(new_table->array->end(),
                           p_table->array->begin() + v_start, p_table->array->begin() + v_end);
  return Value(*new_value);
}

static Value AppendUrlParam(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  if (length != 2) {
    return Value();
  }
  Value* url = exec_state->GetArgument(0);
  Value* array = exec_state->GetArgument(1);
  if (!IsString(url) || !IsTable(array)) {
    return Value();
  }
  String* p_string = StringValue(url);
  Table* p_array = TableValue(array);
  std::stringstream ss;
  ss << p_string->c_str();

  std::vector<Value>* kv_array = p_array->array;
  for (auto it = kv_array->begin(); it != kv_array->end(); it++) {
    Value& kv_map = *it;
    Table* p_table = TableValue(&kv_map);
    if (p_table != nullptr && p_table->map->find("key") != p_table->map->end() &&
        p_table->map->find("value") != p_table->map->end()) {
      Value& key = p_table->map->find("key")->second;
      Value& value = p_table->map->find("value")->second;
      if (IsString(&key) && IsString(&value)) {
        ss << "&" << key.str->c_str() << "=" << value.str->c_str();
      }
    }
  }

  String* new_value = exec_state->string_table()->StringFromUTF8(ss.str());
  return Value(new_value);
}

// createElement("tag_name", "id");
static Value CreateElement(ExecState* exec_state) {
  VNode* node = new VNode(exec_state->GetArgument(1)->str->c_str(),
                          exec_state->GetArgument(0)->str->c_str());
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

  auto styles = exec_state->context()->style_json();
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
  RegisterCFunc(state, "slice", Slice);
  RegisterCFunc(state, "appendUrlParam", AppendUrlParam);
  RegisterCFunc(state, "merge", Merge);
  RegisterCFunc(state, "tostring", ToString);
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
    std::string value;
    json.dump(value);
    if (value.find('.') == std::string::npos) {
      //int
      return Value(static_cast<int64_t>(json.number_value()));
    } else {
      return Value(json.number_value());
    }
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

json11::Json ParseValue2Json(const Value& value) {
  if (value.type != Value::TABLE) {
    return json11::Json();
  }

  Table* p_table = TableValue(&value);
  if (p_table->array->size() > 0) {
    json11::Json::array array;

    for (auto it = p_table->array->begin(); it != p_table->array->end(); it++) {
      if ((*it).type == Value::STRING) {
        array.push_back(json11::Json((*it).str->c_str()));
        continue;
      }

      if ((*it).type == Value::TABLE) {
        array.push_back(ParseValue2Json((*it)));
        continue;
      }
    }

    return json11::Json(array);
  }

  json11::Json::object object;
  for (auto it = p_table->map->begin(); it != p_table->map->end(); it++) {
    if (it->second.type == Value::STRING) {
      object.insert({it->first, json11::Json(it->second.str->c_str())});
      continue;
    }

    if (it->second.type == Value::TABLE) {
      object.insert({it->first, ParseValue2Json(it->second)});
      continue;
    }
  }
  
  return json11::Json(object);
}

void VNodeExecEnv::InitGlobalValue(ExecState* state) {
  const json11::Json& json = state->context()->raw_json();
  Global* global = state->global();
  const json11::Json& data = json["data"];
  Value value = ParseJson2Value(state, data);
  if (value.type != Value::Type::TABLE) {
    value = *(state->getTableFactory()->CreateTable());
  }
  global->Add("_data_main", value);
}

void VNodeExecEnv::InitInitDataValue(ExecState* state,
                                     const std::string& init_data_str) {
  std::string err;
  const json11::Json& json = json11::Json::parse(init_data_str, err);
  if (!err.empty()) {
    LOGE("error parsing init data");
    Value value = *(state->getTableFactory()->CreateTable());
    state->global()->Set("_init_data", value);
    return;
  }

  Value value = ParseJson2Value(state, json);
  if (value.type != Value::Type::TABLE) {
    value = *(state->getTableFactory()->CreateTable());
  }
  state->global()->Set("_init_data", value);
}

void AddStyles(ExecState* state, const std::string& prefix, const json11::Json& style_obj) {
  if (style_obj.is_object()) {
    for (auto it = style_obj.object_items().begin(); it != style_obj.object_items().end(); it++) {
      state->context()->style_json()[prefix + it->first] = const_cast<json11::Json&>(it->second);
    }
  }
}

void VNodeExecEnv::InitStyleList(ExecState* state) {
  json11::Json& json = state->context()->raw_json();
  //body styles

  const json11::Json& style_obj = json["styles"];
  AddStyles(state, "", style_obj);

  const json11::Json& components_obj = json["components"];
  if (components_obj.is_array()) {
    for (auto it = components_obj.array_items().begin();
         it != components_obj.array_items().end(); it++) {
      const json11::Json name = (*it)["name"];
      if (!name.is_string()) {
        continue;
      }
      AddStyles(state, "_" + name.string_value() + "_", (*it)["styles"]);
    }
  }

}

}  // namespace data_render
}  // namespace core
}  // namespace weex