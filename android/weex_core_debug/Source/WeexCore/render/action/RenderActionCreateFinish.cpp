#include "RenderActionCreateFinish.h"

namespace WeexCore {

  RenderActionCreateFinish::RenderActionCreateFinish(std::string pageId) {
    this->mPageId = pageId;
  }

  void RenderActionCreateFinish::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callCreateFinishByWeexCore(mPageId);

    page->jniCallTime(getCurrentTime() - startTime);
  }
}
