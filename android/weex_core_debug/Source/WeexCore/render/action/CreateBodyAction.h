#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"

using namespace WeexCore;

namespace WeexCore {
  class CreateBodyAction : public RenderAction {

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callCreateBodyByWeexCore(mPageId, mComponentType, mRef,
                                                             mTop,
                                                             mBottom, mLeft,
                                                             mRight,
                                                             mHeight,
                                                             mWidth);
    }

  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
