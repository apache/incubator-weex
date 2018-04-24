#ifndef WEEX_PROJECT_RENDERSCROLLERFACTORY_H
#define WEEX_PROJECT_RENDERSCROLLERFACTORY_H

#include <core/render/node/render_scroller.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderScrollerFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderScroller();
    }
  };

}

#endif //WEEX_PROJECT_RENDERSCROLLERFACTORY_H
