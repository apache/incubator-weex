#ifndef WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H
#define WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionRemoveElement : public RenderAction {
  public:

    std::string mPageId;
    std::string mRef;

    RenderActionRemoveElement(std::string pageId, std::string ref);

    void ExecuteAction();
  };
}

#endif //WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H
