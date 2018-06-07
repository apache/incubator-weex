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
#include <core/css/constants_name.h>
#include <core/css/css_value_getter.h>
#include <core/layout/layout.h>
#include <core/render/manager/render_manager.h>
#include <core/render/node/factory/i_render_object.h>
#include <base/ViewUtils.h>
#include <core/render/page/render_page.h>
#include <core/css/constants_value.h>
#include <functional>


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

  typedef std::map<std::string, std::string>::const_iterator StylesIterator;
  typedef std::map<std::string, std::string>::const_iterator AttributesIterator;
  typedef std::set<std::string>::const_iterator EventsIterator;
  typedef std::map<std::string, std::string> StylesMap;
  typedef std::map<std::string, std::string> AttributesMap;
  typedef std::set<std::string> EventsSet;

  class RenderObject : public IRenderObject {

    friend class RenderPage;

  public:
    inline void LayoutBefore() {
      if (isDirty()) {
        onLayoutBefore();
      }

      for(auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
        RenderObject* child = static_cast<RenderObject*>(*it);
        if (child != nullptr) {
          child->LayoutBefore();
        }
      }
    }

    inline void LayoutAfter() {
      if (hasNewLayout()) {
        onLayoutAfter(getLayoutWidth(), getLayoutHeight());
      }

      for(auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
        RenderObject* child = static_cast<RenderObject*>(*it);
        if (child != nullptr) {
          child->LayoutAfter();
        }
      }
    }

    inline bool ViewInit() {
      return (!isnan(getStyleWidth()) && getStyleWidth() > 0) ||
          (IsRootRender() && GetRenderPage() != nullptr && GetRenderPage()->GetRenderContainerWidthWrapContent());
    }

    virtual std::map<std::string, std::string> *GetDefaultStyle() {
      return nullptr;
    }

    virtual std::map<std::string, std::string> *GetDefaultAttr() {
      return nullptr;
    }

    void copyFrom(RenderObject* src){
      IRenderObject::copyFrom(src);
      this->mStyles->insert(src->mStyles->begin(), src->mStyles->end());
      this->mAttributes->insert(src->mAttributes->begin(), src->mAttributes->end());
      this->mEvents->insert(src->mEvents->begin(), src->mEvents->end());
    }

    inline void mapInsertOrAssign(std::map<std::string, std::string> *targetMap, const std::string& key, const std::string& value) {
      std::map<std::string, std::string>::iterator it = targetMap->find(key);
      if(it != targetMap->end()){
        it->second = value;
      }else{
        targetMap->insert({key, value});
      }
    }

   protected:
    bool UpdateStyle(const std::string key, const std::string value,
                      float fallback, std::function<void(float)> functor){
      bool ret = false;
      if (value.empty()) {
        functor(fallback);
        ret = true;
      } else {
        float fvalue = getFloatByViewport(value, GetViewPortWidth());
        if (!isnan(fvalue)) {
          functor(fvalue);
          ret = true;
        }
      }
      return ret;
    }

  public:

    RenderObject();

    ~RenderObject();

    void BindMeasureFunc();

    void onLayoutBefore();

    void onLayoutAfter(float width, float height);

    virtual StyleType ApplyStyle(const std::string &key, const std::string &value, const bool updating) {
      bool  insert = false;
      if(value.length() > 0 && (value.at(0) == JSON_OBJECT_MARK_CHAR || value.at(0) == JSON_ARRAY_MARK_CHAR)){
        mapInsertOrAssign(mStyles, key, value);
        insert = true;
      }

      if (key == ALIGN_ITEMS) {
        setAlignItems(GetWXCoreAlignItem(value));
        return kTypeLayout;
      } else if (key == ALIGN_SELF) {
        setAlignSelf(GetWXCoreAlignSelf(value));
        return kTypeLayout;
      } else if (key == FLEX) {
        if (value.empty()) {
          setFlex(0);
        } else {
          float ret = getFloat(value.c_str());
          if (!isnan(ret)) {
            setFlex(ret);
          }
        }
        return kTypeLayout;
      } else if (key == FLEX_DIRECTION) {
        setFlexDirection(GetWXCoreFlexDirection(value), updating);
        return kTypeLayout;
      } else if (key == JUSTIFY_CONTENT) {
        setJustifyContent(GetWXCoreJustifyContent(value));
        return kTypeLayout;
      } else if (key == FLEX_WRAP) {
        setFlexWrap(GetWXCoreFlexWrap(value));
        return kTypeLayout;
      } else if (key == MIN_WIDTH) {
        UpdateStyle(key, value, NAN, [=](float foo){setMinWidth(foo, updating);});
        return kTypeLayout;
      } else if (key == MIN_HEIGHT) {
        UpdateStyle(key, value, NAN, [=](float foo){setMinHeight(foo);});
        return kTypeLayout;
      } else if (key == MAX_WIDTH) {
        UpdateStyle(key, value, NAN, [=](float foo){setMaxWidth(foo, updating);});
        return kTypeLayout;
      } else if (key == MAX_HEIGHT) {
        UpdateStyle(key, value, NAN, [=](float foo){setMaxHeight(foo);});
        return kTypeLayout;
      } else if (key == HEIGHT) {
        if(UpdateStyle(key, value, NAN, [=](float foo){setStyleHeight(foo);})){
          setStyleHeightLevel(CSS_STYLE);
        }
        return kTypeLayout;
      } else if (key == WIDTH) {
        if(UpdateStyle(key, value, NAN, [=](float foo){setStyleWidth(foo, updating);})){
          setStyleWidthLevel(CSS_STYLE);
        }
        return kTypeLayout;
      } else if (key == POSITION) {
        setStylePositionType(GetWXCorePositionType(value));
        if (value == STICKY) {
          mIsSticky = true;
        }
        mapInsertOrAssign(mStyles, key, value);
        return kTypeStyle;
      } else if (key == LEFT) {
        UpdateStyle(key, value, NAN, [=](float foo){setStylePosition(kPositionEdgeLeft, foo);});
        return kTypeLayout;
      } else if (key == TOP) {
        UpdateStyle(key, value, NAN, [=](float foo){setStylePosition(kPositionEdgeTop, foo);});
        return kTypeLayout;
      } else if (key == RIGHT) {
        UpdateStyle(key, value, NAN, [=](float foo){setStylePosition(kPositionEdgeRight, foo);});
        return kTypeLayout;
      } else if (key == BOTTOM) {
        UpdateStyle(key, value, NAN, [=](float foo){setStylePosition(kPositionEdgeBottom, foo);});
        return kTypeLayout;
      } else if (key == MARGIN) {
        UpdateStyle(key, value, 0, [=](float foo){setMargin(kMarginALL, foo);});
        return kTypeMargin;
      } else if (key == MARGIN_LEFT) {
        UpdateStyle(key, value, 0, [=](float foo){setMargin(kMarginLeft, foo);});
        return kTypeMargin;
      } else if (key == MARGIN_TOP) {
        UpdateStyle(key, value, 0, [=](float foo){setMargin(kMarginTop, foo);});
        return kTypeMargin;
      } else if (key == MARGIN_RIGHT) {
        UpdateStyle(key, value, 0, [=](float foo){setMargin(kMarginRight, foo);});
        return kTypeMargin;
      } else if (key == MARGIN_BOTTOM) {
        UpdateStyle(key, value, 0, [=](float foo){setMargin(kMarginBottom, foo);});
        return kTypeMargin;
      } else if (key == BORDER_WIDTH) {
        UpdateStyle(key, value, 0, [=](float foo){setBorderWidth(kBorderWidthALL, foo);});
        return kTypeBorder;
      } else if (key == BORDER_TOP_WIDTH) {
        UpdateStyle(key, value, 0, [=](float foo){setBorderWidth(kBorderWidthTop, foo);});
        return kTypeBorder;
      } else if (key == BORDER_RIGHT_WIDTH) {
        UpdateStyle(key, value, 0, [=](float foo){setBorderWidth(kBorderWidthRight, foo);});
        return kTypeBorder;
      } else if (key == BORDER_BOTTOM_WIDTH) {
        UpdateStyle(key, value, 0, [=](float foo){setBorderWidth(kBorderWidthBottom, foo);});
        return kTypeBorder;
      } else if (key == BORDER_LEFT_WIDTH) {
        UpdateStyle(key, value, 0, [=](float foo){setBorderWidth(kBorderWidthLeft, foo);});
        return kTypeBorder;
      } else if (key == PADDING) {
        UpdateStyle(key, value, 0, [=](float foo){setPadding(kPaddingALL, foo);});
        return kTypePadding;
      } else if (key == PADDING_LEFT) {
        UpdateStyle(key, value, 0, [=](float foo){setPadding(kPaddingLeft, foo);});
        return kTypePadding;
      } else if (key == PADDING_TOP) {
        UpdateStyle(key, value, 0, [=](float foo){setPadding(kPaddingTop, foo);});
        return kTypePadding;
      } else if (key == PADDING_RIGHT) {
        UpdateStyle(key, value, 0, [=](float foo){setPadding(kPaddingRight, foo);});
        return kTypePadding;
      } else if (key == PADDING_BOTTOM) {
        UpdateStyle(key, value, 0, [=](float foo){setPadding(kPaddingBottom, foo);});
        return kTypePadding;
      } else {
        if(!insert){
          mapInsertOrAssign(mStyles, key, value);
        }
        return kTypeStyle;
      }
    }

    void ApplyDefaultStyle();

    void ApplyDefaultAttr();

    inline RenderObject *GetChild(const Index &index) {
      return static_cast<RenderObject*>(getChildAt(index));
    }

    inline Index IndexOf(const RenderObject *render) {
      if (render == nullptr) {
        return -1;
      } else {
        int i = 0;
        for(auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
          RenderObject* child = static_cast<RenderObject*>(*it);
          if (child != nullptr) {
            if (render->Ref() == child->Ref())
              return i;
          }
          ++i;
        }
      }
      return -1;
    }

    virtual int AddRenderObject(int index, RenderObject *child) {

      if (child == nullptr || index < -1) {
        return index;
      }

      Index count = getChildCount();
      index = index >= count ? -1 : index;
      if (index == -1) {
        addChildAt(child, getChildCount());
      } else {
        addChildAt(child, index);
      }

      child->SetParentRender(this);

      return index;
    }

    inline void RemoveRenderObject(RenderObject *child) {
      removeChild(child);
    }

    inline void AddAttr(std::string key, std::string value) {
      mapInsertOrAssign(mAttributes, key, value);
    }

    virtual void UpdateAttr(std::string key, std::string value) {
      mapInsertOrAssign(mAttributes, key, value);
    }

    virtual StyleType UpdateStyle(std::string key, std::string value) {
      return ApplyStyle(key, value, true);
    }

    inline StyleType AddStyle(std::string key, std::string value) {
      return ApplyStyle(key, value, false);
    }

    inline void AddEvent(std::string event) {
      if (mEvents == nullptr || mEvents->empty()) {
          mEvents = new EventsSet();
      }
      mEvents->insert(event);
    }

    inline void RemoveEvent(const std::string &event) {
      mEvents->erase(event);
    }

    inline RenderPage *GetRenderPage() {
      return RenderManager::GetInstance()->GetPage(PageId());
    }

    inline float GetViewPortWidth() {
      if (mViewPortWidth >= 0)
        return mViewPortWidth;

      RenderPage *page = GetRenderPage();
      if (page == nullptr)
        return kDefaultViewPortWidth;

      return page->ViewPortWidth();
    }

    inline void SetParentRender(RenderObject *render) {
      mParentRender = render;
    }

    inline RenderObject *GetParentRender() {
      return mParentRender;
    }

    inline StylesMap *Styles() const {
      return mStyles;
    }

    inline AttributesMap * Attributes() const {
      return mAttributes;
    }

    inline EventsSet *Events() const {
      return mEvents;
    }

    inline StylesIterator StyleItBegin() {
      return mStyles->begin();
    }

    inline StylesIterator StyleItEnd() {
      return mStyles->end();
    }

    inline AttributesIterator AttrItBegin() {
      return mAttributes->begin();
    }

    inline AttributesIterator AttrItEnd() {
      return mAttributes->end();
    }

    inline EventsIterator EventItBegin() {
      return mEvents->begin();
    }

    inline EventsIterator EventItEnd() {
      return mEvents->end();
    }

    inline const std::string GetAttr(const std::string &key) {
      if (mAttributes == nullptr)
        return "";

      std::map<std::string, std::string>::iterator iter = mAttributes->find(key);
      if (iter != mAttributes->end()) {
        return iter->second;
      } else {
        return "";
      }
    }

    inline const std::string GetStyle(const std::string &key) {
      if (mStyles == nullptr)
        return "";

      std::map<std::string, std::string>::iterator iter = mStyles->find(key);
      if (iter != mStyles->end()) {
        return iter->second;
      } else {
        return "";
      }
    }

    inline void MarkRootRender() {
      mIsRootRender = true;
    }

    inline bool IsRootRender() {
      return mIsRootRender;
    }

    inline bool IsAppendTree() {
      std::string append = GetAttr(APPEND);
      if(append == "tree") {
        return true;
      }
      return false;
    }

    inline bool IsSticky() {
      return mIsSticky;
    }

  private:
    RenderObject *mParentRender;
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    EventsSet *mEvents;
    float mViewPortWidth = -1;
    bool mIsRootRender;
    bool mIsSticky = false;
  };
} //end WeexCore
#endif //RenderObject_h
