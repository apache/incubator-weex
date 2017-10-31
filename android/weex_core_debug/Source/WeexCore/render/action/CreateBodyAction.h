#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"
#include <WeexCore/render/RenderObject.h>
#include <map>

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
                                                             mWidth, mStyles, mAttributes);
    }

    void GenerateAction(std::string pageId, RenderObject *render) {
      this->mAttributes = render->getAttributes();
      this->mStyles = render->getStyles();
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      getLayoutInfo(render->getLayoutNode());
    }
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
