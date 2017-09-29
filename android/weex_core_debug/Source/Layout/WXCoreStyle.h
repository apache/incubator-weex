#ifndef WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H
#define WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H

#include <stdarg.h>
#include <math.h>

/**
 * MainAxis direction
 */
enum WXCoreFlexDirection {
  WXCore_Flex_Direction_Column,
  WXCore_Flex_Direction_Column_Reverse,
  WXCore_Flex_Direction_Row,
  WXCore_Flex_Direction_Row_Reverse,
  WXCore_Flex_Direction_Undefined,
};

/**
 * Controls the position of the element on the MainAxis
 */
enum WXCoreJustifyContent {
  WXCore_Justify_Flex_Start,
  WXCore_Justify_Center,
  WXCore_Justify_Flex_End,
  WXCore_Justify_Space_Between,
  WXCore_Justify_Space_Around,
  WXCore_Justify_Undefined,
};

/**
 * Controls the position of the element on the CrossAxis and whether Stretch
 */
enum WXCoreAlignItems {
  WXCore_AlignItems_Flex_Start,
  WXCore_AlignItems_Center,
  WXCore_AlignItems_Flex_End,
  WXCore_AlignItems_Baseline,
  WXCore_AlignItems_Stretch,
  WXCore_AlignItems_Undefined,
};

/**
 * Controls the count of flexlines
 */
enum WXCoreFlexWrap {
  WXCore_Wrap_NoWrap,
  WXCore_Wrap_Wrap,
  WXCore_Wrap_WrapReverse,
  WXCore_Wrap_Undefined,
};

/**
 * The align-self will overrides the align-items specified by the Flex container.
 * The two attributes have the same range of values.
 */
enum WXCoreAlignSelf {
  WXCore_AlignSelf_Auto = -1,
  WXCore_AlignSelf_Flex_Start = WXCore_AlignItems_Flex_Start,
  WXCore_AlignSelf_Center = WXCore_AlignItems_Center,
  WXCore_AlignSelf_Flex_End = WXCore_AlignItems_Flex_End,
  WXCore_AlignSelf_Baseline = WXCore_AlignItems_Baseline,
  WXCore_AlignSelf_Stretch = WXCore_AlignItems_Stretch,
  WXCore_AlignSelf_Undefined,
};

enum WXCorePositionType {
  WXCore_PositionType_Relative,
  WXCore_PositionType_Absolute,
  WXCore_PositionType_Undefined,
};

enum WXCorePositionEdge {
  WXCore_PositionEdge_Top,
  WXCore_PositionEdge_Bottom,
  WXCore_PositionEdge_Left,
  WXCore_PositionEdge_Right,
  WXCore_PositionEdge_Undefined,
};

enum WXCoreMarginEdge {
  WXCore_Margin_ALL,
  WXCore_Margin_Top,
  WXCore_Margin_Bottom,
  WXCore_Margin_Left,
  WXCore_Margin_Right,
  WXCore_Margin_Undefined,
};

enum WXCorePaddingEdge {
  WXCore_Padding_ALL,
  WXCore_Padding_Top,
  WXCore_Padding_Bottom,
  WXCore_Padding_Left,
  WXCore_Padding_Right,
  WXCore_Padding_Undefined,
};

enum WXCoreBorderWidthEdge {
  WXCore_Border_Width_ALL,
  WXCore_Border_Width_Top,
  WXCore_Border_Width_Bottom,
  WXCore_Border_Width_Left,
  WXCore_Border_Width_Right,
  WXCore_Border_Width_Undefined,
};


typedef struct WXCoreLayoutSize {
  float width;
  float height;
} WXCoreLayoutSize;


class WXCoreMargin {

private:

  float mMarginTop;
  float mMarginBottom;
  float mMarginLeft;
  float mMarginRight;

public:

  WXCoreMargin() : mMarginTop(0),
                   mMarginBottom(0),
                   mMarginLeft(0),
                   mMarginRight(0) {}

