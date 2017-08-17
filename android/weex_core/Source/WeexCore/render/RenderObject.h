#ifndef RenderObject_h
#define RenderObject_h

#include <wtf/text/WTFString.h>
#include <wtf/text/StringHash.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include "../../base/LogUtils.h"
#include "../../yoga/Yoga.h"
#include "../layout/ConstantsName.h"
#include "../layout/WXStyleUtil.h"
#include "../../base/StringUtils.h"
#include "action/RenderAction.h"

using namespace WTF;

namespace WeexCore
{
class RenderPage;

class RenderObject
{
public:
  typedef Vector<RenderObject *> ChildrenList;

private:
  String mRef = "";

  String mType = "";

  RenderPage *mPage;

  RenderObject *mParentRender;

  HashMap<String, String> *mStyle;

  HashMap<String, String> *mAtrributes;

  HashSet<String> *mEvents;

  ChildrenList mChildren;

  YGNodeRef mYGNode;

  int mLeft, mTop, mWidth, mHeight;

public:
  RenderObject(RenderPage *page);
  ~RenderObject();

  void addRenderObject(int index, RenderObject *child);

  void removeRenderObject(RenderObject* child);

  void updateAttr(String key, String value);

  void updateStyle(String key, String value);

  void addEvent(String event);

  void removeEvent(String event);

  RenderObject *getChild(int index);

  int getChildCount();

  void setRef(String ref);

  String getRef();

  void setType(String type);

  String getType();

  YGNodeRef getYGNode();

  void applyStyleToYGNode(String key, String value);

  void printRenderMsg();

  void printYGNodeMsg();

  void setParentRender(RenderObject* render);

  RenderObject* getParentRender();

private:
  void setLayout(int, int, int, int);
};
} //end WeexCore
#endif //RenderObject_h
