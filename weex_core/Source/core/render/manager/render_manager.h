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
#ifndef RenderManager_h
#define RenderManager_h

#include <map>
#include <string>
#include <stdbool.h>
#include <core/layout/measure_func_adapter.h>

namespace WeexCore {

  class RenderPage;

  class RenderManager {

  private:
    RenderManager() {}

    ~RenderManager() {}

    //just to release singleton object
    class Garbo {
    public:
      ~Garbo() {
        if (RenderManager::m_pInstance) {
          delete RenderManager::m_pInstance;
        }
      }
    };

    static Garbo garbo;

  public:

    void Batch(const std::string &pageId);

    // create root node
    bool CreatePage(std::string pageId, const char* data);

    /** use auto constructor is bad idea, it cann't transfer binary, use char* is better */
    bool AddRenderObject(const std::string &pageId, const std::string &parentRef, int index,
                         const char* data);

    bool RemoveRenderObject(const std::string &pageId, const std::string &ref);

    bool MoveRenderObject(const std::string &pageId, const std::string &ref,
                     const std::string &parentRef, int index);

    bool UpdateAttr(const std::string &pageId, const std::string &ref, const char* data);

    bool UpdateStyle(const std::string &pageId, const std::string &ref, const char* data);

    bool AddEvent(const std::string &pageId, const std::string &ref, const std::string &event);

    bool RemoveEvent(const std::string &pageId, const std::string &ref, const std::string &event);

    bool CreateFinish(const std::string &pageId);

    RenderPage *GetPage(const std::string &id);

    bool ClosePage(const std::string &pageId);

    static RenderManager *GetInstance() {
      if (!m_pInstance) {
        m_pInstance = new RenderManager();
      }
      return m_pInstance;
    }

  private:
    static RenderManager *m_pInstance;
    std::map<std::string, RenderPage *> mPages;
  };
}

#endif //RenderManager_h