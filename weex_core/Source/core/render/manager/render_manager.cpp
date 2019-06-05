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

#include <chrono>
#include <utility>
#include <vector>

#include "base/log_defines.h"
#include "base/time_utils.h"
#include "core/common/view_utils.h"
#include "core/css/constants_name.h"
#include "core/layout/measure_func_adapter.h"
#include "core/parser/dom_wson.h"
#include "core/render/node/render_object.h"
#include "core/render/page/render_page.h"
#include "wson/wson_parser.h"

namespace WeexCore {

RenderManager *RenderManager::g_pInstance = nullptr;



bool RenderManager::CreatePage(const std::string& page_id, const char *data) {
    
#if RENDER_LOG
  wson_parser parser(data);
  LOGD("[RenderManager] CreatePage >>>> pageId: %s, dom data: %s",
       pageId.c_str(), parser.toStringUTF8().c_str());
#endif

  RenderPage *page = new RenderPage(page_id);
  pages_.insert(std::pair<std::string, RenderPage *>(page_id, page));

    initDeviceConfig(page, page_id);

  int64_t start_time = getCurrentTime();
  RenderObject *root = Wson2RenderObject(data, page_id, page->reserve_css_styles());
  page->ParseJsonTime(getCurrentTime() - start_time);

  page->set_is_dirty(true);
  return page->CreateRootRender(root);
}

bool RenderManager::CreatePage(const std::string& page_id, RenderObject *root) {
#if RENDER_LOG
  wson_parser parser(data);
  LOGD("[RenderManager] CreatePage >>>> pageId: %s, dom data: %s",
       pageId.c_str(), parser.toStringUTF8().c_str());
#endif

  RenderPage *page = new RenderPage(page_id);
  this->pages_.insert(std::pair<std::string, RenderPage *>(page_id, page));

    initDeviceConfig(page, page_id);

  page->set_is_dirty(true);
  return page->CreateRootRender(root);
}
    
bool RenderManager::CreatePage(const std::string& page_id, std::function<RenderObject* (RenderPage*)> constructRoot) {
#if RENDER_LOG
    LOGD("[RenderManager] CreatePage >>>> pageId: %s", pageId.c_str());
#endif
    
    RenderPage *page = new RenderPage(page_id);
    this->pages_.insert(std::pair<std::string, RenderPage *>(page_id, page));

    initDeviceConfig(page, page_id);
    
    int64_t start_time = getCurrentTime();
    RenderObject *root = constructRoot(page);
    page->ParseJsonTime(getCurrentTime() - start_time);
    
    page->set_is_dirty(true);
    return page->CreateRootRender(root);
}

bool RenderManager::AddRenderObject(const std::string &page_id,
                                    const std::string &parent_ref, int index,
                                    const char *data) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  wson_parser parser(data);
  LOGD(
      "[RenderManager] AddRenderObject >>>> pageId: %s, parentRef: %s, index: "
      "%d, dom data: %s",
      pageId.c_str(), parentRef.c_str(), index, parser.toStringUTF8().c_str());
#endif

  int64_t start_time = getCurrentTime();
  RenderObject *child = Wson2RenderObject(data, page_id, page->reserve_css_styles());
  page->ParseJsonTime(getCurrentTime() - start_time);

  if (child == nullptr) return false;

  page->set_is_dirty(true);
  return page->AddRenderObject(parent_ref, index, child);
}

bool RenderManager::AddRenderObject(const std::string &page_id, const std::string &parent_ref,
                                    int index,  RenderObject *root) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  wson_parser parser(data);
  LOGD(
      "[RenderManager] AddRenderObject >>>> pageId: %s, parentRef: %s, index: "
      "%d, dom data: %s",
      pageId.c_str(), parentRef.c_str(), index, parser.toStringUTF8().c_str());
#endif

  if (root == nullptr) return false;

  page->set_is_dirty(true);
  return page->AddRenderObject(parent_ref, index, root);
}
    
bool RenderManager::AddRenderObject(const std::string &page_id,
                                    const std::string &parent_ref, int index,
                                    std::function<RenderObject* (RenderPage*)> constructRoot) {
    RenderPage *page = GetPage(page_id);
    if (page == nullptr) return false;
    
#if RENDER_LOG
    wson_parser parser(data);
    LOGD(
         "[RenderManager] AddRenderObject >>>> pageId: %s, parentRef: %s, index: "
         "%d, dom data: %s",
         pageId.c_str(), parentRef.c_str(), index, parser.toStringUTF8().c_str());
#endif
    
    RenderObject *root = constructRoot(page);
    if (root == nullptr) return false;
    
    page->set_is_dirty(true);
    return page->AddRenderObject(parent_ref, index, root);
}

bool RenderManager::RemoveRenderObject(const std::string &page_id,
                                       const std::string &ref) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  LOGD("[RenderManager] RemoveRenderObject >>>> pageId: %s, ref: %s",
       pageId.c_str(), ref.c_str());
