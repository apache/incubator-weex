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

    StyleType ApplyStyle(const std::string &key, const std::string &value) {
      if (key == PADDING) {
        UpdateStyle(key,
                    value,
                    0,
                    [=](float foo) {
                      setPadding(kPaddingLeft, foo + defaultNavWidth),
                          setPadding(kPaddingRight, foo + defaultOverflowWidth),
                          setPadding(kPaddingTop, foo),
                          setPadding(kPaddingBottom, foo);
                    });
        return kTypePadding;
      } else if (key == PADDING_LEFT) {
        UpdateStyle(key,
                    value,
                    0,
                    [=](float foo) { setPadding(kPaddingLeft, foo + defaultNavWidth); });
        return kTypePadding;
      } else if (key == PADDING_RIGHT) {
        UpdateStyle(key,
                    value,
                    0,
                    [=](float foo) { setPadding(kPaddingRight, foo + defaultOverflowWidth); });
        return kTypePadding;
      } else {
        return RenderObject::ApplyStyle(key, value);
      }
    }
  };
}

#endif //WEEX_PROJECT_RENDERAPPBAR_H
