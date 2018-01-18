#ifndef WEEX_PROJECT_RENDERCELLFACTORY_H
#define WEEX_PROJECT_RENDERCELLFACTORY_H

#include <WeexCore/render/node/RenderCell.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderCellFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderCell();
    }
  };

}
#endif //WEEX_PROJECT_RENDERCELLFACTORY_H
