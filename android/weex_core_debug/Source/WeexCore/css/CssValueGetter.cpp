#include "CssValueGetter.h"
#include "ConstantsValue.h"
#include "ConstantsName.h"
#include "WeexCore/layout/WXCoreStyle.h"

namespace WeexCore {

  const WXCoreFlexDirection &GetWXCoreFlexDirection(const std::string &value) {
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

  const WXCoreJustifyContent &GetWXCoreJustifyContent(const std::string &value) {
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

  const WXCoreAlignItems &GetWXCoreAlignItem(const std::string &value) {
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

  const WXCoreFlexWrap &GetWXCoreFlexWrap(const std::string &value) {
    if (value == NOWRAP) {
      return WeexCore::kNoWrap;
    } else if (value == WRAP) {
      return WeexCore::kWrap;
    } else if (value == WRAP_REVERSE) {
      return WeexCore::kWrapReverse;
    }
    return WeexCore::kNoWrap;
  }

  const WXCoreAlignSelf &GetWXCoreAlignSelf(const std::string &value) {
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

  const WXCorePositionType &GetWXCorePositionType(const std::string &value) {
    auto ret = kRelative;
    if (value == RELATIVE) {
      ret = kRelative;
    } else if (value == ABSOLUTE) {
      ret = kAbsolute;
    } else if(value == STICKY){
      ret = kSticky;
    }else if(value == FIXED){
      ret = kFixed;
    }
    return ret;
  }
}
