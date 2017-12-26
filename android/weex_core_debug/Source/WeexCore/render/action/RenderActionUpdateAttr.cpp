#include "RenderActionUpdateAttr.h"

namespace WeexCore {

  RenderActionUpdateAttr::RenderActionUpdateAttr(std::string pageId, std::string ref,
                                                 std::vector<std::pair<std::string, std::string> *> *mAttrs) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mAttrs = mAttrs;
  }

  void RenderActionUpdateAttr::ExecuteAction() {
    RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
    long long startTime = getCurrentTime();
    Bridge_Impl_Android::getInstance()->callUpdateAttrByWeexCore(mPageId, mRef, mAttrs);
    if (page != nullptr)
      page->jniCallTime(getCurrentTime() - startTime);
  }
}
