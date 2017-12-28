#include "RenderActionMoveElement.h"

namespace WeexCore {

  RenderActionMoveElement::RenderActionMoveElement(const std::string &pageId, const std::string &ref,
                                                   const std::string &parentRef, const int &index) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mParentRef = parentRef;
    this->mIndex = index;
  }

  void RenderActionMoveElement::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callMoveElement(mPageId, mRef,
                                                        mParentRef, mIndex);
    page->jniCallTime(getCurrentTime() - startTime);
  }
}