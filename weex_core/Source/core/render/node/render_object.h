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
#ifndef CORE_RENDER_NODE_RENDER_OBJECT_H_
#define CORE_RENDER_NODE_RENDER_OBJECT_H_

#include <functional>
#include <map>
#include <set>
#include <string>

#include "core/render/node/factory/render_object_interface.h"

#define JSON_OBJECT_MARK_CHAR '{'
#define JSON_ARRAY_MARK_CHAR '['

#define convert_render_object_to_long(render) ((long)((intptr_t)render))

#define convert_long_to_render_object(ptr) ((RenderObject *)((intptr_t)ptr))

namespace WeexCore {

class RenderObject;

class RenderPage;

typedef enum StyleType {
  kTypeStyle,
  kTypeLayout,
  kTypeMargin,
  kTypePadding,
  kTypeBorder,
  kTypeInheritableLayout
} StyleType;

class RenderObject : public IRenderObject {
  friend class RenderPage;

 public:
  void LayoutBeforeImpl();

  void LayoutPlatformImpl();
    
  void LayoutAfterImpl();

  void CopyFrom(RenderObject *src);

  RenderObject* RichtextParent();

  bool hasShadow(const RenderObject* shadow) const;

  void MapInsertOrAssign(std::map<std::string, std::string> *targetMap,
                         const std::string &key, const std::string &value);

  bool ViewInit();

  virtual std::map<std::string, std::string> *GetDefaultStyle() {
    return nullptr;
  }

  virtual std::map<std::string, std::string> *GetDefaultAttr() {
    return nullptr;
  }

 protected:
  bool UpdateStyleInternal(const std::string key, const std::string value,
                           float fallback, std::function<void(float)> functor);

 public:
  RenderObject();

  virtual ~RenderObject();

  void BindMeasureFunc();

  void OnLayoutBefore();
    
  void OnLayoutPlatform();

  void OnLayoutAfter(float width, float height);

  virtual StyleType ApplyStyle(const std::string &key, const std::string &value,
                               const bool updating);

  void ApplyDefaultStyle(bool reserve);

  void ApplyDefaultAttr();

  Index IndexOf(const RenderObject *render);

  virtual int AddRenderObject(int index, RenderObject *child);

  float GetViewPortWidth();

  const std::string GetAttr(const std::string &key);

  const std::string GetStyle(const std::string &key);

  RenderPage *GetRenderPage();

  virtual void UpdateAttr(std::string key, std::string value);

  virtual StyleType UpdateStyle(std::string key, std::string value);
  
  void MergeStyles(std::vector<std::pair<std::string, std::string>> *src);

  bool IsAppendTree();

  RenderObject *GetChild(const Index &index);

  void RemoveRenderObject(RenderObject *child);

  virtual void AddAttr(std::string key, std::string value);

  StyleType AddStyle(std::string key, std::string value, bool reserve);

  void AddEvent(std::string event);

  void RemoveEvent(const std::string &event);

 public:
  inline void set_parent_render(RenderObject *render) {
    this->parent_render_ = render;
  }

  inline RenderObject *parent_render() { return this->parent_render_; }

  const std::vector<RenderObject*>& get_shadow_objects() const {return shadow_objects_;}

  inline std::map<std::string, std::string> *styles() const {
    return this->styles_;
  }

  inline std::map<std::string, std::string> *attributes() const {
    return this->attributes_;
  }

  inline std::set<std::string> *events() const { return this->events_; }

  inline void set_is_root_render() { this->is_root_render_ = true; }

  inline bool is_root_render() { return this->is_root_render_; }

  inline bool is_sticky() { return this->is_sticky_; }

  bool is_richtext_child() const {return is_richtext_child_;}

  void set_is_richtext_child(const bool is_richtext_child) {is_richtext_child_ = is_richtext_child;}

 private:
  RenderObject *parent_render_;
  std::vector<RenderObject*> shadow_objects_;
  std::map<std::string, std::string> *styles_;
  std::map<std::string, std::string> *attributes_;
  std::set<std::string> *events_;
  bool is_root_render_;
  bool is_sticky_ = false;
  bool is_richtext_child_ = false;
};
}  // namespace WeexCore
#endif  // CORE_RENDER_NODE_RENDER_OBJECT_H_
