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
#ifndef RenderPage_H
#define RenderPage_H

#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <atomic>

namespace WeexCore {

  class RenderAction;

  class RenderObject;

  class RenderPerformance;

  class RenderPage {

  private:

    void TraverseTree(RenderObject *render, int index);

    void PushRenderToRegisterMap(RenderObject *render);

    void RemoveRenderFromRegisterMap(RenderObject *render);

    void SendCreateBodyAction(RenderObject *render);

    void
    SendAddElementAction(RenderObject *child, RenderObject *parent, int index, bool is_recursion,
                         bool will_layout = true);

    void SendRemoveElementAction(const std::string &ref);

    void SendMoveElementAction(const std::string &ref, const std::string &parent_ref, int index);

    void SendLayoutAction(RenderObject *render, int index);

    void SendUpdateStyleAction(RenderObject *render,
                               std::vector<std::pair<std::string, std::string>> *style,
                               std::vector<std::pair<std::string, std::string>> *margin,
                               std::vector<std::pair<std::string, std::string>> *padding,
                               std::vector<std::pair<std::string, std::string>> *border);

    void SendUpdateAttrAction(RenderObject *render,
                              std::vector<std::pair<std::string, std::string>> *attrs);

    void SendCreateFinishAction();

    void SendAppendTreeCreateFinish(const std::string &ref);

    void PostRenderAction(RenderAction *action);

  public:

    explicit RenderPage(std::string page_id);

    ~RenderPage();

    void CalculateLayout();

    bool CreateRootRender(RenderObject *root);

    bool AddRenderObject(const std::string &parent_ref, int insert_posiotn, RenderObject *child);

    bool RemoveRenderObject(const std::string &ref);

    bool MoveRenderObject(const std::string &ref, const std::string &parent_ref, int index);

    bool
    UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string>> *styles);

    bool
    UpdateAttr(const std::string &ref, std::vector<std::pair<std::string, std::string>> *attrs);

    void
    SetDefaultHeightAndWidthIntoRootRender(const float default_width, const float default_height,
                                           const bool is_width_wrap_content,
                                           const bool is_height_wrap_content);

    bool AddEvent(const std::string &ref, const std::string &event);

    bool RemoveEvent(const std::string &ref, const std::string &event);

    bool CreateFinish();

    void Batch();

    void CssLayoutTime(const long long &time);

    void ParseJsonTime(const long long &time);

    void CallBridgeTime(const long long &time);

    std::vector<long> PrintFirstScreenLog();

    std::vector<long> PrintRenderSuccessLog();

    void LayoutImmediately();

    void SendUpdateAttrAction(RenderObject *render, std::map<std::string, std::string> *attrs);

    RenderObject *GetRenderObject(const std::string &ref);

    void SetRootRenderObject(RenderObject *root);

    // ****** Life Cycle ****** //

    void OnRenderPageInit();

    void OnRenderProcessStart();

    void OnRenderProcessExited();

    void OnRenderProcessGone();

    void OnRenderPageClose();


    inline std::string PageId() {
      return this->page_id;
    }

    inline float ViewPortWidth() const {
      return this->viewport_width;
    }

    inline void SetViewPortWidth(float viewport_width) {
      this->viewport_width = viewport_width;
    }

    inline bool isDirty() {
      return this->dirty.load();
    }

    inline void updateDirty(bool dirty) {
      this->dirty.store(dirty);
    }

    inline void SetRenderContainerWidthWrapContent(bool wrap) {
      this->is_render_container_width_wrap_content.store(wrap);
    }

    inline bool GetRenderContainerWidthWrapContent() {
      return this->is_render_container_width_wrap_content.load();
    }

  public:

    static constexpr bool kUseVSync = true;
    std::atomic_bool need_layout{false};
    std::atomic_bool has_fore_layout_action{false};

  private:

    float viewport_width;
    RenderObject *render_root = nullptr;
    std::string page_id;
    std::pair<float, float> render_page_size;
    std::map<std::string, RenderObject *> render_object_registers;
    RenderPerformance *render_performance;
    std::atomic_bool dirty{true};
    std::atomic_bool is_render_container_width_wrap_content{false};
    std::atomic_bool is_render_container_height_wrap_content{false};
  };
}

#endif //RenderManager_h