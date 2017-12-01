#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include <Layout/WXCoreLayout.h>
#include <WeexCore/render/RenderObject.h>
#include <WeexCore/platform/android/BridgeAndroid.h>
#include <string>

namespace WeexCore {

  class RenderAction {
  public:
    STYLES_MAP *mStyles;
    ATTRIBUTES_MAP *mAttributes;
    EVENTS_SET *mEvents;
    PADDING_MAP *mPaddings;
    MARGIN_MAP *mMargins;
    BORDER_MAP *mBorders;
    std::string mPageId;
    std::string mComponentType;
    std::string mParentRef;
    std::string mRef;
    std::string mKey;
    std::string mValue;
    int mIndex;
    float mWidth;
    float mHeight;
    float mTop;
    float mLeft;
    float mBottom;
    float mRight;

    virtual void ExecuteAction() {}

    void getLayoutInfo(WXCoreFlexLayout::WXCoreLayoutNode *node) {
      mTop = node->getLayoutPositionTop();
      mBottom = node->getLayoutPositionBottom();
      mRight = node->getLayoutPositionRight();
      mLeft = node->getLayoutPositionLeft();
      mHeight = node->getLayoutHeight();
      mWidth = node->getLayoutWidth();
    }
  };

}

#endif //WEEXV8_RENDERACTION_H
