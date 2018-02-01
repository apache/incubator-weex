#ifndef WEEX_PROJECT_ADDEVENTACTION_H
#define WEEX_PROJECT_ADDEVENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionAddEvent : public RenderAction {

  public:
    RenderActionAddEvent(std::string pageId, std::string ref, std::string event);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
    std::string mEvent;
  };
}

#endif //WEEX_PROJECT_ADDEVENTACTION_H
