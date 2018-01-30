#ifndef WEEX_PROJECT_RENDERAPPBAR_H
#define WEEX_PROJECT_RENDERAPPBAR_H

#include <WeexCore/render/node/RenderObject.h>
#include "RenderObject.h"
#include <string>

namespace WeexCore {
  class RenderAppBar : public RenderObject {
  public:

    StyleType ApplyStyle(const std::string &key, const std::string &value) {
      if (key == PADDING) {
        float paddingLeft = getFloatByViewport(value,GetRenderPage()->ViewPortWidth()) +
            getFloatByViewport(WXCoreEnvironment::getInstance()->GetOption("defaultNavWidth"),
                               GetRenderPage()->ViewPortWidth());
        float paddingRight = getFloatByViewport(value,GetRenderPage()->ViewPortWidth()) +
            getFloatByViewport(WXCoreEnvironment::getInstance()->GetOption("defaultOverflowWidth"),
                               GetRenderPage()->ViewPortWidth());
        float padding = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());

        setPadding(kPaddingLeft, paddingLeft);
        setPadding(kPaddingRight, paddingRight);
        setPadding(kPaddingTop, padding);
        setPadding(kPaddingBottom, padding);
        mPaddings->insert(
            std::pair<std::string, std::string>(PADDING_LEFT, std::to_string(paddingLeft)));
        mPaddings->insert(
            std::pair<std::string, std::string>(PADDING_RIGHT, std::to_string(paddingRight)));
        mPaddings->insert(
            std::pair<std::string, std::string>(PADDING_TOP, std::to_string(padding)));
        mPaddings->insert(
            std::pair<std::string, std::string>(PADDING_BOTTOM, std::to_string(padding)));
        return kTypePadding;
      } else if (key == PADDING_LEFT) {
        float realValue = getFloatByViewport(value,GetRenderPage()->ViewPortWidth()) +
            getFloatByViewport(WXCoreEnvironment::getInstance()->GetOption("defaultNavWidth"),
                               GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingLeft, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else if (key == PADDING_RIGHT) {
        float realValue = getFloatByViewport(value,GetRenderPage()->ViewPortWidth()) +
            getFloatByViewport(WXCoreEnvironment::getInstance()->GetOption("defaultOverflowWidth"),
                               GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingRight, realValue);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(realValue)));
        return kTypePadding;
      } else {
        return RenderObject::ApplyStyle(key, value);
      }
    }
  };
}

#endif //WEEX_PROJECT_RENDERAPPBAR_H
