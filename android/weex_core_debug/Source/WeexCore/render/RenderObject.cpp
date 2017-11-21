#include "RenderObject.h"
#include "RenderPage.h"

namespace WeexCore {

  RenderObject::RenderObject(RenderPage *page)
      : mPage(page) {
    mStyles = new STYLES_MAP();
    mAttributes = new ATTRIBUTES_MAP();
    mPaddings = new PADDING_MAP();
    mMargins = new MARGIN_MAP();
    mBorders = new BORDER_MAP();
    mEvents = new EVENTS_SET();
  }

  RenderObject::~RenderObject() {

    mPage = nullptr;
    mParentRender = nullptr;

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

    freeWXCoreNode();

    for (int i = 0; i < getChildCount(); i++) {
      RenderObject* render = getChild(i);
      if (nullptr != render) {
        delete render;
        render = nullptr;
      }
    }
  }

/**
 * Synchronize the style of RenderObject to LayoutNode
 */
  void RenderObject::applyStyle(std::string key, std::string value) {
    if (key == ALIGN_ITEMS) {
      setAlignItems(getWXCoreAlignItem(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == ALIGN_SELF) {
      setAlignSelf(getWXCoreAlignSelf(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == FLEX) {
      setFlex(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == FLEX_DIRECTION) {
      setFlexDirection(getWXCoreFlexDirection(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == JUSTIFY_CONTENT) {
      setJustifyContent(getWXCoreJustifyContent(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == FLEX_WRAP) {
      setFlexWrap(getWXCoreFlexWrap(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MIN_WIDTH) {
      setMinWidth(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MIN_HEIGHT) {
      setMinHeight(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MAX_WIDTH) {
      setMaxWidth(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MAX_HEIGHT) {
      setMaxHeight(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == DEFAULT_HEIGHT || key == HEIGHT) {
      setStyleHeight(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == DEFAULT_WIDTH | key == WIDTH) {
      setStyleWidth(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == POSITION) {
      setStylePositionType(getWXCorePositionType(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == LEFT) {
      setStylePosition(WXCore_PositionEdge_Left, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == TOP) {
      setStylePosition(WXCore_PositionEdge_Top, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == RIGHT) {
      setStylePosition(WXCore_PositionEdge_Right, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == BOTTOM) {
      setStylePosition(WXCore_PositionEdge_Bottom, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN) {
      setMargin(WXCore_Margin_ALL, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_LEFT) {
      setMargin(WXCore_Margin_Left, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_TOP) {
      setMargin(WXCore_Margin_Top, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_RIGHT) {
      setMargin(WXCore_Margin_Right, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_BOTTOM) {
      setMargin(WXCore_Margin_Bottom, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_WIDTH) {
      setBorderWidth(WXCore_Border_Width_ALL, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_TOP_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Top, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_RIGHT_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Right, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_BOTTOM_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Bottom, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_LEFT_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Left, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING) {
      setPadding(WXCore_Padding_ALL, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_LEFT) {
      setPadding(WXCore_Padding_Left, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_TOP) {
      setPadding(WXCore_Padding_Top, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_RIGHT) {
      setPadding(WXCore_Padding_Right, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_BOTTOM) {
      setPadding(WXCore_Padding_Bottom, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else {
      mStyles->insert(pair<std::string, std::string>(key, value));
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

//    for (CHILD_LIST_IT it = getChildListItBegin();
//         it != getChildListItEnd(); it++) {
//      if (nullptr != *it) {
//        (*it)->printRenderMsg();
//      }
//    }
  }

  void RenderObject::printYGNodeMsg() {
//    LOGE("yoga ref: %s\n", mRef.c_str());
//    LOGE("yoga type: %s\n", mType.c_str());
//    YGNodePrint(mYGNode, YGPrintOptionsLayout);
//    YGNodePrint(mYGNode, YGPrintOptionsStyle);
//    LOGE("\n\n");

//    for (CHILD_LIST_IT it = getChildListItBegin();
//         it != getChildListItEnd(); it++) {
//      if (nullptr != *it) {
//        (*it)->printYGNodeMsg();
//      }
//    }
  }
} //end WeexCore
