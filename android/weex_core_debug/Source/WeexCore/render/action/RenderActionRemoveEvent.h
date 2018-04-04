#ifndef WEEX_PROJECT_REMOVEEVENTACTION_H
#define WEEX_PROJECT_REMOVEEVENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionRemoveEvent : public RenderAction {

  public:
      RenderActionRemoveEvent(const int &pageId, const std::string &ref, const std::string &event);

    void ExecuteAction();

  public:
    int mPageId;
    std::string mRef;
    std::string mEvent;
  };
}

#endif //WEEX_PROJECT_REMOVEEVENTACTION_H
