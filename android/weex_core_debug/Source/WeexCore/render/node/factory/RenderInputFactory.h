#ifndef WEEX_PROJECT_RENDERINPUTFACTORY_H
#define WEEX_PROJECT_RENDERINPUTFACTORY_H

#include <WeexCore/render/node/RenderInput.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderInputFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderInput();
    }
  };

}

#endif //WEEX_PROJECT_RENDERINPUTFACTORY_H
