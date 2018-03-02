#include "RenderActionAddEvent.h"

namespace WeexCore {

  RenderActionAddEvent::RenderActionAddEvent(std::string pageId, std::string ref, std::string event) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mEvent = event;
  }

  void RenderActionAddEvent::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();

    Bridge_Impl_Android::getInstance()->callAddEvent(mPageId, mRef, mEvent);

    page->JniCallTime(getCurrentTime() - startTime);
    page->AddEventActionJNITime(getCurrentTime() - startTime);
  }

}
