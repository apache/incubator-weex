#ifndef WEEX_PROJECT_RENDERTEXT_H
#define WEEX_PROJECT_RENDERTEXT_H

#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/render/page/RenderPage.h>

namespace WeexCore {
  class RenderText : public RenderObject {
  private:
    inline void UpdateAttr(std::string key, std::string value) {
      RenderObject::UpdateAttr(key, value);
      markDirty();
      GetRenderPage()->CalculateLayout();
    }

    inline StyleType UpdateStyle(std::string key, std::string value) {
      StyleType resultType = RenderObject::ApplyStyle(key, value);
      markDirty();
      return resultType;
    }
  };
}

#endif //WEEX_PROJECT_RENDERTEXT_H
