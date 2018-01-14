#ifndef WEEX_PROJECT_RENDERCOMMONFACTORY_H
#define WEEX_PROJECT_RENDERCOMMONFACTORY_H

#include "IRenderFactory.h"

namespace WeexCore {

  class RenderCommonFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender();
  };

}


#endif //WEEX_PROJECT_RENDERCOMMONFACTORY_H
