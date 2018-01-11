#ifndef WEEX_PROJECT_REMOVEEVENTACTION_H
#define WEEX_PROJECT_REMOVEEVENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionRemoveEvent : public RenderAction {

  public:
      RenderActionRemoveEvent(const std::string &pageId, const std::string &ref, const std::string &event, const std::string &callback);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
    std::string mEvent;
    std::string mCallback;
  };
}

#endif //WEEX_PROJECT_REMOVEEVENTACTION_H
