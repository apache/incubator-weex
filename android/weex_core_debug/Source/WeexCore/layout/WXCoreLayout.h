#ifdef __cplusplus

#ifndef WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#define WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H

#include "WXCoreStyle.h"
#include "WXCoreFlexEnum.h"
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

namespace WeexCore {

  class WXCoreLayoutNode;

  class WXCoreFlexLine;

  enum FormattingContext {
    kBFC,
    kNonBFC,
  } ;

  enum MeasureMode {
    kUnspecified,
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
   * layout-result：layout-height、layout-width、position（left、right、top、bottom）
   */
  struct WXCorelayoutResult {
    WXCoreSize mLayoutSize;
    WXCorePosition mLayoutPosition;

    inline bool isNAN() {
      return mLayoutSize.isNAN() || mLayoutPosition.isNAN();
    }

    inline void reset() {
      mLayoutSize.reset();
      mLayoutPosition.reset();
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

  protected:
      WXCoreLayoutNode() :
              mChildrenFrozen(nullptr),
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


      ~WXCoreLayoutNode() {
          mIsDestroy = true;
        mHasNewLayout = true;
        dirty = true;
        measureFunc = nullptr;
        mParent = nullptr;
        mChildList.clear();
        BFCs.clear();
        NonBFCs.clear();

        if (mChildrenFrozen != nullptr) {
          delete mChildrenFrozen;
          mChildrenFrozen = nullptr;
        }

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
    bool *mChildrenFrozen = nullptr;

    std::vector<WXCoreFlexLine *> mFlexLines;

    std::vector<WXCoreLayoutNode *> mChildList;

    std::vector<WXCoreLayoutNode *> BFCs;

    std::vector<WXCoreLayoutNode *> NonBFCs;

    WXCoreLayoutNode *mParent = nullptr;

    WXCoreCSSStyle *mCssStyle = nullptr;

    MeasureMode widthMeasureMode = kUnspecified;

    MeasureMode heightMeasureMode = kUnspecified;

    WXCorelayoutResult *mLayoutResult = nullptr;

    bool mHasNewLayout;

    bool dirty, widthDirty, heightDirty;

    bool mIsDestroy = true;

    WXCoreMeasureFunc measureFunc = nullptr;

    void *context;

    /** ================================ Cache：Last calculate result =================================== **/

  public:


    /** ================================ Engine Entry Function =================================== **/

    void calculateLayout(const std::pair<float,float>&);

    /** ================================ measureFunc =================================== **/

    inline void setMeasureFunc(WXCoreMeasureFunc measure) {
      measureFunc = measure;
      markDirty();
    }

    inline WXCoreMeasureFunc getMeasureFunc() const {
      return measureFunc;
    }

    inline bool haveMeasureFunc() const {
      return measureFunc != nullptr;
    }

    /** ================================ context =================================== **/


    inline void *getContext() const {
      return context;
    }

    inline void setContext(void * const context) {
      this->context = context;
    }

    inline void copyStyle(WXCoreLayoutNode *srcNode) {
      if (memcmp(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle)) != 0) {
        memcpy(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle));
        markDirty();
      }
    }

    inline void copyMeasureFunc(WXCoreLayoutNode *srcNode) {
      if (memcmp(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc)) != 0) {
        memcpy(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc));
        markDirty();
      }
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

        if(mChildrenFrozen != nullptr) {
          delete mChildrenFrozen;
          mChildrenFrozen = nullptr;
        }
        mChildrenFrozen = new bool[getChildCount(kNonBFC)];
        for(int i=0;i<getChildCount(kNonBFC);i++){
          mChildrenFrozen[i] = false;
        }
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

    inline bool isWrapRequired(const float &mainSize, const float &currentLength, const float &childLength) const {
      return !isSingleFlexLine(mainSize) && mainSize < currentLength + childLength;
    }

    inline bool isSingleFlexLine(const float &mainSize) const {
      return mCssStyle->mFlexWrap == kNoWrap || isnan(mainSize);
    }

