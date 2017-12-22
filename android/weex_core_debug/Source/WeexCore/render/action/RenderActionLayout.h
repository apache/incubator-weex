#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionLayout : public RenderAction {

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

    RenderActionLayout(std::string pageId, RenderObject *render);

    void ExecuteAction();

    void getLayoutInfo(WXCoreLayoutNode *node);
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
