#ifndef RenderObject_h
#define RenderObject_h

#include <string>
#include <map>
#include <set>
#include <jni.h>
#include <WeexCore/layout/WXCoreLayout.h>

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

  class RenderObject : public WXCoreLayoutNode {

    friend class RenderPage;

  private:
    std::string mRef = "";

    std::string mType = "";

    RenderObject *mParentRender;

    StylesMap *mStyles;

    AttributesMap *mAttributes;

    MarginsMap *mMargins;

    PaddingsMap *mPaddings;

    BordersMap *mBorders;

    EventsSet *mEvents;

    // TODO / WXComponent bind
    jobject mComponent_Impl_Android;

    jobject mMeasureFunc_Impl_Android;

    void *mComponent_Impl_iOS;

    void layoutBefore() {
      onLayoutBefore();
      for (Index i = 0; i < getChildCount(); i++) {
        RenderObject *child = getChild(i);
        child->layoutBefore();
      }
    }

    void layoutAfter() {
      onLayoutAfter(getLayoutWidth(), getLayoutHeight());
      for (Index i = 0; i < getChildCount(); i++) {
        RenderObject *child = getChild(i);
        child->layoutAfter();
      }
    }

  public:

    RenderObject();

    ~RenderObject();

    bool bindComponent_Impl_Android(jobject component_Impl_Android);

    bool bindComponent_Impl_iOS(void *component_Impl_iOS);

    bool bindMeasureFunc_Impl_Android(jobject measureFunc_Impl_Android);

    bool bindMeasureFunc_Impl_iOS(WXCoreMeasureFunc measureFunc_Impl_iOS);

    void onLayoutBefore();

    void onLayoutAfter(float width, float height);

    StyleType applyStyle(const std::string &key, const std::string &value);

    void printRenderMsg();

    void printYGNodeMsg();

    inline jobject getComponent_Impl_Android() {
      return mComponent_Impl_Android;
    }

    inline jobject getMeasureFunc_Impl_Android() {
      return mMeasureFunc_Impl_Android;
    }

    inline RenderObject *getChild(Index &index) {
      return (RenderObject *) getChildAt(index);
    }

    inline Index indexOf(RenderObject *render) {
      if (render == nullptr) {
        for (Index i = 0; i < getChildCount(); i++)
          if (getChild(i) == nullptr)
            return i;
      } else {
        for (Index i = 0; i < getChildCount(); i++)
          if (render->getRef() == getChild(i)->getRef())
            return i;
      }
      return -1;
    }

    inline void addRenderObject(int index, RenderObject *child) {

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

    inline void removeRenderObject(RenderObject *child) {
      removeChild(child);
    }

    inline void addAttr(const std::string &key, const std::string &value) {
      mAttributes->insert(std::pair<std::string, std::string>(key, value));
    }

    inline StyleType addStyle(const std::string &key, const std::string &value) {
      return applyStyle(key, value);
    }

    inline void addEvent(const std::string &event) {
      mEvents->insert(event);
    }

    inline void removeEvent(const std::string &event) {
      mEvents->erase(event);
    }

    inline void setRef(const std::string &ref) {
      mRef = ref;
    }

    inline const std::string &getRef() {
      return mRef;
    }

    inline void setType(const std::string &type) {
      mType = type;
    }

    inline std::string getType() {
      return mType;
    }

    inline void setParentRender(RenderObject *render) {
      mParentRender = render;
    }

    inline RenderObject *getParentRender() {
      return mParentRender;
    }

    inline StylesMap *getStyles() {
      return mStyles;
    }

    inline AttributesMap *getAttributes() {
      return mAttributes;
    }

    inline EventsSet *getEvents() {
      return mEvents;
    }

    inline PaddingsMap *getPaddings() {
      return mPaddings;
    }

    inline MarginsMap *getMargins() {
      return mMargins;
    }

    inline BordersMap *getBorders() {
      return mBorders;
    }

    inline StylesIterator getStyleItBegin() {
      return mStyles->begin();
    }

    inline StylesIterator getStyleItEnd() {
      return mStyles->end();
    }

    inline AttributesIterator getAttrItBegin() {
      return mAttributes->begin();
    }

    inline AttributesIterator getAttrItEnd() {
      return mAttributes->end();
    }

    inline EventsIterator getEventItBegin() {
      return mEvents->begin();
    }

    inline EventsIterator getEventItEnd() {
      return mEvents->end();
    }
  };
} //end WeexCore
#endif //RenderObject_h
