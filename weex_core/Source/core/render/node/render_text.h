#ifndef WEEX_PROJECT_RENDERTEXT_H
#define WEEX_PROJECT_RENDERTEXT_H

#include <core/render/node/render_object.h>
#include <core/render/page/render_page.h>

namespace WeexCore {
  class RenderText : public RenderObject {
  private:
    inline void UpdateAttr(std::string key, std::string value) {
      RenderObject::UpdateAttr(key, value);
      markDirty();
      RenderPage* page = GetRenderPage();
      if (page != nullptr)
        GetRenderPage()->Batch();
    }

    inline StyleType UpdateStyle(std::string key, std::string value) {
      StyleType resultType = RenderObject::ApplyStyle(key, value, true);
      markDirty();
      return resultType;
    }
  };
}

#endif //WEEX_PROJECT_RENDERTEXT_H
