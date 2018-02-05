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

  class RenderObject : public IRenderObject, public WXCoreLayoutNode {

    friend class RenderPage;


  private:
    inline void LayoutBefore() {
      if (isDirty()) {
        onLayoutBefore();
      }
      for (Index i = 0; i < getChildCount(); i++) {
        RenderObject *child = GetChild(i);
        child->LayoutBefore();
      }
    }

    inline void LayoutAfter() {
      if (hasNewLayout()) {
        onLayoutAfter(getLayoutWidth(), getLayoutHeight());
      }
      for (Index i = 0; i < getChildCount(); i++) {
        RenderObject *child = GetChild(i);
        child->LayoutAfter();
      }
    }

    inline bool ViewInit() {
      return !isnan(getStyleWidth()) && !isnan(getStyleHeight()) && getStyleWidth() > 0 &&
             getStyleHeight() > 0;
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

    virtual StyleType ApplyStyle(std::string key, std::string value) {
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
        setMinWidth(getFloatByViewport(value, GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == MIN_HEIGHT) {
        setMinHeight(getFloatByViewport(value, GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == MAX_WIDTH) {
        setMaxWidth(getFloatByViewport(value, GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == MAX_HEIGHT) {
        setMaxHeight(getFloatByViewport(value, GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == HEIGHT) {
        setStyleHeightLevel(CSS_STYLE);
        setStyleHeight(getFloatByViewport(value, GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == WIDTH) {
        setStyleWidthLevel(CSS_STYLE);
        setStyleWidth(getFloatByViewport(value, GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == POSITION) {
        setStylePositionType(GetWXCorePositionType(value));
        return kTypeLayout;
      } else if (key == LEFT) {
        setStylePosition(kPositionEdgeLeft, getFloatByViewport(value,
                                                             GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == TOP) {
        setStylePosition(kPositionEdgeTop, getFloatByViewport(value,
                                                            GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == RIGHT) {
        setStylePosition(kPositionEdgeRight, getFloatByViewport(value,
                                                              GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == BOTTOM) {
        setStylePosition(kPositionEdgeBottom, getFloatByViewport(value,
                                                               GetRenderPage()->ViewPortWidth()));
        return kTypeLayout;
      } else if (key == MARGIN) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setMargin(kMarginALL, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_LEFT) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setMargin(kMarginLeft, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_TOP) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setMargin(kMarginTop, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_RIGHT) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setMargin(kMarginRight, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == MARGIN_BOTTOM) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setMargin(kMarginBottom, realValue);
        mMargins->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeMargin;
      } else if (key == BORDER_WIDTH) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setBorderWidth(kBorderWidthALL, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_TOP_WIDTH) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setBorderWidth(kBorderWidthTop, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_RIGHT_WIDTH) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setBorderWidth(kBorderWidthRight, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_BOTTOM_WIDTH) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setBorderWidth(kBorderWidthBottom, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == BORDER_LEFT_WIDTH) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setBorderWidth(kBorderWidthLeft, realValue);
        mBorders->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypeBorder;
      } else if (key == PADDING) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingALL, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_LEFT) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingLeft, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_TOP) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingTop, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_RIGHT) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingRight, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_BOTTOM) {
        float realValue = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
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

    inline RenderObject *GetChild(Index &index) {
      return (RenderObject *) getChildAt(index);
    }

    inline Index IndexOf(RenderObject *render) {
      if (render == nullptr) {
        for (Index i = 0; i < getChildCount(); i++)
          if (GetChild(i) == nullptr)
            return i;
      } else {
        for (Index i = 0; i < getChildCount(); i++)
          if (render->Ref() == GetChild(i)->Ref())
            return i;
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

      LOGEDEBUG("Add render address is %p, ref is %s, type is %s", child, child->Ref().c_str(), child->Type().c_str());
      child->SetParentRender(this);
    }

    inline void RemoveRenderObject(RenderObject *child) {
      LOGEDEBUG("Remove render address is %p, ref is %s, type is %s", child, child->Ref().c_str(), child->Type().c_str());
      removeChild(child);
    }

    inline void AddAttr(std::string key, std::string value) {
      mAttributes->insert(std::pair<std::string, std::string>(key, value));
    }

    virtual void UpdateAttr(std::string key, std::string value) {
      AddAttr(key, value);
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

    inline void RemoveEvent(std::string event) {
      mEvents->erase(event);
    }

    inline RenderPage *GetRenderPage() {
      return RenderManager::GetInstance()->GetPage(PageId());
    }

    inline void SetParentRender(RenderObject *render) {
      mParentRender = render;
    }

    inline RenderObject *GetParentRender() {
      return mParentRender;
    }

    inline StylesMap *Styles() {
      return mStyles;
    }

    inline AttributesMap *Attributes() {
      return mAttributes;
    }

    inline EventsSet *Events() {
      return mEvents;
    }

    inline PaddingsMap *Paddings() {
      return mPaddings;
    }

    inline MarginsMap *Margins() {
      return mMargins;
    }

    inline BordersMap *Borders() {
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

    inline const std::string GetStyle(std::string key) {
      if (mStyles == nullptr)
        return "";
      return mStyles->find(key)->second;
    }

    inline const std::string GetAttr(std::string key) {
      if (mAttributes == nullptr)
        return "";
      return mAttributes->find(key)->second;
    }

  private:
    RenderObject *mParentRender;
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    MarginsMap *mMargins;
    BordersMap *mBorders;
    EventsSet *mEvents;
    jobject mComponent_Impl_Android; // TODO / WXComponent bind
    jobject mMeasureFunc_Impl_Android;
    void *mComponent_Impl_iOS;

  public:
    PaddingsMap *mPaddings;
  };
} //end WeexCore
#endif //RenderObject_h
