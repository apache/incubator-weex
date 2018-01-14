#ifndef RenderObject_h
#define RenderObject_h

#include <string>
#include <map>
#include <set>
#include <jni.h>
#include <WeexCore/layout/WXCoreLayout.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/node/factory/IRenderObject.h>

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

  public:

    RenderObject();

    ~RenderObject();

    bool BindComponentImplAndroid(jobject component_impl_android);

    bool BindComponentImplIOS(void *component_impl_ios);

    bool BindMeasureFuncImplAndroid(jobject measureFunc_impl_android);

    bool BindMeasureFuncImplIOS(WXCoreMeasureFunc measureFunc_impl_ios);

    void onLayoutBefore();

    void onLayoutAfter(float width, float height);

    StyleType ApplyStyle(const std::string &key, const std::string &value);

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

    inline void AddRenderObject(int index, RenderObject *child) {

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
    }

    inline void RemoveRenderObject(RenderObject *child) {
      removeChild(child);
    }

    inline void AddAttr(const std::string &key, const std::string &value) {
      mAttributes->insert(std::pair<std::string, std::string>(key, value));
    }

    virtual void UpdateAttr(const std::string &key, const std::string &value) {
      AddAttr(key, value);
    }

    inline StyleType AddStyle(const std::string &key, const std::string &value) {
      return ApplyStyle(key, value);
    }

    inline void AddEvent(const std::string &event) {
      mEvents->insert(event);
    }

    inline void RemoveEvent(const std::string &event) {
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

  private:
    RenderObject *mParentRender;
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    MarginsMap *mMargins;
    PaddingsMap *mPaddings;
    BordersMap *mBorders;
    EventsSet *mEvents;
    jobject mComponent_Impl_Android; // TODO / WXComponent bind
    jobject mMeasureFunc_Impl_Android;
    void *mComponent_Impl_iOS;
  };
} //end WeexCore
#endif //RenderObject_h
