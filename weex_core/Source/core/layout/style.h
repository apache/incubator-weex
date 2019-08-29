/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifdef __cplusplus

#ifndef WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H
#define WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H


#include "flex_enum.h"
#include <math.h>
#include <cmath>

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

    bool setMargin(const WXCoreMarginEdge &edge, float margin);

    float getMargin(const WXCoreMarginEdge &edge)const;
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

    bool setPadding(const WXCorePaddingEdge &edge, float padding);

    float getPadding(const WXCorePaddingEdge &edge)const;
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

    bool setBorderWidth(const WXCoreBorderWidthEdge &edge, float borderWidth);

    float getBorderWidth(const WXCoreBorderWidthEdge &edge)const;
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

    bool setPosition(const WXCorePositionEdge &edge, float position);

    float getPosition(const WXCorePositionEdge &edge);
  };

  enum DimensionLevel{
    CSS_STYLE = 1,
    INSTANCE_STYLE = 2,
    FALLBACK_STYLE = 3
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

    WXCorePositionType mPositionType;
      

    float mFlexGrow;

    float mMinWidth;

    float mMinHeight;

    float mMaxWidth;

    float mMaxHeight;

    float mStyleWidth;

    float mStyleHeight;

    DimensionLevel mStyleWidthLevel;

    DimensionLevel mStyleHeightLevel;

    WXCoreMargin mMargin;

    WXCorePadding mPadding;

    WXCoreBorderWidth mBorderWidth;

    WXCorePosition mStylePosition;

    WXCoreDirection mDirection;

    constexpr static float kFlexGrowDefault  = 0;
      
    constexpr static WXCoreDirection kDirectionDefault = kDirectionLTR;

    constexpr static WXCoreFlexDirection kFlexDirectionDefault= kFlexDirectionColumn;

    constexpr static WXCoreFlexWrap kFlexWrapDefault = kNoWrap;

    constexpr static WXCoreJustifyContent kFlexJustifyContentDefault = kJustifyFlexStart;

    constexpr static WXCoreAlignItems kFlexAlignItemsDefault = kAlignItemsStretch;

    constexpr static WXCoreAlignSelf kFlexAlignSelfDefault = kAlignSelfAuto;

    constexpr static WXCorePositionType kWXCorePositionTypeDefault = kRelative;

    WXCoreCSSStyle() :
                       mFlexDirection(kFlexDirectionDefault),
                       mFlexWrap(kFlexWrapDefault),
                       mJustifyContent(kFlexJustifyContentDefault),
                       mAlignItems(kFlexAlignItemsDefault),
                       mAlignSelf(kFlexAlignSelfDefault),
                       mFlexGrow(kFlexGrowDefault),
                       mPositionType(kWXCorePositionTypeDefault),
                       mStyleWidth(NAN), mStyleHeight(NAN),
                       mStyleHeightLevel(FALLBACK_STYLE), mStyleWidthLevel(FALLBACK_STYLE),
                       mMaxWidth(NAN), mMaxHeight(NAN),
                       mMinWidth(NAN), mMinHeight(NAN),
                       mDirection(kDirectionInherit) {

    }

    ~WXCoreCSSStyle() {
      mFlexDirection = kFlexDirectionDefault;
      mFlexWrap = kFlexWrapDefault;
      mJustifyContent = kFlexJustifyContentDefault;
      mAlignItems = kFlexAlignItemsDefault;
      mAlignSelf = kFlexAlignSelfDefault;
      mFlexGrow = kFlexGrowDefault;
      mStyleWidth = NAN;
      mStyleHeight = NAN;
      mStyleWidthLevel = FALLBACK_STYLE;
      mStyleHeightLevel = FALLBACK_STYLE;
      mMaxWidth = NAN;
      mMaxHeight = NAN;
      mMinWidth = NAN;
      mMinHeight = NAN;
      mDirection = kDirectionInherit;
    }

    inline float sumPaddingBorderOfEdge(const WXCoreEdge edge){
      switch (edge) {
        case kTop:
          return mPadding.getPadding(kPaddingTop)
              + mBorderWidth.getBorderWidth(kBorderWidthTop);
        case kRight:
          return mPadding.getPadding(kPaddingRight)
              + mBorderWidth.getBorderWidth(kBorderWidthRight);
        case kBottom:
          return mPadding.getPadding(kPaddingBottom)
              + mBorderWidth.getBorderWidth(kBorderWidthBottom);
        case kLeft:
          return mPadding.getPadding(kPaddingLeft)
              + mBorderWidth.getBorderWidth(kBorderWidthLeft);
      }
    }

    float sumMarginOfDirection(bool horizontal){
      if(horizontal){
        return mMargin.getMargin(kMarginLeft) + mMargin.getMargin(kMarginRight);
      }
      else{
        return mMargin.getMargin(kMarginTop) + mMargin.getMargin(kMarginBottom);
      }
    }
  };
}
#endif //WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUMS_H
#endif
