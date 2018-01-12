#include "RenderCellFactory.h"
#include <WeexCore/render/node/RenderCell.h>

namespace WeexCore {
  IRenderObject *RenderCellFactory::CreateRender() {
    return new RenderCell();
  }
}
