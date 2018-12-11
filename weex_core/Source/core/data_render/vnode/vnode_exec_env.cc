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

#include <sstream>
#include "core/data_render/vnode/vnode_exec_env.h"
#include "core/data_render/object.h"
#include "core/data_render/table.h"
#include "core/data_render/class_factory.h"
#include "core/data_render/class_array.h"
#include "core/data_render/class_string.h"
#include "core/data_render/class_object.h"
#include "core/data_render/common_error.h"
#include "core/data_render/js_common_function.h"
#include "core/data_render/vnode/vcomponent.h"
#include "core/data_render/vnode/vnode_render_manager.h"
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {

json11::Json ValueToJSON(const Value& value);

static Value SizeOf(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length > 0) {
        Value *value = exec_state->GetArgument(0);
        if (IsTable(value)) {
            return Value(static_cast<int64_t>(GetTableSize(ValueTo<Table>(value))));
        }
        else if (IsArray(value)) {
            return GetArraySizeValue(ValueTo<Array>(value));
        }
    }
    return Value(static_cast<int64_t>(-1));
}

static Value Merge(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 2) {
        return Value();
    }
    Value *lhs = exec_state->GetArgument(0);
    Value *rhs = exec_state->GetArgument(1);
    if (!IsTable(lhs) && !IsTable(rhs)) {
        return Value();
    }
    Value new_value = exec_state->class_factory()->CreateTable();
    if (IsTable(lhs)) {
        TableCopy(*lhs, new_value);
//        LOGD("[Merge]:lhs:%s\n", TableToString(ValueTo<Table>(lhs)).c_str());
    }
    if (IsTable(rhs)) {
        TableCopy(*rhs, new_value);
//        LOGD("[Merge]:rhs:%s\n", TableToString(ValueTo<Table>(rhs)).c_str());
    }
    return new_value;
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

  const json11::Json& json = ValueToJSON(*table);
  std::string s;
  json.dump(s);
  String* new_value = exec_state->string_table()->StringFromUTF8(s);
  return Value(new_value);
}

static Value Slice(ExecState *exec_state) {
    size_t length = exec_state->GetArgumentCount();
    if (length != 3) {
        return Value();
    }
    Value *array = exec_state->GetArgument(0);
    Value *start = exec_state->GetArgument(1);
    Value *end = exec_state->GetArgument(2);
    if (!IsArray(array) || !IsInt(start) || !IsInt(end)) {
        return Value();
    }
    unsigned int v_start = static_cast<unsigned int>(IntValue(start));
    unsigned int v_end = static_cast<unsigned int>(IntValue(end));
    size_t size = GetArraySize(ValueTo<Array>(array));
    if (v_end > size) {
        v_end = static_cast<unsigned int>(size);
    }
    if (v_start > v_end) {
        v_start = v_end;
    }
    Value new_value = exec_state->class_factory()->CreateArray();
    ArrayCopyFrom(*array, new_value, v_start, v_end);
    return new_value;
}
    
static Value CallNativeModule(ExecState *exec_state) {
    do {
        if (exec_state->GetArgumentCount() < 1) {
            break;
        }
        Value *arg = exec_state->GetArgument(0);
        if (!IsTable(arg)) {
            break;
        }
        Value *module = GetTableValue(ValueTo<Table>(arg), std::string("module"));
        if (!module || !IsString(module)) {
            break;
        }
        Value *method = GetTableValue(ValueTo<Table>(arg), std::string("method"));
        if (!method || !IsString(method)) {
            break;
        }
        Value *args = GetTableValue(ValueTo<Table>(arg), std::string("args"));
        if (!args || !IsArray(args)) {
            break;
        }
        int argc = (int)GetArraySize(ValueTo<Array>(args));
        weex::core::data_render::VNodeRenderManager::GetInstance()->CallNativeModule(exec_state, CStringValue(module), CStringValue(method), argc > 0 ? ArrayToString(ValueTo<Array>(args)) : "", argc);
        
    } while(0);
    
    return Value();
}
    
