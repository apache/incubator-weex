#ifndef WEEX_PROJECT_CREATEFINISHACTION_H
#define WEEX_PROJECT_CREATEFINISHACTION_H

#include "render_action.h"

namespace WeexCore {

  class RenderActionCreateFinish : public render_action {

  public:
    RenderActionCreateFinish(const std::string &pageId);

    void ExecuteAction();

  public:
    std::string mPageId;
  };
}

#endif //WEEX_PROJECT_CREATEFINISHACTION_H
