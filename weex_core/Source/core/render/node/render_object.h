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

#include "core/render/node/factory/i_render_object.h"

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
      mapInsertOrAssign(mAttributes, key, value);
    }

    inline StyleType AddStyle(std::string key, std::string value) {
      return ApplyStyle(key, value, false);
    }

    inline void AddEvent(std::string event) {
      if (mEvents == nullptr || mEvents->empty()) {
          mEvents = new std::set<std::string>();
      }
      mEvents->insert(event);
    }

    inline void RemoveEvent(const std::string &event) {
      mEvents->erase(event);
    }

    inline void SetParentRender(RenderObject *render) {
      mParentRender = render;
    }

    inline RenderObject *GetParentRender() {
      return mParentRender;
    }

    inline std::map<std::string, std::string> *Styles() const {
      return mStyles;
    }

    inline std::map<std::string, std::string> * Attributes() const {
      return mAttributes;
    }

    inline std::set<std::string> *Events() const {
      return mEvents;
    }

    inline void MarkRootRender() {
      mIsRootRender = true;
    }

    inline bool IsRootRender() {
      return mIsRootRender;
    }

    inline bool IsSticky() {
      return mIsSticky;
    }

  private:

    RenderObject *mParentRender;
    std::map<std::string, std::string> *mStyles;
    std::map<std::string, std::string> *mAttributes;
    std::set<std::string> *mEvents;
    float mViewPortWidth = -1;
    bool mIsRootRender;
    bool mIsSticky = false;
  };
} //end WeexCore
#endif //RenderObject_h
