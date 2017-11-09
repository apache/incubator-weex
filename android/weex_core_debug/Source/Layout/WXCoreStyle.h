#ifdef __cplusplus

#ifndef WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H
#define WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H


#include "WXCoreFlexEnum.h"
#include <math.h>

namespace WXCoreFlexLayout {

  /**
   * Margin: margin-left、margin-right、margin-top、margin-bottom
   */
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

    ~WXCoreMargin() {
      mMarginTop = 0;
      mMarginBottom = 0;
      mMarginLeft = 0;
      mMarginRight = 0;
    }

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
        default:
          break;
      }
      return margin;
    }
  };


  /**
   * Padding：padding-left、padding-right、padding-top、padding-bottom
   */
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

    ~WXCorePadding() {
      mPaddingTop = 0;
      mPaddingBottom = 0;
      mPaddingLeft = 0;
      mPaddingRight = 0;
    }

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
        default:
          break;
      }
      return padding;
    }
  };


  /**
   * BorderWidth：borderwidth-left、borderwidth-right、borderwidth-top、borderwidth-bottom
   */
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

    ~WXCoreBorderWidth() {
      mBorderWidthTop = 0;
      mBorderWidthBottom = 0;
      mBorderWidthLeft = 0;
      mBorderWidthRight = 0;
    }

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
        default:
          break;
      }
      return borderWidth;
    }
  };


  /**
   * position：left、right、top、bottom
   */
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

    ~WXCorePosition() {
      reset();
    }

    void reset() {
      mTop = 0;
      mBottom = 0;
      mLeft = 0;
      mRight = 0;
    }

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


  /**
   * css-style
   */
  class WXCoreCSSStyle {
  public:
    /**
     * The direction children items are placed inside the Flexbox layout, it determines the
     * direction of the main axis (and the cross axis, perpendicular to the main axis).
     * The default value is {@link WXCoreFlexDirection #WXCore_Flex_Direction_Row}.
     */
    WXCoreFlexDirection mFlexDirection;

    /**
     * This attribute controls whether the flex container is single-line or multi-line, and the
     * direction of the cross axis.
     * <ul>
     * <li>{@link WXCoreFlexWrap}: The flex container is single-line.</li>
     * <li>{@link WXCoreFlexWrap}: The flex container is multi-line.</li>
     * <li>{@link WXCoreFlexWrap}: The flex container is multi-line. The direction of the
     * cross axis is opposed to the direction as the {@link WXCoreFlexWrap}</li>
     * </ul>
     * The default value is {@link WXCoreFlexWrap #WXCore_Wrap_NoWrap}.
     */
    WXCoreFlexWrap mFlexWrap;

    /**
     * This attribute controls the alignment along the main axis.
     * The default value is {@link WXCoreJustifyContent #WXCore_Justify_Flex_Start}.
     */
    WXCoreJustifyContent mJustifyContent;

    /**
     * This attribute controls the alignment along the cross axis.
     * The default value is {@link WXCoreAlignItems #WXCore_AlignItems_Stretch}.
     */
    WXCoreAlignItems mAlignItems;

    /**
     * This attribute controls the alignment along the cross axis.
     * The default value is {@link WXCoreAlignSelf #WXCore_AlignSelf_Auto}.
     */
    WXCoreAlignSelf mAlignSelf;

    float mFlexGrow;

    float mMinWidth;

    float mMinHeight;

    float mMaxWidth;

    float mMaxHeight;

    float mStyleWidth;

    float mStyleHeight;

    WXCoreMargin mMargin;

    WXCorePadding mPadding;

    WXCoreBorderWidth mBorderWidth;

    WXCorePosition mStylePosition;

    WXCorePositionType mPositionType;

    const static WXCoreFlexDirection FLEX_DIRECTION_DEFAULT;

    const static WXCoreFlexWrap FLEX_WRAP_DEFAULT;

    const static WXCoreJustifyContent FLEX_JUSTIFY_CONTENT_DEFAULT;

    const static WXCoreAlignItems FLEX_ALIGN_ITEMS_DEFAULT;

    const static WXCoreAlignSelf FLEX_ALIGN_SELF_DEFAULT;

    const static float FLEX_DEFAULT;

    const static float FLEX_GROW_DEFAULT;

    const static float MAX_SIZE;

    WXCoreCSSStyle() : mFlexDirection(FLEX_DIRECTION_DEFAULT),
                       mFlexWrap(FLEX_WRAP_DEFAULT),
                       mJustifyContent(FLEX_JUSTIFY_CONTENT_DEFAULT),
                       mAlignItems(FLEX_ALIGN_ITEMS_DEFAULT),
                       mAlignSelf(FLEX_ALIGN_SELF_DEFAULT),
                       mFlexGrow(FLEX_GROW_DEFAULT),
                       mStyleWidth(NAN), mStyleHeight(NAN),
                       mMaxWidth(MAXFLOAT), mMaxHeight(MAXFLOAT),
                       mMinWidth(NAN), mMinHeight(NAN) {

    }

    ~WXCoreCSSStyle() {
      mFlexDirection = FLEX_DIRECTION_DEFAULT;
      mFlexWrap = FLEX_WRAP_DEFAULT;
      mJustifyContent = FLEX_JUSTIFY_CONTENT_DEFAULT;
      mAlignItems = FLEX_ALIGN_ITEMS_DEFAULT;
      mAlignSelf = FLEX_ALIGN_SELF_DEFAULT;
      mFlexGrow = FLEX_GROW_DEFAULT;
      mStyleWidth = NAN;
      mStyleHeight = NAN;
      mMaxWidth = MAXFLOAT;
      mMaxHeight = MAXFLOAT;
      mMinWidth = NAN;
      mMinHeight = NAN;
    }
  };
}
#endif //WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H
#endif