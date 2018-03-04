#include "RenderActionAddElement.h"

namespace WeexCore {

  RenderActionAddElement::RenderActionAddElement(const std::string &pageId, const RenderObject *render,
                                                 const RenderObject *parent, int index) {
    this->mAttributes = render->Attributes();
    this->mStyles = render->Styles();
    this->mEvents = render->Events();
    this->mPaddings = render->GetPadding();
    this->mMargins = render->GetMargin();
    this->mBorders = render->GetBorderWidth();
    this->mPageId = pageId;
    this->mComponentType = render->Type();
    this->mRef = render->Ref();
    this->mParentRef = parent->Ref();
    this->mIndex = index;
  }

  void RenderActionAddElement::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callAddElement(mPageId, mComponentType, mRef,
                                                       mIndex,
                                                       mParentRef, mStyles, mAttributes,
                                                       mEvents,
                                                       mPaddings, mMargins, mBorders);
    page->JniCallTime(getCurrentTime() - startTime);
    page->AddElementActionJNITime(getCurrentTime() - startTime);
  }

}
