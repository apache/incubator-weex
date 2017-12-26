#include "RenderActionCreateBody.h"

namespace WeexCore {

  RenderActionCreateBody::RenderActionCreateBody(std::string pageId, RenderObject *render) {
    this->mAttributes = render->getAttributes();
    this->mStyles = render->getStyles();
    this->mEvents = render->getEvents();
    this->mPaddings = render->getPaddings();
    this->mMargins = render->getMargins();
    this->mBorders = render->getBorders();
    this->mPageId = pageId;
    this->mComponentType = render->getType();
    this->mRef = render->getRef();
  }

  void RenderActionCreateBody::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callCreateBodyByWeexCore(mPageId, mComponentType, mRef,
                                                           mStyles, mAttributes,
                                                           mEvents,
                                                           mPaddings, mMargins, mBorders);
    page->jniCallTime(getCurrentTime() - startTime);
    page->addElementActionJNITime(getCurrentTime() - startTime);
  }
}
