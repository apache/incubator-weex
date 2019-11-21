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


#include "core/bridge/platform/core_side_in_platform.h"

#include "core/common/view_utils.h"
#include "base/string_util.h"
#include "base/log_defines.h"
#include "core/config/core_environment.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_list.h"
#include "core/render/node/render_object.h"
#include "core/render/page/render_page.h"
#include "core/json/JsonRenderManager.h"
#include "core/bridge/eagle_bridge.h"
#include "third_party/json11/json11.hpp"
#include "core/moniter/render_performance.h"
#ifdef OS_ANDROID
#include <android/utils/params_utils.h>
#include <wson/wson.h>
#endif

namespace WeexCore {

CoreSideInPlatform::CoreSideInPlatform() {}
CoreSideInPlatform::~CoreSideInPlatform() {}
void CoreSideInPlatform::SetDefaultHeightAndWidthIntoRootDom(
    const std::string &instance_id, float default_width, float default_height,
    bool is_width_wrap_content, bool is_height_wrap_content) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  page->SetDefaultHeightAndWidthIntoRootRender(default_width, default_height,
                                               is_width_wrap_content,
                                               is_height_wrap_content);
}

void CoreSideInPlatform::OnInstanceClose(const std::string &instance_id) {
  RenderManager::GetInstance()->ClosePage(instance_id);
}

void CoreSideInPlatform::SetStyleWidth(const std::string &instance_id,
                                       const std::string &render_ref,
                                       float width) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;

  RenderObject *render = static_cast<RenderPage*>(page)->GetRenderObject(render_ref);
  if (render == nullptr) return;

  render->setStyleWidthLevel(CSS_STYLE);
  render->setStyleWidth(width, true);
  static_cast<RenderPage*>(page)->set_is_dirty(true);
}

void CoreSideInPlatform::SetStyleHeight(const std::string &instance_id,
                                        const std::string &render_ref,
                                        float height) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;

  RenderObject *render = static_cast<RenderPage*>(page)->GetRenderObject(render_ref);
  if (render == nullptr) return;

  render->setStyleHeightLevel(CSS_STYLE);
  render->setStyleHeight(height);
  static_cast<RenderPage*>(page)->set_is_dirty(true);
}

void CoreSideInPlatform::SetMargin(const std::string &instance_id,
                                   const std::string &render_ref, int edge,
                                   float value) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;

  RenderObject *render = static_cast<RenderPage*>(page)->GetRenderObject(render_ref);
  if (render == nullptr) return;

  if (edge == 0) {
    render->setMargin(kMarginTop, value);
  } else if (edge == 1) {
    render->setMargin(kMarginBottom, value);
  } else if (edge == 2) {
    render->setMargin(kMarginLeft, value);
  } else if (edge == 3) {
    render->setMargin(kMarginRight, value);
  } else if (edge == 4) {
    render->setMargin(kMarginALL, value);
  }
  static_cast<RenderPage*>(page)->set_is_dirty(true);
}

void CoreSideInPlatform::SetPadding(const std::string &instance_id,
                                    const std::string &render_ref, int edge,
                                    float value) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;

  RenderObject *render = static_cast<RenderPage*>(page)->GetRenderObject(render_ref);
  if (render == nullptr) return;

  if (edge == 0) {
    render->setPadding(kPaddingTop, value);
  } else if (edge == 1) {
    render->setPadding(kPaddingBottom, value);
  } else if (edge == 2) {
    render->setPadding(kPaddingLeft, value);
  } else if (edge == 3) {
    render->setPadding(kPaddingRight, value);
  } else if (edge == 4) {
    render->setPadding(kPaddingALL, value);
  }
  static_cast<RenderPage*>(page)->set_is_dirty(true);
}

void CoreSideInPlatform::SetPosition(const std::string &instance_id,
                                     const std::string &render_ref, int edge,
                                     float value) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;

  RenderObject *render = static_cast<RenderPage*>(page)->GetRenderObject(render_ref);
  if (render == nullptr) return;

  if (edge == 0) {
    render->setStylePosition(kPositionEdgeTop, value);
  } else if (edge == 1) {
    render->setStylePosition(kPositionEdgeBottom, value);
  } else if (edge == 2) {
    render->setStylePosition(kPositionEdgeLeft, value);
  } else if (edge == 3) {
    render->setStylePosition(kPositionEdgeRight, value);
  }
  static_cast<RenderPage*>(page)->set_is_dirty(true);
}

void CoreSideInPlatform::MarkDirty(const std::string &instance_id,
                                   const std::string &render_ref) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;

  RenderObject *render = static_cast<RenderPage*>(page)->GetRenderObject(render_ref);
  if (render == nullptr) return;
  render->markDirty();
}

