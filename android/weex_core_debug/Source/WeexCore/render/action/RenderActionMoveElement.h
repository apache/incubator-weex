#ifndef WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
#define WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionMoveElement : public RenderAction {

  public:
    RenderActionMoveElement(const int &pageId, const std::string &ref,
                            const std::string &parentRef,
                            int index);

    void ExecuteAction();

  public:
    int mPageId;
    std::string mRef;
    std::string mParentRef;
    int mIndex;
  };
}

#endif //WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
