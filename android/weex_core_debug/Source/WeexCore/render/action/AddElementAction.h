#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"

class AddElementAction : public RenderAction {

  void ExecuteAction() {
    BridgeAndroid::getInstance()->callAddElementByWeexCore(mPageId, mComponentType, mRef,
                                                           mTop,
                                                           mBottom, mLeft,
                                                           mRight,
                                                           mHeight,
                                                           mWidth, mIndex,
                                                           mParentRef);
  }

};

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
