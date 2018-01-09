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
    float width;
    float height;

    WXCoreSize() : width(0), height(0) {}

    void reset() {
      width = 0;
      height = 0;
    }

    bool isNAN() {
      return isnan(width) || isnan(height);
    }
  };

  /**
   * layout-result：layout-height、layout-width、position（left、right、top、bottom）
   */
  struct WXCorelayoutResult {
    WXCoreSize mLayoutSize;
    WXCorePosition mLayoutPosition;

    bool isNAN() {
      return mLayoutSize.isNAN() || mLayoutPosition.isNAN();
    }

    void reset() {
      mLayoutSize.reset();
      mLayoutPosition.reset();
    }
  };

  typedef WXCoreSize(*WXCoreMeasureFunc)(WXCoreLayoutNode *node, float width,
                                         MeasureMode widthMeasureMode,
                                         float height, MeasureMode heightMeasureMode);

  using Index = std::vector<WXCoreFlexLine *>::size_type;

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
  private:

    /**
     * Holds the 'frozen' state of children during measure. If a view is frozen it will no longer
     * expand regardless of mFlexGrow. Items are indexed by the child's
     * reordered index.
     */
    bool *mChildrenFrozen;

    std::vector<WXCoreFlexLine *> mFlexLines;

    std::vector<WXCoreLayoutNode *> mChildList;

    std::vector<WXCoreLayoutNode *> BFCs;

    std::vector<WXCoreLayoutNode *> NonBFCs;

    WXCoreLayoutNode *mParent;

    WXCoreCSSStyle *mCssStyle;

    MeasureMode widthMeasureMode;

    MeasureMode heightMeasureMode;

    WXCorelayoutResult *mLayoutResult;

    bool mHasNewLayout;

    bool dirty, widthDirty, heightDirty;

    bool mVisible;

    WXCoreMeasureFunc measureFunc;

    void *context;

    /** ================================ Cache：Last calculate result =================================== **/

    WXCoreSize *mLastAvailableSize;

    MeasureMode mLastWidthMode;

    MeasureMode mLastHeightMode;

  public:


    /** ================================ Engine Entry Function =================================== **/

    void calculateLayout();


    /** ================================ lifeCycle =================================== **/

    inline static WXCoreLayoutNode *newWXCoreNode() {
      WXCoreLayoutNode *node = new WXCoreLayoutNode();
      node->markDirty();
      return node;
    }

    WXCoreLayoutNode() :
        mChildrenFrozen(nullptr),
        mParent(nullptr),
        dirty(true),
        widthDirty{false},
        heightDirty{false},
        mHasNewLayout(true),
        mVisible(true),
        measureFunc(nullptr) {
      mCssStyle = new WXCoreCSSStyle();
      mLayoutResult = new WXCorelayoutResult();
      mLastAvailableSize = nullptr;
    }

    void freeWXCoreNode() {
      mHasNewLayout = true;
      dirty = true;
      mVisible = true;
      measureFunc = nullptr;
      mParent = nullptr;
      mChildList.clear();

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

      if (mLastAvailableSize != nullptr) {
        delete mLastAvailableSize;
        mLastAvailableSize = nullptr;
      }
    }

    //TODO this should be private
    void reset() {
      widthMeasureMode = isnan(mCssStyle->mStyleWidth) ? kUnspecified : kExactly;
      heightMeasureMode = isnan(mCssStyle->mStyleHeight) ? kUnspecified : kExactly;
      if (isDirty()) {
        mLayoutResult->reset();
        mFlexLines.clear();
        mChildrenFrozen = new bool[getChildCount(kNonBFC)];
      }
    }

    void resetLayoutResult() {
      mLayoutResult->reset();
      markDirty();
    }

    void copyStyle(WXCoreLayoutNode *srcNode) {
      if (memcmp(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle)) != 0) {
        memcpy(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle));
        markDirty();
      }
    }

    void copyMeasureFunc(WXCoreLayoutNode *srcNode) {
      if (memcmp(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc)) != 0) {
        memcpy(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc));
        markDirty();
      }
    }

    void copyNode(WXCoreLayoutNode *srcNode) {
      copyStyle(srcNode);
      copyMeasureFunc(srcNode);
      for (WXCoreLayoutNode *node : mChildList) {
        WXCoreLayoutNode *temp = newWXCoreNode();
        memcpy(&node, &temp, sizeof(WXCoreLayoutNode));
        srcNode->appendChild(temp);
      }
      markDirty();
    }

    /** ================================ measureFunc =================================== **/

    void setMeasureFunc(WXCoreMeasureFunc measure) {
      measureFunc = measure;
      markDirty();
    }

    WXCoreMeasureFunc getMeasureFunc() {
      return measureFunc;
    }

    bool haveMeasureFunc() {
      return measureFunc != nullptr;
    }

    /** ================================ context =================================== **/


    void *getContext() {
      return context;
    }

    void setContext(void *context) {
      this->context = context;
    }

  private:

    /** ================================ measure =================================== **/


    void setLayoutWidth(const float width) {
      if (mLayoutResult->mLayoutSize.width != width &&
          (!isnan(width) || !isnan(mLayoutResult->mLayoutSize.width))) {
        mLayoutResult->mLayoutSize.width = width;
        widthDirty = true;
        markDirty(false);
      }
    }

    void setLayoutHeight(const float height) {
      if (mLayoutResult->mLayoutSize.height != height &&
          (!isnan(height) || !isnan(mLayoutResult->mLayoutSize.height))) {
        mLayoutResult->mLayoutSize.height = height;
        heightDirty = true;
        markDirty(false);
      }
    }

    void setWidthMeasureMode(MeasureMode measureMode) {
      if (widthMeasureMode != measureMode) {
        widthMeasureMode = measureMode;
        if (getChildCount(kNonBFC) > 0) {
          widthDirty = true;
        }
      }
    }

    void setHeightMeasureMode(MeasureMode measureMode) {
      if (heightMeasureMode != measureMode) {
        heightMeasureMode = measureMode;
        if (getChildCount(kNonBFC) > 0) {
          heightDirty = true;
        }
      }
    }

    float firstLineCrossSize() {
      float sum = sumPaddingBorderAlongAxis(this, !isMainAxisHorizontal(this));
      if (!mFlexLines.empty()) {
        sum += mFlexLines[0]->mCrossSize;
      }
      return sum;
    }

    float getSumOfCrossSize() {
      float sum = sumPaddingBorderAlongAxis(this, !isMainAxisHorizontal(this));
      for (WXCoreFlexLine *flexLine: mFlexLines) {
        sum += flexLine->mCrossSize;
      }
      return sum;
    }

    bool isMainAxisHorizontal(const WXCoreLayoutNode *node) {
      return node->mCssStyle->mFlexDirection == kFlexDirectionRow ||
             node->mCssStyle->mFlexDirection == kFlexDirectionRowReverse;
    }

    bool isCrossExactly() {
      return isMainAxisHorizontal(this) ? heightMeasureMode == kExactly
                                        : widthMeasureMode == kExactly;
    }

    float sumPaddingBorderAlongAxis(const WXCoreLayoutNode *node, bool horizontal) {
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

    bool isWrapRequired(const float mainSize, const float currentLength, const float childLength) {
      return !isSingleFlexLine(mainSize) && mainSize < currentLength + childLength;
    }

    bool isSingleFlexLine(const float mainSize) {
      return mCssStyle->mFlexWrap == kNoWrap || isnan(mainSize);
    }

    void sumFlexGrow(const WXCoreLayoutNode *child, WXCoreFlexLine *flexLine, const Index i,
                     const bool horizontal) {
      if (child->mCssStyle->mFlexGrow > 0) {
        flexLine->mTotalFlexGrow += child->mCssStyle->mFlexGrow;
        if (horizontal) {
          if (!isnan(child->mLayoutResult->mLayoutSize.width)) {
            flexLine->mTotalFlexibleSize += child->mLayoutResult->mLayoutSize.width;
          }
        } else {
          if (!isnan(child->mLayoutResult->mLayoutSize.height)) {
            flexLine->mTotalFlexibleSize += child->mLayoutResult->mLayoutSize.height;
          }
        }
      } else {
        mChildrenFrozen[i] = true;
      }
    }

    void setMeasuredDimensionForFlex(
        float width, MeasureMode widthMeasureMode,
        float height, MeasureMode heightMeasureMode) {
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

    float calcItemSizeAlongAxis(const WXCoreLayoutNode *node, const bool horizontal) {
      float ret;
      if (horizontal) {
        ret = node->mCssStyle->mMargin.getMargin(kMarginLeft) +
              node->mCssStyle->mMargin.getMargin(kMarginRight);
        if (!isnan(node->mLayoutResult->mLayoutSize.width)) {
          ret += node->mLayoutResult->mLayoutSize.width;
        }
      } else {
        ret = node->mCssStyle->mMargin.getMargin(kMarginTop) +
              node->mCssStyle->mMargin.getMargin(kMarginBottom);
        if (!isnan(node->mLayoutResult->mLayoutSize.height)) {
          ret += node->mLayoutResult->mLayoutSize.height;
        }
      }
      return ret;
    }

    bool limitMainSizeForFlexGrow(WXCoreFlexLine *flexLine, const Index childIndex,
                                  const float flexGrow) {
      mChildrenFrozen[childIndex] = true;
      flexLine->mTotalFlexGrow -= flexGrow;
      return true;
    }

    void
    measure(float, float, bool);

    void hypotheticalMeasure(float, float, bool);

    void measureLeafNode(float, float);

    void measureInternalNode(float, float, bool, bool);

    void updateCurrentFlexline(Index, WXCoreFlexLine *, Index, const WXCoreLayoutNode *);

    void measureChild(WXCoreLayoutNode *, float, float, bool, bool);

    std::pair<bool, float>
    limitChildMainSize(WXCoreFlexLine *, const WXCoreLayoutNode *, float, Index);

    void adjustChildSize(WXCoreLayoutNode *, float);

    void stretchViewCrossSize();

    void stretchViewCrossSize(WXCoreLayoutNode *, float);

    Index expandItemsInFlexLine(WXCoreFlexLine *, float, float, Index);

    void checkSizeConstraints(WXCoreLayoutNode *, bool);

    void
    determineMainSize(float width, float height);

    void
    determineCrossSize(float, float, bool);

    void setFrame(float, float, float, float);

    /** ================================ layout =================================== **/

    void layout(float left, float top, float right, float bottom);

    void calcRelativeOffset(float &left, float &top, float &right, float &bottom);

    void calcAbsoluteOffset(float &left, float &top, float &right, float &bottom);

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

    virtual void onLayoutBefore() {

    }

    virtual void onLayoutAfter(float width, float height) {

    }


  public:

    /** ================================ tree =================================== **/

    Index getChildCount(FormattingContext formattingContext) const {
      switch (formattingContext) {
        case kNonBFC:
          return NonBFCs.size();
        case kBFC:
          return BFCs.size();
        default:
          return mChildList.size();
      }
    }

    Index getChildCount() {
      return mChildList.size();
    }

    void removeChildAt(Index index) {
      mChildList.erase(mChildList.begin() + index);
      markDirty();
    }

    void removeChild(WXCoreLayoutNode *child) {
      for (int index = 0; index < mChildList.size(); index++) {
        if (child == mChildList[index]) {
          mChildList.erase(mChildList.begin() + index);
          break;
        }
      }
      markDirty();
    }

    void addChildAt(WXCoreLayoutNode *child, Index index) {
      mChildList.insert(mChildList.begin() + index, child);
      child->mParent = this;
      markDirty();
    }

    void appendChild(WXCoreLayoutNode *child) {
      mChildList.push_back(child);
      child->mParent = this;
      markDirty();
    }

    WXCoreLayoutNode *getChildAt(FormattingContext formattingContext, Index index) {
      switch (formattingContext) {
        case kNonBFC:
          return NonBFCs[index];
        case kBFC:
          return BFCs[index];
        default:
          return mChildList[index];
      }
    }

    WXCoreLayoutNode *getChildAt(Index index) {
      return mChildList[index];
    }

    WXCoreLayoutNode *getParent() {
      return mParent;
    }

    void initFormatingContext(std::vector<WXCoreLayoutNode *> &BFCs);

    std::pair<float, float> calculateBFCDimension();

    bool isBFC(WXCoreLayoutNode *node) {
      return node->mCssStyle->mPositionType == kAbsolute;
    }


    /** ================================ margin =================================== **/

    float getMarginTop() {
      return mCssStyle->mMargin.getMargin(kMarginTop);
    }

    float getMarginBottom() {
      return mCssStyle->mMargin.getMargin(kMarginBottom);
    }

    float getMarginLeft() {
      return mCssStyle->mMargin.getMargin(kMarginLeft);
    }

    float getMarginRight() {
      return mCssStyle->mMargin.getMargin(kMarginRight);
    }

    void setMargin(WXCoreMarginEdge edge, float margin) {
      if (mCssStyle->mMargin.setMargin(edge, margin)) {
        markDirty();
      }
    }

    /** ================================ padding =================================== **/

    float getPaddingLeft() {
      return mCssStyle->mPadding.getPadding(kPaddingLeft);
    }

    float getPaddingRight() {
      return mCssStyle->mPadding.getPadding(kPaddingRight);
    }

    float getPaddingTop() {
      return mCssStyle->mPadding.getPadding(kPaddingTop);
    }

    float getPaddingBottom() {
      return mCssStyle->mPadding.getPadding(kPaddingBottom);
    }

    void setPadding(WXCorePaddingEdge edge, float padding) {
      if (mCssStyle->mPadding.setPadding(edge, padding)) {
        markDirty();
      }
    }


    /** ================================ border-width =================================== **/

    float getBorderWidthLeft() {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthLeft);
    }

    float getBorderWidthRight() {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthRight);
    }

    float getBorderWidthTop() {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthTop);
    }

    float getBorderWidthBottom() {
      return mCssStyle->mBorderWidth.getBorderWidth(kBorderWidthBottom);
    }

    void setBorderWidth(WXCoreBorderWidthEdge edge, float borderWidth) {
      if (mCssStyle->mBorderWidth.setBorderWidth(edge, borderWidth)) {
        markDirty();
      }
    }


    /** ================================ position-type =================================== **/

    void setStylePositionType(WXCorePositionType positionType) {
      if (mCssStyle->mPositionType != positionType) {
        mCssStyle->mPositionType = positionType;
        markDirty();
      }
    }

    WXCorePositionType getStypePositionType() {
      return mCssStyle->mPositionType;
    }


    /** ================================ position =================================== **/

    float getStylePositionTop() {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeTop);
    }

    float getStylePositionBottom() {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom);
    }

    float getStylePositionLeft() {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft);
    }

    float getStylePositionRight() {
      return mCssStyle->mStylePosition.getPosition(kPositionEdgeRight);
    }

    void setStylePosition(WXCorePositionEdge edge, float positionRight) {
      if (mCssStyle->mStylePosition.setPosition(edge, positionRight))
        markDirty();
    }


    /** ================================ dimension =================================== **/

    void setStyleWidth(float width) {
      if (mCssStyle->mStyleWidth != width) {
        mCssStyle->mStyleWidth = width;
        markDirty();
      }
    }

    float getStyleWidth() {
      return mCssStyle->mStyleWidth;
    }

    void setStyleHeight(float height) {
      if (mCssStyle->mStyleHeight != height) {
        mCssStyle->mStyleHeight = height;
        markDirty();
      }
    }

    float getStyleHeight() {
      return mCssStyle->mStyleHeight;
    }

    void setMinWidth(float minWidth) {
      if (mCssStyle->mMinWidth != minWidth) {
        mCssStyle->mMinWidth = minWidth;
        markDirty();
      }
    }

    float getMinWidth() {
      return mCssStyle->mMinWidth;
    }

    void setMaxWidth(float maxWidth) {
      if (mCssStyle->mMaxWidth != maxWidth) {
        mCssStyle->mMaxWidth = maxWidth;
        markDirty();
      }
    }

    float getMaxWidth() {
      return mCssStyle->mMaxWidth;
    }

    void setMinHeight(float minHeight) {
      if (mCssStyle->mMinHeight != minHeight) {
        mCssStyle->mMinHeight = minHeight;
        markDirty();
      }
    }

    float getMinHeight() {
      return mCssStyle->mMinHeight;
    }

    void setMaxHeight(float maxHeight) {
      if (mCssStyle->mMaxHeight != maxHeight) {
        mCssStyle->mMaxHeight = maxHeight;
        markDirty();
      }
    }

    float getMaxHeight() {
      return mCssStyle->mMaxHeight;
    }


    /** ================================ flex-style =================================== **/

    void setFlexDirection(WXCoreFlexDirection flexDirection) {
      if (mCssStyle->mFlexDirection != flexDirection) {
        mCssStyle->mFlexDirection = flexDirection;
        markDirty();
      }
    }

    WXCoreFlexDirection getFlexDirection() {
      return mCssStyle->mFlexDirection;
    }

    void setFlexWrap(WXCoreFlexWrap flexWrap) {
      if (mCssStyle->mFlexWrap != flexWrap) {
        mCssStyle->mFlexWrap = flexWrap;
        markDirty();
      }
    }

    WXCoreFlexWrap getFlexWrap() {
      return mCssStyle->mFlexWrap;
    }

    void setJustifyContent(WXCoreJustifyContent justifyContent) {
      if (mCssStyle->mJustifyContent != justifyContent) {
        mCssStyle->mJustifyContent = justifyContent;
      }
    }

    WXCoreJustifyContent getJustifyContent() {
      return mCssStyle->mJustifyContent;
    }

    void setAlignItems(WXCoreAlignItems alignItems) {
      if (mCssStyle->mAlignItems != alignItems) {
        mCssStyle->mAlignItems = alignItems;
        markDirty();
      }
    }

    WXCoreAlignItems getAlignItems() {
      return mCssStyle->mAlignItems;
    }

    void setAlignSelf(WXCoreAlignSelf alignSelf) {
      if (mCssStyle->mAlignSelf != alignSelf) {
        mCssStyle->mAlignSelf = alignSelf;
        markDirty();
      }
    }

    WXCoreAlignSelf getAlignSelf() {
      return mCssStyle->mAlignSelf;
    }

    void setFlex(float flex) {
      if (mCssStyle->mFlexGrow != flex) {
        mCssStyle->mFlexGrow = flex;
        markDirty();
      }
    }

    float getFlex() {
      return mCssStyle->mFlexGrow;
    }

    /** ================================ layout-result =================================== **/

  public:

    float getLayoutWidth() {
      return mLayoutResult->mLayoutSize.width;
    }

    float getLayoutHeight() {
      return mLayoutResult->mLayoutSize.height;
    }

    float getLayoutPositionTop() {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeTop);
    }

    float getLayoutPositionBottom() {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeBottom);
    }

    float getLayoutPositionLeft() {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeLeft);
    }

    float getLayoutPositionRight() {
      return mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeRight);
    }

  private:

    void setMeasuredDimension(const float width, const float height) {
      mLayoutResult->mLayoutSize.width = width;
      mLayoutResult->mLayoutSize.height = height;
    }


    /** ================================ other =================================== **/

  private:

    void clearDirty() {
      dirty = false;
      widthDirty = false;
      heightDirty = false;
    }

  public:

    bool hasNewLayout() {
      return mHasNewLayout;
    }

    bool isDirty() {
      return dirty;
    }

    void markDirty(bool recursion = true) {
      if (!isDirty()) {
        dirty = true;
        if (getParent() != nullptr && recursion) {
          getParent()->markDirty();
        }
      }
    }

    void setHasNewLayout(bool hasNewLayout) {
      this->mHasNewLayout = hasNewLayout;
    }

    bool isVisible() {
      return mVisible;
    }

    void setVisible(bool visible) {
      mVisible = visible;
    }

    bool isUndefined(float value) {
      return isnan(value);
    }

    float getLargestMainSize() {
      float largestSize = 0;
      for (WXCoreFlexLine *flexLine : mFlexLines) {
        largestSize = std::max(largestSize, flexLine->mMainSize);
      }
      return largestSize;
    }

  };
}
#endif //WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#endif