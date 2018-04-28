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
#ifndef WEEX_PROJECT_RELAYOUTRENDERACTION_H
#define WEEX_PROJECT_RELAYOUTRENDERACTION_H

#include "render_action.h"

namespace WeexCore {

  class RenderActionLayout : public render_action {

  public:
    RenderActionLayout(const std::string &pageId, const RenderObject *render, const int index);

    void ExecuteAction();

    void GetLayoutInfo(const WXCoreLayoutNode *node);

  public:
    EventsSet *mEvents;
    std::string mPageId;
    std::string mRef;
    float mWidth;
    float mHeight;
    float mTop;
    float mLeft;
    float mBottom;
    float mRight;
    int index;
  };
}

#endif //WEEX_PROJECT_RELAYOUTRENDERACTION_H
