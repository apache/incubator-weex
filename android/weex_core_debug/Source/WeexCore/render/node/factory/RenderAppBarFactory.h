#ifndef WEEX_PROJECT_RENDERAPPBARFACTORY_H
#define WEEX_PROJECT_RENDERAPPBARFACTORY_H

#include <WeexCore/render/node/RenderAppBar.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderAppBarFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderAppBar();
    }
  };

}

#endif //WEEX_PROJECT_RENDERAPPBARFACTORY_H
