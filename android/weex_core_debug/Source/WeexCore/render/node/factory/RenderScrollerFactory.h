#ifndef WEEX_PROJECT_RENDERSCROLLERFACTORY_H
#define WEEX_PROJECT_RENDERSCROLLERFACTORY_H

#include <WeexCore/render/node/RenderScroller.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderScrollerFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderScroller();
    }
  };

}

#endif //WEEX_PROJECT_RENDERSCROLLERFACTORY_H
