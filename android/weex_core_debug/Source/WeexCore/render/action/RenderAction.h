#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include "../../platform/android/BridgeAndroid.h"
#include <Layout/WXCoreLayout.h>
#include <string>

using namespace WeexCore;
using namespace WXCoreFlexLayout;

namespace WeexCore {

  class RenderObject;

  class RenderAction {
  public:
    STYLES *mStyles;
    ATTRIBUTES *mAttributes;
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

    void getLayoutInfo(WXCoreLayoutNode *node) {
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
