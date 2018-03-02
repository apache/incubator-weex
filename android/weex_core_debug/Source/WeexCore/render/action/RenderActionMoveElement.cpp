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
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callMoveElement(mPageId, mRef,
                                                        mParentRef, mIndex);
    page->JniCallTime(getCurrentTime() - startTime);
  }
}