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
#ifndef CORE_RENDER_MANAGER_RENDER_MANAGER_H_
#define CORE_RENDER_MANAGER_RENDER_MANAGER_H_

#include <map>
#include <string>

#include "core/css/constants_value.h"
#include "core/render/node/render_object.h"
#include "include/WeexApiHeader.h"

namespace WeexCore {

class RenderPageBase;
class RenderPageCustom;
class RenderPage;
class RenderObject;

class RenderManager {
 private:
  RenderManager() : pages_() {}

  ~RenderManager() {}

  // just to release singleton object
  class Garbo {
   public:
    ~Garbo() {
      if (RenderManager::g_pInstance) {
        delete RenderManager::g_pInstance;
      }
    }
  };

  static Garbo garbo;

 public:
  void Batch(const std::string &page_id);

  // create root node
  bool CreatePage(const std::string& page_id, const char *data);
    
  // create platform page
  bool CreatePage(const std::string& page_id, RenderObject *root);
    
  bool CreatePage(const std::string& page_id, std::function<RenderObject* (RenderPage*)> constructRoot);
    
  // create custom page with self rendering
  RenderPageCustom* CreateCustomPage(const std::string& page_id, const std::string& page_type);

  /** use auto constructor is bad idea, it cann't transfer binary, use char* is
   * better */
  bool AddRenderObject(const std::string &page_id,
                       const std::string &parent_ref, int index,
                       const char *data);

  bool AddRenderObject(const std::string &page_id,
                       const std::string &parent_ref, int index,
                       RenderObject *root);
    
  bool AddRenderObject(const std::string &page_id,
                       const std::string &parent_ref, int index,
                       std::function<RenderObject* (RenderPage*)> constructRoot);

  bool RemoveRenderObject(const std::string &page_id, const std::string &ref);

  bool MoveRenderObject(const std::string &page_id, const std::string &ref,
                        const std::string &parent_ref, int index);

  bool UpdateAttr(const std::string &page_id, const std::string &ref,
                  const char *data);

  bool UpdateAttr(const std::string &page_id, const std::string &ref,
                  std::vector<std::pair<std::string, std::string>> *attrPair);

  bool UpdateStyle(const std::string &page_id, const std::string &ref,
                   const char *data);

  bool UpdateStyle(const std::string &page_id, const std::string &ref,
                   std::vector<std::pair<std::string, std::string>> *stylePair);

  bool AddEvent(const std::string &page_id, const std::string &ref,
                const std::string &event);

  bool RemoveEvent(const std::string &page_id, const std::string &ref,
                   const std::string &event);

  bool CreateFinish(const std::string &page_id);

  std::unique_ptr<ValueWithType> CallNativeModule(const char *page_id, const char *module, const char *method,
                                                  const char *arguments, int arguments_length, const char *options,
                                                  int options_length);
    
  void CallNativeComponent(const char *page_id, const char *ref,
                           const char *method,
                           const char *arguments,
                           int arguments_length,
                           const char *options,
                           int options_length);

  void CallMetaModule(const char *page_id, const char *method, const char *arguments);

  RenderPageBase *GetPage(const std::string &page_id);

  bool ClosePage(const std::string &page_id);
    
  bool ReloadPageLayout(const std::string& page_id);

  float viewport_width(const std::string &page_id);

  void set_viewport_width(const std::string &page_id, float viewport_width);

  void setDeviceWidth(const std::string &page_id, float device_width);

  float DeviceWidth(const std::string &page_id);

  bool round_off_deviation(const std::string &page_id);

  void set_round_off_deviation(const std::string &page_id, bool round_off_deviation);
    
  void setPageArgument(const std::string& pageId, const std::string& key, const std::string& value);
  std::string getPageArgument(const std::string& pageId, const std::string& key);
  std::map<std::string, std::string> removePageArguments(const std::string& pageId); // remove and return the page arguments

  static RenderManager *GetInstance() {
    if (NULL == g_pInstance) {
      g_pInstance = new RenderManager();
    }
    return g_pInstance;
  }

 private:
    void initDeviceConfig(RenderPage *page, const std::string &page_id);
 private:
  static RenderManager *g_pInstance;

  std::map<std::string, RenderPageBase *> pages_;
  std::mutex page_args_mutex_;
  std::map<std::string, std::map<std::string, std::string>> page_args_;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_MANAGER_RENDER_MANAGER_H_
