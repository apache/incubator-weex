#include "RenderObject.h"
#include "RenderPage.h"

namespace WeexCore {

  static void setDefaultStyle(YGNodeRef &ygNode, RenderObject &render) {
    YGNodeStyleSetFlexDirection(ygNode, YGFlexDirectionColumn);
    YGNodeStyleSetJustifyContent(ygNode, YGJustifyFlexStart);
    YGNodeStyleSetAlignItems(ygNode, YGAlignStretch);
    YGNodeStyleSetPositionType(ygNode, YGPositionTypeRelative);

    if (render.getType() == "list") {
      render.updateStyle(FLEX, "1");
    }
  }

  RenderObject::RenderObject(RenderPage *page)
      : mPage(page) {
    mStyle = new std::map<std::string, std::string>();
    mAttributes = new std::map<std::string, std::string>();
    mEvents = new std::set<std::string>();

    YGConfigRef config = YGConfigNew();
    mYGNode = YGNodeNewWithConfig(config);
    setDefaultStyle(mYGNode, *this);
  }

  RenderObject::~RenderObject() {
    mPage = nullptr;
    mParentRender = nullptr;
    mYGNode = nullptr;

    if (mStyle != nullptr) {
      delete mStyle;
      mStyle = nullptr;
    }
    if (mAttributes != nullptr) {
      delete mAttributes;
      mAttributes = nullptr;
    }
    if (mEvents != nullptr) {
      delete mEvents;
      mEvents = nullptr;
    }

    for (CHILD_LIST_IT it = getChildListItBegin();
         it != getChildListItEnd(); it++) {
      if (nullptr != *it) {
        delete *it;
        *it = nullptr;
      }
    }

    mChildren.clear();
  }

  void RenderObject::addRenderObject(int index, RenderObject *child) {
    // insert RenderObject child
//    mChildren.insert(mChildren.begin() + mChildren.size() - 1, child);
    mChildren.push_back(child);
  }

  void RenderObject::removeRenderObject(RenderObject *child) {
    for (CHILD_LIST_IT it = getChildListItBegin();
         it != getChildListItEnd(); it++) {
      if ((*it)->getRef() == child->getRef()) {
        mChildren.erase(it);
        break;
      }
    }
  }

  RenderObject *RenderObject::getChild(int index) {
    if (index < mChildren.size()) {
      return mChildren.at(index);
    } else {
      return NULL;
    }
  }

  void RenderObject::updateAttr(std::string key, std::string value) {
    if (mAttributes == nullptr) {
      mAttributes = new std::map<std::string, std::string>();
    }

    mAttributes->insert(pair<std::string, std::string>(key, value));
  }

  // TODO: to recognize layout style or not
  void RenderObject::updateStyle(std::string key, std::string value) {
    if (mStyle == nullptr) {
      mStyle = new std::map<std::string, std::string>();
    }

    mStyle->insert(pair<std::string, std::string>(key, value));
    applyStyleToYGNode(key, value);
  }

  void RenderObject::addEvent(std::string event) {
    if (mEvents == nullptr) {
      mEvents = new std::set<std::string>();
    }
    mEvents->insert(event);
  }

  void RenderObject::removeEvent(std::string event) {
    if (mEvents == nullptr) {
      return;
    }
    mEvents->erase(event);
  }

  void RenderObject::setLayout(int left, int top, int w, int h) {
    if (mLeft == left && mTop == top && mWidth == w && mHeight == h) {

      return;
    }

    mLeft = left;
    mTop = top;
    mWidth = w;
    mHeight = h;
  }

  void RenderObject::setRef(std::string ref) {
    mRef = ref;
  }

  std::string RenderObject::getRef() {
    return mRef;
  }

  void RenderObject::setType(std::string type) {
    mType = type;
  }

