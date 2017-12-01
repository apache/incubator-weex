#ifndef WEEX_PROJECT_STYLEGETTER_H
#define WEEX_PROJECT_STYLEGETTER_H

#include "ConstantsValue.h"
#include "ConstantsName.h"
#include "WXCoreStyle.h"
#include "WXCoreFlexEnum.h"
#include <string>

inline WXCoreFlexLayout::WXCoreFlexDirection getWXCoreFlexDirection(std::string value) {
  if (value == COLUMN) {
    return WXCoreFlexLayout::WXCore_Flex_Direction_Column;
  } else if (value == ROW) {
    return WXCoreFlexLayout::WXCore_Flex_Direction_Row;
  } else if (value == COLUMN_REVERSE) {
    return WXCoreFlexLayout::WXCore_Flex_Direction_Column_Reverse;
  } else if (value == ROW_REVERSE) {
    return WXCoreFlexLayout::WXCore_Flex_Direction_Row_Reverse;
  }
  return WXCoreFlexLayout::WXCore_Flex_Direction_Column;
}


inline WXCoreFlexLayout::WXCoreJustifyContent getWXCoreJustifyContent(std::string value) {
  if (value == FLEX_START) {
    return WXCoreFlexLayout::WXCore_Justify_Flex_Start;
  } else if (value == FLEX_END) {
    return WXCoreFlexLayout::WXCore_Justify_Flex_End;
  } else if (value == CENTER) {
    return WXCoreFlexLayout::WXCore_Justify_Center;
  } else if (value == SPACE_BETWEEN) {
    return WXCoreFlexLayout::WXCore_Justify_Space_Between;
  } else if (value == SPACE_AROUND) {
    return WXCoreFlexLayout::WXCore_Justify_Space_Around;
  }
  return WXCoreFlexLayout::WXCore_Justify_Flex_Start;
}


inline WXCoreFlexLayout::WXCoreAlignItems getWXCoreAlignItem(std::string value) {
  if (value == STRETCH) {
    return WXCoreFlexLayout::WXCore_AlignItems_Stretch;
  } else if (value == FLEX_START) {
    return WXCoreFlexLayout::WXCore_AlignItems_Flex_Start;
  } else if (value == FLEX_END) {
    return WXCoreFlexLayout::WXCore_AlignItems_Flex_End;
  } else if (value == CENTER) {
    return WXCoreFlexLayout::WXCore_AlignItems_Center;
  }
  return WXCoreFlexLayout::WXCore_AlignItems_Stretch;
}


inline WXCoreFlexLayout::WXCoreFlexWrap getWXCoreFlexWrap(std::string value) {
  if (value == NOWRAP) {
    return WXCoreFlexLayout::WXCore_Wrap_NoWrap;
  } else if (value == WRAP) {
    return WXCoreFlexLayout::WXCore_Wrap_Wrap;
  } else if (value == WRAP_REVERSE) {
    return WXCoreFlexLayout::WXCore_Wrap_WrapReverse;
  }
  return WXCoreFlexLayout::WXCore_Wrap_NoWrap;
}


inline WXCoreFlexLayout::WXCoreAlignSelf getWXCoreAlignSelf(std::string value) {
  if (value == AUTO) {
    return WXCoreFlexLayout::WXCore_AlignSelf_Auto;
  } else if (value == STRETCH) {
    return WXCoreFlexLayout::WXCore_AlignSelf_Stretch;
  } else if (value == FLEX_START) {
    return WXCoreFlexLayout::WXCore_AlignSelf_Flex_Start;
  } else if (value == FLEX_END) {
    return WXCoreFlexLayout::WXCore_AlignSelf_Flex_End;
  } else if (value == CENTER) {
    return WXCoreFlexLayout::WXCore_AlignSelf_Center;
  }
  return WXCoreFlexLayout::WXCore_AlignSelf_Auto;
}


inline WXCoreFlexLayout::WXCorePositionType getWXCorePositionType(std::string value) {
  if (value == RELATIVE) {
    return WXCoreFlexLayout::WXCore_PositionType_Relative;
  } else if (value == ABSOLUTE) {
    return WXCoreFlexLayout::WXCore_PositionType_Absolute;
  }
  return WXCoreFlexLayout::WXCore_PositionType_Relative;
}


#endif //WEEX_PROJECT_STYLEGETTER_H
