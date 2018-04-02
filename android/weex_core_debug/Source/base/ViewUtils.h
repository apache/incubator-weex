#ifndef WEEX_PROJECT_VIEWUTILS_H
#define WEEX_PROJECT_VIEWUTILS_H

#include <WeexCore/config/CoreEnvironment.h>
#include <cmath>
#include <cstdlib>

namespace WeexCore {

  const std::string WX("wx");
  const std::string PX("px");
  const std::string UNDEFINE("undefined");
  const std::string SCALE("scale");
  const std::string AUTO_UNIT("auto");
  const std::string NONE("none");

  inline std::string &Trim(std::string &s) {
    if (s.empty()) {
      return s;
    }
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
  }

  inline float getFloat(const float &src, const float &viewport) {
    if (isnan(src))
      return NAN;

    float realPx = (src * WXCoreEnvironment::getInstance()->DeviceWidth() /
                    viewport);
    float result = realPx > 0.005 && realPx < 1 ? 1 : rint(realPx);
    return result;
  }

  inline float getFloat(const std::string &src, const float &viewport) {
    float ret = NAN;
    if (UNDEFINE == src
        || AUTO_UNIT == src
        || NONE == src
        || src.empty()) {
      return ret;
    }
    char *end;
    float original_value = strtof(src.c_str(), &end);
    if(*end != '\0'){
      original_value = NAN;
    }
    ret = getFloat(original_value, viewport);
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
    float f = atof((char *) temp.c_str());
    float density = atof((char *) WXCoreEnvironment::getInstance()->GetOption(SCALE).c_str());
    return density * f * viewport / WXCoreEnvironment::getInstance()->DeviceWidth();
  }

  inline static float getFloatByViewport(std::string src, const float &viewport) {
    float ret = NAN;
    if (UNDEFINE == src
        || AUTO_UNIT == src
        || NONE == src
        || src.empty()) {
      return ret;
    }
    Trim(src);
    if (endWidth(src, WX)) {
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
    float result = realPx > 0.005 && realPx < 1 ? 1 : rint(realPx);
    return result;
  }
}

#endif //WEEX_PROJECT_VIEWUTILS_H
