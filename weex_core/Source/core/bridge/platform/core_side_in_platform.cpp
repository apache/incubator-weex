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
#include "core_side_in_platform.h"

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

  EagleModeReturn mode = EagleBridge::GetInstance()->RefreshPage(instanceId, init_data.c_str());
  if (mode == EagleModeReturn::EAGLE_ONLY){
    return true;
  } else {
    //mode == EagleModeReturn::EAGLE_AND_SCRIPT || mode == EagleModeReturn::NOT_EAGLE
    //continue;
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
  if (render_strategy != nullptr) {
    if(strcmp(render_strategy, "JSON_RENDER") == 0){
      JsonRenderManager::GetInstance()->CreatePage(script, instanceId, render_strategy);
      return true;
    }

    EagleBridge::GetInstance()->CreatePage(render_strategy, instanceId, func, script, script_length, opts, initData ,extendsApi, 0);
    return true;
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
    if (JsonRenderManager::GetInstance()->ClosePage(instanceId)) {
      return true;
    }
    EagleModeReturn mode = EagleBridge::GetInstance()->DestroyPage(instanceId);
    if (mode == EagleModeReturn::EAGLE_ONLY){
      return true;
    } else {
      //mode == EagleModeReturn::EAGLE_AND_SCRIPT || mode == EagleModeReturn::NOT_EAGLE
      //continue;
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

int64_t CoreSideInPlatform::JsAction(long ctxContainer, int32_t jsActionType, const char *arg) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->JsAction(ctxContainer, jsActionType, arg);
}

}  // namespace WeexCore
