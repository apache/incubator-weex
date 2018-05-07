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
#ifndef WEEX_PROJECT_ADDELEMENTACTION_H
#define WEEX_PROJECT_ADDELEMENTACTION_H

#include "render_action.h"

namespace WeexCore {

  class RenderActionAddElement : public render_action {

  public:
    RenderActionAddElement(const std::string &pageId, const RenderObject *render, const RenderObject *parent,
                           int index, bool willLayout= true);

    void ExecuteAction();

  public:
    StylesMap *mStyles;
    AttributesMap *mAttributes;
    EventsSet *mEvents;
    WXCoreMargin mMargins;
    WXCorePadding mPaddings;
    WXCoreBorderWidth mBorders;
    std::string mPageId;
    std::string mComponentType;
    std::string mParentRef;
    std::string mRef;
    int mIndex;
    bool mWillLayout;
  };
}

#endif //WEEX_PROJECT_ADDELEMENTACTION_H
