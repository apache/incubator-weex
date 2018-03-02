#include "RenderActionRemoveElement.h"

namespace WeexCore {

  RenderActionRemoveElement::RenderActionRemoveElement(const std::string &pageId, const std::string &ref) {
    this->mPageId = pageId;
    this->mRef = ref;
  }

  void RenderActionRemoveElement::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callRemoveElement(mPageId, mRef);
    page->JniCallTime(getCurrentTime() - startTime);
  }
}