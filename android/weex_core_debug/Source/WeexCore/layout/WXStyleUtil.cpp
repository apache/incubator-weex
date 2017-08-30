#include "WXStyleUtil.h"

YGFlexDirection getYGFlexDirection(string value) {
  if (value == COLUMN) {
    return YGFlexDirectionColumn;
  } else if (value == ROW) {
    return YGFlexDirectionRow;
  }
  return YGFlexDirectionColumn;
}

YGJustify getYGJustifyContent(string value) {
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

YGAlign getYGAlignItem(string value) {
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

YGPositionType getYGPositionType(string value) {
  if (value == RELATIVE) {
    return YGPositionTypeRelative;
  } else if (value == ABSOLUTE) {
    return YGPositionTypeAbsolute;
  }
  return YGPositionTypeRelative;
}

YGAlign getYGAlignSelf(string value) {
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

YGWrap getYGWrap(string value) {
  if (value == NOWRAP) {
    return YGWrapNoWrap;
  } else if (value == WRAP) {
    return YGWrapWrap;
  }
  return YGWrapNoWrap;
}

YGOverflow getYGOverflow(string value) {
  if (value == VISIBLE) {
    return YGOverflowVisible;
  } else if (value == HIDDEN) {
    return YGOverflowHidden;
  } else if (value == SCROLL) {
    return YGOverflowScroll;
  }
  return YGOverflowVisible;
}