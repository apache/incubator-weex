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

  constexpr float kLayoutFirstScreenOverflowRadio = 1.2f;

  class render_action;

  class RenderObject;

  class RenderPerformance;

  class RenderPage {

  private:

    void TraverseTree(RenderObject *render, int index);

    void PushRenderToRegisterMap(RenderObject *render);

    void RemoveRenderFromRegisterMap(RenderObject *render);

    void SendCreateBodyAction(RenderObject *render);

    void SendAddElementAction(RenderObject *child, RenderObject *parent, int index, bool is_recursion, bool willLayout= true);

    void SendRemoveElementAction(const std::string &ref);

    void SendMoveElementAction(const std::string &ref, const std::string &parentRef, int index);

    void SendLayoutAction(RenderObject *render, int index);

    void
    SendUpdateStyleAction(RenderObject *render,
                          std::vector<std::pair<std::string, std::string>> *style,
                          std::vector<std::pair<std::string, std::string>> *margin,
                          std::vector<std::pair<std::string, std::string>> *padding,
                          std::vector<std::pair<std::string, std::string>> *border);

    void SendUpdateAttrAction(RenderObject *render, std::vector<std::pair<std::string, std::string>> *attrs);

    void SendCreateFinishAction();

    void SendAppendTreeCreateFinish(const std::string &ref);

    void PostRenderAction(render_action *action);

  public:
    static constexpr bool useVSync = true;
    std::atomic_bool needLayout{false};
    std::atomic_bool hasForeLayoutAction{false};
    RenderPage(std::string pageId);

    ~RenderPage();

    void CalculateLayout();

    bool CreateRootRender(RenderObject *root);

    bool AddRenderObject(const std::string &parentRef, int insertPosiotn, RenderObject *child);

    bool RemoveRenderObject(const std::string &ref);

    bool MoveRenderObject(const std::string &ref, const std::string &parentRef, int index);

    bool UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string>> *styles);

    bool UpdateAttr(const std::string &ref, std::vector<std::pair<std::string, std::string>> *attrs);

    void SetDefaultHeightAndWidthIntoRootRender(const float defaultWidth, const float defaultHeight, const bool isWidthWrapContent, const bool isHeightWrapContent);

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

    inline RenderObject *GetRenderObject(const std::string &ref) {
        std::map<std::string, RenderObject *>::iterator iter = mRenderObjectRegisterMap.find(ref);
        if (iter != mRenderObjectRegisterMap.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

    void SetRootRenderObject(RenderObject *root);

    inline std::string PageId() {
      return mPageId;
    }

    inline float ViewPortWidth() const {
      return mViewPortWidth;
    }

    inline void SetViewPortWidth(float viewPortWidth) {
      this->mViewPortWidth = viewPortWidth;
    }

    inline bool isDirty(){
      return dirty.load();
    }

    inline void updateDirty(bool dirty){
      this->dirty.store(dirty);
    }

    inline void SetRenderContainerWidthWrapContent(bool wrap) {
      this->isRenderContainerWidthWrapContent.store(wrap);
    }

    inline bool GetRenderContainerWidthWrapContent() {
      return isRenderContainerWidthWrapContent.load();
    }

    // ****** Life Cycle ****** //

    void OnRenderPageInit();

    void OnRenderProcessStart();

    void OnRenderProcessExited();

    void OnRenderProcessGone();

    void OnRenderPageClose();

  private:
    bool mAlreadyCreateFinish = false;
    float mViewPortWidth;
    RenderObject *render_root = nullptr;
    std::string mPageId;
    std::pair<float,float> renderPageSize;
    std::map<std::string, RenderObject *> mRenderObjectRegisterMap;
    RenderPerformance *mWXCorePerformance;
    std::atomic_bool dirty{true};
    std::atomic_bool isRenderContainerWidthWrapContent{false};
    std::atomic_bool isRenderContainerHeightWrapContent{false};
  };
}

#endif //RenderManager_h