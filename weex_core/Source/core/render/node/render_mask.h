#ifndef WEEX_PROJECT_RENDERMASK_H
#define WEEX_PROJECT_RENDERMASK_H

#include <core/render/node/render_object.h>
#include <core/config/core_environment.h>
#include <android/bridge/impl/bridge_impl_android.h>
#include <base/ViewUtils.h>
#include <cstdlib>

namespace WeexCore {
  class RenderMask : public RenderObject {
    std::map<std::string, std::string> *GetDefaultStyle() {
      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();

      int width = WXCoreEnvironment::getInstance()->DeviceWidth();
      int height = WXCoreEnvironment::getInstance()->DeviceHeight();

      if (WXCoreEnvironment::getInstance()->GetOption("screen_width_pixels") != "" &&
          WXCoreEnvironment::getInstance()->GetOption("screen_height_pixels") != "") {
        width = atoi(WXCoreEnvironment::getInstance()->GetOption("screen_width_pixels").c_str());
        height = atoi(WXCoreEnvironment::getInstance()->GetOption("screen_height_pixels").c_str());
      }

      if (WXCoreEnvironment::getInstance()->GetOption("status_bar_height") != "") {
        int statusBarHeight = atoi(WXCoreEnvironment::getInstance()->GetOption("status_bar_height").c_str());
        height -= statusBarHeight;
      }

      style->insert(std::pair<std::string, std::string>(POSITION, "absolute"));
      style->insert(std::pair<std::string, std::string>(WIDTH, std::to_string(getWebPxByWidth(width, GetViewPortWidth()))));
      style->insert(std::pair<std::string, std::string>(HEIGHT, std::to_string(getWebPxByWidth(height, GetViewPortWidth()))));
      style->insert(std::pair<std::string, std::string>(TOP, "0"));
      return style;
    }
  };
}
#endif //WEEX_PROJECT_RENDERMASK_H
