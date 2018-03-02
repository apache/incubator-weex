#include "RenderActionRemoveEvent.h"

namespace WeexCore {

  RenderActionRemoveEvent::RenderActionRemoveEvent(const std::string &pageId, const std::string &ref, const std::string &event) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mEvent = event;
  }

  void RenderActionRemoveEvent::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();

    Bridge_Impl_Android::getInstance()->callRemoveEvent(mPageId, mRef, mEvent);

    page->JniCallTime(getCurrentTime() - startTime);
    page->RemoveEventActionJNITime(getCurrentTime() - startTime);
  }

}
