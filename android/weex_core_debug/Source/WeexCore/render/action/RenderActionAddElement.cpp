#include "RenderActionAddElement.h"

namespace WeexCore {

  RenderActionAddElement::RenderActionAddElement(std::string pageId, RenderObject *render,
                                                 RenderObject *parent, int index) {
    this->mAttributes = render->Attributes();
    this->mStyles = render->Styles();
    this->mEvents = render->Events();
    this->mPaddings = render->Paddings();
    this->mMargins = render->Margins();
    this->mBorders = render->Borders();
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