    inline void sumFlexGrow(const WXCoreLayoutNode* const child, WXCoreFlexLine* const flexLine, const Index i) const {
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

    inline bool limitMainSizeForFlexGrow(WXCoreFlexLine* const flexLine, const Index childIndex,
                                  const float flexGrow) {
      mChildrenFrozen[childIndex] = true;
      flexLine->mTotalFlexGrow -= flexGrow;
      return true;
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
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMaxWidth;
        } else if (!isnan(child->mCssStyle->mMinWidth) &&
            childSizeAlongMainAxis < child->mCssStyle->mMinWidth) {
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMinWidth;
        }
      } else {
        if (!isnan(child->mCssStyle->mMaxHeight) &&
            childSizeAlongMainAxis > child->mCssStyle->mMaxHeight) {
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMaxHeight;
        } else if (!isnan(child->mCssStyle->mMinHeight) &&
            childSizeAlongMainAxis < child->mCssStyle->mMinHeight) {
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMinHeight;
        }
      }
      return std::make_pair(needsReexpand, childSizeAlongMainAxis);
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

    void measureChild(WXCoreLayoutNode *, float, float, bool, bool);

    void adjustChildSize(WXCoreLayoutNode *, float);

    void stretchViewCrossSize();

    void stretchViewCrossSize(WXCoreLayoutNode *, float);

    Index expandItemsInFlexLine(WXCoreFlexLine *, float, Index);

    void checkSizeConstraints(WXCoreLayoutNode *, bool);

    void
    determineMainSize(float width, float height);

    void
    determineCrossSize(float, float, bool);

    void setFrame(float, float, float, float);

    /** ================================ layout =================================== **/

    void layout(float left, float top, float right, float bottom, const std::pair<float,float>* = nullptr);

    void calcRelativeOffset(float &left, float &top, float &right, float &bottom) const ;

    void calcAbsoluteOffset(float &left, float &top, float &right, float &bottom, const std::pair<float,float>* = nullptr) const ;

    void onLayout(float left, float top, float right, float bottom);

    void layoutHorizontal(bool isRtl, float left, float top, float right, float bottom);

    void layoutSingleChildHorizontal(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine,
                                     WXCoreFlexWrap flexWrap,
                                     WXCoreAlignItems alignItems, float left, float top,
                                     float right,
                                     float bottom);

    void layoutVertical(bool isRtl, bool fromBottomToTop, float left, float top,
                        float right, float bottom);

    void layoutSingleChildVertical(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine, bool isRtl,
                                   WXCoreAlignItems alignItems, float left, float top, float right,
                                   float bottom);

    void initFormatingContext(std::vector<WXCoreLayoutNode *> &BFCs);

    std::pair<bool,float> calculateBFCWidth(float, float);

    std::pair<bool,float> calculateBFCHeight(float, float);

    std::tuple<bool, float, float> calculateBFCDimension(const std::pair<float,float>&);

    virtual void onLayoutBefore() {

    }

    virtual void onLayoutAfter(float width, float height) {

    }


  public:

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

    inline WXCorePositionType getStypePositionType() const {
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

    inline void setStyleWidth(const float width) {
      if (mCssStyle->mStyleWidth != width) {
        mCssStyle->mStyleWidth = width;
        markDirty();
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

    inline void setMinWidth(const float minWidth) {
      if (mCssStyle->mMinWidth != minWidth) {
        mCssStyle->mMinWidth = minWidth;
        markDirty();
      }
    }

    inline float getMinWidth() const {
      return mCssStyle->mMinWidth;
    }

    inline void setMaxWidth(const float maxWidth) {
      if (mCssStyle->mMaxWidth != maxWidth) {
        mCssStyle->mMaxWidth = maxWidth;
        markDirty();
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


    /** ================================ flex-style =================================== **/

    inline void setFlexDirection(const WXCoreFlexDirection flexDirection) {
      if (mCssStyle->mFlexDirection != flexDirection) {
        mCssStyle->mFlexDirection = flexDirection;
        markDirty();
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

    inline void setFlex(const float flex) {
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

  };
}
#endif //WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#endif