void CoreSideInPlatform::SetViewPortWidth(const std::string &instance_id,
                                          float width) {
  RenderManager::GetInstance()->set_viewport_width(instance_id, width);
}
    
void CoreSideInPlatform::SetDeviceDisplayOfPage(const std::string &instance_id, float width, float height /* unused now */)  {
  RenderManager::GetInstance()->setDeviceWidth(instance_id, width);
}

void CoreSideInPlatform::SetPageRenderType(const std::string &pageId,
                                           const std::string &renderType) {
    RenderManager::GetInstance()->setPageArgument(pageId, "renderType", renderType);
}

void CoreSideInPlatform::RemovePageRenderType(const std::string &pageId) {
  // void
}


void CoreSideInPlatform::SetPageArgument(const std::string &pageId, const std::string& key, const std::string& value){
     RenderManager::GetInstance()->setPageArgument(pageId, key, value);
}

void CoreSideInPlatform::SetDeviceDisplay(const std::string &instance_id, float width, float height, float scale) {
  RenderManager::GetInstance()->setDeviceWidth(instance_id, width);

    /**
     * also update global device with height and options
     * */
  WXCoreEnvironment::getInstance()->SetDeviceWidth(std::to_string(width));
  WXCoreEnvironment::getInstance()->SetDeviceHeight(std::to_string(height));
  WXCoreEnvironment::getInstance()->PutOption(SCALE, std::to_string(scale));
}

void CoreSideInPlatform::SetPageDirty(const std::string &instance_id) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(std::string(instance_id));
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;
  static_cast<RenderPage*>(page)->set_is_dirty(true);
}

void CoreSideInPlatform::ForceLayout(const std::string &instance_id) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page != nullptr) {
    if (!page->is_platform_page()) return;
    static_cast<RenderPage*>(page)->LayoutImmediately();
    static_cast<RenderPage*>(page)->has_fore_layout_action_.store(false);
  }
}

bool CoreSideInPlatform::NotifyLayout(const std::string &instance_id) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page != nullptr) {
    if (!page->is_platform_page()) return false;
      
    if (!static_cast<RenderPage*>(page)->need_layout_.load()) {
      static_cast<RenderPage*>(page)->need_layout_.store(true);
    }

    bool ret = !static_cast<RenderPage*>(page)->has_fore_layout_action_.load() && static_cast<RenderPage*>(page)->is_dirty();
    if (ret) {
      static_cast<RenderPage*>(page)->has_fore_layout_action_.store(true);
    }
    return ret ? true : false;
  }
  return false;
}
    
bool CoreSideInPlatform::RelayoutUsingRawCssStyles(const std::string& instance_id) {
  return RenderManager::GetInstance()->ReloadPageLayout(instance_id);
}

std::vector<int64_t> CoreSideInPlatform::GetFirstScreenRenderTime(
    const std::string &instance_id) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return std::vector<int64_t>();
  } else {
    return page->PrintFirstScreenLog();
  }
}

std::vector<int64_t> CoreSideInPlatform::GetRenderFinishTime(
    const std::string &instance_id) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return std::vector<int64_t>();
  } else {
    return page->PrintRenderSuccessLog();
  }
}

void CoreSideInPlatform::SetRenderContainerWrapContent(
    const std::string &instance_id, bool wrap) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;
  if (!page->is_platform_page()) return;

  static_cast<RenderPage*>(page)->set_is_render_container_width_wrap_content(wrap);
}

void CoreSideInPlatform::BindMeasurementToRenderObject(long ptr) {
  RenderObject *render = convert_long_to_render_object(ptr);
  if (render && measure_function_adapter_exist_) {
    render->BindMeasureFunc();
  }
}

void CoreSideInPlatform::RegisterCoreEnv(const std::string &key,
                                         const std::string &value) {
  WXCoreEnvironment::getInstance()->AddOption(key, value);
}

long CoreSideInPlatform::GetRenderObject(const std::string &instance_id,
                                         const std::string &render_ref) {
  RenderPageBase *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return 0;
  }
  return convert_render_object_to_long(page->GetRenderObject(render_ref));
}

void CoreSideInPlatform::UpdateRenderObjectStyle(long render_ptr,
                                                 const std::string &key,
                                                 const std::string &value) {
  RenderObject *render = convert_long_to_render_object(render_ptr);
  render->UpdateStyle(key, value);
}

void CoreSideInPlatform::UpdateRenderObjectAttr(long render_ptr,
                                                const std::string &key,
                                                const std::string &value) {
  RenderObject *render = convert_long_to_render_object(render_ptr);
  render->UpdateAttr(key, value);
  render->markDirty(true);
}

