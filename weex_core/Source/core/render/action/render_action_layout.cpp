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
#include "render_action_layout.h"

namespace WeexCore {

  RenderActionLayout::RenderActionLayout(const std::string &pageId, const RenderObject *render,int index) {
    this->mPageId = pageId;
    this->mRef = render->Ref();
    this->index = index;
    GetLayoutInfo(render);
  }

  void RenderActionLayout::ExecuteAction() {
      WeexCoreManager::getInstance()->getPlatformBridge()->callLayout(mPageId.c_str(), mRef.c_str(),
                                                                      mTop, mBottom, mLeft, mRight,
                                                                      mHeight, mWidth, index);
  }

  void RenderActionLayout::GetLayoutInfo(const WXCoreLayoutNode *node) {
    mTop = node->getLayoutPositionTop();
    mBottom = node->getLayoutPositionBottom();
    mRight = node->getLayoutPositionRight();
    mLeft = node->getLayoutPositionLeft();
    mHeight = node->getLayoutHeight();
    mWidth = node->getLayoutWidth();
  }
}
