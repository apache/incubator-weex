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

namespace WeexCore {

class RenderPage;

class RenderManager {
 private:
  RenderManager() {
    this->viewport_width_ = kDefaultViewPortWidth;
  }

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
  bool CreatePage(std::string page_id, const char *data);

  /** use auto constructor is bad idea, it cann't transfer binary, use char* is
   * better */
  bool AddRenderObject(const std::string &page_id,
                       const std::string &parent_ref, int index,
                       const char *data);

  bool RemoveRenderObject(const std::string &page_id, const std::string &ref);

  bool MoveRenderObject(const std::string &page_id, const std::string &ref,
                        const std::string &parent_ref, int index);

  bool UpdateAttr(const std::string &page_id, const std::string &ref,
                  const char *data);

  bool UpdateStyle(const std::string &page_id, const std::string &ref,
                   const char *data);

  bool AddEvent(const std::string &page_id, const std::string &ref,
                const std::string &event);

  bool RemoveEvent(const std::string &page_id, const std::string &ref,
                   const std::string &event);

  bool CreateFinish(const std::string &page_id);

  bool CallNativeModule(const char *pageId, const char *module, const char *method,
                        const char *arguments, int argumentsLength, const char *options,
                        int optionsLength);

  bool CallMetaModule(const char *method, const char *arguments);

  RenderPage *GetPage(const std::string &page_id);

  bool ClosePage(const std::string &page_id);

  static RenderManager *GetInstance() {
    if (!g_pInstance) {
      g_pInstance = new RenderManager();
    }
    return g_pInstance;
  }

  inline float viewport_width() const { return this->viewport_width_; }

  inline void set_viewport_width(float viewport_width) {
    this->viewport_width_ = viewport_width;
  }

 private:
  static RenderManager *g_pInstance;
  std::map<std::string, RenderPage *> pages_;
  float viewport_width_ = -1;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_MANAGER_RENDER_MANAGER_H_
