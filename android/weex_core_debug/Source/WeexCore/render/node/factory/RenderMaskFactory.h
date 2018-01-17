#ifndef WEEX_PROJECT_RENDERMASKFACTORY_H
#define WEEX_PROJECT_RENDERMASKFACTORY_H

#include <WeexCore/render/node/RenderMask.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderMaskFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderMask();
    }
  };

}

#endif //WEEX_PROJECT_RENDERMASKFACTORY_H
