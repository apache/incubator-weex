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
#include "css_value_getter.h"
#include "constants_value.h"
#include "constants_name.h"
#include "core/layout/style.h"

namespace WeexCore {

  const WXCoreFlexDirection GetWXCoreFlexDirection(const std::string &value) {
    const char *c_value = value.c_str();
    if(strcmp(c_value, COLUMN) == 0) {
      return WeexCore::kFlexDirectionColumn;
    } else if (strcmp(c_value, ROW) == 0) {
      return WeexCore::kFlexDirectionRow;
    } else if (strcmp(c_value, COLUMN_REVERSE) == 0) {
      return WeexCore::kFlexDirectionColumnReverse;
    } else if (strcmp(c_value, ROW_REVERSE) == 0) {
      return WeexCore::kFlexDirectionRowReverse;
    }
    return WeexCore::kFlexDirectionColumn;
  }

  const WXCoreJustifyContent GetWXCoreJustifyContent(const std::string &value) {
    const char *c_value = value.c_str();
    if (strcmp(c_value, FLEX_START) == 0) {
      return WeexCore::kJustifyFlexStart;
    } else if (strcmp(c_value, FLEX_END) == 0) {
      return WeexCore::kJustifyFlexEnd;
    } else if (strcmp(c_value, CENTER) == 0) {
      return WeexCore::kJustifyCenter;
    } else if (strcmp(c_value, SPACE_BETWEEN) == 0) {
      return WeexCore::kJustifySpaceBetween;
    } else if (strcmp(c_value, SPACE_AROUND) == 0) {
      return WeexCore::kJustifySpaceAround;
    }
    return WeexCore::kJustifyFlexStart;
  }

  const WXCoreAlignItems GetWXCoreAlignItem(const std::string &value) {
    const char *c_value = value.c_str();
    if (strcmp(c_value, STRETCH) == 0) {
      return WeexCore::kAlignItemsStretch;
    } else if (strcmp(c_value, FLEX_START) == 0) {
      return WeexCore::kAlignItemsFlexStart;
    } else if (strcmp(c_value, FLEX_END) == 0) {
      return WeexCore::kAlignItemsFlexEnd;
    } else if (strcmp(c_value, CENTER) == 0) {
      return WeexCore::kAlignItemsCenter;
    }
    return WeexCore::kAlignItemsStretch;
  }

  const WXCoreFlexWrap GetWXCoreFlexWrap(const std::string &value) {
    const char *c_value = value.c_str();
    if (strcmp(c_value, NOWRAP) == 0) {
      return WeexCore::kNoWrap;
    } else if (strcmp(c_value, WRAP) == 0) {
      return WeexCore::kWrap;
    } else if (strcmp(c_value, WRAP_REVERSE) == 0) {
      return WeexCore::kWrapReverse;
    }
    return WeexCore::kNoWrap;
  }

  const WXCoreAlignSelf GetWXCoreAlignSelf(const std::string &value) {
    const char *c_value = value.c_str();
    if (strcmp(c_value, AUTO) == 0) {
      return WeexCore::kAlignSelfAuto;
    } else if (strcmp(c_value, STRETCH) == 0) {
      return WeexCore::kAlignSelfStretch;
    } else if (strcmp(c_value, FLEX_START) == 0) {
      return WeexCore::kAlignSelfFlexStart;
    } else if (strcmp(c_value, FLEX_END) == 0) {
      return WeexCore::kAlignSelfFlexEnd;
    } else if (strcmp(c_value, CENTER) == 0) {
      return WeexCore::kAlignSelfCenter;
    }
    return WeexCore::kAlignSelfAuto;
  }

  const WXCorePositionType GetWXCorePositionType(const std::string &value) {
    const char *c_value = value.c_str();
    auto ret = kRelative;
    if (strcmp(c_value, RELATIVE) == 0) {
      ret = kRelative;
    } else if (strcmp(c_value, ABSOLUTE) == 0) {
      ret = kAbsolute;
    } else if(strcmp(c_value, STICKY) == 0){
      ret = kSticky;
    }else if(strcmp(c_value, FIXED) == 0){
      ret = kFixed;
    }
    return ret;
  }
}
