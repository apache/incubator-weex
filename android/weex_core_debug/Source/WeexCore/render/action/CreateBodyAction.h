#ifndef WEEX_PROJECT_CREATEBODYACTION_H
#define WEEX_PROJECT_CREATEBODYACTION_H

#include "RenderAction.h"

namespace WeexCore {
  class CreateBodyAction : public RenderAction {

  public:

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callCreateBodyByWeexCore(mPageId, mComponentType, mRef,
                                                             mTop,
                                                             mBottom, mLeft,
                                                             mRight,
                                                             mHeight,
                                                             mWidth, mStyles, mAttributes,
                                                             mEvents,
                                                             mPaddings, mMargins, mBorders);
    }

    void GenerateAction(std::string pageId, RenderObject *render) {
      this->mAttributes = render->getAttributes();
      this->mStyles = render->getStyles();
      this->mEvents = render->getEvents();
      this->mPaddings = render->getPaddings();
      this->mMargins = render->getMargins();
      this->mBorders = render->getBorders();
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      getLayoutInfo(render);
    }
  };
}

#endif //WEEX_PROJECT_CREATEBODYACTION_H
