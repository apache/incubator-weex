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

#include "bridge.h"
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <core/render/node/render_object.h>

namespace WeexCore {
    void Bridge::setStyleWidth(const char *instanceId, const char *ref, float value) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page == nullptr)
            return;

        RenderObject *render = page->GetRenderObject(std::string(ref));
        if (render == nullptr)
            return;

        render->setStyleWidthLevel(CSS_STYLE);
        render->setStyleWidth(value, true);
        page->set_is_dirty(true);
    }

    void Bridge::setStyleHeight(const char *instanceId, const char *ref, float value) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page == nullptr)
            return;

        RenderObject *render = page->GetRenderObject(std::string(ref));
        if (render == nullptr)
            return;

        render->setStyleHeightLevel(CSS_STYLE);
        render->setStyleHeight(value);
        page->set_is_dirty(true);
    }


    void Bridge::setMargin(const char *instanceId, const char *ref, int32_t edge, float value) {
         RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
         if (page == nullptr)
           return;

         RenderObject *render = page->GetRenderObject(std::string(ref));
         if (render == nullptr)
           return;

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

    void Bridge::setPadding(const char *instanceId, const char *ref, int32_t edge, float value) {
         RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
         if (page == nullptr)
           return;

         RenderObject *render = page->GetRenderObject(std::string(ref));
         if (render == nullptr)
           return;

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

    void Bridge::setPosition(const char *instanceId, const char *ref, int32_t edge, float value) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page == nullptr)
            return;

        RenderObject *render = page->GetRenderObject(std::string(ref));
        if (render == nullptr)
            return;

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

    void Bridge::markDirty(const char *instanceId, const char *ref, bool dirty) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page == nullptr)
            return;

        if (dirty) {
            RenderObject *render = page->GetRenderObject(std::string(ref));
            if (render == nullptr)
                return;
            render->markDirty();
        }
    }

    void Bridge::setDefaultHeightAndWidthIntoRootDom(const char *instanceId, const float defaultWidth,
                                                     const float defaultHeight,
                                                     const bool isWidthWrapContent, const bool isHeightWrapContent) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page == nullptr) {
            return;
        }

#if RENDER_LOG
        LOGD("[JNI] SetDefaultHeightAndWidthIntoRootDom >>>> pageId: %s, defaultWidth: %f, defaultHeight: %f",
       page->PageId().c_str(), defaultWidth,defaultHeight);
#endif
        page->SetDefaultHeightAndWidthIntoRootRender(defaultWidth, defaultHeight, isWidthWrapContent,
                                                     isHeightWrapContent);
    }

    void Bridge::setRenderContainerWrapContent(const char *instanceId, bool wrap) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page == nullptr)
            return;

        page->set_is_render_container_width_wrap_content(wrap);

    }

    void Bridge::forceLayout(const char *instanceId) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page != nullptr) {

#if RENDER_LOG
            LOGD("[JNI] ForceLayout >>>> pageId: %s, needForceLayout: %s", jString2StrFast(env, instanceId).c_str(), page->hasForeLayoutAction.load()?"true":"false");
#endif
            page->LayoutImmediately();
            page->has_fore_layout_action_.store(false);
        }
    }

    bool Bridge::notifyLayout(const char *instanceId) {
        RenderPage *page = RenderManager::GetInstance()->GetPage(std::string(instanceId));
        if (page != nullptr) {

            if (!page->need_layout_.load()) {
                page->need_layout_.store(true);
            }

            bool ret = !page->has_fore_layout_action_.load() && page->is_dirty();
            if (ret) {
                page->has_fore_layout_action_.store(true);
            }
        }
    }

    void Bridge::onInstanceClose(const char *instanceId) {
        RenderManager::GetInstance()->ClosePage(std::string(instanceId));
    }
}