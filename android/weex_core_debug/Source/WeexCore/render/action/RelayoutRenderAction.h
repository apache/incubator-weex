#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>

using namespace WeexCore;

namespace WeexCore {

  class RelayoutRenderAction : public RenderAction {

  public:

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callReLayoutByWeexCore(mPageId, mRef,
                                                           mTop, mBottom,
                                                           mLeft, mRight,
                                                           mHeight, mWidth);
    }

    void GenerateAction(std::string pageId, RenderObject *render) {
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      getLayoutInfo(render->getLayoutNode());
    }
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
