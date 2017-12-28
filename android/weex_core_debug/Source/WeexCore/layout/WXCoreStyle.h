#ifdef __cplusplus

#ifndef WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H
#define WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H


#include "WXCoreFlexEnum.h"
#include <math.h>

namespace WeexCore {

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

    bool setMargin(const WXCoreMarginEdge &edge, const float &margin);

    float getMargin(const WXCoreMarginEdge &edge);
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

    bool setPadding(const WXCorePaddingEdge &edge, const float &padding);

    float getPadding(const WXCorePaddingEdge &edge);
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

    bool setBorderWidth(const WXCoreBorderWidthEdge &edge, const float &borderWidth);

    float getBorderWidth(const WXCoreBorderWidthEdge &edge);
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
    WXCorePosition() : mTop(NAN),
                       mBottom(NAN),
                       mLeft(NAN),
                       mRight(NAN) {}

    ~WXCorePosition() {
      reset();
    }

    inline bool isNAN() {
      return isnan(mTop) || isnan(mBottom) || isnan(mLeft) || isnan(mRight);
    }

    inline void reset() {
      mTop = 0;
      mBottom = 0;
      mLeft = 0;
      mRight = 0;
    }

    bool setPosition(const WXCorePositionEdge &edge, const float &position);

    float getPosition(const WXCorePositionEdge &edge);
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