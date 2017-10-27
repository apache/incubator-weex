#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"

using namespace WeexCore;

namespace WeexCore {

  class RelayoutRenderAction : public RenderAction {

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callReLayoutByWeexCore(mPageId, mRef,
                                                           mTop, mBottom,
                                                           mLeft, mRight,
                                                           mHeight, mWidth);
    }
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
