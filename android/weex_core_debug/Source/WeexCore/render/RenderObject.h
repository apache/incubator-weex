#ifndef RenderObject_h
#define RenderObject_h

#include <vector>
#include <string>
#include "./base/LogUtils.h"
#include "../../yoga/Yoga.h"
#include "../layout/ConstantsName.h"
#include "../layout/WXStyleUtil.h"
#include "./base/StringUtils.h"
#include "action/RenderAction.h"
#include <map>
#include <set>

namespace WeexCore {
  class RenderObject;

  class RenderPage;

  class RenderObject {
  public:
    typedef std::vector<RenderObject *> ChildrenList;

  private:
    std::string mRef = "";

    std::string mType = "";

    RenderPage *mPage;

    RenderObject *mParentRender;

    std::map<std::string, std::string> *mStyle;

    std::map<std::string, std::string> *mAtrributes;

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

  private:
    void setLayout(int, int, int, int);
  };
} //end WeexCore
#endif //RenderObject_h
