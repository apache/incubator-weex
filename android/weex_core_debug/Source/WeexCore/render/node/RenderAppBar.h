#ifndef WEEX_PROJECT_RENDERAPPBAR_H
#define WEEX_PROJECT_RENDERAPPBAR_H

#include <WeexCore/render/node/RenderObject.h>
#include "RenderObject.h"
#include <string>
#include <WeexCore/platform/android/base/string/StringUtils.h>

namespace WeexCore {
  class RenderAppBar : public RenderObject {

  private:
    float defaultNavWidth;
    float defaultOverflowWidth;

    std::map<std::string, std::string> *GetDefaultStyle() {
      defaultNavWidth = Str2Float(
          WXCoreEnvironment::getInstance()->GetOption("defaultNavWidth"));

      defaultOverflowWidth = Str2Float(
          WXCoreEnvironment::getInstance()->GetOption("defaultOverflowWidth"));

      std::string appbar_color = WXCoreEnvironment::getInstance()->GetOption("appbar_color");
      std::string appbar_background_color = WXCoreEnvironment::getInstance()->GetOption(
          "appbar_background_color");

      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();
      style->insert(std::pair<std::string, std::string>(PADDING_LEFT, "0"));
      style->insert(std::pair<std::string, std::string>(PADDING_RIGHT, "0"));

      if (!appbar_color.empty() && appbar_color != "")
        style->insert(std::pair<std::string, std::string>(COLOR, appbar_color));
      if (!appbar_background_color.empty() && appbar_background_color != "")
        style->insert(std::pair<std::string, std::string>(BACKGROUND_COLOR, appbar_background_color));
      return style;
    }

  public:

    StyleType ApplyStyle(std::string key, std::string value) {

      if (key == PADDING) {
        float paddingLeft =
            getFloatByViewport(value, GetRenderPage()->ViewPortWidth()) + defaultNavWidth;
        float paddingRight =
            getFloatByViewport(value, GetRenderPage()->ViewPortWidth()) + defaultOverflowWidth;
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
        float paddingLeft =
            getFloatByViewport(value, GetRenderPage()->ViewPortWidth()) + defaultNavWidth;
        setPadding(kPaddingLeft, paddingLeft);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(paddingLeft)));
        return kTypePadding;
      } else if (key == PADDING_RIGHT) {
        float paddingRight =
            getFloatByViewport(value, GetRenderPage()->ViewPortWidth()) + defaultOverflowWidth;
        setPadding(kPaddingRight, paddingRight);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(paddingRight)));
        return kTypePadding;
      } else if (key == PADDING_TOP) {
        float padding = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingTop, padding);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(padding)));
        return kTypePadding;
      } else if (key == PADDING_BOTTOM) {
        float padding = getFloatByViewport(value, GetRenderPage()->ViewPortWidth());
        setPadding(kPaddingBottom, padding);
        mPaddings->insert(std::pair<std::string, std::string>(key, std::to_string(padding)));
        return kTypePadding;
      } else {
        return RenderObject::ApplyStyle(key, value);
      }
    }
  };
}

#endif //WEEX_PROJECT_RENDERAPPBAR_H
