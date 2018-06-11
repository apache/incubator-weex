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
#include "core/render/manager/render_manager.h"
#include "core/render/page/render_page.h"
#include "core/render/node/render_object.h"
#include "base/TimeUtils.h"
#include "core/parser/dom_wson.h"
#include "core/layout/measure_func_adapter.h"

namespace WeexCore {

  RenderManager *RenderManager::g_pInstance = nullptr;

  bool RenderManager::CreatePage(std::string page_id, const char *data) {

#if RENDER_LOG
    wson_parser parser(data);
    LOGD("[RenderManager] CreatePage >>>> pageId: %s, dom data: %s", pageId.c_str(), parser.toStringUTF8().c_str());
#endif

    RenderPage *page = new RenderPage(page_id);
    this->pages.insert(std::pair<std::string, RenderPage *>(page_id, page));

    long long startTime = getCurrentTime();
    RenderObject *root = Wson2RenderObject(data, page_id);
    page->ParseJsonTime(getCurrentTime() - startTime);

    page->updateDirty(true);
    return page->CreateRootRender(root);
  }

  bool RenderManager::AddRenderObject(const std::string &page_id, const std::string &parent_ref,
                                      int index, const char *data) {

    RenderPage *page = GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    wson_parser parser(data);
    LOGD("[RenderManager] AddRenderObject >>>> pageId: %s, parentRef: %s, index: %d, dom data: %s",
         pageId.c_str(), parentRef.c_str(), index, parser.toStringUTF8().c_str());
#endif

    long long startTime = getCurrentTime();
    RenderObject *child = Wson2RenderObject(data, page_id);
    page->ParseJsonTime(getCurrentTime() - startTime);

    if (child == nullptr)
      return false;

    page->updateDirty(true);
    return page->AddRenderObject(parent_ref, index, child);
  }

  bool RenderManager::RemoveRenderObject(const std::string &page_id, const std::string &ref) {

    RenderPage *page = this->GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] RemoveRenderObject >>>> pageId: %s, ref: %s", pageId.c_str(),
         ref.c_str());
#endif

    page->updateDirty(true);
    return page->RemoveRenderObject(ref);
  }

  bool RenderManager::MoveRenderObject(const std::string &page_id, const std::string &ref,
                                       const std::string &parent_ref, int index) {
    RenderPage *page = this->GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] MoveRenderObject >>>> pageId: %s, ref: %s, parentRef: %s, index: %d",
         pageId.c_str(), ref.c_str(), parentRef.c_str(), index);
#endif

    page->updateDirty(true);
    return page->MoveRenderObject(ref, parent_ref, index);
  }

  bool RenderManager::UpdateAttr(const std::string &page_id, const std::string &ref,
                                 const char *data) {
    RenderPage *page = this->GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    wson_parser parser(data);
    LOGD("[RenderManager] UpdateAttr >>>> pageId: %s, ref: %s, data: %s",
         pageId.c_str(), ref.c_str(), parser.toStringUTF8().c_str());
#endif

    long long startTime = getCurrentTime();
    std::vector <std::pair<std::string, std::string>> *attrs = Wson2Pairs(data);
    page->ParseJsonTime(getCurrentTime() - startTime);

    page->updateDirty(true);
    return page->UpdateAttr(ref, attrs);
  }

  bool RenderManager::UpdateStyle(const std::string &page_id, const std::string &ref,
                                  const char *data) {
    RenderPage *page = this->GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    wson_parser parser(data);
    LOGD("[RenderManager] UpdateStyle >>>> pageId: %s, ref: %s, data: %s",
         pageId.c_str(), ref.c_str(), parser.toStringUTF8().c_str());
#endif

    long long startTime = getCurrentTime();
    std::vector <std::pair<std::string, std::string>> *styles = Wson2Pairs(data);
    page->ParseJsonTime(getCurrentTime() - startTime);

    page->updateDirty(true);
    return page->UpdateStyle(ref, styles);
  }

  bool RenderManager::AddEvent(const std::string &page_id, const std::string &ref,
                               const std::string &event) {
    RenderPage *page = this->GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] AddEvent >>>> pageId: %s, ref: %s, event: %s",
         pageId.c_str(), ref.c_str(), event.c_str());
#endif

    page->updateDirty(true);
    return page->AddEvent(ref, event);
  }

  bool RenderManager::RemoveEvent(const std::string &page_id, const std::string &ref,
                                  const std::string &event) {
    RenderPage *page = this->GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] RemoveEvent >>>> pageId: %s, ref: %s, event: %s",
         pageId.c_str(), ref.c_str(), event.c_str());
#endif

    page->updateDirty(true);
    return page->RemoveEvent(ref, event);
  }

  bool RenderManager::CreateFinish(const std::string &page_id) {
    RenderPage *page = GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] CreateFinish >>>> pageId: %s", pageId.c_str());
#endif

    page->updateDirty(true);
    return page->CreateFinish();
  }

  RenderPage *RenderManager::GetPage(const std::string &page_id) {
    std::map<std::string, RenderPage *>::iterator iter = this->pages.find(page_id);
    if (iter != this->pages.end()) {
      return iter->second;
    } else {
      return nullptr;
    }
  }

  bool RenderManager::ClosePage(const std::string &page_id) {
    RenderPage *page = GetPage(page_id);
    if (page == nullptr)
      return false;

#if RENDER_LOG
    LOGD("[RenderManager] ClosePage >>>> pageId: %s", pageId.c_str());
#endif
    page->OnRenderPageClose();
    this->pages.erase(page_id);
    delete page;
    page = nullptr;
  }

  void RenderManager::Batch(const std::string &page_id) {
    RenderPage *page = this->GetPage(page_id);
    if (page == nullptr)
      return;

    page->Batch();
  }

} //namespace WeexCore
