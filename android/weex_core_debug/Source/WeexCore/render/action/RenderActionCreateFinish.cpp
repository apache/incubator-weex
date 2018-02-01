#include "RenderActionCreateFinish.h"

namespace WeexCore {

  RenderActionCreateFinish::RenderActionCreateFinish(std::string pageId) {
    this->mPageId = pageId;
  }

  void RenderActionCreateFinish::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callCreateFinish(mPageId);

    page->JniCallTime(getCurrentTime() - startTime);
  }
}
