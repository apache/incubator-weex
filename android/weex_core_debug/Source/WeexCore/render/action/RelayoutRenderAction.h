#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"

class RelayoutRenderAction : public RenderAction {

  void ExecuteAction() {
    BridgeAndroid::getInstance()->callReLayoutByWeexCore(mPageId, mRef,
                                                         mPosition.mTop, mPosition.mBottom,
                                                         mPosition.mLeft, mPosition.mRight,
                                                         mRenderSize.mHeight, mRenderSize.mWidth);
  }
};

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
