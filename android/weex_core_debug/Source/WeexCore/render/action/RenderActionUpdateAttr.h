#ifndef WEEX_PROJECT_UPDATEATTRACTION_H
#define WEEX_PROJECT_UPDATEATTRACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionUpdateAttr : public RenderAction {

  public:

    std::string mPageId;
    std::string mRef;
    std::vector<std::pair<std::string, std::string> *> *mAttrs;


    RenderActionUpdateAttr(const std::string &pageId, const std::string &ref,
                           std::vector<std::pair<std::string, std::string> *> *mAttrs);

    void ExecuteAction();
  };
}

#endif //WEEX_PROJECT_UPDATEATTRACTION_H
