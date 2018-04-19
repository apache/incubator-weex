#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionLayout : public RenderAction {

  public:
    RenderActionLayout(const std::string &pageId, const RenderObject *render, const int index);

    void ExecuteAction();

    void GetLayoutInfo(const WXCoreLayoutNode *node);

  public:
    EventsSet *mEvents;
    std::string mPageId;
    std::string mRef;
    float mWidth;
    float mHeight;
    float mTop;
    float mLeft;
    float mBottom;
    float mRight;
    int index;
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
