#ifndef WEEX_PROJECT_UPDATEATTRACTION_H
#define WEEX_PROJECT_UPDATEATTRACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class UpdateAttrAction : public RenderAction {

  public:

    std::string mPageId;
    std::string mRef;
    std::vector<std::pair<std::string, std::string> *> *mAttrs;

    void ExecuteAction() {
      RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
      long long startTime = getCurrentTime();
      BridgeAndroid::getInstance()->callUpdateAttrByWeexCore(mPageId, mRef, mAttrs);
      if (page != nullptr)
        page->jniCallTime(getCurrentTime() - startTime);
    }

    void
    GenerateAction(std::string pageId, std::string ref,
                   std::vector<std::pair<std::string, std::string> *> *mAttrs) {
      this->mPageId = pageId;
      this->mRef = ref;
      this->mAttrs = mAttrs;
    }
  };
}

#endif //WEEX_PROJECT_UPDATEATTRACTION_H
