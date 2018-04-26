#ifndef WEEX_PROJECT_RENDERAPPBARFACTORY_H
#define WEEX_PROJECT_RENDERAPPBARFACTORY_H

#include <core/render/node/render_appbar.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderAppBarFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderAppBar();
    }
  };

}

#endif //WEEX_PROJECT_RENDERAPPBARFACTORY_H
