/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef WEEX_PROJECT_VIEWUTILS_H
#define WEEX_PROJECT_VIEWUTILS_H

#include <core/config/core_environment.h>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace WeexCore {

  const std::string WX("wx");
  const std::string PX("px");
  const std::string UNDEFINE("undefined");
  const std::string SCALE("scale");
  const std::string AUTO_UNIT("auto");
  const std::string NONE("none");

  template<typename T>
  inline std::string to_string(const T &n) {
    std::ostringstream stm;
    stm << n;
    return stm.str();
  }

  inline std::string &Trim(std::string &s) {
    if (s.empty()) {
      return s;
    }
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
  }

  inline float getFloat(const char* src) {
    char *end;
    float ret = strtof(src, &end);
    if(*end != '\0'){
      ret = NAN;
    }
    return ret;
  }

  inline float getFloat(const float &src, const float &viewport) {
    if (isnan(src))
      return NAN;

    float realPx = (src * WXCoreEnvironment::getInstance()->DeviceWidth() /
                    viewport);
    float result = realPx > 0.005 && realPx < 1 ? 1.0f : realPx;
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
    float original_value = getFloat(src.c_str());
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
    float f = getFloat(temp.c_str());
    float density = getFloat(WXCoreEnvironment::getInstance()->GetOption(SCALE).c_str());
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
    float result = realPx > 0.005 && realPx < 1 ? 1.0f : realPx;
    return result;
  }
}

#endif //WEEX_PROJECT_VIEWUTILS_H
