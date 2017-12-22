#include "RenderActionUpdateStyle.h"

namespace WeexCore {

  RenderActionUpdateStyle::RenderActionUpdateStyle(std::string pageId, std::string ref,
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

  void RenderActionUpdateStyle::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    BridgeAndroid::getInstance()->callUpdateStyleByWeexCore(mPageId, mRef, mStyle, mMargin,
                                                            mPadding, mBorder);
    if (page != nullptr)
      page->jniCallTime(getCurrentTime() - startTime);
  }
}
