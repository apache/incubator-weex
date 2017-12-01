#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class LayoutRenderAction : public RenderAction {

  public:

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callLayoutByWeexCore(mPageId, mRef,
                                                           mTop, mBottom,
                                                           mLeft, mRight,
                                                           mHeight, mWidth);
    }

    void GenerateAction(std::string pageId, RenderObject *render) {
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      getLayoutInfo(render);
    }
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
