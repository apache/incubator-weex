#ifndef WEEX_PROJECT_CREATEFINISHACTION_H
#define WEEX_PROJECT_CREATEFINISHACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionCreateFinish : public RenderAction {

  public:
    RenderActionCreateFinish(const int &pageId);

    void ExecuteAction();

  public:
    int mPageId;
  };
}

#endif //WEEX_PROJECT_CREATEFINISHACTION_H
