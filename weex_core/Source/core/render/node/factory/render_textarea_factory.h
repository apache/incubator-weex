#ifndef WEEX_PROJECT_RENDERTEXTAREAFACTORY_H
#define WEEX_PROJECT_RENDERTEXTAREAFACTORY_H

#include <core/render/node/render_textarea.h>
#include "i_render_factory.h"

namespace WeexCore {

  class RenderTextAreaFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderTextArea();
    }
  };

}

#endif //WEEX_PROJECT_RENDERTEXTAREAFACTORY_H
