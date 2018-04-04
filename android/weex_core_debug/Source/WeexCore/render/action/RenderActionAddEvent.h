#ifndef WEEX_PROJECT_ADDEVENTACTION_H
#define WEEX_PROJECT_ADDEVENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionAddEvent : public RenderAction {

  public:
    RenderActionAddEvent(const int &pageId, const std::string &ref, const std::string &event);

    void ExecuteAction();

  public:
    int mPageId;
    std::string mRef;
    std::string mEvent;
  };
}

#endif //WEEX_PROJECT_ADDEVENTACTION_H
