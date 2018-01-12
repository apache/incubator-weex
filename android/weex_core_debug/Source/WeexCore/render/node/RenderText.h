#ifndef WEEX_PROJECT_RENDERTEXT_H
#define WEEX_PROJECT_RENDERTEXT_H

#include <WeexCore/render/node/RenderObject.h>

namespace WeexCore {
  class RenderText : public RenderObject {
  public:
  private:
    void UpdateAttr(const std::string &key, const std::string &value);
  };
}

#endif //WEEX_PROJECT_RENDERTEXT_H
