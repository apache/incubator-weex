#ifndef WEEX_PROJECT_RENDERSWITCHFACTORY_H
#define WEEX_PROJECT_RENDERSWITCHFACTORY_H

#include <WeexCore/render/node/RenderSwitch.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderSwitchFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderSwitch();
    }
  };

}

#endif //WEEX_PROJECT_RENDERSWITCHFACTORY_H
