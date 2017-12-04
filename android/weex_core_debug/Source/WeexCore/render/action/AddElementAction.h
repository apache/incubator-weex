#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class AddElementAction : public RenderAction {

  public:

    void ExecuteAction() {
      RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
      long long startTime = getCurrentTime();
      BridgeAndroid::getInstance()->callAddElementByWeexCore(mPageId, mComponentType, mRef,
                                                             mTop,
                                                             mBottom, mLeft,
                                                             mRight,
                                                             mHeight,
                                                             mWidth, mIndex,
                                                             mParentRef, mStyles, mAttributes,
                                                             mEvents,
                                                             mPaddings, mMargins, mBorders);
      if (page != nullptr)
        page->jniCallTime(getCurrentTime() - startTime);
    }

    void GenerateAction(std::string pageId, RenderObject *render, RenderObject *parent, int index) {
      this->mAttributes = render->getAttributes();
      this->mStyles = render->getStyles();
      this->mEvents = render->getEvents();
      this->mPaddings = render->getPaddings();
      this->mMargins = render->getMargins();
      this->mBorders = render->getBorders();
      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      this->mParentRef = parent->getRef();
      this->mIndex = index;
      getLayoutInfo(render);
    }
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
