#ifndef WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
#define WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionMoveElement : public RenderAction {

  public:
    RenderActionMoveElement(const int &pageId, const int &ref,
                            const int &parentRef, int index);

    void ExecuteAction();

  public:
    int mPageId;
    int mRef;
    int mParentRef;
    int mIndex;
  };
}

#endif //WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
