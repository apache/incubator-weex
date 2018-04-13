#include "RenderActionCreateBody.h"

namespace WeexCore {

  RenderActionCreateBody::RenderActionCreateBody(const int &pageId, const RenderObject *render) {
    this->mAttributes = render->Attributes();
    this->mStyles = render->Styles();
    this->mEvents = render->Events();
    this->mMargins = render->GetMargins();
    this->mPaddings = render->GetPaddings();
    this->mBorders = render->GetBorders();
    this->mPageId = pageId;
    this->mComponentType = render->Type();
    this->mRef = render->Ref();
  }

  void RenderActionCreateBody::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callCreateBody(mPageId, mComponentType.c_str(), mRef.c_str(),
                                                       mStyles, mAttributes, mEvents,
                                                       mMargins, mPaddings, mBorders);
    page->JniCallTime(getCurrentTime() - startTime);
    page->AddElementActionJNITime(getCurrentTime() - startTime);
  }
}
