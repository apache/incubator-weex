#ifndef WEEX_PROJECT_UPDATESTYLEACTION_H
#define WEEX_PROJECT_UPDATESTYLEACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class UpdateStyleAction : public RenderAction {

  public:

    void ExecuteAction() {
      RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
      long long startTime = getCurrentTime();
      BridgeAndroid::getInstance()->callUpdateStyleByWeexCore(mPageId, mRef, mKey, mValue);
      if (page != nullptr)
        page->jniCallTime(getCurrentTime() - startTime);
    }

    void
    GenerateAction(std::string pageId, std::string ref, std::string key, std::string value) {
      this->mPageId = pageId;
      this->mRef = ref;
      this->mKey = key;
      this->mValue = value;
    }
  };
}

#endif //WEEX_PROJECT_UPDATESTYLEACTION_H
