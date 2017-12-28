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
    TypeStyle, TypeLayout, TypeMargin, TypePadding, TypeBorder
  } Style_Type;

  typedef std::map<std::string, std::string>::const_iterator STYLE_IT;
  typedef std::map<std::string, std::string>::const_iterator ATTR_IT;
  typedef std::set<std::string>::const_iterator EVENT_IT;
  typedef std::map<std::string, std::string> STYLES_MAP;
  typedef std::map<std::string, std::string> ATTRIBUTES_MAP;
  typedef std::set<std::string> EVENTS_SET;
  typedef std::map<std::string, std::string> MARGIN_MAP;
  typedef std::map<std::string, std::string> PADDING_MAP;
  typedef std::map<std::string, std::string> BORDER_MAP;

  class RenderObject : public WXCoreLayoutNode {
  public:

  private:
    std::string mRef = "";

    std::string mType = "";

    RenderObject *mParentRender;

    STYLES_MAP *mStyles;

    ATTRIBUTES_MAP *mAttributes;

    MARGIN_MAP *mMargins;

    PADDING_MAP *mPaddings;

    BORDER_MAP *mBorders;

    EVENTS_SET *mEvents;

    // TODO / WXComponent bind
    jobject mComponent_Impl_Android;

    jobject mMeasureFunc_Impl_Android;

    void *mComponent_Impl_iOS;

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

    inline RenderObject *getChild(uint32_t &index) {
      return (RenderObject *) getChildAt(index);
    }

    inline uint32_t indexOf(RenderObject *render) {
      if (render == nullptr) {
        for (uint32_t i = 0; i < getChildCount(); i++)
          if (getChild(i) == nullptr)
            return i;
      } else {
        for (uint32_t i = 0; i < getChildCount(); i++)
          if (render->getRef() == getChild(i)->getRef())
            return i;
      }
      return -1;
    }

    inline void addRenderObject(int index, RenderObject *child) {

      if (child == nullptr || index < -1) {
        return;
      }

      int count = getChildCount();
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

    inline STYLES_MAP *getStyles() {
      return mStyles;
    }

    inline ATTRIBUTES_MAP *getAttributes() {
      return mAttributes;
    }

    inline EVENTS_SET *getEvents() {
      return mEvents;
    }

    inline PADDING_MAP *getPaddings() {
      return mPaddings;
    }

    inline MARGIN_MAP *getMargins() {
      return mMargins;
    }

    inline BORDER_MAP *getBorders() {
      return mBorders;
    }

    inline STYLE_IT getStyleItBegin() {
      return mStyles->begin();
    }

    inline STYLE_IT getStyleItEnd() {
      return mStyles->end();
    }

    inline ATTR_IT getAttrItBegin() {
      return mAttributes->begin();
    }

    inline ATTR_IT getAttrItEnd() {
      return mAttributes->end();
    }

    inline EVENT_IT getEventItBegin() {
      return mEvents->begin();
    }

    inline EVENT_IT getEventItEnd() {
      return mEvents->end();
    }
  };
} //end WeexCore
#endif //RenderObject_h
