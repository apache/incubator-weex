#include "render_action_layout.h"

namespace WeexCore {

  RenderActionLayout::RenderActionLayout(const std::string &pageId, const RenderObject *render,int index) {
    this->mPageId = pageId;
    this->mRef = render->Ref();
    this->index = index;
    GetLayoutInfo(render);
  }

  void RenderActionLayout::ExecuteAction() {
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callLayout(mPageId.c_str(), mRef.c_str(),
                                                   mTop, mBottom, mLeft, mRight,
                                                   mHeight, mWidth, index);
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
