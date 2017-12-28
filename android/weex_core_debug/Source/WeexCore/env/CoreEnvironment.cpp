#include "CoreEnvironment.h"
#include "base/CoreConstants.h"
#include <stdlib.h>

namespace WeexCore {
  WXCoreEnvironment *WXCoreEnvironment::m_pInstance = nullptr;

  bool WXCoreEnvironment::setPlatform(const std::string &platformName) {
    if (platformName.empty())
      return false;
    this->mPlatformName = platformName;
    if (platformName == OS_Android || platformName == OS_iOS) {
      return true;
    } else {
      return false;
    }
  }

  bool WXCoreEnvironment::isAndroid() {
    return mPlatformName == OS_Android;
  }

  bool WXCoreEnvironment::isIOS() {
    return mPlatformName == OS_iOS;
  }

  bool WXCoreEnvironment::setDeviceWidth(const std::string &width) {
    if (width.empty())
      return false;
    mDeviceWidth = atof(width.c_str());
    return true;
  }

  bool WXCoreEnvironment::setDeviceHeight(const std::string &height) {
    if (height.empty())
      return false;
    mDeviceHeight = atof(height.c_str());
    return true;
  }

  float WXCoreEnvironment::getDeviceWidth() {
    return mDeviceWidth;
  }

  float WXCoreEnvironment::getDeviceHeight() {
    return mDeviceHeight;
  }
}
