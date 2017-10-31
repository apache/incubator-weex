#include "RenderObject.h"
#include "RenderPage.h"

namespace WeexCore {

  RenderObject::RenderObject(RenderPage *page)
      : mPage(page) {
    mStyles = new STYLES();
    mAttributes = new ATTRIBUTES();
    mEvents = new EVENTS();
    mLayoutNode = WXCoreLayoutNode::newWXCoreNode();
  }

  RenderObject::~RenderObject() {

    mPage = nullptr;
    mParentRender = nullptr;

    if (mLayoutNode != nullptr) {
      delete mLayoutNode;
      mLayoutNode = nullptr;
    }

    if (mStyles != nullptr) {
      delete mStyles;
      mStyles = nullptr;
    }
    if (mAttributes != nullptr) {
      delete mAttributes;
      mAttributes = nullptr;
    }
    if (mEvents != nullptr) {
      delete mEvents;
      mEvents = nullptr;
    }

    mLayoutNode->freeWXCoreNode();

    for (CHILD_LIST_IT it = getChildListItBegin();
         it != getChildListItEnd(); it++) {
      if (nullptr != *it) {
        delete *it;
        *it = nullptr;
      }
    }
    mChildren.clear();
  }

/**
 * Synchronize the style of RenderObject to YGNode
 */
  void RenderObject::applyStyleToYGNode(std::string key, std::string value) {
    if (mStyles == nullptr)
      return;

    if (key == ALIGN_ITEMS) {
      mLayoutNode->setAlignItems(getWXCoreAlignItem(value));
    } else if (key == ALIGN_SELF) {
      mLayoutNode->setAlignSelf(getWXCoreAlignSelf(value));
    } else if (key == FLEX) {
      mLayoutNode->setFlex(stringToNum<float>(value));
    } else if (key == FLEX_DIRECTION) {
      mLayoutNode->setFlexDirection(getWXCoreFlexDirection(value));
    } else if (key == JUSTIFY_CONTENT) {
      mLayoutNode->setJustifyContent(getWXCoreJustifyContent(value));
    } else if (key == FLEX_WRAP) {
      mLayoutNode->setFlexWrap(getWXCoreFlexWrap(value));
    } else if (key == MIN_WIDTH) {
      mLayoutNode->setMinWidth(stringToNum<float>(value));
    } else if (key == MIN_HEIGHT) {
      mLayoutNode->setMinHeight(stringToNum<float>(value));
    } else if (key == MAX_WIDTH) {
      mLayoutNode->setMaxWidth(stringToNum<float>(value));
    } else if (key == MAX_HEIGHT) {
      mLayoutNode->setMaxHeight(stringToNum<float>(value));
    } else if (key == DEFAULT_HEIGHT || key == HEIGHT) {
      mLayoutNode->setStyleHeight(stringToNum<float>(value));
    } else if (key == DEFAULT_WIDTH | key == WIDTH) {
      mLayoutNode->setStyleWidth(stringToNum<float>(value));
    } else if (key == POSITION) {
      mLayoutNode->setStylePositionType(getWXCorePositionType(value));
    } else if (key == LEFT) {
      mLayoutNode->setStylePosition(WXCore_PositionEdge_Left, stringToNum<float>(value));
    } else if (key == TOP) {
      mLayoutNode->setStylePosition(WXCore_PositionEdge_Top, stringToNum<float>(value));
    } else if (key == RIGHT) {
      mLayoutNode->setStylePosition(WXCore_PositionEdge_Right, stringToNum<float>(value));
    } else if (key == BOTTOM) {
      mLayoutNode->setStylePosition(WXCore_PositionEdge_Bottom, stringToNum<float>(value));
    } else if (key == MARGIN) {
      mLayoutNode->setMargin(WXCore_Margin_ALL, stringToNum<float>(value));
    } else if (key == MARGIN_LEFT) {
      mLayoutNode->setMargin(WXCore_Margin_Left, stringToNum<float>(value));
    } else if (key == MARGIN_TOP) {
      mLayoutNode->setMargin(WXCore_Margin_Top, stringToNum<float>(value));
    } else if (key == MARGIN_RIGHT) {
      mLayoutNode->setMargin(WXCore_Margin_Right, stringToNum<float>(value));
    } else if (key == MARGIN_BOTTOM) {
      mLayoutNode->setMargin(WXCore_Margin_Bottom, stringToNum<float>(value));
    } else if (key == BORDER_WIDTH) {
      mLayoutNode->setBorderWidth(WXCore_Border_Width_ALL, stringToNum<float>(value));
    } else if (key == BORDER_TOP_WIDTH) {
      mLayoutNode->setBorderWidth(WXCore_Border_Width_Top, stringToNum<float>(value));
    } else if (key == BORDER_RIGHT_WIDTH) {
      mLayoutNode->setBorderWidth(WXCore_Border_Width_Right, stringToNum<float>(value));
    } else if (key == BORDER_BOTTOM_WIDTH) {
      mLayoutNode->setBorderWidth(WXCore_Border_Width_Bottom, stringToNum<float>(value));
    } else if (key == BORDER_LEFT_WIDTH) {
      mLayoutNode->setBorderWidth(WXCore_Border_Width_Left, stringToNum<float>(value));
    } else if (key == PADDING) {
      mLayoutNode->setPadding(WXCore_Padding_ALL, stringToNum<float>(value));
    } else if (key == PADDING_LEFT) {
      mLayoutNode->setPadding(WXCore_Padding_Left, stringToNum<float>(value));
    } else if (key == PADDING_TOP) {
      mLayoutNode->setPadding(WXCore_Padding_Top, stringToNum<float>(value));
    } else if (key == PADDING_RIGHT) {
      mLayoutNode->setPadding(WXCore_Padding_Right, stringToNum<float>(value));
    } else if (key == PADDING_BOTTOM) {
      mLayoutNode->setPadding(WXCore_Padding_Bottom, stringToNum<float>(value));
    } else {
    }
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

//    LOGE("[RenderObject::Render tree] \n %s", result.c_str());

    for (CHILD_LIST_IT it = getChildListItBegin();
         it != getChildListItEnd(); it++) {
      if (nullptr != *it) {
//        (*it)->printRenderMsg();
      }
    }
  }

  void RenderObject::printYGNodeMsg() {
//    LOGE("yoga ref: %s\n", mRef.c_str());
//    LOGE("yoga type: %s\n", mType.c_str());
//    YGNodePrint(mYGNode, YGPrintOptionsLayout);
//    YGNodePrint(mYGNode, YGPrintOptionsStyle);
//    LOGE("\n\n");

    for (CHILD_LIST_IT it = getChildListItBegin();
         it != getChildListItEnd(); it++) {
      if (nullptr != *it) {
//        (*it)->printYGNodeMsg();
      }
    }
  }
} //end WeexCore