#endif

  page->set_is_dirty(true);
  return page->RemoveRenderObject(ref);
}

bool RenderManager::MoveRenderObject(const std::string &page_id,
                                     const std::string &ref,
                                     const std::string &parent_ref, int index) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  LOGD(
      "[RenderManager] MoveRenderObject >>>> pageId: %s, ref: %s, parentRef: "
      "%s, index: %d",
      pageId.c_str(), ref.c_str(), parentRef.c_str(), index);
#endif

  page->set_is_dirty(true);
  return page->MoveRenderObject(ref, parent_ref, index);
}

bool RenderManager::UpdateAttr(const std::string &page_id,
                               const std::string &ref, const char *data) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  wson_parser parser(data);
  LOGD("[RenderManager] UpdateAttr >>>> pageId: %s, ref: %s, data: %s",
       pageId.c_str(), ref.c_str(), parser.toStringUTF8().c_str());
#endif

  int64_t start_time = getCurrentTime();
  std::vector<std::pair<std::string, std::string>> *attrs = Wson2Pairs(data);
  page->ParseJsonTime(getCurrentTime() - start_time);

  page->set_is_dirty(true);
  return page->UpdateAttr(ref, attrs);
}

bool RenderManager::UpdateAttr(const std::string &page_id, const std::string &ref,
                               std::vector<std::pair<std::string, std::string>> *attrPair) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  wson_parser parser(data);
  LOGD("[RenderManager] UpdateAttr >>>> pageId: %s, ref: %s, data: %s",
       pageId.c_str(), ref.c_str(), parser.toStringUTF8().c_str());
#endif

  page->set_is_dirty(true);
  return page->UpdateAttr(ref, attrPair);
}

bool RenderManager::UpdateStyle(const std::string &page_id,
                                const std::string &ref, const char *data) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  wson_parser parser(data);
  LOGD("[RenderManager] UpdateStyle >>>> pageId: %s, ref: %s, data: %s",
       pageId.c_str(), ref.c_str(), parser.toStringUTF8().c_str());
#endif

  int64_t start_time = getCurrentTime();
  std::vector<std::pair<std::string, std::string>> *styles = Wson2Pairs(data);
  page->ParseJsonTime(getCurrentTime() - start_time);

  page->set_is_dirty(true);
  return page->UpdateStyle(ref, styles);
}


bool RenderManager::UpdateStyle(const std::string &page_id, const std::string &ref,
                                std::vector<std::pair<std::string, std::string>> *stylePair) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  wson_parser parser(data);
  LOGD("[RenderManager] UpdateStyle >>>> pageId: %s, ref: %s, data: %s",
       pageId.c_str(), ref.c_str(), parser.toStringUTF8().c_str());
#endif


  page->set_is_dirty(true);
  return page->UpdateStyle(ref, stylePair);
}


bool RenderManager::AddEvent(const std::string &page_id, const std::string &ref,
                             const std::string &event) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  LOGD("[RenderManager] AddEvent >>>> pageId: %s, ref: %s, event: %s",
       pageId.c_str(), ref.c_str(), event.c_str());
#endif

  page->set_is_dirty(true);
  return page->AddEvent(ref, event);
}

bool RenderManager::RemoveEvent(const std::string &page_id,
                                const std::string &ref,
                                const std::string &event) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  LOGD("[RenderManager] RemoveEvent >>>> pageId: %s, ref: %s, event: %s",
       pageId.c_str(), ref.c_str(), event.c_str());
#endif

  page->set_is_dirty(true);
  return page->RemoveEvent(ref, event);
}

bool RenderManager::CreateFinish(const std::string &page_id) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  LOGD("[RenderManager] CreateFinish >>>> pageId: %s", pageId.c_str());
#endif

  page->set_is_dirty(true);
  bool b = page->CreateFinish();

#if RENDER_LOG
  auto end_time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
  LOGE("DATA_RENDER, Wx End %lld",end_time);
#endif
  return b;
}

void RenderManager::CallNativeModule(const char *page_id, const char *module, const char *method,
                                     const char *arguments, int arguments_length,
                                     const char *options, int options_length) {
  if (strcmp(module, "meta") == 0) {
    CallMetaModule(page_id, method, arguments);
  }
}
    
void RenderManager::CallMetaModule(const char *page_id, const char *method, const char *arguments) {
  if (strcmp(method, "setViewport") == 0) {
    wson_parser parser(arguments);
    if (parser.isArray(parser.nextType())) {
      int size = parser.nextArraySize();
      for (int i = 0; i < size; i++) {
        uint8_t value_type = parser.nextType();
        if (parser.isMap(value_type)) {
          int map_size = parser.nextMapSize();
          for (int j = 0; j < map_size; j++) {
            std::string key = parser.nextMapKeyUTF8();
            std::string value = parser.nextStringUTF8(parser.nextType());
            if (key == WIDTH) {
              setPageArgument(page_id, "viewportwidth", value);
            }
            else if (key == ROUND_OFF_DEVIATION) {
              setPageArgument(page_id, "roundoffdeviation", value);
            }
            else if (key == "deviceWidth") {
              setPageArgument(page_id, "devicewidth", value);
            }
            else if (key == "deviceHeight") {
              // unsupported now
            }
            else if (key == "reserveCssStyles") {
              setPageArgument(page_id, "reserveCssStyles", value);
            }
          }
        }
      }
    }
  }
}

