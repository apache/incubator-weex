#include "render_action_add_event.h"

namespace WeexCore {

  RenderActionAddEvent::RenderActionAddEvent(const std::string &pageId, const std::string &ref, const std::string &event) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mEvent = event;
  }

  void RenderActionAddEvent::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();

    Bridge_Impl_Android::getInstance()->callAddEvent(mPageId.c_str(), mRef.c_str(), mEvent.c_str());

    page->JniCallTime(getCurrentTime() - startTime);
    page->AddEventActionJNITime(getCurrentTime() - startTime);
  }

}
