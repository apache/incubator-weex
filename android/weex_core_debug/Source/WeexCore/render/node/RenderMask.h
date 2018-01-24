#ifndef WEEX_PROJECT_RENDERMASK_H
#define WEEX_PROJECT_RENDERMASK_H

#include "RenderObject.h"
#include <WeexCore/env/CoreEnvironment.h>
#include <WeexCore/platform/android/bridge/impl/WXBridge_Impl_Android.h>
#include <base/ViewUtils.h>

namespace WeexCore {
  class RenderMask : public RenderObject {
    std::map<std::string, std::string> *GetDefaultStyle() {
      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();

//        std::string mMessage = "DeviceHeight=" + std::to_string(WXCoreEnvironment::getInstance()->DeviceHeight())
//                               + ", DeviceWidth=" + std::to_string(WXCoreEnvironment::getInstance()->DeviceWidth())
//                               + ", StatusBarHeight=" + std::to_string(WXCoreEnvironment::getInstance()->StatusBarHeight());
//        Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

      int height = WXCoreEnvironment::getInstance()->DeviceHeight();
//      int height = WXCoreEnvironment::getInstance()->DeviceHeight() - WXCoreEnvironment::getInstance()->StatusBarHeight();
      int width = WXCoreEnvironment::getInstance()->DeviceWidth();

      style->insert(std::pair<std::string, std::string>(POSITION, "absolute"));
      style->insert(std::pair<std::string, std::string>(WIDTH, std::to_string(getWebPxByWidth(width, GetRenderPage()->ViewPortWidth()))));
      style->insert(std::pair<std::string, std::string>(HEIGHT, std::to_string(getWebPxByWidth(height, GetRenderPage()->ViewPortWidth()))));
      style->insert(std::pair<std::string, std::string>(TOP, "0"));
      return style;
    }
  };
}
#endif //WEEX_PROJECT_RENDERMASK_H
