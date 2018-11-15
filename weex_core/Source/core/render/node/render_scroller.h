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
#ifndef CORE_RENDER_NODE_RENDER_SCROLLER_H_
#define CORE_RENDER_NODE_RENDER_SCROLLER_H_

#include <map>
#include <string>

#include "core/render/node/render_object.h"

namespace WeexCore {
class RenderScroller : public RenderObject {
  bool is_set_flex_ = false;

  std::map<std::string, std::string> *GetDefaultStyle() override;

  void set_flex(const float flex) override;

  inline float CalculateFreeSpaceAlongMainAxis(
      const float &width, const float &height,
      const float &current_length) const override {
    return NAN;
  }

protected:
    void onLayout(const float left, const float top, const float right, const float bottom,
                                  WXCoreLayoutNode *const absoulteItem, WXCoreFlexLine *const flexLine) override;

    // Since scroll only use ltr to layout children actually,
    // so we need override this method to return calculated inherit direction as normal render_object do
    inline WXCoreDirection getLayoutDirection() const override {
        WXCoreDirection styleDirection = this->getDirection();
        if (styleDirection != kDirectionInherit) {
            return styleDirection;
        } else if (this->getParent() != nullptr) {
            WXCoreLayoutNode *parent = this->getParent();
            return parent->getLayoutDirection();
        }
        return WEEXCORE_CSS_DEFAULT_DIRECTION;
    }

};
}  // namespace WeexCore
#endif  // CORE_RENDER_NODE_RENDER_SCROLLER_H_
