#ifndef WEEX_PROJECT_RENDERINDICATORFACTORY_H
#define WEEX_PROJECT_RENDERINDICATORFACTORY_H

#include <core/render/node/render_indicator.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderIndicatorFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderIndicator();
    }
  };

}

#endif //WEEX_PROJECT_RENDERINDICATORFACTORY_H
