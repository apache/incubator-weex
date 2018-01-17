#ifndef WEEX_PROJECT_RENDERLISTFACTORY_H
#define WEEX_PROJECT_RENDERLISTFACTORY_H

#include <WeexCore/render/node/RenderList.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderListFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderList();
    }
  };

}

#endif //WEEX_PROJECT_RENDERLISTFACTORY_H
