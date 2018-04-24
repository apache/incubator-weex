#ifndef WEEX_PROJECT_RENDERCELLFACTORY_H
#define WEEX_PROJECT_RENDERCELLFACTORY_H

#include <core/render/node/render_cell.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderCellFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderCell();
    }
  };

}
#endif //WEEX_PROJECT_RENDERCELLFACTORY_H
