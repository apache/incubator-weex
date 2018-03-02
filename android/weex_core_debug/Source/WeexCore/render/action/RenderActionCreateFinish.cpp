#include "RenderActionCreateFinish.h"

namespace WeexCore {

  RenderActionCreateFinish::RenderActionCreateFinish(const std::string &pageId) {
    this->mPageId = pageId;
  }

  void RenderActionCreateFinish::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callCreateFinish(mPageId);

    page->JniCallTime(getCurrentTime() - startTime);
  }
}
