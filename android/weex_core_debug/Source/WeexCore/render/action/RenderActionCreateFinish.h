#ifndef WEEX_PROJECT_CREATEFINISHACTION_H
#define WEEX_PROJECT_CREATEFINISHACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionCreateFinish : public RenderAction {

  public:

    std::string mPageId;

    RenderActionCreateFinish(std::string pageId);

    void ExecuteAction();
  };
}

#endif //WEEX_PROJECT_CREATEFINISHACTION_H
