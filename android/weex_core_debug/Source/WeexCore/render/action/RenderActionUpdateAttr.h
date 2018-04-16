#ifndef WEEX_PROJECT_UPDATEATTRACTION_H
#define WEEX_PROJECT_UPDATEATTRACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionUpdateAttr : public RenderAction {

  public:
    RenderActionUpdateAttr(const int &pageId, const int &ref,
                           std::vector<std::pair<std::string, std::string>> *mAttrs);

    void ExecuteAction();

  public:
    int mPageId;
    int mRef;
    std::vector<std::pair<std::string, std::string>> *mAttrs;
  };
}

#endif //WEEX_PROJECT_UPDATEATTRACTION_H
