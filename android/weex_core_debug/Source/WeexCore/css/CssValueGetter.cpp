#include "CssValueGetter.h"
#include "ConstantsValue.h"
#include "ConstantsName.h"
#include "WeexCore/layout/WXCoreStyle.h"

namespace WeexCore {

  WXCoreFlexDirection GetWXCoreFlexDirection(const std::string &value) {
    if (value == COLUMN) {
      return WeexCore::kFlexDirectionColumn;
    } else if (value == ROW) {
      return WeexCore::kFlexDirectionRow;
    } else if (value == COLUMN_REVERSE) {
      return WeexCore::kFlexDirectionColumnReverse;
    } else if (value == ROW_REVERSE) {
      return WeexCore::kFlexDirectionRowReverse;
    }
    return WeexCore::kFlexDirectionColumn;
  }

  WXCoreJustifyContent GetWXCoreJustifyContent(const std::string &value) {
    if (value == FLEX_START) {
      return WeexCore::kJustifyFlexStart;
    } else if (value == FLEX_END) {
      return WeexCore::kJustifyFlexEnd;
    } else if (value == CENTER) {
      return WeexCore::kJustifyCenter;
    } else if (value == SPACE_BETWEEN) {
      return WeexCore::kJustifySpaceBetween;
    } else if (value == SPACE_AROUND) {
      return WeexCore::kJustifySpaceAround;
    }
    return WeexCore::kJustifyFlexStart;
  }

  WXCoreAlignItems GetWXCoreAlignItem(const std::string &value) {
    if (value == STRETCH) {
      return WeexCore::kAlignItemsStretch;
    } else if (value == FLEX_START) {
      return WeexCore::kAlignItemsFlexStart;
    } else if (value == FLEX_END) {
      return WeexCore::kAlignItemsFlexEnd;
    } else if (value == CENTER) {
      return WeexCore::kAlignItemsCenter;
    }
    return WeexCore::kAlignItemsStretch;
  }

  WXCoreFlexWrap GetWXCoreFlexWrap(const std::string &value) {
    if (value == NOWRAP) {
      return WeexCore::kNoWrap;
    } else if (value == WRAP) {
      return WeexCore::kWrap;
    } else if (value == WRAP_REVERSE) {
      return WeexCore::kWrapReverse;
    }
    return WeexCore::kNoWrap;
  }

  WXCoreAlignSelf GetWXCoreAlignSelf(const std::string &value) {
    if (value == AUTO) {
      return WeexCore::kAlignSelfAuto;
    } else if (value == STRETCH) {
      return WeexCore::kAlignSelfStretch;
    } else if (value == FLEX_START) {
      return WeexCore::kAlignSelfFlexStart;
    } else if (value == FLEX_END) {
      return WeexCore::kAlignSelfFlexEnd;
    } else if (value == CENTER) {
      return WeexCore::kAlignSelfCenter;
    }
    return WeexCore::kAlignSelfAuto;
  }

  WXCorePositionType GetWXCorePositionType(const std::string &value) {
    if (value == RELATIVE) {
      return WeexCore::kRelative;
    } else if (value == ABSOLUTE) {
      return WeexCore::kAbsolute;
    }
    return WeexCore::kRelative;
  }
}
