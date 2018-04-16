#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionLayout : public RenderAction {

  public:
    RenderActionLayout(const int &pageId, const RenderObject *render);

    void ExecuteAction();

    void GetLayoutInfo(const WXCoreLayoutNode *node);

  public:
    EventsSet *mEvents;
    int mPageId;
    int mRef;
    float mWidth;
    float mHeight;
    float mTop;
    float mLeft;
    float mBottom;
    float mRight;
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
