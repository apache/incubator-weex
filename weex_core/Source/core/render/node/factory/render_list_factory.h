#ifndef WEEX_PROJECT_RENDERLISTFACTORY_H
#define WEEX_PROJECT_RENDERLISTFACTORY_H

#include <core/render/node/render_list.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderListFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderList();
    }
  };

}

#endif //WEEX_PROJECT_RENDERLISTFACTORY_H