long CoreSideInPlatform::CopyRenderObject(long render_ptr) {
  RenderObject *render = convert_long_to_render_object(render_ptr);
  RenderObject *copy =
      static_cast<RenderObject *>(RenderCreator::GetInstance()->CreateRender(
          render->type(), render->ref()));
  copy->CopyFrom(render);
  if (render->type() == WeexCore::kRenderCellSlot ||
      render->type() == WeexCore::kRenderCell) {
    RenderList *renderList = static_cast<RenderList *>(render->getParent());
    if (renderList != nullptr) {
      renderList->AddCellSlotCopyTrack(copy);
    } else {
      LOGE("CopyRenderObject: %s", "copy error parent null");
    }
  }
  return convert_render_object_to_long(copy);
}

void CoreSideInPlatform::SetMeasureFunctionAdapter() {
  measure_function_adapter_exist_ = true;
}

void CoreSideInPlatform::SetPlatform(const std::string &platformName) {
  WXCoreEnvironment::getInstance()->SetPlatform(platformName);
}

void CoreSideInPlatform::SetDeviceWidthAndHeight(float width, float height) {
  WXCoreEnvironment::getInstance()->set_device_width(width);
  WXCoreEnvironment::getInstance()->set_device_height(height);
}

void CoreSideInPlatform::AddOption(const std::string &key,
                                   const std::string &value) {
  WXCoreEnvironment::getInstance()->AddOption(key, value);
}

int CoreSideInPlatform::RefreshInstance(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
#ifdef OS_ANDROID
  if(params.size() < 2)
    return false;

  if(params[1]->value.string->length <= 0)
    return false;

  std::string init_data = weex::base::to_utf8(params[1]->value.string->content,
                                              params[1]->value.string->length);
  auto handler = EagleBridge::GetInstance()->data_render_handler();
  if (handler && handler->RefreshPage(instanceId, init_data)) {
    std::vector<VALUE_WITH_TYPE*> msg;

    VALUE_WITH_TYPE* event = getValueWithTypePtr();
    event->type = ParamsType::BYTEARRAY;
    auto buffer = wson_buffer_new();
    wson_push_type_uint8_string(
        buffer, reinterpret_cast<const uint8_t*>(instanceId), strlen(instanceId));
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
    ScriptBridge* bridge = WeexCore::WeexCoreManager::Instance()->script_bridge();
    if (!bridge){
      return false;
    }
    bridge->script_side()->ExecJS(
        instanceId, "", "callJS", msg);
    freeParams(msg);
    return true;
  }
  return ExecJS(instanceId, nameSpace, func, params);
#else
  return 0;
#endif
}

int CoreSideInPlatform::InitFramework(
    const char *script, std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->InitFramework(script, params);
}

int CoreSideInPlatform::InitAppFramework(
    const char *instanceId, const char *appFramework,
    std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->InitAppFramework(instanceId, appFramework, params);
}

int CoreSideInPlatform::CreateAppContext(const char *instanceId,
                                         const char *jsBundle) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->CreateAppContext(instanceId, jsBundle);
}

std::unique_ptr<WeexJSResult> CoreSideInPlatform::ExecJSOnAppWithResult(const char *instanceId,
                                                      const char *jsBundle) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->ExecJSOnAppWithResult(instanceId, jsBundle);
}

int CoreSideInPlatform::CallJSOnAppContext(
    const char *instanceId, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->CallJSOnAppContext(instanceId, func, params);
}

int CoreSideInPlatform::DestroyAppContext(const char *instanceId) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->DestroyAppContext(instanceId);
}

int CoreSideInPlatform::ExecJsService(const char *source) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->ExecJsService(source);
}

int CoreSideInPlatform::ExecTimeCallback(const char *source) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->ExecTimeCallback(source);
}

int CoreSideInPlatform::ExecJS(const char *instanceId, const char *nameSpace,
                               const char *func,
                               std::vector<VALUE_WITH_TYPE *> &params) {
  return WeexCoreManager::Instance()->script_bridge()->script_side()->ExecJS(
      instanceId, nameSpace, func, params);
}

std::unique_ptr<WeexJSResult> CoreSideInPlatform::ExecJSWithResult(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->ExecJSWithResult(instanceId, nameSpace, func, params);
}

void CoreSideInPlatform::ExecJSWithCallback(
    const char *instanceId, const char *nameSpace, const char *func,
    std::vector<VALUE_WITH_TYPE *> &params, long callback_id) {
  WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->ExecJSWithCallback(instanceId, nameSpace, func, params, callback_id);
}

