#ifndef WEEX_PROJECT_RENDERTEXTAREAFACTORY_H
#define WEEX_PROJECT_RENDERTEXTAREAFACTORY_H

#include <WeexCore/render/node/RenderTextArea.h>
#include "IRenderFactory.h"

namespace WeexCore {

  class RenderTextAreaFactory : public IRenderFactory {
  public:
    IRenderObject *CreateRender() {
      return new RenderTextArea();
    }
  };

}

#endif //WEEX_PROJECT_RENDERTEXTAREAFACTORY_H
