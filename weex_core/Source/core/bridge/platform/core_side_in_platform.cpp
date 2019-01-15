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
#include "base/string_util.h"
#include "base/log_defines.h"
#include "core/config/core_environment.h"
#include "core/data_render/vnode/vnode_render_manager.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_list.h"
#include "core/render/node/render_object.h"
#include "core/render/page/render_page.h"

namespace WeexCore {

CoreSideInPlatform::CoreSideInPlatform() {}
CoreSideInPlatform::~CoreSideInPlatform() {}
void CoreSideInPlatform::SetDefaultHeightAndWidthIntoRootDom(
    const std::string &instance_id, float default_width, float default_height,
    bool is_width_wrap_content, bool is_height_wrap_content) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

#if RENDER_LOG
  LOGD(
      "[JNI] SetDefaultHeightAndWidthIntoRootDom >>>> pageId: %s, "
      "defaultWidth: %f, defaultHeight: %f",
      page->PageId().c_str(), defaultWidth, defaultHeight);
#endif

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
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  RenderObject *render = page->GetRenderObject(render_ref);
  if (render == nullptr) return;

  render->setStyleWidthLevel(CSS_STYLE);
  render->setStyleWidth(width, true);
  page->set_is_dirty(true);
}

void CoreSideInPlatform::SetStyleHeight(const std::string &instance_id,
                                        const std::string &render_ref,
                                        float height) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  RenderObject *render = page->GetRenderObject(render_ref);
  if (render == nullptr) return;

  render->setStyleHeightLevel(CSS_STYLE);
  render->setStyleHeight(height);
  page->set_is_dirty(true);
}

void CoreSideInPlatform::SetMargin(const std::string &instance_id,
                                   const std::string &render_ref, int edge,
                                   float value) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  RenderObject *render = page->GetRenderObject(render_ref);
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
  page->set_is_dirty(true);
}

void CoreSideInPlatform::SetPadding(const std::string &instance_id,
                                    const std::string &render_ref, int edge,
                                    float value) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  RenderObject *render = page->GetRenderObject(render_ref);
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
  page->set_is_dirty(true);
}

void CoreSideInPlatform::SetPosition(const std::string &instance_id,
                                     const std::string &render_ref, int edge,
                                     float value) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  RenderObject *render = page->GetRenderObject(render_ref);
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
  page->set_is_dirty(true);
}

void CoreSideInPlatform::MarkDirty(const std::string &instance_id,
                                   const std::string &render_ref) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  RenderObject *render = page->GetRenderObject(render_ref);
  if (render == nullptr) return;
  render->markDirty();
}

void CoreSideInPlatform::SetViewPortWidth(const std::string &instance_id,
                                          float width) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  page->set_viewport_width(width);
}

void CoreSideInPlatform::SetPageDirty(const std::string &instance_id) {
  RenderPage *page =
      RenderManager::GetInstance()->GetPage(std::string(instance_id));
  if (page != nullptr) {
    page->set_is_dirty(true);
  }
}

void CoreSideInPlatform::ForceLayout(const std::string &instance_id) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page != nullptr) {
#if RENDER_LOG
    LOGD("[JNI] ForceLayout >>>> pageId: %s, needForceLayout: %s",
         jString2StrFast(env, instanceId).c_str(),
         page->hasForeLayoutAction.load() ? "true" : "false");
#endif

    page->LayoutImmediately();
    page->has_fore_layout_action_.store(false);
  }
}

bool CoreSideInPlatform::NotifyLayout(const std::string &instance_id) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page != nullptr) {
    if (!page->need_layout_.load()) {
      page->need_layout_.store(true);
    }

    bool ret = !page->has_fore_layout_action_.load() && page->is_dirty();
    if (ret) {
      page->has_fore_layout_action_.store(true);
    }
    return ret ? true : false;
  }
  return false;
}

std::vector<int64_t> CoreSideInPlatform::GetFirstScreenRenderTime(
    const std::string &instance_id) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return std::vector<int64_t>();
  } else {
    return page->PrintFirstScreenLog();
  }
}

std::vector<int64_t> CoreSideInPlatform::GetRenderFinishTime(
    const std::string &instance_id) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return std::vector<int64_t>();
  } else {
    return page->PrintRenderSuccessLog();
  }
}

void CoreSideInPlatform::SetRenderContainerWrapContent(
    const std::string &instance_id, bool wrap) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  page->set_is_render_container_width_wrap_content(wrap);
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
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return 0;
  }

  RenderObject *render = page->GetRenderObject(render_ref);
  if (render == nullptr) {
    return 0;
  }
  return convert_render_object_to_long(render);
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
  auto node_manager =
      weex::core::data_render::VNodeRenderManager::GetInstance();
  // First check if page is rendered with data render strategy.

  if(params.size() < 2)
    return false;

  if(params[1]->value.string->length <= 0)
    return false;

  std::string init_data = weex::base::to_utf8(params[1]->value.string->content,
                                              params[1]->value.string->length);

  if (node_manager->RefreshPage(instanceId, init_data)) {
    return true;
  }
  return ExecJS(instanceId, nameSpace, func, params);
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
    std::function<void(const char *)> exec_js =
        [instanceId = std::string(instanceId), func = std::string(func),
         opts = std::string(opts), initData = std::string(initData),
         extendsApi = std::string(extendsApi)](const char *result) {
          // FIXME Now only support vue, this should be fixed
          std::string error;
          auto opts_json = json11::Json::parse(opts, error);
          std::map<std::string, json11::Json> &opts_map =
              const_cast<std::map<std::string, json11::Json> &>(
                  opts_json.object_items());
          opts_map["bundleType"] = "Vue";
          std::vector<INIT_FRAMEWORK_PARAMS*> params;
          WeexCoreManager::Instance()
              ->script_bridge()
              ->script_side()
              ->CreateInstance(instanceId.c_str(), func.c_str(), result,
                               opts_json.dump().c_str(), initData.c_str(),
                               extendsApi.c_str(),params);
        };
    if (strcmp(render_strategy, "DATA_RENDER") == 0) {
      auto node_manager =
          weex::core::data_render::VNodeRenderManager::GetInstance();
      node_manager->CreatePage(script, instanceId, render_strategy, initData,
                               exec_js);

      return true;
    } else if (strcmp(render_strategy, "DATA_RENDER_BINARY") == 0) {
      auto node_manager =
          weex::core::data_render::VNodeRenderManager::GetInstance();
      node_manager->CreatePage(script, script_length, instanceId,
                               render_strategy, initData, exec_js);

      return true;
    }
  }

  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->CreateInstance(instanceId, func, script, opts, initData, extendsApi, params);
}

std::unique_ptr<WeexJSResult> CoreSideInPlatform::ExecJSOnInstance(const char *instanceId,
                                                 const char *script) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->ExecJSOnInstance(instanceId, script);
}

int CoreSideInPlatform::DestroyInstance(const char *instanceId) {
  auto node_manager =
      weex::core::data_render::VNodeRenderManager::GetInstance();
  if (node_manager->ClosePage(instanceId)) {
    return true;
  }
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->DestroyInstance(instanceId);
}

int CoreSideInPlatform::UpdateGlobalConfig(const char *config) {
  return WeexCoreManager::Instance()
      ->script_bridge()
      ->script_side()
      ->UpdateGlobalConfig(config);
}
}  // namespace WeexCore
