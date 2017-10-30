#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>

using namespace WeexCore;

namespace WeexCore {

  class AddElementAction : public RenderAction {

  public:

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callAddElementByWeexCore(mPageId, mComponentType, mRef,
                                                             mTop,
                                                             mBottom, mLeft,
                                                             mRight,
                                                             mHeight,
                                                             mWidth, mIndex,
                                                             mParentRef);
    }

    void GenerateAction(std::string pageId, RenderObject *render, RenderObject *parent, int index) {
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      this->mParentRef = parent->getRef();
      this->mIndex = index;
      getLayoutInfo(render->getLayoutNode());
    }
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
