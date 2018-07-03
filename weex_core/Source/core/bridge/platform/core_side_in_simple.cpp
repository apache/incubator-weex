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
//
// Created by yxp on 2018/6/20.
//

#include "core_side_in_simple.h"
#include "android/base/log_utils.h"
#include "core/config/core_environment.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_list.h"
#include "core/render/node/render_object.h"
#include "core/render/page/render_page.h"

namespace WeexCore {
void CoreSideInSimple::SetDefaultHeightAndWidthIntoRootDom(
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

void CoreSideInSimple::OnInstanceClose(const std::string &instance_id) {
  RenderManager::GetInstance()->ClosePage(instance_id);
}

void CoreSideInSimple::SetStyleWidth(const std::string &instance_id,
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

void CoreSideInSimple::SetStyleHeight(const std::string &instance_id,
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

void CoreSideInSimple::SetMargin(const std::string &instance_id,
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
void CoreSideInSimple::SetPadding(const std::string &instance_id,
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

void CoreSideInSimple::SetPosition(const std::string &instance_id,
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

void CoreSideInSimple::MarkDirty(const std::string &instance_id,
                                 const std::string &render_ref) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  RenderObject *render = page->GetRenderObject(render_ref);
  if (render == nullptr) return;
  render->markDirty();
}

void CoreSideInSimple::SetViewPortWidth(const std::string &instance_id,
                                        float width) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  page->set_viewport_width(width);
}

void CoreSideInSimple::ForceLayout(const std::string &instance_id) {
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

bool CoreSideInSimple::NotifyLayout(const std::string &instance_id) {
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
std::vector<int64_t> CoreSideInSimple::GetFirstScreenRenderTime(
    const std::string &instance_id) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return std::vector<int64_t>();
  } else {
    return page->PrintFirstScreenLog();
  }
}
std::vector<int64_t> CoreSideInSimple::GetRenderFinishTime(
    const std::string &instance_id) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) {
    return std::vector<int64_t>();
  } else {
    return page->PrintRenderSuccessLog();
  }
}
void CoreSideInSimple::SetRenderContainerWrapContent(
    const std::string &instance_id, bool wrap) {
  RenderPage *page = RenderManager::GetInstance()->GetPage(instance_id);
  if (page == nullptr) return;

  page->set_is_render_container_width_wrap_content(wrap);
}

void CoreSideInSimple::BindMeasurementToRenderObject(long ptr) {
  RenderObject *render = convert_long_to_render_object(ptr);
  if (render && measure_function_adapter_exist_) {
    render->BindMeasureFunc();
  }
}

void CoreSideInSimple::RegisterCoreEnv(const std::string &key,
                                       const std::string &value) {
  WXCoreEnvironment::getInstance()->AddOption(key, value);
}

long CoreSideInSimple::GetRenderObject(const std::string &instance_id,
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
void CoreSideInSimple::UpdateRenderObjectStyle(long render_ptr,
                                               const std::string &key,
                                               const std::string &value) {
  RenderObject *render = convert_long_to_render_object(render_ptr);
  render->UpdateStyle(key, value);
}

void CoreSideInSimple::UpdateRenderObjectAttr(long render_ptr,
                                              const std::string &key,
                                              const std::string &value) {
  RenderObject *render = convert_long_to_render_object(render_ptr);
  render->UpdateAttr(key, value);
  render->markDirty(true);
}

long CoreSideInSimple::CopyRenderObject(long render_ptr) {
  RenderObject *render = convert_long_to_render_object(render_ptr);
  RenderObject *copy =
      static_cast<RenderObject *>(RenderCreator::GetInstance()->CreateRender(
          render->type(), render->ref()));
  copy->copyFrom(render);
  if (render->type() == WeexCore::kRenderCellSlot ||
      render->type() == WeexCore::kRenderCell) {
    RenderList *renderList = static_cast<RenderList *>(render->getParent());
    if (renderList != nullptr) {
      renderList->AddCellSlotCopyTrack(copy);
    } else {
      LOGE("LayoutRenderObject", "copy error parent null");
    }
  }
  return convert_render_object_to_long(copy);
}

void CoreSideInSimple::SetMeasureFunctionAdapter() {
  measure_function_adapter_exist_ = true;
}

void CoreSideInSimple::SetPlatform(const std::string &platformName) {
  WXCoreEnvironment::getInstance()->SetPlatform(platformName);
}

void CoreSideInSimple::SetDeviceWidthAndHeight(float width, float height) {
  WXCoreEnvironment::getInstance()->set_device_width(width);
  WXCoreEnvironment::getInstance()->set_device_height(height);
}

void CoreSideInSimple::AddOption(const std::string &key,
                                 const std::string &value) {
  WXCoreEnvironment::getInstance()->AddOption(key, value);
}

    ////
    int CoreSideInSimple::InitFramework(const char *script,
                                         std::vector<INIT_FRAMEWORK_PARAMS *> params) {
//      return WeexCoreManager::getJSBridge()->InitFrameWork(script, params);
      return 0;
    }

    int CoreSideInSimple::InitAppFramework(
            const char *instanceId, const char *appFramework,
            std::vector<INIT_FRAMEWORK_PARAMS *> params) {
//      return core_side_functions_->init_app_framework(instanceId, appFramework, params);
      return 0;
    }

    int CoreSideInSimple::CreateAppContext(const char *instanceId,
                                            const char *jsBundle) {
//      return core_side_functions_->create_app_context(instanceId, jsBundle);
      return 0;
    }

    const char *CoreSideInSimple::ExecJSOnAppWithResult(const char *instanceId,
                                                         const char *jsBundle) {
//      return core_side_functions_->exec_js__on_app_with_result(instanceId, jsBundle);
      return 0;
    }

    int CoreSideInSimple::CallJSOnAppContext(const char *instanceId, const char *func,
                                              std::vector<VALUE_WITH_TYPE *> params) {
//      return core_side_functions_->call_js_on_app_context(instanceId, func, params);
      return 0;
    }

    int CoreSideInSimple::DestroyAppContext(const char *instanceId) {
//      return core_side_functions_->destroy_app_context(instanceId);
      return 0;
    }

    int CoreSideInSimple::ExecJsService(const char *source) {
//      return core_side_functions_->exec_js_service(source);
      return 0;
    }

    int CoreSideInSimple::ExecTimeCallback(const char *source) {
      return 0;
    }

    int CoreSideInSimple::ExecJS(const char *instanceId, const char *nameSpace,
                                  const char *func,
                                  std::vector<VALUE_WITH_TYPE *> params) {
//      return core_side_functions_->exec_js(instanceId, nameSpace, func, params);
      return 0;
    }

    WeexJSResult CoreSideInSimple::ExecJSWithResult(
            const char *instanceId, const char *nameSpace, const char *func,
            std::vector<VALUE_WITH_TYPE *> params) {
//      return core_side_functions_->exec_js_with_result(instanceId, nameSpace, func, params);
      return WeexJSResult();
    }

    int CoreSideInSimple::CreateInstance(const char *instanceId, const char *func,
                                          const char *script, const char *opts,
                                          const char *initData, const char *extendsApi) {
//      return core_side_functions_->create_instance(instanceId, func, script, opts, initData,
//                                                   extendsApi);
      return 0;
    }

    const char *CoreSideInSimple::ExecJSOnInstance(const char *instanceId,
                                                    const char *script) {
//      return core_side_functions_->exec_js_on_instance(instanceId, script);
      return nullptr;
    }

    int CoreSideInSimple::DestroyInstance(const char *instanceId) {
//      return core_side_functions_->destroy_instance(instanceId);
      return 0;
    }

    int CoreSideInSimple::UpdateGlobalConfig(const char *config) {
//      return core_side_functions_->update_global_config(config);
      return 0;
    }
}  // namespace WeexCore
