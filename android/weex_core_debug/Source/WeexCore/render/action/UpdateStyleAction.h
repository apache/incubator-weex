#ifndef WEEX_PROJECT_UPDATESTYLEACTION_H
#define WEEX_PROJECT_UPDATESTYLEACTION_H

#include "RenderAction.h"
#include <WeexCore/render/RenderObject.h>

using namespace WeexCore;

namespace WeexCore {

  class UpdateStyleAction : public RenderAction {

  public:

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callUpdateStyleByWeexCore(mPageId, mRef, mKey, mValue);
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
