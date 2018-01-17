#ifndef WEEX_PROJECT_RENDERCOMMONFACTORY_H
#define WEEX_PROJECT_RENDERCOMMONFACTORY_H

#include <WeexCore/render/node/RenderObject.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderCommonFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderObject();
    }
  };

}


#endif //WEEX_PROJECT_RENDERCOMMONFACTORY_H
