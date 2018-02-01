#ifndef WEEX_PROJECT_REMOVEEVENTACTION_H
#define WEEX_PROJECT_REMOVEEVENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionRemoveEvent : public RenderAction {

  public:
      RenderActionRemoveEvent(std::string pageId, std::string ref, std::string event);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
    std::string mEvent;
  };
}

#endif //WEEX_PROJECT_REMOVEEVENTACTION_H
