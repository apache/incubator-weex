#ifndef RenderObject_h
#define RenderObject_h

#include <string>
#include <map>
#include <set>
#include <jni.h>
#include <WeexCore/css/ConstantsName.h>
#include <WeexCore/css/CssValueGetter.h>
#include <WeexCore/layout/WXCoreLayout.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/node/factory/IRenderObject.h>
#include <base/ViewUtils.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/platform/android/base/LogUtils.h>
#include <WeexCore/css/ConstantsValue.h>

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
  typedef std::map<std::string, std::string> MarginsMap;
  typedef std::map<std::string, std::string> PaddingsMap;
  typedef std::map<std::string, std::string> BordersMap;

  class RenderObject : public IRenderObject {

    friend class RenderPage;


  private:
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
      return !isnan(getStyleWidth()) && getStyleWidth() > 0;
    }

    virtual std::map<std::string, std::string> *GetDefaultStyle() {
      return nullptr;
    }

    virtual std::map<std::string, std::string> *GetDefaultAttr() {
      return nullptr;
    }

  public:

    RenderObject();

    ~RenderObject();

    bool BindComponentImplAndroid(jobject component_impl_android);

    bool BindComponentImplIOS(void *component_impl_ios);

    bool BindMeasureFuncImplAndroid(jobject measureFunc_impl_android);

    bool BindMeasureFuncImplIOS(WXCoreMeasureFunc measureFunc_impl_ios);

    void onLayoutBefore();

    void onLayoutAfter(float width, float height);

    virtual StyleType ApplyStyle(const std::string &key, const std::string &value) {
      if (key == ALIGN_ITEMS) {
        setAlignItems(GetWXCoreAlignItem(value));
        return kTypeLayout;
      } else if (key == ALIGN_SELF) {
        setAlignSelf(GetWXCoreAlignSelf(value));
        return kTypeLayout;
      } else if (key == FLEX) {
        setFlex(atof(value.c_str()));
        return kTypeLayout;
      } else if (key == FLEX_DIRECTION) {
        setFlexDirection(GetWXCoreFlexDirection(value));
        return kTypeLayout;
      } else if (key == JUSTIFY_CONTENT) {
        setJustifyContent(GetWXCoreJustifyContent(value));
        return kTypeLayout;
      } else if (key == FLEX_WRAP) {
        setFlexWrap(GetWXCoreFlexWrap(value));
        return kTypeLayout;
      } else if (key == MIN_WIDTH) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMinWidth(realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeLayout;
      } else if (key == MIN_HEIGHT) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMinWidth(realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeLayout;
      } else if (key == MAX_WIDTH) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMinWidth(realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeLayout;
      } else if (key == MAX_HEIGHT) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMinWidth(realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeLayout;
      } else if (key == HEIGHT) {
        setStyleHeightLevel(CSS_STYLE);
        setStyleHeight(getFloatByViewport(value, GetViewPortWidth()));
        return kTypeLayout;
      } else if (key == WIDTH) {
        setStyleWidthLevel(CSS_STYLE);
        setStyleWidth(getFloatByViewport(value, GetViewPortWidth()));
        return kTypeLayout;
      } else if (key == POSITION) {
        setStylePositionType(GetWXCorePositionType(value));
        mStyles->insert(std::pair<std::string, std::string>(key, value));
        return kTypeLayout;
      } else if (key == LEFT) {
        setStylePosition(kPositionEdgeLeft, getFloatByViewport(value,
                                                               GetViewPortWidth()));
        return kTypeLayout;
      } else if (key == TOP) {
        setStylePosition(kPositionEdgeTop, getFloatByViewport(value,
                                                              GetViewPortWidth()));
        return kTypeLayout;
      } else if (key == RIGHT) {
        setStylePosition(kPositionEdgeRight, getFloatByViewport(value,
                                                                GetViewPortWidth()));
        return kTypeLayout;
      } else if (key == BOTTOM) {
        setStylePosition(kPositionEdgeBottom, getFloatByViewport(value,
                                                                 GetViewPortWidth()));
        return kTypeLayout;
      } else if (key == MARGIN) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMargin(kMarginALL, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_LEFT) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMargin(kMarginLeft, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_TOP) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMargin(kMarginTop, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_RIGHT) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMargin(kMarginRight, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_BOTTOM) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setMargin(kMarginBottom, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == BORDER_WIDTH) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setBorderWidth(kBorderWidthALL, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_TOP_WIDTH) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setBorderWidth(kBorderWidthTop, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_RIGHT_WIDTH) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setBorderWidth(kBorderWidthRight, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_BOTTOM_WIDTH) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setBorderWidth(kBorderWidthBottom, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_LEFT_WIDTH) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setBorderWidth(kBorderWidthLeft, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == PADDING) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setPadding(kPaddingALL, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_LEFT) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setPadding(kPaddingLeft, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_TOP) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setPadding(kPaddingTop, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_RIGHT) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setPadding(kPaddingRight, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_BOTTOM) {
        float realValue = getFloatByViewport(value, GetViewPortWidth());
        setPadding(kPaddingBottom, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else {
        mStyles->insert(std::pair<std::string, std::string>(key, value));
        return kTypeStyle;
      }
    }

    void ApplyDefaultStyle();

    void ApplyDefaultAttr();

    void PrintRenderMsg();

    void PrintLayoutMsg();

    inline jobject GetComponentImplAndroid() {
      return mComponent_Impl_Android;
    }

    inline jobject GetMeasureFuncImplAndroid() {
      return mMeasureFunc_Impl_Android;
    }

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

    virtual void AddRenderObject(int index, RenderObject *child) {

      if (child == nullptr || index < -1) {
        return;
      }

      Index count = getChildCount();
      index = index >= count ? -1 : index;
      if (index == -1) {
        addChildAt(child, getChildCount());
      } else {
        addChildAt(child, index);
      }

      child->SetParentRender(this);
    }

    inline void RemoveRenderObject(RenderObject *child) {
      removeChild(child);
    }

    inline void AddAttr(std::string key, std::string value) {
      mAttributes->insert(std::pair<std::string, std::string>(key, value));
    }

    virtual void UpdateAttr(std::string key, std::string value) {
      AddAttr(key, value);
    }

    virtual StyleType UpdateStyle(std::string key, std::string value) {
      return ApplyStyle(key, value);
    }

    inline StyleType AddStyle(std::string key, std::string value) {
      return ApplyStyle(key, value);
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

    inline PaddingsMap *Paddings() const {
      return mPaddings;
    }

    inline MarginsMap *Margins() const {
      return mMargins;
    }

    inline BordersMap *Borders() const {
      return mBorders;
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

  private:
    RenderObject *mParentRender;
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    MarginsMap *mMargins;
    BordersMap *mBorders;
    EventsSet *mEvents;
    jobject mComponent_Impl_Android;
    jobject mMeasureFunc_Impl_Android;
    void *mComponent_Impl_iOS;
    float mViewPortWidth = -1;

  public:
    PaddingsMap *mPaddings;
  };
} //end WeexCore
#endif //RenderObject_h
