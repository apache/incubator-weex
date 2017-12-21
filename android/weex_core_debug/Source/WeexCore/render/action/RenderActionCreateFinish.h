#ifndef WEEX_PROJECT_CREATEFINISHACTION_H
#define WEEX_PROJECT_CREATEFINISHACTION_H

#include "RenderAction.h"

namespace WeexCore {

  class CreateFinishAction : public RenderAction {

  public:

    std::string mPageId;

    void ExecuteAction() {
      RenderPage *page = RenderManager::getInstance()->getPage(mPageId);
      long long startTime = getCurrentTime();
      BridgeAndroid::getInstance()->callCreateFinishByWeexCore(mPageId);

      page->jniCallTime(getCurrentTime() - startTime);
    }

    void GenerateAction(std::string pageId) {
      this->mPageId = pageId;
    }
  };
}

#endif //WEEX_PROJECT_CREATEFINISHACTION_H
