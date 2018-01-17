#ifndef WEEX_PROJECT_RENDERTEXTFACTORY_H
#define WEEX_PROJECT_RENDERTEXTFACTORY_H

#include <WeexCore/render/node/RenderText.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderTextFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderText();
    }
  };

}

#endif //WEEX_PROJECT_RENDERTEXTFACTORY_H
