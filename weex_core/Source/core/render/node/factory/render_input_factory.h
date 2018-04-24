#ifndef WEEX_PROJECT_RENDERINPUTFACTORY_H
#define WEEX_PROJECT_RENDERINPUTFACTORY_H

#include <core/render/node/render_input.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderInputFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderInput();
    }
  };

}

#endif //WEEX_PROJECT_RENDERINPUTFACTORY_H
