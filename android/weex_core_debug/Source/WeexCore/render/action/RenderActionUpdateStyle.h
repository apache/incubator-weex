#ifndef WEEX_PROJECT_UPDATESTYLEACTION_H
#define WEEX_PROJECT_UPDATESTYLEACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class RenderActionUpdateStyle : public RenderAction {

  public:

    RenderActionUpdateStyle(const std::string &pageId, const std::string &ref,
                            std::vector<std::pair<std::string, std::string> *> *style,
                            std::vector<std::pair<std::string, std::string> *> *margin,
                            std::vector<std::pair<std::string, std::string> *> *padding,
                            std::vector<std::pair<std::string, std::string> *> *border);

    void ExecuteAction();

  public:
    std::string mPageId;
    std::string mRef;
    std::vector<std::pair<std::string, std::string> *> *mStyle;
    std::vector<std::pair<std::string, std::string> *> *mMargin;
    std::vector<std::pair<std::string, std::string> *> *mPadding;
    std::vector<std::pair<std::string, std::string> *> *mBorder;
  };
}

#endif //WEEX_PROJECT_UPDATESTYLEACTION_H
