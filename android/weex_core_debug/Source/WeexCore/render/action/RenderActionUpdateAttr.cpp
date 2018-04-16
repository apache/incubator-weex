#include "RenderActionUpdateAttr.h"

namespace WeexCore {

  RenderActionUpdateAttr::RenderActionUpdateAttr(const int &pageId, const int &ref,
                                                 std::vector<std::pair<std::string, std::string>> *mAttrs) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mAttrs = mAttrs;
  }

  void RenderActionUpdateAttr::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callUpdateAttr(mPageId, mRef, mAttrs);
    if (page != nullptr)
      page->JniCallTime(getCurrentTime() - startTime);
  }
}
