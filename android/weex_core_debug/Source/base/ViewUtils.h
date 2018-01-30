#ifndef WEEX_PROJECT_VIEWUTILS_H
#define WEEX_PROJECT_VIEWUTILS_H

#include <../WeexCore/env/CoreEnvironment.h>
#include <cmath>

namespace WeexCore {
  const std::string WX("wx");
  const std::string PX("px");
  const std::string UNDEFINE("undefined");
  const std::string SCALE("scale");
  const std::string AUTO_UNIT("auto");

  inline float getFloat(const float &src, const float &viewport){
    float realPx = (src * WXCoreEnvironment::getInstance()->DeviceWidth() /
        viewport);
    float result = realPx > 0.005 && realPx < 1 ? 1 : (float) rint(realPx);
    return result;
  }

  inline float getFloat(const std::string &src, const float &viewport){
    return getFloat(std::stof(src), viewport);
  }

  inline bool endWidth(const std::string &src, const std::string &suffix){
    return src.size() > suffix.size() &&
        src.compare(src.size() - suffix.size(), suffix.size(), suffix) == 0;
  }

  inline float transferWx(const std::string &stringWithWXPostfix, const float &viewport) {
    std::string temp = stringWithWXPostfix;
    if (endWidth(stringWithWXPostfix, WX)) {
      temp = stringWithWXPostfix.substr(0, stringWithWXPostfix.size() - WX.size());
    }
    float f = std::stof(temp);
    float density = std::stof(WXCoreEnvironment::getInstance()->GetOption(SCALE));
    return density * f * viewport / WXCoreEnvironment::getInstance()->DeviceWidth();
  }

  inline static float getFloatByViewport(const std::string &temp, const float &viewport) {
    float ret = NAN;
    if (UNDEFINE == temp
        || AUTO_UNIT == temp
        || temp.empty()) { ;
    } else if (endWidth(temp, WX)) {
      ret = getFloat(transferWx(temp, viewport), viewport);
    } else if (endWidth(temp, PX)) {
      ret = getFloat(temp.substr(0, temp.size() - PX.size()), viewport);
    } else {
      ret = getFloat(temp, viewport);
    }
    return ret;
  }

  inline static float getWebPxByWidth(float pxValue, float customViewport) {
    float realPx = (pxValue * customViewport / WXCoreEnvironment::getInstance()->DeviceWidth());
    float result = realPx > 0.005 && realPx < 1 ? 1 : (float) rint(realPx);
    return result;
  }
}

#endif //WEEX_PROJECT_VIEWUTILS_H
