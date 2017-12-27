#ifndef WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
#define WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionMoveElement : public RenderAction {

  public:

    std::string mPageId;
    std::string mRef;
    std::string mParentRef;
    int mIndex;

    RenderActionMoveElement(std::string pageId, std::string ref, std::string parentRef,
                            int index);

    void ExecuteAction();
  };
}

#endif //WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
