#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "RenderAction.h"
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>

using namespace WeexCore;

namespace WeexCore {

  class AddElementAction : public RenderAction {

    void ExecuteAction() {
      BridgeAndroid::getInstance()->callAddElementByWeexCore(mPageId, mComponentType, mRef,
                                                             mTop,
                                                             mBottom, mLeft,
                                                             mRight,
                                                             mHeight,
                                                             mWidth, mIndex,
                                                             mParentRef);
    }

    void
    GenerateAction(std::string pageId, RenderObject *render, RenderObject *parent, std::string key,
                   std::string value,
                   ...) {

      va_list arg_ptr;
      int index;
      va_start(arg_ptr, value);
      index = va_arg(arg_ptr, int);

      this->mPageId = pageId;
      this->mComponentType = render->getType();
      this->mRef = render->getRef();
      this->mParentRef = render->getRef();
      this->mIndex = index;
      getLayoutInfo(render->getLayoutNode());
    }
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
