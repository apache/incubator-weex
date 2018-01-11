#include "RenderActionCreateBody.h"

namespace WeexCore {

  RenderActionCreateBody::RenderActionCreateBody(const std::string &pageId, RenderObject *render) {
    this->mAttributes = render->Attributes();
    this->mStyles = render->Styles();
    this->mEvents = render->Events();
    this->mPaddings = render->Paddings();
    this->mMargins = render->Margins();
    this->mBorders = render->Borders();
    this->mPageId = pageId;
    this->mComponentType = render->Type();
    this->mRef = render->Ref();
  }

  void RenderActionCreateBody::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callCreateBody(mPageId, mComponentType, mRef,
                                                       mStyles, mAttributes,
                                                       mEvents,
                                                       mPaddings, mMargins, mBorders);
    page->JniCallTime(getCurrentTime() - startTime);
    page->AddElementActionJNITime(getCurrentTime() - startTime);
  }
}
