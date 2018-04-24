#ifndef WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
#define WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H

#include "render_action.h"

namespace WeexCore {

  class RenderActionMoveElement : public render_action {

  public:
    RenderActionMoveElement(const std::string &pageId, const std::string &ref,
                            const std::string &parentRef, int index);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
    std::string mParentRef;
    int mIndex;
  };
}

#endif //WEEX_PROJECT_RENDERACTIONMOVEELEMENT_H
