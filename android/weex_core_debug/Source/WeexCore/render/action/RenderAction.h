#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include "../../platform/android/BridgeAndroid.h"

using namespace WeexCore;

namespace WeexCore {

  class RenderAction {
  public:
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

  };

}

#endif //WEEXV8_RENDERACTION_H
