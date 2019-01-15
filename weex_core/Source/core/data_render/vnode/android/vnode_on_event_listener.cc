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
#ifdef OS_ANDROID

#include "core/data_render/vnode/vnode_on_event_listener.h"
#include "android/utils/params_utils.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/vnode/android/conversion.h"
#include "core/data_render/vnode/vcomponent.h"
#include "core/manager/weex_core_manager.h"
#include "include/WeexApiHeader.h"
#include "third_party/json11/json11.hpp"

namespace weex {
namespace core {
namespace data_render {
static const char* kMethodCallJS = "callJS";
static const char* kMethodFireEvent = "fireEvent";
static const char* kKeyMethod = "method";
static const char* kKeyArgs = "args";
static const char* kKeyParams = "params";

void VNodeOnEventListener::OnEvent(VNode* node, const std::string& event,
                                   const std::string& json_args,
                                   const std::string dom_changes,
                                   const VNode::Params& param_list) {
  auto page_id = node->component()->exec_state()->context()->page_id();
  std::vector<VALUE_WITH_TYPE*> params;
  // page_id
  params.push_back(Conversion::GenValueWithType(page_id.c_str()));

  // args -> { method: 'fireEvent', args: [ref, "nodeEvent", args , domChanges, {params: [ {"templateId": templateId, "componentId": id, "type": type, "params" : [...]} ]}] }
  VALUE_WITH_TYPE* args = getValueWithTypePtr();
  args->type = ParamsType::JSONSTRING;
  std::vector<json11::Json> args_object_list;
  std::map<std::string, json11::Json> args_object;
  // method: 'FireEvent'
  args_object.insert({kKeyMethod, kMethodFireEvent});

  std::vector<json11::Json> args_in_args_object;
  // ref TODO make sure the difference between node id and ref
  if (!node->ref().empty()) {
    args_in_args_object.push_back(node->ref());
  } else {
    args_in_args_object.push_back(node->node_id());
  }
  // type -> "nodeEvent"
  args_in_args_object.push_back("nodeEvent");
  // args
  std::string error;
  args_in_args_object.push_back(json11::Json::parse(json_args, error));
  // domChanges
  args_in_args_object.push_back(json11::Json::parse(dom_changes, error));
  // params -> [ {"templateId": templateId, "componentId": id, "type": type, "params" : [...]} ]
  std::map<std::string, json11::Json> params_object;
  std::vector<json11::Json> array_in_params_object;
  {
    std::map<std::string, json11::Json> inner_object;
    // templateId
    inner_object.insert({"templateId", node->component()->template_id()});
    // componentId
    inner_object.insert({"componentId", node->component()->id()});
    // type
    inner_object.insert({"type", event});
    // params
    std::vector<json11::Json> array_in_inner_object;
    for (auto it = param_list.begin(); it != param_list.end(); it++) {
      array_in_inner_object.push_back(Conversion::GenJSON(&*it));
    }
    inner_object.insert({kKeyParams, array_in_inner_object});
    array_in_params_object.push_back(inner_object);
  }
  params_object.insert({kKeyParams, array_in_params_object});
  args_in_args_object.push_back(params_object);
  args_object.insert({kKeyArgs, args_in_args_object});
  args_object_list.push_back(args_object);

  json11::Json final_json(args_object_list);
  auto temp = final_json.dump();
  auto final_json_str = temp.c_str();
  auto utf16_key = weex::base::to_utf16(const_cast<char*>(final_json_str),
                                        strlen(final_json_str));
  args->value.string = genWeexString(
      reinterpret_cast<const uint16_t*>(utf16_key.c_str()), utf16_key.size());
  params.push_back(args);

  WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      page_id.c_str(), "", kMethodCallJS, params);
  freeParams(params);
}

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif