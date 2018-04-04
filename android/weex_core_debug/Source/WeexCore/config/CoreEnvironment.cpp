#include "CoreEnvironment.h"
#include "base/CoreConstants.h"
#include <stdlib.h>
#include <base/ViewUtils.h>

namespace WeexCore {

  WXCoreEnvironment *WXCoreEnvironment::m_pInstance = nullptr;

  bool WXCoreEnvironment::SetPlatform(std::string platformName) {
    if (platformName.empty())
      return false;
    this->mPlatformName = platformName;
    if (platformName == OS_Android || platformName == OS_iOS) {
      return true;
    } else {
      return false;
    }
  }

  bool WXCoreEnvironment::IsAndroid() {
    return mPlatformName == OS_Android;
  }

  bool WXCoreEnvironment::IsIOS() {
    return mPlatformName == OS_iOS;
  }

  bool WXCoreEnvironment::SetDeviceWidth(const std::string &width) {
    if (width.empty() || isnan(getFloat(width.c_str())))
      return false;
    mDeviceWidth = getFloat(width.c_str());
    return true;
  }

  bool WXCoreEnvironment::SetDeviceHeight(const std::string &height) {
    if (height.empty() || isnan(getFloat(height.c_str())))
      return false;
    mDeviceHeight = getFloat(height.c_str());
    return true;
  }

  const float &WXCoreEnvironment::DeviceWidth() {
    return mDeviceWidth;
  }

  const float &WXCoreEnvironment::DeviceHeight() {
    return mDeviceHeight;
  }

  const std::string &WXCoreEnvironment::GetOption(const std::string &key) {
    std::map<std::string, std::string>::iterator iter = mOptions.find(key);
    if (iter != mOptions.end()) {
      return iter->second;
    } else {
      return "";
    }
  }

  void WXCoreEnvironment::AddOption(std::string key, std::string value) {
    mOptions.insert(std::pair<std::string, std::string>(key, value));
  }
}
