#include "RenderActionAddEvent.h"

namespace WeexCore {

  RenderActionAddEvent::RenderActionAddEvent(const std::string &pageId, const std::string &ref, const std::string &event, const std::string &callback) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mEvent = event;
    this->mCallback = callback;
  }

  void RenderActionAddEvent::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    long long startTime = getCurrentTime();

    Bridge_Impl_Android::getInstance()->callAddEvent(mPageId, mRef, mEvent, mCallback);

    page->JniCallTime(getCurrentTime() - startTime);
    page->AddEventActionJNITime(getCurrentTime() - startTime);
  }

}
