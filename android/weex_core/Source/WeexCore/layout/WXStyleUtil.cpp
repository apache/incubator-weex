//
// Created by miomin on 2017/7/28.
//
#include "WXStyleUtil.h"

using namespace WTF;

YGFlexDirection getYGFlexDirection(String value) {
  if (value == COLUMN) {
    return YGFlexDirectionColumn;
  } else if (value == ROW) {
    return YGFlexDirectionRow;
  }
  return YGFlexDirectionColumn;
}

YGJustify getYGJustifyContent(String value) {
  if (value == FLEX_START) {
    return YGJustifyFlexStart;
  } else if (value == FLEX_END) {
    return YGJustifyFlexEnd;
  } else if (value == CENTER) {
    return YGJustifyCenter;
  } else if (value == SPACE_BETWEEN) {
    return YGJustifySpaceBetween;
  }
  return YGJustifyFlexStart;
}

YGAlign getYGAlignItem(String value) {
  if (value == STRETCH) {
    return YGAlignStretch;
  } else if (value == FLEX_START) {
    return YGAlignFlexStart;
  } else if (value == FLEX_END) {
    return YGAlignFlexEnd;
  } else if (value == CENTER) {
    return YGAlignCenter;
  }
  return YGAlignStretch;
}

YGPositionType getYGPositionType(String value) {
  if (value == RELATIVE) {
    return YGPositionTypeRelative;
  } else if (value == ABSOLUTE) {
    return YGPositionTypeAbsolute;
  }
  return YGPositionTypeRelative;
}

YGAlign getYGAlignSelf(String value) {
  if (value == AUTO) {
    return YGAlignAuto;
  } else if (value == STRETCH) {
    return YGAlignStretch;
  } else if (value == FLEX_START) {
    return YGAlignFlexStart;
  } else if (value == FLEX_END) {
    return YGAlignFlexEnd;
  } else if (value == CENTER) {
    return YGAlignCenter;
  }
  return YGAlignAuto;
}

YGWrap getYGWrap(String value) {
  if (value == NOWRAP) {
    return YGWrapNoWrap;
  } else if (value == WRAP) {
    return YGWrapWrap;
  }
  return YGWrapNoWrap;
}

YGOverflow getYGOverflow(String value) {
  if (value == VISIBLE) {
    return YGOverflowVisible;
  } else if (value == HIDDEN) {
    return YGOverflowHidden;
  } else if (value == SCROLL) {
    return YGOverflowScroll;
  }
  return YGOverflowVisible;
}