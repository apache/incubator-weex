#include "RenderActionLayout.h"

namespace WeexCore {

  RenderActionLayout::RenderActionLayout(std::string pageId, RenderObject *render) {
    this->mPageId = pageId;
    this->mRef = render->getRef();
    getLayoutInfo(render);
  }

  void RenderActionLayout::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    BridgeAndroid::getInstance()->callLayoutByWeexCore(mPageId, mRef,
                                                       mTop, mBottom,
                                                       mLeft, mRight,
                                                       mHeight, mWidth);
    page->jniCallTime(getCurrentTime() - startTime);
    page->layoutActionJniTime(getCurrentTime() - startTime);
  }

  void RenderActionLayout::getLayoutInfo(WXCoreLayoutNode *node) {
    mTop = node->getLayoutPositionTop();
    mBottom = node->getLayoutPositionBottom();
    mRight = node->getLayoutPositionRight();
    mLeft = node->getLayoutPositionLeft();
    mHeight = node->getLayoutHeight();
    mWidth = node->getLayoutWidth();
  }
}
