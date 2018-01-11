#ifndef WEEX_PROJECT_ADDEVENTACTION_H
#define WEEX_PROJECT_ADDEVENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionAddEvent : public RenderAction {

  public:
    RenderActionAddEvent(const std::string &pageId, const std::string &ref, const std::string &event, const std::string &callback);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
    std::string mEvent;
    std::string mCallback;
  };
}

#endif //WEEX_PROJECT_ADDEVENTACTION_H
