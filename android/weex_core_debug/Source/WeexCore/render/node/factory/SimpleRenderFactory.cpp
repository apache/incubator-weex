#include "SimpleRenderFactory.h"
#include <WeexCore/render/node/RenderObject.h>

namespace WeexCore {
  IRenderObject* RenderCommonFactory::CreateRender() {
    return new RenderObject();
  }
}