  std::string RenderObject::getType() {
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
  void RenderObject::applyStyleToYGNode(std::string key, std::string value) {
    if (mStyle == nullptr)
      return;

    if (key == ALIGN_ITEMS) {
      YGNodeStyleSetAlignItems(mYGNode, getYGAlignItem(value));
    } else if (key == ALIGN_SELF) {
      YGNodeStyleSetAlignSelf(mYGNode, getYGAlignSelf(value));
    } else if (key == FLEX) {
      YGNodeStyleSetFlex(mYGNode, stringToNum<float>(value));
    } else if (key == FLEX_DIRECTION) {
      YGNodeStyleSetFlexDirection(mYGNode, getYGFlexDirection(value));
    } else if (key == JUSTIFY_CONTENT) {
      YGNodeStyleSetJustifyContent(mYGNode, getYGJustifyContent(value));
    } else if (key == FLEX_WRAP) {
      YGNodeStyleSetFlexWrap(mYGNode, getYGWrap(value));
    } else if (key == MIN_WIDTH) {
      YGNodeStyleSetMinWidth(mYGNode, stringToNum<float>(value));
    } else if (key == MIN_HEIGHT) {
      YGNodeStyleSetMinHeight(mYGNode, stringToNum<float>(value));
    } else if (key == MAX_WIDTH) {
      YGNodeStyleSetMaxWidth(mYGNode, stringToNum<float>(value));
    } else if (key == MAX_HEIGHT) {
      YGNodeStyleSetMaxHeight(mYGNode, stringToNum<float>(value));
    } else if (key == OVERFLOW) {
      YGNodeStyleSetOverflow(mYGNode, getYGOverflow(value));
    } else if (key == DEFAULT_HEIGHT || key == HEIGHT) {
      YGNodeStyleSetHeight(mYGNode, stringToNum<float>(value));
    } else if (key == DEFAULT_WIDTH | key == WIDTH) {
      YGNodeStyleSetWidth(mYGNode, stringToNum<float>(value));
    } else if (key == POSITION) {
      YGNodeStyleSetPositionType(mYGNode, getYGPositionType(value));
    } else if (key == LEFT) {
      YGNodeStyleSetPosition(mYGNode, YGEdgeLeft, stringToNum<float>(value));
    } else if (key == TOP) {
      YGNodeStyleSetPosition(mYGNode, YGEdgeTop, stringToNum<float>(value));
    } else if (key == RIGHT) {
      YGNodeStyleSetPosition(mYGNode, YGEdgeRight, stringToNum<float>(value));
    } else if (key == BOTTOM) {
      YGNodeStyleSetPosition(mYGNode, YGEdgeBottom, stringToNum<float>(value));
    } else if (key == MARGIN) {
      YGNodeStyleSetMargin(mYGNode, YGEdgeAll, stringToNum<float>(value));
    } else if (key == MARGIN_LEFT) {
      YGNodeStyleSetMargin(mYGNode, YGEdgeLeft, stringToNum<float>(value));
    } else if (key == MARGIN_TOP) {
      YGNodeStyleSetMargin(mYGNode, YGEdgeTop, stringToNum<float>(value));
    } else if (key == MARGIN_RIGHT) {
      YGNodeStyleSetMargin(mYGNode, YGEdgeRight, stringToNum<float>(value));
    } else if (key == MARGIN_BOTTOM) {
      YGNodeStyleSetMargin(mYGNode, YGEdgeBottom, stringToNum<float>(value));
    } else if (key == BORDER_WIDTH) {
      YGNodeStyleSetBorder(mYGNode, YGEdgeAll, stringToNum<float>(value));
    } else if (key == BORDER_TOP_WIDTH) {
      YGNodeStyleSetBorder(mYGNode, YGEdgeTop, stringToNum<float>(value));
    } else if (key == BORDER_RIGHT_WIDTH) {
      YGNodeStyleSetBorder(mYGNode, YGEdgeRight, stringToNum<float>(value));
    } else if (key == BORDER_BOTTOM_WIDTH) {
      YGNodeStyleSetBorder(mYGNode, YGEdgeBottom, stringToNum<float>(value));
    } else if (key == BORDER_LEFT_WIDTH) {
      YGNodeStyleSetBorder(mYGNode, YGEdgeLeft, stringToNum<float>(value));
    } else if (key == PADDING) {
      YGNodeStyleSetPadding(mYGNode, YGEdgeAll, stringToNum<float>(value));
    } else if (key == PADDING_LEFT) {
      YGNodeStyleSetPadding(mYGNode, YGEdgeLeft, stringToNum<float>(value));
    } else if (key == PADDING_TOP) {
      YGNodeStyleSetPadding(mYGNode, YGEdgeTop, stringToNum<float>(value));
    } else if (key == PADDING_RIGHT) {
      YGNodeStyleSetPadding(mYGNode, YGEdgeRight, stringToNum<float>(value));
    } else if (key == PADDING_BOTTOM) {
      YGNodeStyleSetPadding(mYGNode, YGEdgeBottom, stringToNum<float>(value));
    } else {
    }
  }

  void RenderObject::setParentRender(RenderObject *render) {
    mParentRender = render;
  }

  RenderObject *RenderObject::getParentRender() {
    return mParentRender;
  }

  void RenderObject::printRenderMsg() {
    std::string result("ref: ");
    result.append(mRef);
    result.append("\n");

    result.append("type: ");
    result.append(mType);
    result.append("\n");


    result.append("attr:\n");

    ATTR_IT attr_it = getAttrItBegin();
    ATTR_IT attr_end = getAttrItEnd();
    for (; attr_it != attr_end; ++attr_it) {
      result.append("   ");
      result.append(attr_it->first);
      result.append(":");
      result.append(attr_it->second);
      result.append("\n");
    }


    result.append("style:\n");

    STYLE_IT style_it = getStyleItBegin();
    STYLE_IT style_end = getStyleItEnd();
    for (; style_it != style_end; ++style_it) {
      result.append("   ");
      result.append(style_it->first);
      result.append(":");
      result.append(style_it->second);
      result.append("\n");
    }


    if (mEvents != nullptr) {
      result.append("event:\n");

      EVENT_IT event_it = getEventItBegin();
      EVENT_IT event_end = getEventItEnd();

      for (; event_it != event_end; ++event_it) {
        result.append("   ");
        result.append(*event_it);
        result.append("\n");
      }
    }

    LOGE("[RenderObject::Render tree] \n %s", result.c_str());

    for (CHILD_LIST_IT it = getChildListItBegin();
         it != getChildListItEnd(); it++) {
      if (nullptr != *it) {
        (*it)->printRenderMsg();
      }
    }
  }

  void RenderObject::printYGNodeMsg() {
    LOGE("yoga ref: %s\n", mRef.c_str());
    LOGE("yoga type: %s\n", mType.c_str());
    YGNodePrint(mYGNode, YGPrintOptionsLayout);
    YGNodePrint(mYGNode, YGPrintOptionsStyle);
    LOGE("\n\n");

    for (CHILD_LIST_IT it = getChildListItBegin();
         it != getChildListItEnd(); it++) {
      if (nullptr != *it) {
        (*it)->printYGNodeMsg();
      }
    }
  }

  STYLE_IT RenderObject::getStyleItBegin() {
    return mStyle->begin();
  }

  STYLE_IT RenderObject::getStyleItEnd() {
    return mStyle->end();
  }

  ATTR_IT RenderObject::getAttrItBegin() {
    return mAttributes->begin();
  }

  ATTR_IT RenderObject::getAttrItEnd() {
    return mAttributes->end();
  }

  EVENT_IT RenderObject::getEventItBegin() {
    return mEvents->begin();
  }

  EVENT_IT RenderObject::getEventItEnd() {
    return mEvents->end();
  }

  CHILD_LIST_IT RenderObject::getChildListItBegin() {
    return mChildren.begin();
  }

  CHILD_LIST_IT RenderObject::getChildListItEnd() {
    return mChildren.end();
  }
} //end WeexCore
