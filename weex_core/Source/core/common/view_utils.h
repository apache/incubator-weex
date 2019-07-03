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

#include <cmath>
#include <cstdlib>
#include <sstream>

#include "core/config/core_environment.h"

namespace WeexCore {

  constexpr char WX[] = "wx";
  constexpr char PX[] = "px";
  constexpr char UNDEFINE[] = "undefined";
  constexpr char SCALE[] = "scale";
  constexpr char AUTO_UNIT[] = "auto";
  constexpr char NONE[] = "none";

  constexpr char STATUS_BAR_HEIGHT[] = "status_bar_height";

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
    float ret = (float) strtod(src, &end);

    if(*end != '\0'){
      ret = NAN;
    }
    return ret;
  }

  inline float getFloat(const float &src, const float &viewport, const float& device_width, const bool &round_off_deviation) {
    if (isnan(src))
      return NAN;

    float realPx = (src * device_width /
                    viewport);
#if OS_IOS
    return realPx;
#else

    float result;
    if (round_off_deviation) {
      result = realPx > 0.005 && realPx < 1 ? 1.0f : realPx;
    } else {
      result = realPx > 0.005 && realPx < 1 ? 1.0f : rint(realPx);
    }
    return result;
#endif
  }

  inline bool getBool(const std::string &src) {
    if (strcmp(src.c_str(), "true") == 0) {
      return true;
    } else {
      return false;
    }
  }

  inline float getFloat(const std::string &src, const float &viewport, const float &device_width, const bool &round_off_deviation) {
    float ret = NAN;
    if (UNDEFINE == src
        || AUTO_UNIT == src
        || NONE == src
        || src.empty()) {
      return ret;
    }
    float original_value = getFloat(src.c_str());
    ret = getFloat(original_value, viewport, device_width, round_off_deviation);
    return ret;
  }

  inline bool endWidth(const std::string &src, const std::string &suffix) {
    return src.size() > suffix.size() &&
           src.compare(src.size() - suffix.size(), suffix.size(), suffix) == 0;
  }

  inline float transferWx(const std::string &stringWithWXPostfix, const float &viewport,
                          const float &device_width) {
    std::string temp = stringWithWXPostfix;
    if (endWidth(stringWithWXPostfix, WX)) {
      temp = stringWithWXPostfix.substr(0, stringWithWXPostfix.size() - strlen(WX));
    }
    float f = getFloat(temp.c_str());
    float density = getFloat(WXCoreEnvironment::getInstance()->GetOption(SCALE).c_str());
    return density * f * viewport / device_width; WXCoreEnvironment::getInstance()->DeviceWidth();
  }

  inline static float getFloatByViewport(std::string src, const float &viewport,
          const float &device_width, const bool &round_off_deviation) {
    float ret = NAN;
    if (UNDEFINE == src
        || AUTO_UNIT == src
        || NONE == src
        || src.empty()) {
      return ret;
    }
    Trim(src);
    if (endWidth(src, WX)) {
      ret = getFloat(transferWx(src, viewport, device_width), viewport,  device_width, round_off_deviation);
    } else if (endWidth(src, PX)) {
      ret = getFloat(src.substr(0, src.size() - strlen(PX)), viewport, device_width, round_off_deviation);
    } else {
      ret = getFloat(src, viewport, device_width, round_off_deviation);
    }
    return ret;
  }

  inline static float getWebPxByWidth(float pxValue, float customViewport) {
    if (isnan(pxValue))
      return NAN;

    float realPx = (pxValue * customViewport / WXCoreEnvironment::getInstance()->DeviceWidth());
#if OS_IOS
    return realPx;
#else
    float result = realPx > 0.005 && realPx < 1 ? 1.0f : realPx;
    return result;
#endif
  }
}

#endif //WEEX_PROJECT_VIEWUTILS_H
