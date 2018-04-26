#ifndef WEEX_PROJECT_RENDERCOMMONFACTORY_H
#define WEEX_PROJECT_RENDERCOMMONFACTORY_H

#include <core/render/node/render_object.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderCommonFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderObject();
    }
  };

}


#endif //WEEX_PROJECT_RENDERCOMMONFACTORY_H
