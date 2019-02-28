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
#ifndef CORE_RENDER_PAGE_RENDER_PAGE_H_
#define CORE_RENDER_PAGE_RENDER_PAGE_H_

#include <atomic>
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "render_page_base.h"
#include "core/css/constants_value.h"

namespace WeexCore {

class RenderAction;
class RenderObject;

class RenderPage: public RenderPageBase {
 private:
  void TraverseTree(RenderObject *render, long index);

  void SendCreateBodyAction(RenderObject *render);

  void SendAddElementAction(RenderObject *child, RenderObject *parent,
                            int index, bool is_recursion,
                            bool will_layout = true);

  void SendRemoveElementAction(const std::string &ref);

  void SendMoveElementAction(const std::string &ref,
                             const std::string &parent_ref, int index);

  void SendLayoutAction(RenderObject *render, int index);

  void SendUpdateStyleAction(
      RenderObject *render,
      std::vector<std::pair<std::string, std::string>> *style,
      std::vector<std::pair<std::string, std::string>> *margin,
      std::vector<std::pair<std::string, std::string>> *padding,
      std::vector<std::pair<std::string, std::string>> *border);

  void SendUpdateAttrAction(
      RenderObject *render,
      std::vector<std::pair<std::string, std::string>> *attrs);

  void SendCallNativeModuleAction(const std::string &module,
                                  const std::string &method, const std::string &args, int argc = 0);
    
  void SendCreateFinishAction();

  void SendRenderSuccessAction();

  void SendAppendTreeCreateFinish(const std::string &ref);

  void PostRenderAction(RenderAction *action);
  
  void LayoutInner();

public:
  explicit RenderPage(const std::string& page_id);

  ~RenderPage();

  void CalculateLayout();

  bool CreateRootRender(RenderObject *root);

  bool AddRenderObject(const std::string &parent_ref, int insert_posiotn,
                       RenderObject *child);

  virtual bool RemoveRenderObject(const std::string &ref) override;

  virtual bool MoveRenderObject(const std::string &ref, const std::string &parent_ref, int index) override;

  virtual bool UpdateStyle(const std::string &ref,
                   std::vector<std::pair<std::string, std::string>> *styles) override;

  virtual bool UpdateAttr(const std::string &ref,
                  std::vector<std::pair<std::string, std::string>> *attrs) override;

  virtual void SetDefaultHeightAndWidthIntoRootRender(
      const float default_width, const float default_height,
      const bool is_width_wrap_content, const bool is_height_wrap_content) override;

  virtual bool AddEvent(const std::string &ref, const std::string &event) override;

  virtual bool RemoveEvent(const std::string &ref, const std::string &event) override;

  virtual bool CreateFinish() override;

  void Batch();

  void LayoutImmediately();

  void SendUpdateAttrAction(RenderObject *render,
                            std::map<std::string, std::string> *attrs);

  virtual RenderObject *GetRenderObject(const std::string &ref) override;
    
  void CallNativeModule(const std::string &module,
                          const std::string &method, const std::string &args, int argc = 0);

  void SetRootRenderObject(RenderObject *root);
    
  // ****** Render object managing ****** //
  
  void PushRenderToRegisterMap(RenderObject *render);
    
  void RemoveRenderFromRegisterMap(RenderObject *render);

  // ****** Life Cycle ****** //

  void OnRenderPageInit();

  void OnRenderProcessStart();

  void OnRenderProcessExited();

  void OnRenderProcessGone();

  virtual void OnRenderPageClose() override;

 public:

  inline bool is_dirty() { return this->is_dirty_.load(); }

  inline void set_is_dirty(bool dirty) { this->is_dirty_.store(dirty); }

  inline void set_is_render_container_width_wrap_content(bool wrap) {
    this->is_render_container_width_wrap_content_.store(wrap);
  }

  inline bool is_render_container_width_wrap_content() {
    return this->is_render_container_width_wrap_content_.load();
  }

  virtual float GetViewportWidth() override { return viewport_width_; }
  virtual void SetViewportWidth(float value) override { viewport_width_ = value; };
  virtual bool GetRoundOffDeviation() override { return round_off_deviation_; }
  virtual void SetRoundOffDeviation(bool value) override { round_off_deviation_ = value; }

  inline void set_before_layout_needed(bool v) { is_before_layout_needed_.store(v); }

  inline void set_platform_layout_needed(bool v) { is_platform_layout_needed_.store(v); }

  inline void set_after_layout_needed(bool v) { is_after_layout_needed_.store(v); }

 public:
  static constexpr bool kUseVSync = true;
  std::atomic_bool need_layout_{false};
  std::atomic_bool has_fore_layout_action_{false};

 private:
  RenderObject *render_root_ = nullptr;
  std::pair<float, float> render_page_size_;
  std::map<std::string, RenderObject *> render_object_registers_;
  std::atomic_bool is_dirty_{true};
  std::atomic_bool is_render_container_width_wrap_content_{false};
  std::atomic_bool is_render_container_height_wrap_content_{false};
  std::atomic_bool is_before_layout_needed_{true};
  std::atomic_bool is_platform_layout_needed_{false};
  std::atomic_bool is_after_layout_needed_{true};
  float viewport_width_ = -1;
  bool round_off_deviation_ = kDefaultRoundOffDeviation;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_PAGE_RENDER_PAGE_H_
