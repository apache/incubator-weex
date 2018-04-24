#ifndef WEEX_PROJECT_RENDERSWITCHFACTORY_H
#define WEEX_PROJECT_RENDERSWITCHFACTORY_H

#include <core/render/node/render_switch.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderSwitchFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderSwitch();
    }
  };

}

#endif //WEEX_PROJECT_RENDERSWITCHFACTORY_H
