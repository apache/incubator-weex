#ifndef WEEX_PROJECT_RENDERTEXTFACTORY_H
#define WEEX_PROJECT_RENDERTEXTFACTORY_H

#include "IRenderFactory.h"

namespace WeexCore {

  class RenderTextFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender();
  };

}

#endif //WEEX_PROJECT_RENDERTEXTFACTORY_H