static Value RequireModule(ExecState *exec_state) {
    do {
        if (!exec_state->GetArgumentCount()) {
            break;
        }
        Value *arg = exec_state->GetArgument(0);
        if (!IsString(arg)) {
            break;
        }
        std::string module_name = CStringValue(arg);
        std::string module_info;
        if (!weex::core::data_render::VNodeRenderManager::GetInstance()->RequireModule(exec_state, module_name, module_info)) {
            break;
        }
        return StringToValue(exec_state, module_info);
        
    } while (0);
    
    return Value();
}
    
static Value RegisterModules(ExecState *exec_state) {
    do {
        if (!exec_state->GetArgumentCount()) {
            break;
        }
        Value *arg = exec_state->GetArgument(0);
        if (!IsArray(arg)) {
            break;
        }
        Array *array = ValueTo<Array>(arg);
        if (array->items.size() > 0) {
            std::vector<std::string> args;
            for (int i = 0; i < array->items.size(); i++) {
                Value item = array->items[i];
                if (!IsString(&item)) {
                    continue;
                }
                args.push_back(CStringValue(&item));
            }
            if (args.size() > 0) {
                weex::core::data_render::VNodeRenderManager::GetInstance()->ExecuteRegisterModules(exec_state, args);
            }
        }
        
    } while (0);
    return Value();
}

static Value AppendUrlParam(ExecState *exec_state) {
  size_t length = exec_state->GetArgumentCount();
  if (length != 2) {
    return Value();
  }
  Value* url = exec_state->GetArgument(0);
  Value* array = exec_state->GetArgument(1);
  if (!IsString(url) || !IsArray(array)) {
    return Value();
  }
  String* p_string = StringValue(url);
  Array* p_array = ValueTo<Array>(array);
  std::stringstream ss;
  ss << p_string->c_str();

  std::vector<Value> kv_array = p_array->items;
  for (auto it = kv_array.begin(); it != kv_array.end(); it++) {
    Value& kv_map = *it;
    Table* p_table = ValueTo<Table>(&kv_map);
    if (p_table != nullptr && p_table->map.find("key") != p_table->map.end() &&
        p_table->map.find("value") != p_table->map.end()) {
      Value& key = p_table->map.find("key")->second;
      Value& value = p_table->map.find("value")->second;
      if (IsString(&key) && IsString(&value)) {
        ss << "&" << key.str->c_str() << "=" << value.str->c_str();
      }
    }
  }

  String* new_value = exec_state->string_table()->StringFromUTF8(ss.str());
  return Value(new_value);
}

// saveComponentDataAndProps(component, data, props);
static Value SaveComponentDataAndProps(ExecState* exec_state) {
  VComponent *component = exec_state->GetArgument(0)->type == Value::Type::NIL ?
                    nullptr : reinterpret_cast<VComponent *>(exec_state->GetArgument(0)->cptr);
  Value *data = exec_state->GetArgument(1);
  Value *props = exec_state->GetArgument(2);
  if (component) {
      component->set_data(ValueTo<Table>(data));
      component->set_props(ValueTo<Table>(props));
  }
  return Value();
}

// appendChildComponent(parent, child);
static Value AppendChildComponent(ExecState* exec_state) {
  VNode *parent = exec_state->GetArgument(0)->type == Value::Type::NIL ?
                    nullptr : reinterpret_cast<VNode *>(exec_state->GetArgument(0)->cptr);
  VNode *child = reinterpret_cast<VNode *>(exec_state->GetArgument(1)->cptr);
  if (parent && child) {
      static_cast<VComponent *>(parent)->AppendChildComponent(static_cast<VComponent *>(child));
  }
  return Value();
}

