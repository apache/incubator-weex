#include "RenderText.h"
#include <WeexCore/render/page/RenderPage.h>

namespace WeexCore {

  void RenderText::UpdateAttr(const std::string &key, const std::string &value) {
    RenderObject::UpdateAttr(key, value);
    markDirty();
    GetRenderPage()->CalculateLayout();
  }
}