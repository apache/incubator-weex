#include "RenderActionRemoveElement.h"

namespace WeexCore {

  RenderActionRemoveElement::RenderActionRemoveElement(std::string pageId, std::string ref) {
    this->mPageId = pageId;
    this->mRef = ref;
  }

  void RenderActionRemoveElement::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    BridgeAndroid::getInstance()->callRemoveElement(mPageId, mRef);
    page->jniCallTime(getCurrentTime() - startTime);
  }
}