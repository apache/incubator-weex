#ifndef WEEX_PROJECT_WXCOREENVIRONMENT_H
#define WEEX_PROJECT_WXCOREENVIRONMENT_H

#include <string>

namespace WeexCore {

  class WXCoreEnvironment {

  private:

    WXCoreEnvironment() {}

    ~WXCoreEnvironment() {}

    static WXCoreEnvironment *m_pInstance;

    //just to release singleton object
    class Garbo {
    public:
      ~Garbo() {
        if (WXCoreEnvironment::m_pInstance) {
          delete WXCoreEnvironment::m_pInstance;
        }
      }
    };

    static Garbo garbo;

    std::string mPlatformName;

    float mDeviceWidth;

    float mDeviceHeight;

  public:

    bool SetPlatform(const std::string &platformName);

    bool IsAndroid();

    bool IsIOS();

    bool SetDeviceWidth(const std::string &width);

    bool SetDeviceHeight(const std::string &height);

    float DeviceWidth();

    float DeviceHeight();

    static WXCoreEnvironment *getInstance() {
      if (!m_pInstance) {
        m_pInstance = new WXCoreEnvironment();
      }
      return m_pInstance;
    }
  };
}

#endif //WEEX_PROJECT_WXCOREENVIRONMENT_H
