#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class LayoutRenderAction : public RenderAction {

  public:

    EVENTS_SET *mEvents;
    std::string mPageId;
    std::string mRef;
    float mWidth;
    float mHeight;
    float mTop;
    float mLeft;
    float mBottom;
    float mRight;

    void ExecuteAction() {
      RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
      long long startTime = getCurrentTime();
      BridgeAndroid::getInstance()->callLayoutByWeexCore(mPageId, mRef,
                                                         mTop, mBottom,
                                                         mLeft, mRight,
                                                         mHeight, mWidth);
      page->jniCallTime(getCurrentTime() - startTime);
      page->layoutActionJniTime(getCurrentTime() - startTime);
    }

    void getLayoutInfo(WXCoreLayoutNode *node) {
      mTop = node->getLayoutPositionTop();
      mBottom = node->getLayoutPositionBottom();
      mRight = node->getLayoutPositionRight();
      mLeft = node->getLayoutPositionLeft();
      mHeight = node->getLayoutHeight();
      mWidth = node->getLayoutWidth();
    }

    void GenerateAction(std::string pageId, RenderObject *render) {
      this->mPageId = pageId;
      this->mRef = render->getRef();
      getLayoutInfo(render);
    }
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
