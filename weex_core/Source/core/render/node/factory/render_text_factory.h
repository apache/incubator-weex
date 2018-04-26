#ifndef WEEX_PROJECT_RENDERTEXTFACTORY_H
#define WEEX_PROJECT_RENDERTEXTFACTORY_H

#include <core/render/node/render_text.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderTextFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderText();
    }
  };

}

#endif //WEEX_PROJECT_RENDERTEXTFACTORY_H
