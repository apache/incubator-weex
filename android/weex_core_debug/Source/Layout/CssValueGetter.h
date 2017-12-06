#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "ConstantsValue.h"
#include "ConstantsName.h"
#include "WXCoreStyle.h"
#include "WXCoreFlexEnum.h"
#include <string>

inline WeexCore::WXCoreFlexDirection getWXCoreFlexDirection(std::string value) {
  if (value == COLUMN) {
    return WeexCore::WXCore_Flex_Direction_Column;
  } else if (value == ROW) {
    return WeexCore::WXCore_Flex_Direction_Row;
  } else if (value == COLUMN_REVERSE) {
    return WeexCore::WXCore_Flex_Direction_Column_Reverse;
  } else if (value == ROW_REVERSE) {
    return WeexCore::WXCore_Flex_Direction_Row_Reverse;
  }
  return WeexCore::WXCore_Flex_Direction_Column;
}


inline WeexCore::WXCoreJustifyContent getWXCoreJustifyContent(std::string value) {
  if (value == FLEX_START) {
    return WeexCore::WXCore_Justify_Flex_Start;
  } else if (value == FLEX_END) {
    return WeexCore::WXCore_Justify_Flex_End;
  } else if (value == CENTER) {
    return WeexCore::WXCore_Justify_Center;
  } else if (value == SPACE_BETWEEN) {
    return WeexCore::WXCore_Justify_Space_Between;
  } else if (value == SPACE_AROUND) {
    return WeexCore::WXCore_Justify_Space_Around;
  }
  return WeexCore::WXCore_Justify_Flex_Start;
}


inline WeexCore::WXCoreAlignItems getWXCoreAlignItem(std::string value) {
  if (value == STRETCH) {
    return WeexCore::WXCore_AlignItems_Stretch;
  } else if (value == FLEX_START) {
    return WeexCore::WXCore_AlignItems_Flex_Start;
  } else if (value == FLEX_END) {
    return WeexCore::WXCore_AlignItems_Flex_End;
  } else if (value == CENTER) {
    return WeexCore::WXCore_AlignItems_Center;
  }
  return WeexCore::WXCore_AlignItems_Stretch;
}


inline WeexCore::WXCoreFlexWrap getWXCoreFlexWrap(std::string value) {
  if (value == NOWRAP) {
    return WeexCore::WXCore_Wrap_NoWrap;
  } else if (value == WRAP) {
    return WeexCore::WXCore_Wrap_Wrap;
  } else if (value == WRAP_REVERSE) {
    return WeexCore::WXCore_Wrap_WrapReverse;
  }
  return WeexCore::WXCore_Wrap_NoWrap;
}


inline WeexCore::WXCoreAlignSelf getWXCoreAlignSelf(std::string value) {
  if (value == AUTO) {
    return WeexCore::WXCore_AlignSelf_Auto;
  } else if (value == STRETCH) {
    return WeexCore::WXCore_AlignSelf_Stretch;
  } else if (value == FLEX_START) {
    return WeexCore::WXCore_AlignSelf_Flex_Start;
  } else if (value == FLEX_END) {
    return WeexCore::WXCore_AlignSelf_Flex_End;
  } else if (value == CENTER) {
    return WeexCore::WXCore_AlignSelf_Center;
  }
  return WeexCore::WXCore_AlignSelf_Auto;
}


inline WeexCore::WXCorePositionType getWXCorePositionType(std::string value) {
  if (value == RELATIVE) {
    return WeexCore::WXCore_PositionType_Relative;
  } else if (value == ABSOLUTE) {
    return WeexCore::WXCore_PositionType_Absolute;
  }
  return WeexCore::WXCore_PositionType_Relative;
}


#endif //WEEX_PROJECT_STYLEGETTER_H
