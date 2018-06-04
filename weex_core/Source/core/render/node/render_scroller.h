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
#ifndef WEEX_PROJECT_RENDERSCROLLER_H
#define WEEX_PROJECT_RENDERSCROLLER_H

#include <core/render/node/render_object.h>

namespace WeexCore {
  class RenderScroller : public RenderObject {

    bool mIsSetFlex = false;

    std::map<std::string, std::string> *GetDefaultStyle() {
      std::map<std::string, std::string> *style = new std::map<std::string, std::string>();

      bool isVertical = true;
      RenderObject *parent = (RenderObject *) getParent();

      if (parent != nullptr) {
        if (parent->GetAttr(SCROLL_DIRECTION) == HORIZONTAL) {
          isVertical = false;
        }
      }

      std::string prop = isVertical ? HEIGHT : WIDTH;

      if (prop == HEIGHT && isnan(getStyleHeight()) &&  !mIsSetFlex) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      } else if (prop == WIDTH && isnan(getStyleWidth()) &&  !mIsSetFlex) {
        style->insert(std::pair<std::string, std::string>(FLEX, "1"));
      }

      return style;
    }

    inline void setFlex(const float flex) {
       mIsSetFlex = true;
       WXCoreLayoutNode::setFlex(flex);
    }


      float calcFreeSpaceAlongMainAxis(const float &width, const float &height, const float &currentLength) const override {
      return NAN;
    }

  };
}
#endif //WEEX_PROJECT_RENDERSCROLLER_H
