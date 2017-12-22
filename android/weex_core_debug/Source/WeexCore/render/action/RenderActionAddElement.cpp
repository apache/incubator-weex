#include "RenderActionAddElement.h"

namespace WeexCore {

  RenderActionAddElement::RenderActionAddElement(std::string pageId, RenderObject *render,
                                                 RenderObject *parent, int index) {
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
  }

  void RenderActionAddElement::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    BridgeAndroid::getInstance()->callAddElementByWeexCore(mPageId, mComponentType, mRef,
                                                           mIndex,
                                                           mParentRef, mStyles, mAttributes,
                                                           mEvents,
                                                           mPaddings, mMargins, mBorders);
    page->jniCallTime(getCurrentTime() - startTime);
    page->addElementActionJNITime(getCurrentTime() - startTime);
  }

}