RenderPage *RenderManager::GetPage(const std::string &page_id) {
  std::map<std::string, RenderPage *>::iterator iter =
      this->pages_.find(page_id);
  if (iter != this->pages_.end()) {
    return iter->second;
  } else {
    return nullptr;
  }
}

bool RenderManager::ClosePage(const std::string &page_id) {
  removePageArguments(page_id);
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) return false;

#if RENDER_LOG
  LOGD("[RenderManager] ClosePage >>>> pageId: %s", pageId.c_str());
#endif
  page->OnRenderPageClose();
  this->pages_.erase(page_id);
  delete page;
  page = nullptr;
    return true;
}
  
bool RenderManager::ReloadPageLayout(const std::string& page_id)
{
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) return false;
  return page->ReapplyStyles();
}

float RenderManager::viewport_width(const std::string &page_id) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) return kDefaultViewPortWidth;

  return page->viewport_width();
}

void RenderManager::set_viewport_width(const std::string &page_id, float viewport_width) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) {
      // page is not created yet, we should store the view port value
      setPageArgument(page_id, "viewportwidth", std::to_string(viewport_width));
      return;
  }

  page->set_viewport_width(viewport_width);
}

float RenderManager::DeviceWidth(const std::string &page_id) {
  RenderPage *page = GetPage(page_id);
  if(page == nullptr){
    return WXCoreEnvironment::getInstance()->DeviceWidth();
  }
  return page->device_width();
}

void RenderManager::setDeviceWidth(const std::string &page_id, float device_width) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) {
    // page is not created yet, we should store the device width value
    setPageArgument(page_id, "devicewidth", std::to_string(device_width));
    return;
  }
  page->set_device_width(device_width);
}

bool RenderManager::round_off_deviation(const std::string &page_id) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) return kDefaultRoundOffDeviation;

  return page->round_off_deviation();
}

void RenderManager::set_round_off_deviation(const std::string &page_id, bool round_off_deviation) {
  RenderPage *page = GetPage(page_id);
  if (page == nullptr) {
    setPageArgument(page_id, "roundoffdeviation", round_off_deviation ? "true" : "false");
    return;
  }

  page->set_round_off_deviation(round_off_deviation);
}
    
void RenderManager::setPageArgument(const std::string& pageId, const std::string& key, const std::string& value) {
    if (pageId.empty() || key.empty()) {
        return;
    }
    std::lock_guard<std::mutex> guard(page_args_mutex_);
    page_args_[pageId][key] = value;
}

std::string RenderManager::getPageArgument(const std::string& pageId, const std::string& key) {
    std::lock_guard<std::mutex> guard(page_args_mutex_);
    auto findPage = page_args_.find(pageId);
    if (findPage != page_args_.end()) {
        auto findKey = findPage->second.find(key);
        if (findKey != findPage->second.end()) {
            return findKey->second;
        }
    }
    return "";
}

std::map<std::string, std::string> RenderManager::removePageArguments(const std::string& pageId) {
    std::lock_guard<std::mutex> guard(page_args_mutex_);
    std::map<std::string, std::string> result;
    auto findPage = page_args_.find(pageId);
    if (findPage != page_args_.end()) {
        std::swap(result, findPage->second);
        page_args_.erase(findPage);
    }
    return result;
}

void RenderManager::Batch(const std::string &page_id) {
  RenderPage *page = this->GetPage(page_id);
  if (page == nullptr) return;

  page->Batch();
}

void RenderManager::initDeviceConfig(RenderPage *page, const std::string &page_id) {
  if (page == nullptr) return;
  
  auto viewPortWidth = getPageArgument(page_id, "viewportwidth");
  if (!viewPortWidth.empty()) {
    page->set_viewport_width(getFloat(viewPortWidth.c_str()));
  }
  
  auto deviceWidth = getPageArgument(page_id, "devicewidth");
  if (!deviceWidth.empty()) {
    page->set_device_width(getFloat(deviceWidth.c_str()));
  }
  
  auto roundOff = getPageArgument(page_id, "roundoffdeviation");
  if (!roundOff.empty()) {
    page->set_round_off_deviation(getBool(roundOff));
  }
  
  auto reserveCssStyles = getPageArgument(page_id, "reserveCssStyles");
  if (!reserveCssStyles.empty()) {
    page->set_reserve_css_styles(getBool(reserveCssStyles));
  }
}

}  // namespace WeexCore
