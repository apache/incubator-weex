#ifndef WEEX_PROJECT_RENDERMASKFACTORY_H
#define WEEX_PROJECT_RENDERMASKFACTORY_H

#include <core/render/node/render_mask.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderMaskFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderMask();
    }
  };

}

#endif //WEEX_PROJECT_RENDERMASKFACTORY_H
