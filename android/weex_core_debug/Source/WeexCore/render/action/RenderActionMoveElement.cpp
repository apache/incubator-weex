#include "RenderActionMoveElement.h"

namespace WeexCore {

  RenderActionMoveElement::RenderActionMoveElement(const std::string &pageId, const std::string &ref,
                                                   const std::string &parentRef, int index) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mParentRef = parentRef;
    this->mIndex = index;
  }

  void RenderActionMoveElement::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callMoveElement(mPageId.c_str(), mRef.c_str(), mParentRef.c_str(), mIndex);
    page->JniCallTime(getCurrentTime() - startTime);
  }
}