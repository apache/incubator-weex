#ifndef WEEX_PROJECT_UPDATESTYLEACTION_H
#define WEEX_PROJECT_UPDATESTYLEACTION_H

#include "RenderAction.h"
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>

using namespace WeexCore;

namespace WeexCore {

  class UpdateStyleAction : public RenderAction {

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callUpdateStyleByWeexCore(mPageId, mRef, mKey, mValue);
    }

    void
    GenerateAction(std::string pageId, RenderObject *render, RenderObject *parent, std::string key,
                   std::string value,
                   ...) {
      this->mPageId = pageId;
      this->mRef = render->getRef();
      this->mKey = key;
      this->mValue = value;
    }
  };

}
#endif //WEEX_PROJECT_UPDATESTYLEACTION_H
