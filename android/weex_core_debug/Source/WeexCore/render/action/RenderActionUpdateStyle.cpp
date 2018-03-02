#include "RenderActionUpdateStyle.h"

namespace WeexCore {

  RenderActionUpdateStyle::RenderActionUpdateStyle(const std::string &pageId, const std::string &ref,
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
    RenderPage *page = RenderManager::GetInstance()->GetPage(mPageId);
    if (page == nullptr)
      return;

    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callUpdateStyle(mPageId, mRef, mStyle, mMargin,
                                                        mPadding, mBorder);
    if (page != nullptr)
      page->JniCallTime(getCurrentTime() - startTime);
  }
}
