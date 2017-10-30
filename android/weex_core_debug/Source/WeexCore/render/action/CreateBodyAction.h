#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>

using namespace WeexCore;

namespace WeexCore {
  class CreateBodyAction : public RenderAction {

  public:

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callCreateBodyByWeexCore(mPageId, mComponentType, mRef,
                                                             mTop,
                                                             mBottom, mLeft,
                                                             mRight,
                                                             mHeight,
                                                             mWidth);
    }

    void GenerateAction(std::string pageId, RenderObject *render) {
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      getLayoutInfo(render->getLayoutNode());
    }
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
