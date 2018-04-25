#ifndef WEEX_PROJECT_REMOVEEVENTACTION_H
#define WEEX_PROJECT_REMOVEEVENTACTION_H

#include "render_action.h"

namespace WeexCore {

  class RenderActionRemoveEvent : public render_action {

  public:
      RenderActionRemoveEvent(const std::string &pageId, const std::string &ref, const std::string &event);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
    std::string mEvent;
  };
}

#endif //WEEX_PROJECT_REMOVEEVENTACTION_H
