#ifndef WEEX_CORE_RENDER_ACTION_APPENDTREE_CREATEFINISH_H
#define WEEX_CORE_RENDER_ACTION_APPENDTREE_CREATEFINISH_H

#include "render_action.h"

namespace WeexCore {

  class RenderActionAppendTreeCreateFinish : public render_action {

  public:
    RenderActionAppendTreeCreateFinish(const std::string &pageId, const std::string &ref);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
  };
}

#endif //WEEX_CORE_RENDER_ACTION_APPENDTREE_CREATEFINISH_H