// createComponent(template_id, "template_name", "tag_name", "id", ref);
static Value CreateComponent(ExecState* exec_state) {
  int template_id = 0;
  if (exec_state->GetArgument(0)->type == Value::Type::NUMBER) {
    template_id = static_cast<int>(exec_state->GetArgument(0)->i);
  }
  auto template_name = exec_state->GetArgument(1)->str;
  Value* arg_node_id = exec_state->GetArgument(3);
  std::string node_id;
  if (IsString(arg_node_id)) {
    node_id = CStringValue(arg_node_id);
  } else if (IsInt(arg_node_id)) {
    std::ostringstream os;
    os << IntValue(arg_node_id);
    node_id = "vn_" + os.str();
  } else {
    throw VMExecError("CreateElement only support int for string");
  }
  std::string tag_name = exec_state->GetArgument(2)->str->c_str();
  std::string ref = "";
  if (exec_state->GetArgumentCount() > 4 &&
      exec_state->GetArgument(4)->type == Value::Type::STRING) {
    ref = exec_state->GetArgument(4)->str->c_str();
  }
  LOGD("[VM][VNode][CreateDocument]: %s  %s\n", node_id.c_str(), tag_name.c_str());
  VComponent* component = NULL;
  if (tag_name == "root") {
    component = new VComponent(exec_state, template_id, template_name->c_str(),
                               "div", node_id, ref);
    if (exec_state->context()->root() == nullptr) {
      exec_state->context()->set_root(component);
    }
  } else {
    component = new VComponent(exec_state, template_id, template_name->c_str(),
                               tag_name, node_id, ref);
  }
  if (exec_state->context()->root() == nullptr) {
    exec_state->context()->set_root(component);
  }
  Value result;
  result.type = Value::Type::CPTR;
  result.cptr = component;
  return result;
}
    
static Value UpdateElement(ExecState *exec_state) {
    do {
        if (exec_state->GetArgumentCount() < 2) {
            throw VMExecError("UpdateElement needs >= 2 args");
        }
        Value *prev = exec_state->GetArgument(0);
        Value *next = exec_state->GetArgument(1);
        if (!IsCptr(prev) || !IsCptr(next)) {
            throw VMExecError("UpdateElement only supporting cptr");
        }
        VNode *vn_prev = reinterpret_cast<VNode *>(prev->cptr);
        VNode *vn_next = reinterpret_cast<VNode *>(next->cptr);
        VNodeRenderManager::GetInstance()->PatchVNode(exec_state, vn_prev, vn_next);
        
    } while (0);
    
    return Value();
}
    
static size_t g_node_id = 0;

// createElement("tag_name", "id", ref);
static Value CreateElement(ExecState *exec_state) {
    std::string tag_name = exec_state->GetArgument(0)->str->c_str();
    Value *arg_id = exec_state->GetArgument(1);
    std::string arg_id_str;
    if (IsString(arg_id)) {
        arg_id_str = CStringValue(arg_id);
    }
    else if (IsInt(arg_id)) {
        std::ostringstream os;
        os << IntValue(arg_id) ;
        arg_id_str = "vn_" + os.str();
    }
    else {
        throw VMExecError("CreateElement only support int for string");
    }
    std::string node_id,ref;
    std::ostringstream os;
    os << g_node_id++;
    node_id = "vn_" + os.str();
    ref = arg_id_str;
    if (exec_state->GetArgumentCount() > 2 && exec_state->GetArgument(2)->type == Value::Type::STRING) {
        ref = exec_state->GetArgument(2)->str->c_str();
    }
    LOGD("[VM][VNode][CreateElement]: %s  %s\n", node_id.c_str(), tag_name.c_str());
    VNode *node = NULL;
    if (tag_name == "root") {
        node = new VNode("div", "vn_r", "vn_r");
        exec_state->context()->set_root(node);
    }
    else {
        node = new VNode(tag_name, node_id, ref);
        if (exec_state->context()->root() == nullptr) {
            exec_state->context()->set_root(node);
        }
    }
    Value result;
    result.type = Value::Type::CPTR;
    result.cptr = node;
    return result;
}
    
static void AppendChild(ExecState *exec_state, VNode *parent, VNode *children) {
    if (parent && children) {
        parent->AddChild(children);
    }
}

