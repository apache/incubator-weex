#ifndef WEEX_PROJECT_REMOVEEVENTACTION_H
#define WEEX_PROJECT_REMOVEEVENTACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionRemoveEvent : public RenderAction {

  public:
      RenderActionRemoveEvent(const int &pageId, const int &ref, const std::string &event);

    void ExecuteAction();

  public:
    int mPageId;
    int mRef;
    std::string mEvent;
  };
}

#endif //WEEX_PROJECT_REMOVEEVENTACTION_H
