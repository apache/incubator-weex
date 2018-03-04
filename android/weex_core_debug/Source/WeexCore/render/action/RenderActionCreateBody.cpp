#include "RenderActionCreateBody.h"

namespace WeexCore {

  RenderActionCreateBody::RenderActionCreateBody(const std::string &pageId, const RenderObject *render) {
    this->mAttributes = render->Attributes();
    this->mStyles = render->Styles();
    this->mEvents = render->Events();
    this->mPaddings = render->GetPadding();
    this->mMargins = render->GetMargin();
    this->mBorders = render->GetBorderWidth();
    this->mPageId = pageId;
    this->mComponentType = render->Type();
    this->mRef = render->Ref();
  }

  void RenderActionCreateBody::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callCreateBody(mPageId, mComponentType, mRef,
                                                       mStyles, mAttributes,
                                                       mEvents,
                                                       mPaddings, mMargins, mBorders);
    page->JniCallTime(getCurrentTime() - startTime);
    page->AddElementActionJNITime(getCurrentTime() - startTime);
  }
}
