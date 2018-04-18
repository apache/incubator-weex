#ifndef WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H
#define WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionRemoveElement : public RenderAction {
  public:
    RenderActionRemoveElement(const std::string &pageId, const std::string &ref);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
  };
}

#endif //WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H
