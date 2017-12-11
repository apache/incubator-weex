#ifndef WEEX_PROJECT_UPDATESTYLEACTION_H
#define WEEX_PROJECT_UPDATESTYLEACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class UpdateStyleAction : public RenderAction {

  public:

    std::string mPageId;
    std::string mRef;
    std::vector<std::pair<std::string, std::string> *> *mStyle;
    std::vector<std::pair<std::string, std::string> *> *mMargin;
    std::vector<std::pair<std::string, std::string> *> *mPadding;
    std::vector<std::pair<std::string, std::string> *> *mBorder;

    void ExecuteAction() {
      RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
      long long startTime = getCurrentTime();
      BridgeAndroid::getInstance()->callUpdateStyleByWeexCore(mPageId, mRef, mStyle, mMargin,
                                                              mPadding, mBorder);
      if (page != nullptr)
        page->jniCallTime(getCurrentTime() - startTime);
    }

    void
    GenerateAction(std::string pageId, std::string ref,
                   std::vector<std::pair<std::string, std::string> *> *style,
                   std::vector<std::pair<std::string, std::string> *> *margin,
                   std::vector<std::pair<std::string, std::string> *> *padding,
                   std::vector<std::pair<std::string, std::string> *> *border) {
      this->mPageId = pageId;
      this->mRef = ref;
      this->mStyle = style;
      this->mMargin = margin;
      this->mPadding = padding;
      this->mBorder = border;
    }
  };
}

#endif //WEEX_PROJECT_UPDATESTYLEACTION_H