  bool setMargin(WXCoreMarginEdge edge, float margin) {
    bool dirty = false;
    switch (edge) {
      case WXCore_Margin_ALL:
        if (mMarginLeft != margin
            || mMarginTop != margin
            || mMarginRight != margin
            || mMarginBottom != margin) {
          mMarginLeft += margin;
          mMarginTop += margin;
          mMarginRight += margin;
          mMarginBottom += margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Left:
        if (mMarginLeft != margin) {
          mMarginLeft = margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Top:
        if (mMarginTop != margin) {
          mMarginTop = margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Right:
        if (mMarginRight != margin) {
          mMarginRight = margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Bottom:
        if (mMarginBottom != margin) {
          mMarginBottom = margin;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float getMargin(WXCoreMarginEdge edge) {
    float margin = 0;
    switch (edge) {
      case WXCore_Margin_Left:
        margin = mMarginLeft;
        break;
      case WXCore_Margin_Top:
        margin = mMarginTop;
        break;
      case WXCore_Margin_Right:
        margin = mMarginRight;
        break;
      case WXCore_Margin_Bottom:
        margin = mMarginBottom;
        break;
    }
    return margin;
  }
};

class WXCorePadding {

private:

  float mPaddingTop;
  float mPaddingBottom;
  float mPaddingLeft;
  float mPaddingRight;

public:

  WXCorePadding() : mPaddingTop(0),
                    mPaddingBottom(0),
                    mPaddingLeft(0),
                    mPaddingRight(0) {}

  bool setPadding(WXCorePaddingEdge edge, float padding) {
    bool dirty = false;
    switch (edge) {
      case WXCore_Padding_ALL:
        if (mPaddingLeft != padding
            || mPaddingTop != padding
            || mPaddingRight != padding
            || mPaddingBottom != padding) {
          mPaddingLeft += padding;
          mPaddingTop += padding;
          mPaddingRight += padding;
          mPaddingBottom += padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Left:
        if (mPaddingLeft != padding) {
          mPaddingLeft = padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Top:
        if (mPaddingTop != padding) {
          mPaddingTop = padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Right:
        if (mPaddingRight != padding) {
          mPaddingRight = padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Bottom:
        if (mPaddingBottom != padding) {
          mPaddingBottom = padding;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float getPadding(WXCorePaddingEdge edge) {
    float padding = 0;
    switch (edge) {
      case WXCore_Padding_Left:
        padding = mPaddingLeft;
        break;
      case WXCore_Padding_Top:
        padding = mPaddingTop;
        break;
      case WXCore_Padding_Right:
        padding = mPaddingRight;
        break;
      case WXCore_Padding_Bottom:
        padding = mPaddingBottom;
        break;
    }
    return padding;
  }
};

class WXCoreBorderWidth {

private:

  float mBorderWidthTop;
  float mBorderWidthBottom;
  float mBorderWidthLeft;
  float mBorderWidthRight;

public:

  WXCoreBorderWidth() : mBorderWidthTop(0),
                        mBorderWidthBottom(0),
                        mBorderWidthLeft(0),
                        mBorderWidthRight(0) {}

  bool setBorderWidth(WXCoreBorderWidthEdge edge, float borderWidth) {
    bool dirty = false;
    switch (edge) {
      case WXCore_Border_Width_ALL:
        if (mBorderWidthLeft != borderWidth
            || mBorderWidthTop != borderWidth
            || mBorderWidthRight != borderWidth
            || mBorderWidthBottom != borderWidth) {
          mBorderWidthLeft += borderWidth;
          mBorderWidthTop += borderWidth;
          mBorderWidthRight += borderWidth;
          mBorderWidthBottom += borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Left:
        if (mBorderWidthLeft != borderWidth) {
          mBorderWidthLeft = borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Top:
        if (mBorderWidthTop != borderWidth) {
          mBorderWidthTop = borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Right:
        if (mBorderWidthRight != borderWidth) {
          mBorderWidthRight = borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Bottom:
        if (mBorderWidthBottom != borderWidth) {
          mBorderWidthBottom = borderWidth;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float getBorderWidth(WXCoreBorderWidthEdge edge) {
    float borderWidth = 0;
    switch (edge) {
      case WXCore_Border_Width_Left:
        borderWidth = mBorderWidthLeft;
        break;
      case WXCore_Border_Width_Top:
        borderWidth = mBorderWidthTop;
        break;
      case WXCore_Border_Width_Right:
        borderWidth = mBorderWidthRight;
        break;
      case WXCore_Border_Width_Bottom:
        borderWidth = mBorderWidthBottom;
        break;
    }
    return borderWidth;
  }
};

class WXCorePosition {
private:

  float mTop;
  float mBottom;
  float mLeft;
  float mRight;

public:

  WXCorePosition() : mTop(0),
                     mBottom(0),
                     mLeft(0),
                     mRight(0) {}

  bool setPosition(WXCorePositionEdge edge, float position) {
    bool dirty = false;
    switch (edge) {
      case WXCore_PositionEdge_Left:
        if (mLeft != position) {
          mLeft = position;
          dirty = true;
        }
        break;
      case WXCore_PositionEdge_Top:
        if (mTop != position) {
          mTop = position;
          dirty = true;
        }
        break;
      case WXCore_PositionEdge_Right:
        if (mRight != position) {
          mRight = position;
          dirty = true;
        }
        break;
      case WXCore_PositionEdge_Bottom:
        if (mBottom != position) {
          mBottom = position;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float getPosition(WXCorePositionEdge edge) {
    float position = 0;
    switch (edge) {
      case WXCore_PositionEdge_Left:
        position = mLeft;
        break;
      case WXCore_PositionEdge_Top:
        position = mTop;
        break;
      case WXCore_PositionEdge_Right:
        position = mRight;
        break;
      case WXCore_PositionEdge_Bottom:
        position = mBottom;
        break;
    }
    return position;
  }
};

#endif //WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H
