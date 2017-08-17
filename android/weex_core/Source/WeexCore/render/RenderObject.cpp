#include "RenderObject.h"
#include "RenderPage.h"

using namespace WTF;

namespace WeexCore {
typedef typename HashMap<String, String>::const_iterator::Keys iterator_keys;

static void setDefaultStyle(YGNodeRef ygNode) {
  YGNodeStyleSetFlexDirection(ygNode, YGFlexDirectionColumn);
  YGNodeStyleSetJustifyContent(ygNode, YGJustifyFlexStart);
  YGNodeStyleSetAlignItems(ygNode, YGAlignStretch);
  YGNodeStyleSetPositionType(ygNode, YGPositionTypeRelative);
}

RenderObject::RenderObject(RenderPage *page)
        : mPage(page) {
  mStyle = new HashMap<String, String>();
  mAtrributes = new HashMap<String, String>();
  mEvents = new HashSet<String>();

  YGConfigRef config = YGConfigNew();
  mYGNode = YGNodeNewWithConfig(config);
  setDefaultStyle(mYGNode);
}

RenderObject::~RenderObject() {
  LOGD("[~RenderObject] delete ref : %s，have %d child, parentRef: %s", mRef.utf8().data(), mChildren.size(),
       mParentRender->getRef().utf8().data());
  mPage = NULL;
  mParentRender = NULL;
  mYGNode = NULL;

  if (mStyle != NULL) {
    delete mStyle;
    mStyle = NULL;
  }
  if (mAtrributes != NULL) {
    delete mAtrributes;
    mAtrributes = NULL;
  }
  if (mEvents != NULL) {
    delete mEvents;
    mEvents = NULL;
  }

  for (Vector<RenderObject *>::iterator it = mChildren.begin(); it != mChildren.end(); it++) {
    if (NULL != *it) {
      delete *it;
      *it = NULL;
    }
  }

  mChildren.clear();
}

void RenderObject::addRenderObject(int index, RenderObject *child) {
  LOGD("[RenderObject::addRenderObject] index: %d, parent ref: %s, child ref: %s", index, mRef.utf8().data(),
       child->getRef().utf8().data());
  // insert RenderObject child
  mChildren.insert(mChildren.size(), child);

  //TODO: create RenderAction info
  mPage->addRenderAction(new RenderAction());
  LOGD("[RenderObject::addRenderObject] call succeed");
}

void RenderObject::removeRenderObject(RenderObject *child) {
  for (unsigned i = 0; i < mChildren.size(); i++) {
    if (mChildren[i] != NULL && mChildren[i]->getRef() == child->getRef()) {
      LOGD("[RenderObject::removeRenderObject] parent ref: %s ,remove child ref: %s, index: %d",
           mRef.utf8().data(), mChildren[i]->getRef().utf8().data(), i);
      mChildren.remove(i);
      break;
    }
  }

  //TODO: create RenderAction info
  mPage->addRenderAction(new RenderAction());
}

RenderObject *RenderObject::getChild(int index) {
  if (index < mChildren.size()) {
    return mChildren[index];
  } else {
    return NULL;
  }
}

void RenderObject::updateAttr(String key, String value) {
  if (mAtrributes == NULL) {
    mAtrributes = new HashMap<String, String>();
  }

  mAtrributes->set(key, value);
  //
  // RenderAction action = new RenderAction()
  //TODO: create RenderAction info
  mPage->addRenderAction(new RenderAction());
}

// TODO: to recognize layout style or not
void RenderObject::updateStyle(String key, String value) {
  if (mStyle == NULL) {
    mStyle = new HashMap<String, String>();
  }

  mStyle->set(key, value);
  applyStyleToYGNode(key, value);

  //TODO: create RenderAction info
  mPage->addRenderAction(new RenderAction());
}

void RenderObject::addEvent(String event) {
  if (mEvents == NULL) {
    mEvents = new HashSet<String>();
  }
  mEvents->add(event);

  //TODO: create RenderAction info
  mPage->addRenderAction(new RenderAction());
}

void RenderObject::removeEvent(String event) {
  if (mEvents == NULL) {
    return;
  }
  mEvents->remove(event);

  //TODO: create RenderAction info
  mPage->addRenderAction(new RenderAction());
}

void RenderObject::setLayout(int left, int top, int w, int h) {
  if (mLeft == left && mTop == top && mWidth == w && mHeight == h) {

    return;
  }

  //TODO RenderAction
  mPage->addRenderAction(new RenderAction());

  mLeft = left;
  mTop = top;
  mWidth = w;
  mHeight = h;
}

void RenderObject::setRef(String ref) {
  mRef = ref;
}

String RenderObject::getRef() {
  return mRef;
}

void RenderObject::setType(String type) {
  mType = type;
}

String RenderObject::getType() {
  return mType;
}

int RenderObject::getChildCount() {
  return mChildren.size();
}

YGNodeRef RenderObject::getYGNode() {
  return mYGNode;
}

/**
* Synchronize the style of RenderObject to YGNode
*/
void RenderObject::applyStyleToYGNode(String key, String value) {
  if (mStyle == NULL)
    return;

  if (key == ALIGN_ITEMS) {
    YGNodeStyleSetAlignItems(mYGNode, getYGAlignItem(value));
  } else if (key == ALIGN_SELF) {
    YGNodeStyleSetAlignSelf(mYGNode, getYGAlignSelf(value));
  } else if (key == FLEX) {
    YGNodeStyleSetFlex(mYGNode, value.toFloat());
  } else if (key == FLEX_DIRECTION) {
    YGNodeStyleSetFlexDirection(mYGNode, getYGFlexDirection(value));
  } else if (key == JUSTIFY_CONTENT) {
    YGNodeStyleSetJustifyContent(mYGNode, getYGJustifyContent(value));
  } else if (key == FLEX_WRAP) {
    YGNodeStyleSetFlexWrap(mYGNode, getYGWrap(value));
  } else if (key == MIN_WIDTH) {
    YGNodeStyleSetMinWidth(mYGNode, value.toFloat());
  } else if (key == MIN_HEIGHT) {
    YGNodeStyleSetMinHeight(mYGNode, value.toFloat());
  } else if (key == MAX_WIDTH) {
    YGNodeStyleSetMaxWidth(mYGNode, value.toFloat());
  } else if (key == MAX_HEIGHT) {
    YGNodeStyleSetMaxHeight(mYGNode, value.toFloat());
  } else if (key == OVERFLOW) {
    YGNodeStyleSetOverflow(mYGNode, getYGOverflow(value));
  } else if (key == DEFAULT_HEIGHT || key == HEIGHT) {
    YGNodeStyleSetHeight(mYGNode, value.toFloat());
  } else if (key == DEFAULT_WIDTH | key == WIDTH) {
    YGNodeStyleSetWidth(mYGNode, value.toFloat());
  } else if (key == POSITION) {
    YGNodeStyleSetPositionType(mYGNode, getYGPositionType(value));
  } else if (key == LEFT) {
    YGNodeStyleSetPosition(mYGNode, YGEdgeLeft, value.toFloat());
  } else if (key == TOP) {
    YGNodeStyleSetPosition(mYGNode, YGEdgeTop, value.toFloat());
  } else if (key == RIGHT) {
    YGNodeStyleSetPosition(mYGNode, YGEdgeRight, value.toFloat());
  } else if (key == BOTTOM) {
    YGNodeStyleSetPosition(mYGNode, YGEdgeBottom, value.toFloat());
  } else if (key == MARGIN) {
    YGNodeStyleSetMargin(mYGNode, YGEdgeAll, value.toFloat());
  } else if (key == MARGIN_LEFT) {
    YGNodeStyleSetMargin(mYGNode, YGEdgeLeft, value.toFloat());
  } else if (key == MARGIN_TOP) {
    YGNodeStyleSetMargin(mYGNode, YGEdgeTop, value.toFloat());
  } else if (key == MARGIN_RIGHT) {
    YGNodeStyleSetMargin(mYGNode, YGEdgeRight, value.toFloat());
  } else if (key == MARGIN_BOTTOM) {
    YGNodeStyleSetMargin(mYGNode, YGEdgeBottom, value.toFloat());
  } else if (key == BORDER_WIDTH) {
    YGNodeStyleSetBorder(mYGNode, YGEdgeAll, value.toFloat());
  } else if (key == BORDER_TOP_WIDTH) {
    YGNodeStyleSetBorder(mYGNode, YGEdgeTop, value.toFloat());
  } else if (key == BORDER_RIGHT_WIDTH) {
    YGNodeStyleSetBorder(mYGNode, YGEdgeRight, value.toFloat());
  } else if (key == BORDER_BOTTOM_WIDTH) {
    YGNodeStyleSetBorder(mYGNode, YGEdgeBottom, value.toFloat());
  } else if (key == BORDER_LEFT_WIDTH) {
    YGNodeStyleSetBorder(mYGNode, YGEdgeLeft, value.toFloat());
  } else if (key == PADDING) {
    YGNodeStyleSetPadding(mYGNode, YGEdgeAll, value.toFloat());
  } else if (key == PADDING_LEFT) {
    YGNodeStyleSetPadding(mYGNode, YGEdgeLeft, value.toFloat());
  } else if (key == PADDING_TOP) {
    YGNodeStyleSetPadding(mYGNode, YGEdgeTop, value.toFloat());
  } else if (key == PADDING_RIGHT) {
    YGNodeStyleSetPadding(mYGNode, YGEdgeRight, value.toFloat());
  } else if (key == PADDING_BOTTOM) {
    YGNodeStyleSetPadding(mYGNode, YGEdgeBottom, value.toFloat());
  } else {
    LOGD("[RenderObject::applyStyleToYGNode] [else] %s: %s", wtfString2cstr(key), wtfString2cstr(value));
  }

  LOGD("[RenderObject::applyStyleToYGNode] %s: %s", wtfString2cstr(key), wtfString2cstr(value));
}

void RenderObject::setParentRender(RenderObject *render) {
  mParentRender = render;
}

RenderObject *RenderObject::getParentRender() {
  return mParentRender;
}

void RenderObject::printRenderMsg() {
  String result("ref: ");
  result.append(mRef);
  result.append("\n");

  result.append("type: ");
  result.append(mType);
  result.append("\n");


  result.append("attr:\n");

  iterator_keys attr_it = mAtrributes->begin().keys();
  iterator_keys attr_end = mAtrributes->end().keys();
  for (; attr_it != attr_end; ++attr_it) {
    result.append("   ");
    result.append(*attr_it);
    result.append(":");
    result.append(mAtrributes->get(*attr_it));
    result.append("\n");
  }


  result.append("style:\n");

  iterator_keys style_it = mStyle->begin().keys();
  iterator_keys style_end = mStyle->end().keys();
  for (; style_it != style_end; ++style_it) {
    result.append("   ");
    result.append(*style_it);
    result.append(":");
    result.append(mStyle->get(*style_it));
    result.append("\n");
  }


  if (mEvents != NULL) {
    result.append("event:\n");

    HashSet<String>::iterator event_it = mEvents->begin();
    HashSet<String>::iterator event_end = mEvents->end();

    for (; event_it != event_end; ++event_it) {
      result.append("   ");
      result.append(mEvents->take(event_it));
      result.append("\n");
    }
  }

  LOGD("[RenderObject::Render tree] \n %s", wtfString2cstr(result));

  for (unsigned i = 0; i < mChildren.size(); i++) {
    getChild(i)->printRenderMsg();
  }
}

void RenderObject::printYGNodeMsg() {
  LOGD("yoga ref: %s\n", wtfString2cstr(mRef));
  LOGD("yoga type: %s\n", wtfString2cstr(mType));
  YGNodePrint(mYGNode, YGPrintOptionsLayout);
  YGNodePrint(mYGNode, YGPrintOptionsStyle);
  LOGD("\n\n");

  for (unsigned i = 0; i < mChildren.size(); i++) {
    getChild(i)->printYGNodeMsg();
  }
}
} //end WeexCore
