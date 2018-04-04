#ifndef WEEX_PROJECT_RENDERSCROLLER_H
#define WEEX_PROJECT_RENDERSCROLLER_H

#include <WeexCore/platform/android/bridge/impl/WXBridge_Impl_Android.h>
#include <WeexCore/render/node/RenderObject.h>

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

      if (prop == HEIGHT && isnan(getStyleHeight()) && isnan(getFlex())) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      } else if (prop == WIDTH && isnan(getStyleWidth()) && isnan(getFlex())) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      }

      return style;
    }

    float calcFreeSpaceAlongMainAxis(const float &width, const float &height, const float &currentLength) const override {
      return NAN;
    }

  };
}
#endif //WEEX_PROJECT_RENDERSCROLLER_H
