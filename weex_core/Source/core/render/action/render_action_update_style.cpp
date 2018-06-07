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
#include <core/manager/weex_core_manager.h>
#include "render_action_update_style.h"

namespace WeexCore {

  RenderActionUpdateStyle::RenderActionUpdateStyle(const std::string &pageId, const std::string &ref,
                                                   std::vector<std::pair<std::string, std::string>> *style,
                                                   std::vector<std::pair<std::string, std::string>> *margin,
                                                   std::vector<std::pair<std::string, std::string>> *padding,
                                                   std::vector<std::pair<std::string, std::string>> *border) {
    this->mPageId = pageId;
    this->mRef = ref;
    this->mStyle = style;
    this->mMargin = margin;
    this->mPadding = padding;
    this->mBorder = border;
  }

  void RenderActionUpdateStyle::ExecuteAction() {
      WeexCoreManager::getInstance()->getPlatformBridge()->callUpdateStyle(mPageId.c_str(), mRef.c_str(), mStyle, mMargin,
                                                                           mPadding, mBorder);
  }
}
