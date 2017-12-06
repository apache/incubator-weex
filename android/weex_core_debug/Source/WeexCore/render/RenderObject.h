#ifndef RenderObject_h
#define RenderObject_h

#include <string>
#include <map>
#include <set>
#include <jni.h>
#include <Layout/WXCoreLayout.h>

namespace WeexCore {

  class RenderObject;

  class RenderPage;

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

    RenderPage *mPage;

    RenderObject *mParentRender;

    STYLES_MAP *mStyles;

    ATTRIBUTES_MAP *mAttributes;

    MARGIN_MAP *mMargins;

    PADDING_MAP *mPaddings;

    BORDER_MAP *mBorders;

    EVENTS_SET *mEvents;

    jobject mComponent_Impl_Android;

    jobject mMeasureFunc_Impl_Android;

    void *mComponent_Impl_iOS;

  public:

    RenderObject(RenderPage *page);

    ~RenderObject();

    void bindComponent_Impl_Android(jobject component_Impl_Android);

    void bindComponent_Impl_iOS(void *component_Impl_iOS);

    void bindMeasureFunc_Impl_Android(jobject measureFunc_Impl_Android);

    void bindMeasureFunc_Impl_iOS(WXCoreMeasureFunc measureFunc_Impl_iOS);

    void onLayoutBefore();

    void onLayoutAfter(float width, float height);

    void applyStyle(std::string key, std::string value);

    void printRenderMsg();

    void printYGNodeMsg();

    inline jobject getComponent_Impl_Android() {
      return mComponent_Impl_Android;
    }

    inline jobject getMeasureFunc_Impl_Android() {
      return mMeasureFunc_Impl_Android;
    }

    inline RenderObject *getChild(uint32_t index) {
      return (RenderObject *) getChildAt(index);
    }

    inline void addRenderObject(int index, RenderObject *child) {
      // insert RenderObject child
      addChildAt(child, getChildCount());
    }

    inline void removeRenderObject(RenderObject *child) {
      removeChild(child);
    }

    inline void addAttr(std::string key, std::string value) {
      mAttributes->insert(std::pair<std::string, std::string>(key, value));
    }

    inline void addAttr(std::map<std::string, std::string> attr) {
//      mAttributes->insert(attr);
    }

    inline void addStyle(std::string key, std::string value) {
      applyStyle(key, value);
    }

    inline void addStyle(std::map<std::string, std::string> style) {
      STYLE_IT style_begin = style.begin();
      STYLE_IT style_end = style.end();
      for (; style_begin != style_end; ++style_begin) {
        applyStyle(style_begin->first, style_begin->second);
      }
    }

    inline void addEvent(std::string event) {
      mEvents->insert(event);
    }

    inline void removeEvent(std::string event) {
      mEvents->erase(event);
    }

    inline void setRef(std::string ref) {
      mRef = ref;
    }

    inline std::string getRef() {
      return mRef;
    }

    inline void setType(std::string type) {
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
