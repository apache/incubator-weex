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
      if(isDirty()) {
        onLayoutBefore();
      }
      for (Index i = 0; i < getChildCount(); i++) {
        RenderObject *child = getChild(i);
        child->layoutBefore();
      }
    }

    void layoutAfter() {
      if(hasNewLayout()) {
        onLayoutAfter(getLayoutWidth(), getLayoutHeight());
      }
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

     jobject getComponent_Impl_Android() {
      return mComponent_Impl_Android;
    }

    jobject getMeasureFunc_Impl_Android() {
      return mMeasureFunc_Impl_Android;
    }

    RenderObject *getChild(Index &index) {
      return (RenderObject *) getChildAt(index);
    }

    Index indexOf(RenderObject *render) {
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

    void addRenderObject(int index, RenderObject *child) {

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

    void removeRenderObject(RenderObject *child) {
      removeChild(child);
    }

    void addAttr(const std::string &key, const std::string &value) {
      mAttributes->insert(std::pair<std::string, std::string>(key, value));
    }

    StyleType addStyle(const std::string &key, const std::string &value) {
      return applyStyle(key, value);
    }

    void addEvent(const std::string &event) {
      mEvents->insert(event);
    }

    void removeEvent(const std::string &event) {
      mEvents->erase(event);
    }

    void setRef(const std::string &ref) {
      mRef = ref;
    }

    const std::string &getRef() {
      return mRef;
    }

    void setType(const std::string &type) {
      mType = type;
    }

    std::string getType() {
      return mType;
    }

    void setParentRender(RenderObject *render) {
      mParentRender = render;
    }

    RenderObject *getParentRender() {
      return mParentRender;
    }

    StylesMap *getStyles() {
      return mStyles;
    }

    AttributesMap *getAttributes() {
      return mAttributes;
    }

    EventsSet *getEvents() {
      return mEvents;
    }

    PaddingsMap *getPaddings() {
      return mPaddings;
    }

    MarginsMap *getMargins() {
      return mMargins;
    }

    BordersMap *getBorders() {
      return mBorders;
    }

    StylesIterator getStyleItBegin() {
      return mStyles->begin();
    }

    StylesIterator getStyleItEnd() {
      return mStyles->end();
    }

    AttributesIterator getAttrItBegin() {
      return mAttributes->begin();
    }

    AttributesIterator getAttrItEnd() {
      return mAttributes->end();
    }

    EventsIterator getEventItBegin() {
      return mEvents->begin();
    }

    EventsIterator getEventItEnd() {
      return mEvents->end();
    }
  };
} //end WeexCore
#endif //RenderObject_h
