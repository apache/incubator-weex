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

#ifndef WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#define WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H

#include <string.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include "style.h"
#include "flex_enum.h"

namespace WeexCore {

  class WXCoreLayoutNode;

  class WXCoreFlexLine;

  enum FormattingContext {
    kBFC,
    kNonBFC,
  } ;

  enum MeasureMode {
    kUnspecified = 0,
    kExactly,
  } ;

  struct WXCoreSize {
   private:
    float hypotheticalWidth;
    float hypotheticalHeight;
   public:
    friend class WXCoreLayoutNode;
    float width;
    float height;

    WXCoreSize() : hypotheticalWidth(NAN),
                   hypotheticalHeight(NAN),
                   width(0), height(0) {}

    inline void reset() {
      hypotheticalWidth = NAN ;
      hypotheticalHeight = NAN;
      width = 0;
      height = 0;
    }

    inline bool isNAN() {
      return isnan(width) || isnan(height);
    }
  };

  /**
   * layout-result：layout-height、layout-width、position（left、right、top、bottom）、direction
   */
  struct WXCorelayoutResult {
    WXCoreDirection mLayoutDirection;
    WXCoreSize mLayoutSize;
    WXCorePosition mLayoutPosition;

    inline bool isNAN() {
      return mLayoutSize.isNAN() || mLayoutPosition.isNAN();
    }

    inline void reset() {
        mLayoutSize.reset();
        mLayoutPosition.reset();
        mLayoutDirection = kDirectionInherit;
    }
  };

  typedef WXCoreSize(*WXCoreMeasureFunc)(WXCoreLayoutNode *node, float width,
                                         MeasureMode widthMeasureMode,
                                         float height, MeasureMode heightMeasureMode);

  using Index = std::vector<WXCoreLayoutNode *>::size_type;

  /**
   * flie line
   */
  class WXCoreFlexLine {
  public:
    float mMainSize;

    float mCrossSize;

    Index mItemCount;

    float mTotalFlexGrow;

    float mTotalFlexibleSize;

    /**
     * Store the indices of the children views whose mAlignSelf property is stretch.
     * The stored indices are the absolute indices including all children in the Flexbox,
     * not the relative indices in this flex line.
     */
    std::vector<Index> mIndicesAlignSelfStretch;

    WXCoreFlexLine() : mMainSize(0),
                       mCrossSize(0),
                       mItemCount(0),
                       mTotalFlexGrow(0),
                       mTotalFlexibleSize(0) {
    }

    ~WXCoreFlexLine() {
      mMainSize = 0;
      mCrossSize = 0;
      mItemCount = 0;
      mTotalFlexGrow = 0;
      mTotalFlexibleSize = 0;
      mIndicesAlignSelfStretch.clear();
    }
  };

  /**
   * Layout node
   */
  class WXCoreLayoutNode {

  public:
      WXCoreLayoutNode() :
              mParent(nullptr),
              dirty(true),
              widthDirty{false},
              heightDirty{false},
              mHasNewLayout(true),
              mIsDestroy(false),
              measureFunc(nullptr) {
        mCssStyle = new WXCoreCSSStyle();
        mLayoutResult = new WXCorelayoutResult();
      }

      virtual ~WXCoreLayoutNode() {
        mIsDestroy = true;
        mHasNewLayout = true;
        dirty = true;
        measureFunc = nullptr;
        mParent = nullptr;
        mChildList.clear();
        BFCs.clear();
        NonBFCs.clear();
        mChildrenFrozen.clear();

        for (WXCoreFlexLine *flexLine : mFlexLines) {
          if (flexLine != nullptr) {
            delete flexLine;
            flexLine = nullptr;
          }
        }
        mFlexLines.clear();

        if (mCssStyle != nullptr) {
          delete mCssStyle;
          mCssStyle = nullptr;
        }

        if (mLayoutResult != nullptr) {
          delete mLayoutResult;
          mLayoutResult = nullptr;
        }
      }

  private:

    /**
     * Holds the 'frozen' state of children during measure. If a view is frozen it will no longer
     * expand regardless of mFlexGrow. Items are indexed by the child's
     * reordered index.
     */
    std::vector<bool> mChildrenFrozen;

    std::vector<WXCoreFlexLine *> mFlexLines;

    std::vector<WXCoreLayoutNode *> mChildList;

    std::vector<WXCoreLayoutNode *> BFCs;

    std::vector<WXCoreLayoutNode *> NonBFCs;

    WXCoreLayoutNode *mParent = nullptr;

