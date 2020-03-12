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
#include "core/bridge/eagle_bridge_ext.h"
#include "third_party/json11/json11.hpp"
#include "core/manager/weex_core_manager.h"

#ifdef OS_ANDROID
#include "wson/wson.h"
#include "android/utils/params_utils.h"
#include "base/string_util.h"
#endif
namespace WeexCore {
namespace EagleExt{

std::function<void(const char*, const char*)> CreatePageDownloadExec(const char* instanceId,
                                         const char* func,
                                         const char* script,
                                         int script_length,
                                         const char* opts,
                                         const char* initData,
                                         const char* extendsApi){
  std::function<void(const char*, const char*)> exec_js =
      [instanceId = std::string(instanceId), func = std::string(func),
          opts = std::string(opts), initData = std::string(initData),
          extendsApi = std::string(extendsApi)](const char* result, const char* bundleType) {
        std::string error;
        auto opts_json = json11::Json::parse(opts, error);
        std::map<std::string, json11::Json>& opts_map =
            const_cast<std::map<std::string, json11::Json>&>(
                opts_json.object_items());
        opts_map["bundleType"] = bundleType;
        std::vector<INIT_FRAMEWORK_PARAMS*> params;
        auto bridge =         WeexCoreManager::Instance()
                                  ->script_bridge()
                                  ->script_side();
        if(!bridge){
            return;
        }
        bridge->CreateInstance(instanceId.c_str(), func.c_str(), result,
                             opts_json.dump().c_str(), initData.c_str(),
                             strcmp("Rax", bundleType) ? "\0" : extendsApi.c_str(),
                             params);
      };
  return exec_js;
}
#ifdef OS_ANDROID
void RefreshPageEagle(const char* page_id, const char* init_data) {
  std::vector<VALUE_WITH_TYPE*> msg;

  VALUE_WITH_TYPE* event = getValueWithTypePtr();
  event->type = ParamsType::BYTEARRAY;
  auto buffer = wson_buffer_new();
  wson_push_type_uint8_string(
      buffer, reinterpret_cast<const uint8_t*>(page_id), strlen(page_id));
  event->value.byteArray = genWeexByteArray(
      static_cast<const char*>(buffer->data), buffer->position);
  wson_buffer_free(buffer);
  msg.push_back(event);

  // args -> { method: 'fireEvent', args: [ref, "nodeEvent", args , domChanges, {params: [ {"templateId": templateId, "componentId": id, "type": type, "params" : [...]} ]}] }
  VALUE_WITH_TYPE* args = getValueWithTypePtr();
  args->type = ParamsType::JSONSTRING;
  json11::Json final_json = json11::Json::array{
      json11::Json::object{
          {"method", "fireEvent"},
          {"args",
                     json11::Json::array{
                         "", "refresh", json11::Json::array{}, "",
                         json11::Json::object{
                             {"params",
                                 json11::Json::array{
                                     json11::Json::object{
                                         {"data", init_data}
                                     }
                                 }}
                         }
                     }}
      }
  };
  std::string out = final_json.dump();
  auto utf16_key = weex::base::to_utf16(const_cast<char*>(out.c_str()),
                                        out.length());
  args->value.string = genWeexString(
      reinterpret_cast<const uint16_t*>(utf16_key.c_str()), utf16_key.size());
  msg.push_back(args);
  auto bridge = WeexCore::WeexCoreManager::Instance()->script_bridge()->script_side();
  if (!bridge){
    return;
  }
  bridge->ExecJS(
      page_id, "", "callJS", msg);
  freeParams(msg);
}
#endif
}
}