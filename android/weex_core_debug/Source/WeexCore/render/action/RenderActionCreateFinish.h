#ifndef WEEX_PROJECT_CREATEFINISHACTION_H
#define WEEX_PROJECT_CREATEFINISHACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionCreateFinish : public RenderAction {

  public:
    RenderActionCreateFinish(std::string pageId);

    void ExecuteAction();

  public:
    std::string mPageId;
  };
}

#endif //WEEX_PROJECT_CREATEFINISHACTION_H
