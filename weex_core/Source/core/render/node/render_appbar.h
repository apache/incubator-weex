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
#ifndef WEEX_PROJECT_RENDERAPPBAR_H
#define WEEX_PROJECT_RENDERAPPBAR_H

#include <core/render/node/render_object.h>
#include <string>
//#include <android/base/string/string_utils.h>

namespace WeexCore {
  class RenderAppBar : public RenderObject {

  private:
    float defaultNavWidth;
    float defaultOverflowWidth;

    std::map<std::string, std::string> *GetDefaultStyle() {
      defaultNavWidth = getFloat(WXCoreEnvironment::getInstance()->GetOption("defaultNavWidth").c_str());

      defaultOverflowWidth = getFloat(WXCoreEnvironment::getInstance()->GetOption("defaultOverflowWidth").c_str());

      std::string appbar_color = WXCoreEnvironment::getInstance()->GetOption("appbar_color");
      std::string appbar_background_color = WXCoreEnvironment::getInstance()->GetOption(
          "appbar_background_color");

      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();
      style->insert(std::pair<std::string, std::string>(PADDING_LEFT, "0"));
      style->insert(std::pair<std::string, std::string>(PADDING_RIGHT, "0"));

      if (!appbar_color.empty() && appbar_color != "" && !StyleExist(COLOR))
        style->insert(std::pair<std::string, std::string>(COLOR, appbar_color));
      if (!appbar_background_color.empty() && appbar_background_color != "" && !StyleExist(BACKGROUND_COLOR))
        style->insert(std::pair<std::string, std::string>(BACKGROUND_COLOR, appbar_background_color));
      return style;
    }

    inline bool StyleExist(const std::string &key) {
      std::string value = GetStyle(key);
      return !value.empty() && value != "";
    }

  public:

    StyleType ApplyStyle(const std::string &key, const std::string &value, const bool updating) {
      if (key == PADDING) {
        UpdateStyle(key,
                    value,
                    0,
                    [=](float foo) {
                      setPadding(kPaddingLeft, foo + defaultNavWidth),
                          setPadding(kPaddingRight, foo + defaultOverflowWidth),
                          setPadding(kPaddingTop, foo),
                          setPadding(kPaddingBottom, foo);
                    });
        return kTypePadding;
      } else if (key == PADDING_LEFT) {
        UpdateStyle(key,
                    value,
                    0,
                    [=](float foo) { setPadding(kPaddingLeft, foo + defaultNavWidth); });
        return kTypePadding;
      } else if (key == PADDING_RIGHT) {
        UpdateStyle(key,
                    value,
                    0,
                    [=](float foo) { setPadding(kPaddingRight, foo + defaultOverflowWidth); });
        return kTypePadding;
      } else {
        return RenderObject::ApplyStyle(key, value, updating);
      }
    }
  };
}

#endif //WEEX_PROJECT_RENDERAPPBAR_H