// appendChild(parent, node);
static Value AppendChild(ExecState *exec_state) {
    do {
        VNode *parent = exec_state->GetArgument(0)->type == Value::Type::NIL ?
        nullptr : reinterpret_cast<VNode *>(exec_state->GetArgument(0)->cptr);
        Value *childrens = exec_state->GetArgument(1);
        if (IsString(childrens) && parent->tag_name() != "text") {
            throw VMExecError("AppendChild only support string for span");
        }
        else if (IsNil(childrens)) {
            break;
        }
        else if (!IsArray(childrens) && !IsCptr(childrens) && !IsString(childrens)) {
            throw VMExecError("AppendChild unsupport array or cptr");
        }
        if (IsArray(childrens)) {
            std::vector<Value> items = ValueTo<Array>(childrens)->items;
            for (int i = 0; i < items.size(); i++) {
                if (IsNil(&items[i])) {
                    continue;
                }
                if (!IsCptr(&items[i])) {
                    throw VMExecError("AppendChild unspport array or cptr");
                }
                VNode *children = reinterpret_cast<VNode *>(items[i].cptr);
                AppendChild(exec_state, parent, children);
            }
        }
        else if (IsString(childrens) && parent) {
            parent->SetAttribute("value", CStringValue(childrens));
        }
        else {
            VNode *children = reinterpret_cast<VNode *>(exec_state->GetArgument(1)->cptr);
            LOGD("[VM][VNode][AppendChild]: %s  %s\n", parent ? parent->ref().c_str() : "null", children->ref().c_str());
            AppendChild(exec_state, parent, children);
        }
        
    } while (0);
    
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


// setProps(node, "value");
static Value SetProps(ExecState *exec_state) {
    VNode *node = reinterpret_cast<VNode *>(exec_state->GetArgument(0)->cptr);
    if (node == nullptr) {
        return Value();
    }
    Value *p_value = exec_state->GetArgument(1);
    if (p_value->type == Value::TABLE) {
        Table *table = ValueTo<Table>(p_value);
        //LOGD("[SetProps]:table:%s\n", TableToString(table).c_str());
        for (auto iter = table->map.begin(); iter != table->map.end(); iter++) {
            if (iter->first == "style") {
                Value style = iter->second;
                if (style.type == Value::TABLE) {
                    Table *style_table = ValueTo<Table>(&style);
                    for (auto iter_style = style_table->map.begin(); iter_style != style_table->map.end(); iter_style++) {
                        switch (iter_style->second.type) {
                            case Value::STRING:
                            {
                                node->SetStyle(iter_style->first, iter_style->second.str->c_str());
                                break;
                            }
                            case Value::INT:
                            {
                                node->SetStyle(iter_style->first, base::to_string(iter_style->second.i));
                                break;
                            }
                            case Value::NUMBER:
                            {
                                node->SetStyle(iter_style->first, base::to_string(iter_style->second.n));
                                break;
                            }
                            default:
                                LOGE("can't support type:%i\n", iter_style->second.type);
                                break;
                        }
                    }
                }
            }
            else {
                switch (iter->second.type) {
                    case Value::STRING:
                    {
                        node->SetAttribute(iter->first, iter->second.str->c_str());
                        break;
                    }
                    case Value::INT:
                    {
                        node->SetAttribute(iter->first, base::to_string(iter->second.i));
                        break;
                    }
                    case Value::FUNC:
                    {
                        std::string::size_type pos = iter->first.find("on");
                        if (pos != 0) {
                            throw VMExecError("AddEvent isn't a function");
                        }
                        std::string event = iter->first.substr(pos + 2);
                        transform(event.begin(), event.end(), event.begin(), ::tolower);
                        FuncState *func_state = iter->second.f;
                        node->AddEvent(event, func_state, func_state->class_inst());
                        break;
                    }
                    case Value::FUNC_INST:
                    {
                        std::string::size_type pos = iter->first.find("on");
                        if (pos != 0) {
                            throw VMExecError("AddEvent isn't a function");
                        }
                        std::string event = iter->first.substr(pos + 2);
                        transform(event.begin(), event.end(), event.begin(), ::tolower);
                        FuncInstance *func_inst = ValueTo<FuncInstance>(&iter->second);
                        FuncState *func_state = func_inst->func_;
                        node->AddEvent(event, func_inst, func_state->class_inst());
                        break;
                    }
                    case Value::NUMBER:
                    {
                        node->SetStyle(iter->first, base::to_string(iter->second.n));
                        break;
                    }
                    default:
                        LOGE("can't support type:%i\n", iter->second.type);
                        break;
                }
            }
        }
    }
    return Value();
}

// setClassList(node, "class-name");
static Value SetClassList(ExecState* exec_state) {
  VNode* node = reinterpret_cast<VNode*>(exec_state->GetArgument(0)->cptr);
  Value* key = exec_state->GetArgument(1);
  if (key->type != Value::Type::STRING) {
      return Value();
  }

  if (node == nullptr) {
    return Value();
  }

  auto styles = exec_state->context()->style_json();
  const json11::Json& style = styles[key->str->c_str()];
  if (style.is_null()) {
    return Value();
  }
  const json11::Json::object& items = style.object_items();
  for (auto it = items.begin(); it != items.end(); it++) {
    node->SetStyle(it->first, it->second.string_value());
  }
  return Value();
}

// setStyle(node, key, value);
static Value SetStyle(ExecState* exec_state) {
  VNode* node = reinterpret_cast<VNode*>(exec_state->GetArgument(0)->cptr);
  Value* key = exec_state->GetArgument(1);
  Value* value = exec_state->GetArgument(2);
  if (node == nullptr || key->type != Value::Type::STRING ||
      value->type == Value::Type::NIL) {
    return Value();
  }
  node->SetStyle(key->str->c_str(), ToString(value));
  return Value();
}

void VNodeExecEnv::ImportExecEnv(ExecState *state) {
    state->Register("sizeof", SizeOf);
    state->Register("slice", Slice);
    state->Register("appendUrlParam", AppendUrlParam);
    state->Register("merge", Merge);
    state->Register("tostring", ToString);
    state->Register("createElement", CreateElement);
    state->Register("updateElement", UpdateElement);
    state->Register("createComponent", CreateComponent);
    state->Register("saveComponentDataAndProps", SaveComponentDataAndProps);
    state->Register("appendChildComponent", AppendChildComponent);
    state->Register("appendChild", AppendChild);
    state->Register("encodeURIComponent", encodeURIComponent);
    state->Register("encodeURI", encodeURIComponent);
    state->Register("setAttr", SetAttr);
    state->Register("setProps", SetProps);
    state->Register("setClassList", SetClassList);
    state->Register("setStyle", SetStyle);
    state->Register("__callNativeModule", CallNativeModule);
    // __registerModules deprecated in sversion 5.8 +
    state->Register("__registerModules", RegisterModules);
    // __requireModule supporting in sversion 5.8 +
    state->Register("__requireModule", RequireModule);
    state->Register("Array", state->class_factory()->ClassArray());
    state->Register("String", state->class_factory()->ClassString());
    state->Register("JSON", state->class_factory()->ClassJSON());
    state->Register("Object", state->class_factory()->ClassObject());
    state->Register("RegExp", state->class_factory()->ClassRegExp());
    state->Register("window", state->class_factory()->ClassWindow());
    state->Register("Math", state->class_factory()->ClassMath());
    state->Register("console", state->class_factory()->ClassConsole());
    RegisterJSCommonFunction(state);
}

Value JSONToValue(ExecState *state, const json11::Json& json) {
    if (json.is_null()) {
        return Value();
    }
    else if (json.is_bool()) {
        return Value(json.bool_value());
    }
    else if (json.is_number()) {
        std::string value;
        json.dump(value);
        if (value.find('.') == std::string::npos) {
            //int
            return Value(static_cast<int64_t>(json.number_value()));
        }
        else {
            return Value(json.number_value());
        }
    }
    else if (json.is_string()) {
        String *p_str = state->string_table()->StringFromUTF8(json.string_value());
        return Value(p_str);
    }
    else if (json.is_array()) {
        Value value = state->class_factory()->CreateArray();
        const json11::Json::array& data_objects = json.array_items();
        int64_t array_size = data_objects.size();
        for (int index = 0; index < array_size; index++) {
            // will be free by table
            Value key(index);
            Value val(JSONToValue(state, json[index]));
            SetArray(ValueTo<Array>(&value), &key, val);
        }
        return value;
    }
    else if (json.is_object()) {
        Value value = state->class_factory()->CreateTable();
        const json11::Json::object& data_objects = json.object_items();
        for (auto it = data_objects.begin(); it != data_objects.end(); it++) {
            // will be free by table
            Value key(state->string_table()->StringFromUTF8(it->first));
            Value val(JSONToValue(state, it->second));
            SetTableValue(ValueTo<Table>(&value), &key, val);
        }
        return value;
        
    }
    else {
        return Value();
    }
};

json11::Json ValueToJSON(const Value& value) {
    if (value.type != Value::TABLE) {
        return json11::Json();
    }
    Table *p_table = ValueTo<Table>(&value);
    json11::Json::object object;
    for (auto it = p_table->map.begin(); it != p_table->map.end(); it++) {
        if (it->second.type == Value::STRING) {
            object.insert({it->first, json11::Json(it->second.str->c_str())});
            continue;
        }
        if (it->second.type == Value::TABLE) {
            object.insert({it->first, ValueToJSON(it->second)});
            continue;
        }
    }
    return json11::Json(object);
}

void VNodeExecEnv::ParseData(ExecState* state) {
  const json11::Json& json = state->context()->raw_json();
  Variables* global = state->global();
  const json11::Json& data = json["data"];
  Value value = JSONToValue(state, data);
  if (value.type != Value::Type::TABLE) {
    value = state->class_factory()->CreateTable();
  }
  global->Add("_data_main", value);

  // Set component data and props
  Value components_data = state->class_factory()->CreateTable();
  Value components_props = state->class_factory()->CreateTable();
  const json11::Json& components_obj = json["components"];
  if (components_obj.is_array()) {
    for (auto it = components_obj.array_items().begin();
         it != components_obj.array_items().end(); it++) {
      const json11::Json name = (*it)["name"];
      if (!name.is_string()) {
        continue;
      }
      auto temp_data = JSONToValue(state, (*it)["data"]);
      Value key(state->string_table()->StringFromUTF8(name.string_value()));
      SetTableValue(ValueTo<Table>(&components_data), &key, temp_data);
      auto temp_props = JSONToValue(state, (*it)["props"]);
      SetTableValue(ValueTo<Table>(&components_props), &key, temp_props);
    }
  }
  global->Add("_components_data", components_data);
  global->Add("_components_props", components_props);
}

void VNodeExecEnv::ImportExecData(ExecState *state, const std::string& init_data_str) {
  std::string err;
  const json11::Json& json = json11::Json::parse(init_data_str, err);
  if (!err.empty()) {
    LOGE("error parsing init data");
    Value value = state->class_factory()->CreateTable();
    state->global()->Set("_init_data", value);
    state->global()->Set("__weex_data__", value);
    return;
  }

  Value value = JSONToValue(state, json);
  if (value.type != Value::Type::TABLE) {
    value = state->class_factory()->CreateTable();
  }
  state->global()->Set("_init_data", value);
  state->global()->Set("__weex_data__", value);
}

void AddStyles(ExecState* state, const std::string& prefix, const json11::Json& style_obj) {
  if (style_obj.is_object()) {
    for (auto it = style_obj.object_items().begin(); it != style_obj.object_items().end(); it++) {
      state->context()->style_json()[prefix + it->first] = const_cast<json11::Json&>(it->second);
    }
  }
}

void VNodeExecEnv::ParseStyle(ExecState *state) {
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

void VNodeExecEnv::ParseScript(ExecState *state) {
    json11::Json& json = state->context()->raw_json();
    const json11::Json& script_array = json["script"];
    state->context()->set_script_json(script_array);
}

Value StringToValue(ExecState *exec_state,const std::string &str) {
    Value ret;
    do {
        std::string err;
        json11::Json json = json11::Json::parse(str, err);
        if (!err.empty() || json.is_null()) {
            break;
        }
        ret = JSONToValue(exec_state, json);
        
    } while (0);
    
    return ret;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
