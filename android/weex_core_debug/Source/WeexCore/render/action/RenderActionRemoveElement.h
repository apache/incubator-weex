#ifndef WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H
#define WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionRemoveElement : public RenderAction {
  public:
    RenderActionRemoveElement(const int &pageId, const int &ref);

    void ExecuteAction();

  public:
    int mPageId;
    int mRef;
  };
}

#endif //WEEX_PROJECT_RENDERACTIONREMOVEELEMENT_H
