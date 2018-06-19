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
#include <cstdlib>
#include <utility>

#include "base/ViewUtils.h"
#include "core/render/manager/render_manager.h"
#include "core/config/core_environment.h"
#include "core/css/constants_name.h"
#include "core/render/node/render_mask.h"

namespace WeexCore {

std::map<std::string, std::string> *RenderMask::GetDefaultStyle() {
  std::map<std::string, std::string> *style =
      new std::map<std::string, std::string>();

  int width = WXCoreEnvironment::getInstance()->DeviceWidth();
  int height = WXCoreEnvironment::getInstance()->DeviceHeight();

  if (WXCoreEnvironment::getInstance()->GetOption("screen_width_pixels") !=
          "" &&
      WXCoreEnvironment::getInstance()->GetOption("screen_height_pixels") !=
          "") {
    width = atoi(WXCoreEnvironment::getInstance()
                     ->GetOption("screen_width_pixels")
                     .c_str());
    height = atoi(WXCoreEnvironment::getInstance()
                      ->GetOption("screen_height_pixels")
                      .c_str());
  }

  if (WXCoreEnvironment::getInstance()->GetOption("status_bar_height") != "") {
    int status_bar_height = atoi(WXCoreEnvironment::getInstance()
                                   ->GetOption("status_bar_height")
                                   .c_str());
    height -= status_bar_height;
  }

  style->insert(std::pair<std::string, std::string>(POSITION, "absolute"));
  style->insert(std::pair<std::string, std::string>(
      WIDTH, to_string(getWebPxByWidth(width, RenderManager::GetInstance()->viewport_width()))));
  style->insert(std::pair<std::string, std::string>(
      HEIGHT, to_string(getWebPxByWidth(height, RenderManager::GetInstance()->viewport_width()))));
  style->insert(std::pair<std::string, std::string>(TOP, "0"));
  return style;
}
}  // namespace WeexCore
