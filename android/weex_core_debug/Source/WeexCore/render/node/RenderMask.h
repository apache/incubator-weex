#ifndef WEEX_PROJECT_RENDERMASK_H
#define WEEX_PROJECT_RENDERMASK_H

#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/env/CoreEnvironment.h>
#include <WeexCore/platform/android/bridge/impl/WXBridge_Impl_Android.h>
#include <base/ViewUtils.h>

namespace WeexCore {
  class RenderMask : public RenderObject {
    std::map<std::string, std::string> *GetDefaultStyle() {
      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();

      int height = WXCoreEnvironment::getInstance()->DeviceHeight();
      int width = WXCoreEnvironment::getInstance()->DeviceWidth();

      style->insert(std::pair<std::string, std::string>(POSITION, "absolute"));
      style->insert(std::pair<std::string, std::string>(WIDTH, std::to_string(getWebPxByWidth(width, GetViewPortWidth()))));
      style->insert(std::pair<std::string, std::string>(HEIGHT, std::to_string(getWebPxByWidth(height, GetViewPortWidth()))));
      style->insert(std::pair<std::string, std::string>(TOP, "0"));
      return style;
    }
  };
}
#endif //WEEX_PROJECT_RENDERMASK_H
