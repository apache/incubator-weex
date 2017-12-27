#include "RenderActionMoveElement.h"

namespace WeexCore {

  RenderActionMoveElement::RenderActionMoveElement(std::string pageId, std::string ref,
                                                   std::string parentRef, int index) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mParentRef = parentRef;
    this->mIndex = index;
  }

  void RenderActionMoveElement::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    BridgeAndroid::getInstance()->callMoveElement(mPageId, mRef,
                                                  mParentRef, mIndex);
    page->jniCallTime(getCurrentTime() - startTime);
  }
}