#include "RenderActionRemoveEvent.h"

namespace WeexCore {

  RenderActionRemoveEvent::RenderActionRemoveEvent(const std::string &pageId, const std::string &ref, const std::string &event, const std::string &callback) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mEvent = event;
    this->mCallback = callback;
  }

  void RenderActionRemoveEvent::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    long long startTime = getCurrentTime();

    Bridge_Impl_Android::getInstance()->callRemoveEvent(mPageId, mRef, mEvent, mCallback);

    page->JniCallTime(getCurrentTime() - startTime);
    page->RemoveEventActionJNITime(getCurrentTime() - startTime);
  }

}