int CoreSideInPlatform::CreateInstance(const char *instanceId, const char *func,
                                       const char *script, int script_length,
                                       const char *opts,
                                       const char *initData,
                                       const char *extendsApi, std::vector<INIT_FRAMEWORK_PARAMS*>& params,
                                       const char *render_strategy) {

  // First check about DATA_RENDER mode
  if (render_strategy != nullptr) {
    std::function<void(const char *, const char *)> exec_js =
        [instanceId = std::string(instanceId), func = std::string(func),
         opts = std::string(opts), initData = std::string(initData),
         extendsApi = std::string(extendsApi)](const char *result, const char *bundleType) {
          std::string error;
          auto opts_json = json11::Json::parse(opts, error);
          std::map<std::string, json11::Json> &opts_map =
              const_cast<std::map<std::string, json11::Json> &>(
                  opts_json.object_items());
          opts_map["bundleType"] = bundleType;
          std::vector<INIT_FRAMEWORK_PARAMS*> params;
          WeexCoreManager::Instance()
              ->script_bridge()
              ->script_side()
              ->CreateInstance(instanceId.c_str(), func.c_str(), result,
                               opts_json.dump().c_str(), initData.c_str(),
                               strcmp("Rax", bundleType) ? "\0" : extendsApi.c_str(),
                               params);
        };
    if (strcmp(render_strategy, "DATA_RENDER") == 0) {
        auto handler = EagleBridge::GetInstance()->data_render_handler();
        if(handler){
          handler->CreatePage(script, instanceId, render_strategy, initData, exec_js);
        }
        else{
          WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(
            instanceId, "CreatePageWithContent", 
            "There is no data_render_handler when createInstance with DATA_RENDER mode");
        }

      return true;
    } else if (strcmp(render_strategy, "DATA_RENDER_BINARY") == 0) {
      std::string error;
      std::string env_str;
      std::string option = "{}";
      auto opts_json_value = json11::Json::parse(opts, error);
      if (error.empty()) {
        auto env_obj = opts_json_value["env"];
        auto bundleUrl = opts_json_value["bundleUrl"];
        env_str = "";
        if (env_obj.is_object()) {
          const json11::Json& options = env_obj["options"];
          const json11::Json::object& options_obj = options.object_items();
          json11::Json::object new_env{
              env_obj.object_items()
          };
          for(auto &it :options_obj){
            new_env[it.first] = it.second;
          }
          env_str = json11::Json(new_env).dump();
        }

        json11::Json::object new_option{
            {"bundleUrl", bundleUrl},
            {"weex", json11::Json::object{
                {"config",opts_json_value}
            }}
        };
        option = json11::Json(new_option).dump();
      }
      
      auto handler = EagleBridge::GetInstance()->data_render_handler();
      if(handler){
        handler->CreatePage(script, static_cast<size_t>(script_length), instanceId, option, env_str, initData, exec_js);
      }
      else{
        WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(
          instanceId, "CreatePageWithContent", 
          "There is no data_render_handler when createInstance with DATA_RENDER_BINARY mode");
      }
      return true;
    }

    if(strcmp(render_strategy, "JSON_RENDER") == 0){
       JsonRenderManager::GetInstance()->CreatePage(script, instanceId, render_strategy);
       return true;
    }
  }


  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->CreateInstance(instanceId, func, script, opts, initData, extendsApi, params);
}

std::unique_ptr<WeexJSResult> CoreSideInPlatform::ExecJSOnInstance(const char *instanceId,
                                                 const char *script, int type) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->ExecJSOnInstance(instanceId, script,type);
}

int CoreSideInPlatform::DestroyInstance(const char *instanceId) {
    auto handler = EagleBridge::GetInstance()->data_render_handler();
    if (handler != nullptr) {
      handler->DestroyInstance(instanceId);
    }
    if (JsonRenderManager::GetInstance()->ClosePage(instanceId)) {
      return true;
    }
    auto script_side = WeexCoreManager::Instance()->script_bridge()->script_side();
    if (script_side) {
        return script_side->DestroyInstance(instanceId);
    }
    return true;
}

int CoreSideInPlatform::UpdateGlobalConfig(const char *config) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->UpdateGlobalConfig(config);
}


int CoreSideInPlatform::UpdateInitFrameworkParams(const std::string &key, const std::string &value,
                                                  const std::string &desc) {
  return WeexCoreManager::Instance()
          ->script_bridge()
          ->script_side()
          ->UpdateInitFrameworkParams(key, value, desc);
}

void CoreSideInPlatform::SetLogType(const int logType, const bool isPerf) {
  WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->SetLogType(logType,isPerf);
}

double CoreSideInPlatform::GetLayoutTime(const char* instanceId) const {
    RenderPageBase *page = RenderManager::GetInstance()->GetPage(instanceId);
    if (page == nullptr) return 0;
    if (!page->is_platform_page()) return 0;
    return page->getPerformance()->cssLayoutTime;
}


}  // namespace WeexCore
