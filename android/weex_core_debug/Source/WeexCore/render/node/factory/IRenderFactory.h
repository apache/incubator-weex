#ifndef WEEX_PROJECT_IRENDERFACTORY_H
#define WEEX_PROJECT_IRENDERFACTORY_H

#include "IRenderObject.h"

namespace WeexCore {
  class IRenderFactory {
  public:
    virtual IRenderObject *CreateRender() = 0;
  };
}

#endif //WEEX_PROJECT_IRENDERFACTORY_H
