#ifndef WEEX_PROJECT_RENDERINDICATORFACTORY_H
#define WEEX_PROJECT_RENDERINDICATORFACTORY_H

#include <WeexCore/render/node/RenderIndicator.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderIndicatorFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderIndicator();
    }
  };

}

#endif //WEEX_PROJECT_RENDERINDICATORFACTORY_H
