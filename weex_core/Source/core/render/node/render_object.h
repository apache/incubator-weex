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
#ifndef RenderObject_h
#define RenderObject_h

#include <string>
#include <map>
#include <set>
#include <functional>

#include "core/render/node/factory/render_object_interface.h"

#define JSON_OBJECT_MARK_CHAR  '{'
#define JSON_ARRAY_MARK_CHAR  '['

#define convert_render_object_to_long(render)    ((jlong)((intptr_t)render))

#define convert_long_to_render_object(ptr)   ((RenderObject *)((intptr_t)ptr))

namespace WeexCore {

  class RenderObject;

  class RenderPage;

  typedef enum StyleType {
    kTypeStyle, kTypeLayout, kTypeMargin, kTypePadding, kTypeBorder
  } StyleType;

  class RenderObject : public IRenderObject {

    friend class RenderPage;

  public:

    void LayoutBefore();

    void LayoutAfter();

    void copyFrom(RenderObject* src);

    void mapInsertOrAssign(std::map<std::string, std::string> *targetMap, const std::string& key, const std::string& value);

    bool ViewInit();

    virtual std::map<std::string, std::string> *GetDefaultStyle() {
      return nullptr;
    }

    virtual std::map<std::string, std::string> *GetDefaultAttr() {
      return nullptr;
    }

  protected:

    bool UpdateStyleInternal(const std::string key, const std::string value, float fallback,
                             std::function<void(float)> functor);

  public:

    explicit RenderObject();

    ~RenderObject();

    void BindMeasureFunc();

    void onLayoutBefore();

    void onLayoutAfter(float width, float height);

    virtual StyleType ApplyStyle(const std::string &key, const std::string &value, const bool updating);

    void ApplyDefaultStyle();

    void ApplyDefaultAttr();

    Index IndexOf(const RenderObject *render);

    virtual int AddRenderObject(int index, RenderObject *child);

    float GetViewPortWidth();

    const std::string GetAttr(const std::string &key);

    const std::string GetStyle(const std::string &key);

    RenderPage *GetRenderPage();

    virtual void UpdateAttr(std::string key, std::string value);

    virtual StyleType UpdateStyle(std::string key, std::string value);

    bool IsAppendTree();

    inline RenderObject *GetChild(const Index &index) {
      return static_cast<RenderObject*>(getChildAt(index));
    }

    inline void RemoveRenderObject(RenderObject *child) {
      removeChild(child);
    }

    inline void AddAttr(std::string key, std::string value) {
      mapInsertOrAssign(this->attributes, key, value);
    }

    inline StyleType AddStyle(std::string key, std::string value) {
      return ApplyStyle(key, value, false);
    }

    inline void AddEvent(std::string event) {
      if (this->events == nullptr || this->events->empty()) {
        this->events = new std::set<std::string>();
      }
      this->events->insert(event);
    }

    inline void RemoveEvent(const std::string &event) {
      this->events->erase(event);
    }

    inline void SetParentRender(RenderObject *render) {
      this->parent_render = render;
    }

    inline RenderObject *GetParentRender() {
      return this->parent_render;
    }

    inline std::map<std::string, std::string> *Styles() const {
      return this->styles;
    }

    inline std::map<std::string, std::string> * Attributes() const {
      return this->attributes;
    }

    inline std::set<std::string> *Events() const {
      return this->events;
    }

    inline void MarkRootRender() {
      this->is_root_render = true;
    }

    inline bool IsRootRender() {
      return this->is_root_render;
    }

    inline bool IsSticky() {
      return this->is_sticky;
    }

  private:

    RenderObject *parent_render;
    std::map<std::string, std::string> *styles;
    std::map<std::string, std::string> *attributes;
    std::set<std::string> *events;
    float viewport_width = -1;
    bool is_root_render;
    bool is_sticky = false;
  };
} //end WeexCore
#endif //RenderObject_h
