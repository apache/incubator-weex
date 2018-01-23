#ifndef WEEX_PROJECT_RENDERSCROLLER_H
#define WEEX_PROJECT_RENDERSCROLLER_H

#include <WeexCore/platform/android/bridge/impl/WXBridge_Impl_Android.h>
#include "RenderObject.h"

namespace WeexCore {
  class RenderScroller : public RenderObject {

    std::map<std::string, std::string> *GetDefaultStyle() {
      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();

      bool isVertical = true;
      RenderObject *parent = (RenderObject *) getParent();

      if (parent != nullptr) {
        if (parent->GetAttr(SCROLL_DIRECTION) == HORIZONTAL) {
          isVertical = false;
        }
      }

      std::string prop = isVertical ? HEIGHT : WIDTH;

        std::string mMessage = "GetStyle(prop).empty()=" + std::to_string(GetStyle(prop).empty()) + "   GetStyle(FLEX).empty()=" + std::to_string(GetStyle(FLEX).empty());
        Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

      if (GetStyle(prop).empty() &&
          GetStyle(FLEX).empty()) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      }

      return style;
    }
  };
}
#endif //WEEX_PROJECT_RENDERSCROLLER_H
