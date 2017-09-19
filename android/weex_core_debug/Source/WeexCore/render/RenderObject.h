#ifndef RenderObject_h
#define RenderObject_h

#include <vector>
#include <string>
#include "./base/LogUtils.h"
#include <yoga/Yoga.h>
#include "../layout/ConstantsName.h"
#include "../layout/WXStyleUtil.h"
#include "./base/StringUtils.h"
#include "action/RenderAction.h"
#include <map>
#include <set>

namespace WeexCore {

  class RenderObject;

  class RenderPage;

  typedef std::vector<RenderObject *> ChildrenList;
  typedef std::map<std::string, std::string>::const_iterator STYLE_IT;
  typedef std::map<std::string, std::string>::const_iterator ATTR_IT;
  typedef std::set<std::string>::const_iterator EVENT_IT;
  typedef ChildrenList::iterator CHILD_LIST_IT;

  class RenderObject {
  public:

  private:
    std::string mRef = "";

    std::string mType = "";

    RenderPage *mPage;

    RenderObject *mParentRender;

    std::map<std::string, std::string> *mStyle;

    std::map<std::string, std::string> *mAttributes;

    std::set<std::string> *mEvents;

    ChildrenList mChildren;

    YGNodeRef mYGNode;

    int mLeft, mTop, mWidth, mHeight;

  public:
    RenderObject(RenderPage *page);

    ~RenderObject();

    void addRenderObject(int index, RenderObject *child);

    void removeRenderObject(RenderObject *child);

    void updateAttr(std::string key, std::string value);

    void updateStyle(std::string key, std::string value);

    void addEvent(std::string event);

    void removeEvent(std::string event);

    RenderObject *getChild(int index);

    int getChildCount();

    void setRef(std::string ref);

    std::string getRef();

    void setType(std::string type);

    std::string getType();

    YGNodeRef getYGNode();

    void applyStyleToYGNode(std::string key, std::string value);

    void printRenderMsg();

    void printYGNodeMsg();

    void setParentRender(RenderObject *render);

    RenderObject *getParentRender();

    STYLE_IT getStyleItBegin();

    STYLE_IT getStyleItEnd();

    ATTR_IT getAttrItBegin();

    ATTR_IT getAttrItEnd();

    EVENT_IT getEventItBegin();

    EVENT_IT getEventItEnd();

    CHILD_LIST_IT getChildListItBegin();

    CHILD_LIST_IT getChildListItEnd();

  private:
    void setLayout(int, int, int, int);
  };
} //end WeexCore
#endif //RenderObject_h
