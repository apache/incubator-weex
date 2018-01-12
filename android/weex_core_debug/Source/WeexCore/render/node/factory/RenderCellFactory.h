#ifndef WEEX_PROJECT_RENDERCELLFACTORY_H
#define WEEX_PROJECT_RENDERCELLFACTORY_H

#include "IRenderFactory.h"

namespace WeexCore {

  class RenderCellFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender();
  };

}
#endif //WEEX_PROJECT_RENDERCELLFACTORY_H
