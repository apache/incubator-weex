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
#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "core/layout/flex_enum.h"
#include <string>

namespace WeexCore {

  const WXCoreFlexDirection GetWXCoreFlexDirection(const std::string &value);

  const WXCoreJustifyContent GetWXCoreJustifyContent(const std::string &value);

  const WXCoreAlignItems GetWXCoreAlignItem(const std::string &value);

  const WXCoreFlexWrap GetWXCoreFlexWrap(const std::string &value);

  const WXCoreAlignSelf GetWXCoreAlignSelf(const std::string &value);

  const WXCorePositionType GetWXCorePositionType(const std::string &value);
}


#endif //WEEX_PROJECT_STYLEGETTER_H
