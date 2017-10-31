#ifndef RenderObject_h
#define RenderObject_h

#include <vector>
#include <string>
#include <map>
#include <set>
#include <Layout/ConstantsName.h>
#include <Layout/CssValueGetter.h>
#include <Layout/WXCoreLayout.h>
#include <Layout/WXCoreStyle.h>
#include <base/android/string/StringUtils.h>
#include <base/android/LogUtils.h>

namespace WeexCore {

  class RenderObject;

  class RenderPage;

  typedef std::vector<RenderObject *> ChildrenList;
  typedef std::map<std::string, std::string>::const_iterator STYLE_IT;
  typedef std::map<std::string, std::string>::const_iterator ATTR_IT;
  typedef std::set<std::string>::const_iterator EVENT_IT;
  typedef ChildrenList::iterator CHILD_LIST_IT;
  typedef std::map<std::string, std::string> STYLES;
  typedef std::map<std::string, std::string> ATTRIBUTES;
  typedef std::set<std::string> EVENTS;

  class RenderObject {
  public:

  private:
    std::string mRef = "";

    std::string mType = "";

    RenderPage *mPage;

    RenderObject *mParentRender;

    STYLES *mStyles;

    ATTRIBUTES *mAttributes;

    EVENTS *mEvents;

    ChildrenList mChildren;

    WXCoreLayoutNode *mLayoutNode;

  public:

    RenderObject(RenderPage *page);

    ~RenderObject();

    inline void addRenderObject(int index, RenderObject *child) {
      // insert RenderObject child
//    mChildren.insert(mChildren.begin() + mChildren.size() - 1, child);
      mLayoutNode->addChildAt(child->getLayoutNode(), getChildCount());
      mChildren.push_back(child);
    }

    inline void removeRenderObject(RenderObject *child) {
      for (CHILD_LIST_IT it = getChildListItBegin();
           it != getChildListItEnd(); it++) {
        if ((*it)->getRef() == child->getRef()) {
          mChildren.erase(it);
          break;
        }
      }
    }

    inline void updateAttr(std::string key, std::string value) {
      if (mAttributes == nullptr) {
        mAttributes = new ATTRIBUTES();
      }

      mAttributes->insert(pair<std::string, std::string>(key, value));
    }

    inline void updateStyle(std::string key, std::string value) {
      if (mStyles == nullptr) {
        mStyles = new STYLES();
      }

      mStyles->insert(pair<std::string, std::string>(key, value));
      applyStyleToYGNode(key, value);
    }

    inline void addEvent(std::string event) {
      if (mEvents == nullptr) {
        mEvents = new EVENTS();
      }
      mEvents->insert(event);
    }

    inline void removeEvent(std::string event) {
      if (mEvents == nullptr) {
        return;
      }
      mEvents->erase(event);
    }

    inline RenderObject *getChild(int index) {
      if (index < mChildren.size()) {
        return mChildren.at(index);
      } else {
        return nullptr;
      }
    }

    inline int getChildCount() {
      return mChildren.size();
    }

    inline void setRef(std::string ref) {
      mRef = ref;
    }

    inline std::string getRef() {
      return mRef;
    }

    inline void setType(std::string type) {
      mType = type;

      if (type.compare("list") == 0) {
        mLayoutNode->setFlex(1);
      }
    }

    inline std::string getType() {
      return mType;
    }

    inline WXCoreLayoutNode *getLayoutNode() {
      return mLayoutNode;
    }

    inline void setParentRender(RenderObject *render) {
      mParentRender = render;
    }

    inline RenderObject *getParentRender() {
      return mParentRender;
    }

    inline STYLES *getStyles() {
      return mStyles;
    }

    inline ATTRIBUTES *getAttributes() {
      return mAttributes;
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

    inline CHILD_LIST_IT getChildListItBegin() {
      return mChildren.begin();
    }

    inline CHILD_LIST_IT getChildListItEnd() {
      return mChildren.end();
    }

    void applyStyleToYGNode(std::string key, std::string value);

    void printRenderMsg();

    void printYGNodeMsg();
  };
} //end WeexCore
#endif //RenderObject_h
