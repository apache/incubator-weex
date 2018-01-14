#include "RenderTextFactory.h"
#include <WeexCore/render/node/RenderText.h>

namespace WeexCore {
  IRenderObject *RenderTextFactory::CreateRender() {
    return new RenderText();
  }
}
