#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "ConstantsValue.h"
#include "ConstantsName.h"
#include "WXCoreStyle.h"
#include "WXCoreFlexEnum.h"
#include <string>

using namespace WXCoreFlexLayout;

inline WXCoreFlexDirection getWXCoreFlexDirection(std::string value) {
  if (value == COLUMN) {
    return WXCore_Flex_Direction_Column;
  } else if (value == ROW) {
    return WXCore_Flex_Direction_Row;
  } else if (value == COLUMN_REVERSE) {
    return WXCore_Flex_Direction_Column_Reverse;
  } else if (value == ROW_REVERSE) {
    return WXCore_Flex_Direction_Row_Reverse;
  }
  return WXCore_Flex_Direction_Row;
}


inline WXCoreJustifyContent getWXCoreJustifyContent(std::string value) {
  if (value == FLEX_START) {
    return WXCore_Justify_Flex_Start;
  } else if (value == FLEX_END) {
    return WXCore_Justify_Flex_End;
  } else if (value == CENTER) {
    return WXCore_Justify_Center;
  } else if (value == SPACE_BETWEEN) {
    return WXCore_Justify_Space_Between;
  } else if (value == SPACE_AROUND) {
    return WXCore_Justify_Space_Around;
  }
  return WXCore_Justify_Flex_Start;
}


inline WXCoreAlignItems getWXCoreAlignItem(std::string value) {
  if (value == STRETCH) {
    return WXCore_AlignItems_Stretch;
  } else if (value == FLEX_START) {
    return WXCore_AlignItems_Flex_Start;
  } else if (value == FLEX_END) {
    return WXCore_AlignItems_Flex_End;
  } else if (value == CENTER) {
    return WXCore_AlignItems_Center;
  } else if (value == BASELINE) {
    return WXCore_AlignItems_Baseline;
  }
  return WXCore_AlignItems_Stretch;
}


inline WXCoreFlexWrap getWXCoreFlexWrap(std::string value) {
  if (value == NOWRAP) {
    return WXCore_Wrap_NoWrap;
  } else if (value == WRAP) {
    return WXCore_Wrap_Wrap;
  } else if (value == WRAP_REVERSE) {
    return WXCore_Wrap_WrapReverse;
  }
  return WXCore_Wrap_NoWrap;
}


inline WXCoreAlignSelf getWXCoreAlignSelf(std::string value) {
  if (value == AUTO) {
    return WXCore_AlignSelf_Auto;
  } else if (value == STRETCH) {
    return WXCore_AlignSelf_Stretch;
  } else if (value == FLEX_START) {
    return WXCore_AlignSelf_Flex_Start;
  } else if (value == FLEX_END) {
    return WXCore_AlignSelf_Flex_End;
  } else if (value == CENTER) {
    return WXCore_AlignSelf_Center;
  } else if (value == BASELINE) {
    return WXCore_AlignSelf_Baseline;
  }
  return WXCore_AlignSelf_Auto;
}


inline WXCorePositionType getWXCorePositionType(std::string value) {
  if (value == RELATIVE) {
    return WXCore_PositionType_Relative;
  } else if (value == ABSOLUTE) {
    return WXCore_PositionType_Absolute;
  }
  return WXCore_PositionType_Relative;
}


#endif //WEEX_PROJECT_STYLEGETTER_H
