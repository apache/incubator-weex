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

  inline float getFloat(const float &src, const float &viewport) {
    if (isnan(src))
      return NAN;

    float realPx = (src * WXCoreEnvironment::getInstance()->DeviceWidth() /
        viewport);
    float result = realPx > 0.005 && realPx < 1 ? 1 : (float) rint(realPx);
    return result;
  }

  inline float getFloat(const std::string &src, const float &viewport){
    float ret = NAN;
    if (UNDEFINE == src
        || AUTO_UNIT == src
        || src.empty()) {
      return ret;
    }
    ret = getFloat(std::stof(src), viewport);
    return ret;
  }

  inline bool endWidth(const std::string &src, const std::string &suffix) {
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

  inline static float getFloatByViewport(const std::string &src, const float &viewport) {
    float ret = NAN;
    if (UNDEFINE == src
        || AUTO_UNIT == src
        || src.empty()) {
    } else if (endWidth(src, WX)) {
      ret = getFloat(transferWx(src, viewport), viewport);
    } else if (endWidth(src, PX)) {
      ret = getFloat(src.substr(0, src.size() - PX.size()), viewport);
    } else {
      ret = getFloat(src, viewport);
    }
    return ret;
  }

  inline static float getWebPxByWidth(float pxValue, float customViewport) {
    if (isnan(pxValue))
      return NAN;

    float realPx = (pxValue * customViewport / WXCoreEnvironment::getInstance()->DeviceWidth());
    float result = realPx > 0.005 && realPx < 1 ? 1 : (float) rint(realPx);
    return result;
  }
}

#endif //WEEX_PROJECT_VIEWUTILS_H