    WXCoreCSSStyle *mCssStyle = nullptr;

    MeasureMode widthMeasureMode = kUnspecified;

    MeasureMode heightMeasureMode = kUnspecified;

    WXCorelayoutResult *mLayoutResult = nullptr;

    WXCorePosition *absoultePositon = nullptr;

    bool mHasNewLayout;

    bool dirty, widthDirty, heightDirty;

    bool mIsDestroy = true;
      
    bool mNeedsPlatformDependentLayout = false;

    WXCoreMeasureFunc measureFunc = nullptr;

    void *context = nullptr;

    /** ================================ Cache：Last calculate result =================================== **/

  public:


    /** ================================ Engine Entry Function =================================== **/

    void calculateLayout(const std::pair<float,float>&);

    /** ================================ measureFunc =================================== **/

    inline void setMeasureFunc(WXCoreMeasureFunc measure) {
      measureFunc = measure;
      markDirty();
    }

    inline bool haveMeasureFunc() const {
      return nullptr != measureFunc;
    }

    inline WXCoreMeasureFunc getMeasureFunc() const {
      return measureFunc;
    }

      /** ================================ context =================================== **/


    inline void *getContext() const {
      return context;
    }

    inline void setContext(void * const context) {
      this->context = context;
    }
      
    inline void copyStyle(WXCoreLayoutNode *srcNode) {
      if (srcNode != nullptr && memcmp(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle)) != 0) {
        memcpy(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle));
        markDirty();
      }
    }
      
    void copyFrom(WXCoreLayoutNode* srcNode){
        if (srcNode == nullptr) return;
        
        memcpy(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle));
    }

    inline void copyMeasureFunc(WXCoreLayoutNode *srcNode) {
      if (srcNode != nullptr && memcmp(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc)) != 0) {
        memcpy(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc));
        markDirty();
      }
    }
      
      /** ================================ custom =================================== **/
    inline bool getNeedsPlatformDependentLayout() const {
      return mNeedsPlatformDependentLayout;
    }
      
    inline void setNeedsPlatformDependentLayout(bool v) {
      this->mNeedsPlatformDependentLayout = v;
    }
      
  private:

    /** ================================ measure =================================== **/

    inline void reset() {
      if (isDirty()) {
        mLayoutResult->reset();
        for (WXCoreFlexLine *flexLine : mFlexLines) {
          if (flexLine != nullptr) {
            delete flexLine;
            flexLine = nullptr;
          }
        }
        mFlexLines.clear();

        mChildrenFrozen.assign(getChildCount(kNonBFC), false);
      }
      widthMeasureMode = isnan(mCssStyle->mStyleWidth) ? kUnspecified : kExactly;
      heightMeasureMode = isnan(mCssStyle->mStyleHeight) ? kUnspecified : kExactly;
    }

    inline void setLayoutWidth(const float width) {
      if (mLayoutResult->mLayoutSize.width != width &&
          (!isnan(width) || !isnan(mLayoutResult->mLayoutSize.width))) {
        mLayoutResult->mLayoutSize.width = width;
        widthDirty = true;
        markDirty(false);
      }
    }

    inline void setLayoutHeight(const float height) {
      if (mLayoutResult->mLayoutSize.height != height &&
          (!isnan(height) || !isnan(mLayoutResult->mLayoutSize.height))) {
        mLayoutResult->mLayoutSize.height = height;
        heightDirty = true;
        markDirty(false);
      }
    }

    inline void setWidthMeasureMode(const MeasureMode measureMode) {
      if (widthMeasureMode != measureMode) {
        widthMeasureMode = measureMode;
        if (getChildCount(kNonBFC) > 0) {
          widthDirty = true;
        }
      }
    }

    inline void setHeightMeasureMode(const MeasureMode measureMode) {
      if (heightMeasureMode != measureMode) {
        heightMeasureMode = measureMode;
        if (getChildCount(kNonBFC) > 0) {
          heightDirty = true;
        }
      }
    }

    inline float firstLineCrossSize() const {
      float sum = sumPaddingBorderAlongAxis(this, !isMainAxisHorizontal(this));
      if (!mFlexLines.empty()) {
        sum += mFlexLines[0]->mCrossSize;
      }
      return sum;
    }

    inline float getSumOfCrossSize() const {
      float sum = sumPaddingBorderAlongAxis(this, !isMainAxisHorizontal(this));
      for (WXCoreFlexLine *flexLine: mFlexLines) {
        sum += flexLine->mCrossSize;
      }
      return sum;
    }

    inline bool isMainAxisHorizontal(const WXCoreLayoutNode* const node) const {
      return node->mCssStyle->mFlexDirection == kFlexDirectionRow ||
             node->mCssStyle->mFlexDirection == kFlexDirectionRowReverse;
    }

    inline bool isCrossExactly() const {
      return isMainAxisHorizontal(this) ? heightMeasureMode == kExactly
                                        : widthMeasureMode == kExactly;
    }

    inline float sumPaddingBorderAlongAxis(const WXCoreLayoutNode* const node, bool horizontal) const {
      float paddingBorderAlongAxis;
      if (horizontal) {
        paddingBorderAlongAxis =
            node->mCssStyle->mPadding.getPadding(kPaddingLeft) +
            node->mCssStyle->mPadding.getPadding(kPaddingRight) +
            node->mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthLeft) +
            node->mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthRight);
      } else {
        paddingBorderAlongAxis =
            node->mCssStyle->mPadding.getPadding(kPaddingTop) +
            node->mCssStyle->mPadding.getPadding(kPaddingBottom) +
            node->mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthTop) +
            node->mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthBottom);
      }
      return paddingBorderAlongAxis;
    }

    inline bool isWrapRequired(const float &width, const float &height,
                               const float &currentLength, const float &childLength) const {
      float freeMainSize = CalculateFreeSpaceAlongMainAxis(width, height, currentLength);
      return !isSingleFlexLine(freeMainSize) && freeMainSize < childLength;
    }

    //If width/height is NAN, ret is NAN, which property we use on purpose.
    virtual float CalculateFreeSpaceAlongMainAxis(const float &width, const float &height,
                                                  const float &currentLength) const{
      float ret;
      if(isMainAxisHorizontal(this)){
        ret = width - sumPaddingBorderAlongAxis(this, true) - currentLength;
      }
      else{
        ret = height - sumPaddingBorderAlongAxis(this, false) - currentLength;
      }
      return ret;
    }

    inline bool isSingleFlexLine(const float &mainSize) const {
      return mCssStyle->mFlexWrap == kNoWrap || isnan(mainSize);
    }

    inline void sumFlexGrow(const WXCoreLayoutNode* const child, WXCoreFlexLine* const flexLine, Index i){
      if (child->mCssStyle->mFlexGrow > 0) {
        flexLine->mTotalFlexGrow += child->mCssStyle->mFlexGrow;
        mChildrenFrozen[i] = false;
        if (isMainAxisHorizontal(this)) {
          if (!isnan(child->mLayoutResult->mLayoutSize.hypotheticalWidth)) {
            flexLine->mTotalFlexibleSize += child->mLayoutResult->mLayoutSize.hypotheticalWidth;
          }
        } else {
          if (!isnan(child->mLayoutResult->mLayoutSize.hypotheticalHeight)) {
            flexLine->mTotalFlexibleSize += child->mLayoutResult->mLayoutSize.hypotheticalHeight;
          }
        }
      } else {
        mChildrenFrozen[i] = true;
      }
    }

    inline void setMeasuredDimensionForFlex(
        const float width, const MeasureMode widthMeasureMode,
        const float height, const MeasureMode heightMeasureMode){
      float actualWidth, actualHeight;
      if (isMainAxisHorizontal(this)) {
        actualWidth = widthMeasureMode == kExactly ? width : getLargestMainSize();
        actualHeight = heightMeasureMode == kExactly ? height : getSumOfCrossSize();
      } else {
        actualHeight = heightMeasureMode == kExactly ? height : getLargestMainSize();
        actualWidth = widthMeasureMode == kExactly ? width : firstLineCrossSize();
      }
      setMeasuredDimension(actualWidth, actualHeight);
    }

    inline float calcItemSizeAlongAxis(const WXCoreLayoutNode* const node, const bool horizontal, const bool useHypotheticalSize = false) const {
      float ret;
      if (horizontal) {
        ret = node->mCssStyle->mMargin.getMargin(kMarginLeft) +
            node->mCssStyle->mMargin.getMargin(kMarginRight);
        ret += useHypotheticalSize ? node->mLayoutResult->mLayoutSize.hypotheticalWidth
                                   : node->mLayoutResult->mLayoutSize.width;
      } else {
        ret = node->mCssStyle->mMargin.getMargin(kMarginTop) +
            node->mCssStyle->mMargin.getMargin(kMarginBottom);
        ret += useHypotheticalSize ? node->mLayoutResult->mLayoutSize.hypotheticalHeight
                                   : node->mLayoutResult->mLayoutSize.height;
      }
      return ret;
    }

    inline void limitMainSizeForFlexGrow(WXCoreFlexLine* const flexLine, const Index childIndex,
                                  const float flexGrow) {
      mChildrenFrozen[childIndex] = true;
      flexLine->mTotalFlexGrow -= flexGrow;
    }

    inline void setMeasuredDimension(const float width, const float height) {
      mLayoutResult->mLayoutSize.width = width;
      mLayoutResult->mLayoutSize.height = height;
    }

    inline std::pair<bool, float> limitChildMainSize(WXCoreFlexLine* const flexLine, const WXCoreLayoutNode* const child,
                                                                       float childSizeAlongMainAxis, const Index childIndex){
      bool needsReexpand = false;
      if (isMainAxisHorizontal(this)) {
        if (!isnan(child->mCssStyle->mMaxWidth) &&
            childSizeAlongMainAxis > child->mCssStyle->mMaxWidth) {
          needsReexpand = true;
          childSizeAlongMainAxis = child->mCssStyle->mMaxWidth;
        } else if (!isnan(child->mCssStyle->mMinWidth) &&
            childSizeAlongMainAxis < child->mCssStyle->mMinWidth) {
          needsReexpand = true;
          childSizeAlongMainAxis = child->mCssStyle->mMinWidth;
        }
      } else {
        if (!isnan(child->mCssStyle->mMaxHeight) &&
            childSizeAlongMainAxis > child->mCssStyle->mMaxHeight) {
          needsReexpand = true;
          childSizeAlongMainAxis = child->mCssStyle->mMaxHeight;
        } else if (!isnan(child->mCssStyle->mMinHeight) &&
            childSizeAlongMainAxis < child->mCssStyle->mMinHeight) {
          needsReexpand = true;
          childSizeAlongMainAxis = child->mCssStyle->mMinHeight;
        }
      }
      limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
      return std::make_pair(needsReexpand, childSizeAlongMainAxis);
    }

    void updateLeftRightForAbsolute(float &left, float &right,
                                    const WXCorePadding &parentPadding,
                                    const WXCoreBorderWidth &parentBorder,
                                    const WXCoreSize &parentSize) const {
      if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft))) {
        if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeRight))) {
          ;
        } else {
          right += parentSize.width -
              (parentBorder.getBorderWidth(kBorderWidthRight) +
                  mCssStyle->mStylePosition.getPosition(kPositionEdgeRight)
                  + mLayoutResult->mLayoutSize.width);
          left += parentSize.width -
              (parentBorder.getBorderWidth(kBorderWidthRight) +
                  mCssStyle->mStylePosition.getPosition(kPositionEdgeRight)
                  + mLayoutResult->mLayoutSize.width);
        }
      } else {
        left += parentBorder.getBorderWidth(kBorderWidthLeft) +
            mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft);
        right += parentBorder.getBorderWidth(kBorderWidthLeft) +
            mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft);
      }
    }

    void updateTopBottomForAbsolute(float &top, float &bottom,
                                    const WXCorePadding &parentPadding,
                                    const WXCoreBorderWidth &parentBorder,
                                    const WXCoreSize &parentSize) const {
      if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeTop))) {
        if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom))) {
          ;
        } else {
          top += parentSize.height -
              (parentBorder.getBorderWidth(kBorderWidthBottom) +
                  mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom)
                  + mLayoutResult->mLayoutSize.height);
          bottom += parentSize.height -
              (parentBorder.getBorderWidth(kBorderWidthBottom) +
                  mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom)
                  + mLayoutResult->mLayoutSize.height);
        }
      } else {
        top += parentBorder.getBorderWidth(kBorderWidthTop) +
            mCssStyle->mStylePosition.getPosition(kPositionEdgeTop);
        bottom += parentBorder.getBorderWidth(kBorderWidthTop) +
            mCssStyle->mStylePosition.getPosition(kPositionEdgeTop);
      }
    }

    /** ================================ other =================================== **/

    inline void clearDirty() {
      dirty = false;
      widthDirty = false;
      heightDirty = false;
    }

    void
    measure(float, float, bool);

    void hypotheticalMeasure(float, float, bool = false);

    void measureLeafNode(float, float, bool, bool);

    void measureInternalNode(float, float, bool, bool);

    void updateCurrentFlexline(Index, WXCoreFlexLine *, Index, const WXCoreLayoutNode *, bool);

    void measureChild(WXCoreLayoutNode* , float, float, float, bool, bool);

    void adjustChildSize(WXCoreLayoutNode *, float);

    void adjustChildSize(const WXCoreLayoutNode *child,
                         const float currentMainSize,
                         const float parentWidth,
                         const float parentHeight,
                         float &childWidth,
                         float &childHeight) const;

    void stretchViewCrossSize();

    void stretchViewCrossSize(WXCoreLayoutNode *, float);

    Index expandItemsInFlexLine(WXCoreFlexLine *, float, Index);

    void checkSizeConstraints(WXCoreLayoutNode *, bool);

    void
    determineMainSize(float width, float height);

    void
    determineCrossSize(float, float, bool);

    void
    determineCrossSize(float, float, WXCoreFlexLine *);

    void setFrame(float, float, float, float);

    void setFrame(WXCorePosition*,float, float, float, float);

    /** ================================ layout =================================== **/

    void layout(float left, float top, float right, float bottom, bool, const std::pair<float,float>* = nullptr);

    void calcRelativeOffset(float &left, float &top, float &right, float &bottom) const ;

    void calcAbsoluteOffset(float &left, float &top, float &right, float &bottom, const std::pair<float,float>* = nullptr);

    void positionAbsoluteFlexItem(float &left, float &top, float &right, float &bottom);

    void layoutHorizontal(bool isRtl, float left, float top, float right, float bottom,
                          WXCoreLayoutNode*, WXCoreFlexLine *const flexLine);

    void layoutFlexlineHorizontal(const float width,
                                         const WXCoreFlexLine *const flexLine,
                                         float &childLeft,
                                         float &childRight,
                                         float &spaceBetweenItem) const;

    void layoutSingleChildHorizontal(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine,
                                     WXCoreFlexWrap flexWrap, WXCoreAlignItems alignItems,
                                     float, float, float, float, bool);

    void layoutSingleChildHorizontal(const bool isRtl,
                                    const bool,
                                    float childBottom, float childTop,
                                    WXCoreFlexLine *const flexLine,
                                    WXCoreLayoutNode *const child,
                                    float&, float&);

    void layoutVertical(bool isRtl, bool fromBottomToTop, float left, float top, float right, float bottom,
                        WXCoreLayoutNode*, WXCoreFlexLine *const flexLine);

    void layoutFlexlineVertical(const float height,
                                const WXCoreFlexLine *const flexLine,
                                float &childTop,
                                float &childBottom,
                                float &spaceBetweenItem) const;
    void layoutSingleChildVertical(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine,
                                   bool isRtl, WXCoreAlignItems alignItems,
                                   float, float, float, float, bool);

    void layoutSingleChildVertical(const bool isRtl, const bool fromBottomToTop,
                                   const bool absoluteFlexItem,
                                   const float childLeft, const float childRight,
                                   WXCoreFlexLine *const flexLine,
                                   WXCoreLayoutNode *const child,
                                   float& ,float&);

    void updateFlexLineForAbsoluteItem(WXCoreLayoutNode *const absoluteFlexItem, WXCoreFlexLine *const flexLine);

    void initFormatingContext(std::vector<WXCoreLayoutNode *> &BFCs);

    std::pair<bool,float> calculateBFCWidth(float, float);

    std::pair<bool,float> calculateBFCHeight(float, float);

    std::tuple<bool, float, float> calculateBFCDimension(const std::pair<float,float>&);

    virtual void OnLayoutBefore() {

    }

    virtual void OnLayoutAfter(float width, float height) {

    }


  public:
    virtual void onLayout(float left, float top, float right, float bottom, WXCoreLayoutNode* = nullptr, WXCoreFlexLine *const flexLine = nullptr);
    /** ================================ tree =================================== **/

    inline Index getChildCount(FormattingContext formattingContext) const {
      switch (formattingContext) {
        case kNonBFC:
          return NonBFCs.size();
        case kBFC:
          return BFCs.size();
        default:
          return mChildList.size();
      }
    }

    inline Index getChildCount() const {
      return mChildList.size();
    }

    inline std::vector<WXCoreLayoutNode *>::const_iterator ChildListIterBegin() {
      return mChildList.cbegin();
    }

    inline std::vector<WXCoreLayoutNode *>::const_iterator ChildListIterEnd() {
      return mChildList.cend();
    }

    inline bool hasChild(const WXCoreLayoutNode* const child){
       if(std::find(mChildList.begin(), mChildList.end(), child) != mChildList.end()){
         return true;
       }else{
         return false;
       }
    }

    inline void removeChild(const WXCoreLayoutNode* const child) {
      for (int index = 0; index < mChildList.size(); index++) {
        if (child == mChildList[index]) {
          mChildList.erase(mChildList.begin() + index);
          break;
        }
      }
      markDirty();
    }

    inline void addChildAt(WXCoreLayoutNode* const child, Index index) {
      mChildList.insert(mChildList.begin() + index, child);
      child->mParent = this;
      markDirty();
    }

    inline WXCoreLayoutNode *getChildAt(const FormattingContext formattingContext, const Index index) const {
      switch (formattingContext) {
        case kNonBFC:
          return NonBFCs[index];
        case kBFC:
          return BFCs[index];
        default:
          return mChildList[index];
      }
    }

    inline WXCoreLayoutNode *getChildAt(const Index index) const {
      return mChildList[index];
    }

    inline WXCoreLayoutNode *getParent() const {
      return mParent;
    }

    inline void setParent(WXCoreLayoutNode * const parent, WXCoreLayoutNode * const child) const {
       child->mParent = parent;
    }

    inline bool isBFC(WXCoreLayoutNode* const node) const {
      return node->mCssStyle->mPositionType == kAbsolute || node->mCssStyle->mPositionType == kFixed;
    }

    /** ================================ margin =================================== **/

    inline float getMarginTop() const {
      return mCssStyle->mMargin.getMargin(kMarginTop);
    }

    inline float getMarginBottom() const {
      return mCssStyle->mMargin.getMargin(kMarginBottom);
    }

    inline float getMarginLeft() const  {
      return mCssStyle->mMargin.getMargin(kMarginLeft);
    }

    inline float getMarginRight() const {
      return mCssStyle->mMargin.getMargin(kMarginRight);
    }

    inline void setMargin(const WXCoreMarginEdge &edge, const float margin) {
      if (mCssStyle->mMargin.setMargin(edge, margin)) {
        markDirty();
      }
    }

    inline const WXCoreMargin &GetMargins() const {
      return mCssStyle->mMargin;
    }

    /** ================================ padding =================================== **/

    inline float getPaddingLeft() const {
      return mCssStyle->mPadding.getPadding(kPaddingLeft);
    }

    inline float getPaddingRight() const {
      return mCssStyle->mPadding.getPadding(kPaddingRight);
    }

    inline float getPaddingTop() const {
      return mCssStyle->mPadding.getPadding(kPaddingTop);
    }

    inline float getPaddingBottom() const {
      return mCssStyle->mPadding.getPadding(kPaddingBottom);
    }

    inline void setPadding(const WXCorePaddingEdge edge, const float padding) {
      if (mCssStyle->mPadding.setPadding(edge, padding)) {
        markDirty();
      }
    }

    inline const WXCorePadding &GetPaddings() const {
      return mCssStyle->mPadding;
    }

    /** ================================ border-width =================================== **/

    inline float getBorderWidthLeft() const {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthLeft);
    }

    inline float getBorderWidthRight() const {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthRight);
    }

    inline float getBorderWidthTop() const {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthTop);
    }

    inline float getBorderWidthBottom() const {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthBottom);
    }

    inline void setBorderWidth(const WXCoreBorderWidthEdge edge, const float borderWidth) {
      if (mCssStyle->mBorderWidth.setBorderWidth(edge, borderWidth)) {
        markDirty();
      }
    }

    inline const WXCoreBorderWidth &GetBorders() const {
      return mCssStyle->mBorderWidth;
    }

    /** ================================ position-type =================================== **/

    inline void setStylePositionType(const WXCorePositionType positionType) {
      if (mCssStyle->mPositionType != positionType) {
        mCssStyle->mPositionType = positionType;
        markDirty();
      }
    }

    inline WXCorePositionType getStylePositionType() const {
      return mCssStyle->mPositionType;
    }


    /** ================================ position =================================== **/

    inline float getStylePositionTop() const {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeTop);
    }

    inline float getStylePositionBottom() const {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom);
    }

    inline float getStylePositionLeft() const {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft);
    }

    inline float getStylePositionRight() const {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeRight);
    }

    inline void setStylePosition(const WXCorePositionEdge edge, const float positionRight) {
      if (mCssStyle->mStylePosition.setPosition(edge, positionRight))
        markDirty();
    }


    /** ================================ dimension =================================== **/

    inline void setStyleWidthLevel(const DimensionLevel level) const {
      if (mCssStyle->mStyleWidthLevel != level) {
        mCssStyle->mStyleWidthLevel = level;
      }
    }

    inline void setStyleHeightLevel(const DimensionLevel level) const {
      if (mCssStyle->mStyleHeightLevel != level) {
        mCssStyle->mStyleHeightLevel = level;
      }
    }

    inline DimensionLevel getStyleHeightLevel() const {
      return mCssStyle->mStyleHeightLevel;
    }

    inline DimensionLevel getStyleWidthLevel() const {
      return mCssStyle->mStyleWidthLevel;
    }

    inline void setStyleWidth(const float width, const bool updating) {
      if (mCssStyle->mStyleWidth != width) {
        mCssStyle->mStyleWidth = width;
        markDirty();
        if(updating) {
          markChildrenDirty(true);
        }
      }
    }

    inline void setStyleWidthToNAN() {
      if (!isnan(mCssStyle->mStyleWidth)) {
        mCssStyle->mStyleWidth = NAN;
        markDirty();
        markChildrenDirty(true);
      }
    }

    inline float getStyleWidth() const {
      return mCssStyle->mStyleWidth;
    }

    inline void setStyleHeight(const float height) {
      if (mCssStyle->mStyleHeight != height) {
        mCssStyle->mStyleHeight = height;
        markDirty();
      }
    }

    inline float getStyleHeight() const {
      return mCssStyle->mStyleHeight;
    }

    inline void setMinWidth(const float minWidth, const bool updating) {
      if (mCssStyle->mMinWidth != minWidth) {
        mCssStyle->mMinWidth = minWidth;
        markDirty();
        if(updating) {
          markChildrenDirty(true);
        }
      }
    }

    inline float getMinWidth() const {
      return mCssStyle->mMinWidth;
    }

    inline void setMaxWidth(const float maxWidth, const bool updating) {
      if (mCssStyle->mMaxWidth != maxWidth) {
        mCssStyle->mMaxWidth = maxWidth;
        markDirty();
        if(updating) {
          markChildrenDirty(true);
        }
      }
    }

    inline float getMaxWidth() const {
      return mCssStyle->mMaxWidth;
    }

    inline void setMinHeight(const float minHeight) {
      if (mCssStyle->mMinHeight != minHeight) {
        mCssStyle->mMinHeight = minHeight;
        markDirty();
      }
    }

    inline float getMinHeight() const {
      return mCssStyle->mMinHeight;
    }

    inline void setMaxHeight(const float maxHeight) {
      if (mCssStyle->mMaxHeight != maxHeight) {
        mCssStyle->mMaxHeight = maxHeight;
        markDirty();
      }
    }

    inline float getMaxHeight() const {
      return mCssStyle->mMaxHeight;
    }

      inline void setDirection(const WXCoreDirection direction, const bool updating) {
          if (nullptr == mCssStyle) return;
          
          if (mCssStyle->mDirection != direction) {
              mCssStyle->mDirection = direction;
              markDirty();
              if (updating) {
                  for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
                      (*it)->markInheritableDirty();
                  }
              }
          }
      }

    inline WXCoreDirection getDirection() const {
        if (mCssStyle == nullptr) {
            return WEEXCORE_CSS_DEFAULT_DIRECTION;
        }
        return mCssStyle->mDirection;
    }
    
    /** ================================ CSS direction For RTL =================================== **/
      
    void determineChildLayoutDirection(const WXCoreDirection direction);
      
    WXCoreDirection getLayoutDirectionFromPathNode();
      
    /** ================================ flex-style =================================== **/

    inline void setFlexDirection(const WXCoreFlexDirection flexDirection, const bool updating) {
      if (mCssStyle->mFlexDirection != flexDirection) {
        mCssStyle->mFlexDirection = flexDirection;
        markDirty();
        if (updating) {
          for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
            (*it)->markDirty(false);
          }
        }
      }
    }

    inline WXCoreFlexDirection getFlexDirection() const {
      return mCssStyle->mFlexDirection;
    }

    inline void setFlexWrap(const WXCoreFlexWrap flexWrap) {
      if (mCssStyle->mFlexWrap != flexWrap) {
        mCssStyle->mFlexWrap = flexWrap;
        markDirty();
      }
    }

    inline WXCoreFlexWrap getFlexWrap() const {
      return mCssStyle->mFlexWrap;
    }

    inline void setJustifyContent(const WXCoreJustifyContent justifyContent) {
      if (mCssStyle->mJustifyContent != justifyContent) {
        mCssStyle->mJustifyContent = justifyContent;
      }
    }

    inline WXCoreJustifyContent getJustifyContent() const {
      return mCssStyle->mJustifyContent;
    }

    inline void setAlignItems(const WXCoreAlignItems alignItems) {
      if (mCssStyle->mAlignItems != alignItems) {
        mCssStyle->mAlignItems = alignItems;
        markDirty();
      }
    }

    inline WXCoreAlignItems getAlignItems() const {
      return mCssStyle->mAlignItems;
    }

    inline void setAlignSelf(const WXCoreAlignSelf alignSelf) {
      if (mCssStyle->mAlignSelf != alignSelf) {
        mCssStyle->mAlignSelf = alignSelf;
        markDirty();
      }
    }

    inline WXCoreAlignSelf getAlignSelf() const {
      return mCssStyle->mAlignSelf;
    }

    virtual void set_flex(const float flex) {
      if (mCssStyle->mFlexGrow != flex) {
        mCssStyle->mFlexGrow = flex;
        markDirty();
      }
    }

    inline float getFlex() const {
      return mCssStyle->mFlexGrow;
    }

    /** ================================ layout-result =================================== **/

    inline float getLayoutWidth() const {
      return mLayoutResult->mLayoutSize.width;
    }

    inline float getLayoutHeight() const {
      return mLayoutResult->mLayoutSize.height;
    }

    inline float getLayoutPositionTop() const {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeTop);
    }

    inline float getLayoutPositionBottom() const {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeBottom);
    }

    inline float getLayoutPositionLeft() const {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeLeft);
    }

    inline float getLayoutPositionRight() const  {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeRight);
    }
      
    virtual inline WXCoreDirection getLayoutDirection() const {
      if (nullptr == mLayoutResult) {
        return WEEXCORE_CSS_DEFAULT_DIRECTION;
      }
      return mLayoutResult->mLayoutDirection;
    }

    inline void setLayoutDirection(WXCoreDirection direction) {
        if (nullptr == mLayoutResult) return;
        mLayoutResult->mLayoutDirection = direction;
    }
    inline bool hasNewLayout() const {
      return mHasNewLayout;
    }

    inline bool isDirty() const {
      return dirty;
    }

    inline void markDirty(const bool recursion = true) {
      if (!isDirty()) {
        dirty = true;
        if (getParent() != nullptr && recursion) {
          getParent()->markDirty();
        }
      }
    }
      
    void markAllDirty() {
      markDirty(false);
      for (WXCoreLayoutNode* c : mChildList) {
          c->markAllDirty();
      }
    }

    bool markChildrenDirty(const bool updatedNode = false) {
      bool ret = false;
      if(getChildCount() == 0){
        if(measureFunc!= nullptr){
          ret = true;
        }
      }
      else {
        //isnan(mCssStyle->mStyleWidth) XOR updatedNode
        if(isnan(mCssStyle->mStyleWidth) != updatedNode){
          for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
            ret = ((*it)->markChildrenDirty() || ret) ;
          }
        }
      }
      dirty = ret || dirty;
      return ret;
    }

    void markInheritableDirty() {
        if (resetInheritableSet()) {
            // if some style was inherited from parent, reset those styles
            // then mark self dirty
            markDirty(false);
            
            // traverse children to mark dirty
            if(getChildCount() == 0){
                return;
            }
            else {
                for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
                    (*it)->markInheritableDirty();
                }
            }
        }
    }
      
    /**
    * if some style was inherited from parent, reset those styles, then return true, eles return false
    */
    bool resetInheritableSet() {
      if (mCssStyle == nullptr || mLayoutResult == nullptr) return false;
        
      bool hasInheritedStyle = false;
      if (mCssStyle->mDirection == kDirectionInherit) {
          mLayoutResult->mLayoutDirection = kDirectionInherit;
          hasInheritedStyle = true;
      }
      return hasInheritedStyle;
    }
      
    inline void setHasNewLayout(const bool hasNewLayout) {
      this->mHasNewLayout = hasNewLayout;
    }

    inline float getLargestMainSize() const {
      float largestSize = 0;
      for (WXCoreFlexLine *flexLine : mFlexLines) {
        largestSize = std::max(largestSize, flexLine->mMainSize);
      }
      return largestSize + sumPaddingBorderAlongAxis(this, isMainAxisHorizontal(this));
    }
      
    inline void rewriteLayoutResult(float left, float top, float width, float height) {
      if (mLayoutResult != nullptr) {
          mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeLeft, left);
          mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeTop, top);
          mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeRight, left + width);
          mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeBottom, top + height);
          mLayoutResult->mLayoutSize.width = width;
          mLayoutResult->mLayoutSize.height = height;
      }
    }
  };
}
#endif //WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#endif
