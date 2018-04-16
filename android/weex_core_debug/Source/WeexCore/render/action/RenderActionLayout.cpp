#include "RenderActionLayout.h"

namespace WeexCore {

  RenderActionLayout::RenderActionLayout(const int &pageId, const RenderObject *render) {
    this->mPageId = pageId;
    this->mRef = render->Ref();
    GetLayoutInfo(render);
  }

  void RenderActionLayout::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callLayout(mPageId, mRef,
                                                   mTop, mBottom, mLeft, mRight,
                                                   mHeight, mWidth);
    page->JniCallTime(getCurrentTime() - startTime);
    page->LayoutActionJniTime(getCurrentTime() - startTime);
  }

  void RenderActionLayout::GetLayoutInfo(const WXCoreLayoutNode *node) {
    mTop = node->getLayoutPositionTop();
    mBottom = node->getLayoutPositionBottom();
    mRight = node->getLayoutPositionRight();
    mLeft = node->getLayoutPositionLeft();
    mHeight = node->getLayoutHeight();
    mWidth = node->getLayoutWidth();
  }
}
