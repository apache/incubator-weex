#ifndef WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#define WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H

#include "WXCoreStyle.h"
#include "WXCoreFlexEnum.h"
#include <vector>
#include <memory.h>

namespace WXCoreFlexLayout {

  class WXCoreLayoutNode;

  class WXCoreFlexLine;

  typedef struct WXCoreSize {
    float height;
    float width;

    void reset() {
      height = 0;
      width = 0;
    }
  } WXCoreSize;

  /**
   * layout-result：layout-height、layout-width、position（left、right、top、bottom）
   */
  typedef struct WXCorelayoutResult {
    WXCoreSize mLayoutSize;
    WXCorePosition mLayoutPosition;

    void reset() {
      mLayoutSize.reset();
      mLayoutPosition.reset();
    }
  } WXCorelayoutResult;

  typedef WXCoreSize (*WXCoreMeasureFunc)(WXCoreLayoutNode *node, float width, float height);


  /**
   * flie line
   */
  class WXCoreFlexLine {
  public:
    float mMainSize;

    float mCrossSize;

    uint32_t mItemCount;

    float mTotalFlexGrow;

    /**
     * The sum of the cross size used before this flex line.
     */
    float mSumCrossSizeBefore;

    float mTotalFlexibleSize;

    /**
     * Store the indices of the children views whose mAlignSelf property is stretch.
     * The stored indices are the absolute indices including all children in the Flexbox,
     * not the relative indices in this flex line.
     */
    std::vector<uint32_t> mIndicesAlignSelfStretch;

    WXCoreFlexLine() : mMainSize(0),
                       mCrossSize(0),
                       mItemCount(0),
                       mTotalFlexGrow(0),
                       mSumCrossSizeBefore(0),
                       mTotalFlexibleSize(0) {
    }

    ~WXCoreFlexLine() {
      mMainSize = 0;
      mCrossSize = 0;
      mItemCount = 0;
      mTotalFlexGrow = 0;
      mSumCrossSizeBefore = 0;
      mTotalFlexibleSize = 0;
      mIndicesAlignSelfStretch.clear();
    }
  };


  /**
   * Layout node
   */
  class WXCoreLayoutNode {
  public:
    static WXCoreLayoutNode *newWXCoreNode() {
      return new WXCoreLayoutNode();
    }

    void freeWXCoreNode() {
      mHasNewLayout = true;
      mIsDirty = false;
      mVisible = true;
      measureFunc = nullptr;
      mParent = nullptr;
      mChildrenFrozen_oldlength = 0;
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
    }

  private:

    WXCoreLayoutNode() :
        mChildrenFrozen(nullptr),
        mChildrenFrozen_oldlength(0),
        mParent(nullptr),
        mHasNewLayout(true),
        mIsDirty(false),
        mVisible(true),
        measureFunc(nullptr) {
      mCssStyle = new WXCoreCSSStyle();
      mLayoutResult = new WXCorelayoutResult();
    }

    /**
     * Holds the 'frozen' state of children during measure. If a view is frozen it will no longer
     * expand regardless of mFlexGrow. Items are indexed by the child's
     * reordered index.
     */
    bool *mChildrenFrozen;

    uint32_t mChildrenFrozen_oldlength;

    std::vector<WXCoreFlexLine *> mFlexLines;

    std::vector<WXCoreLayoutNode *> mChildList;

    WXCoreLayoutNode *mParent;

    WXCoreCSSStyle *mCssStyle;

    WXCorelayoutResult *mLayoutResult;

    bool mHasNewLayout;

    bool mIsDirty;

    bool mVisible;

    WXCoreMeasureFunc measureFunc;

  public:

    /** ================================ measureFunc =================================== **/

    void setMeasureFunc(WXCoreMeasureFunc measure) {
      measureFunc = measure;
    }

    WXCoreMeasureFunc getMeasureFunc() {
      return measureFunc;
    }

    bool haveMeasureFunc() {
      return measureFunc == nullptr ? false : true;
    }

  private:

    /** ================================ measure =================================== **/

    void onMeasure(float width, float height);

    void measureHorizontal(float width, float height);

    void measureVertical(float width, float height);

    void checkSizeConstraints(WXCoreLayoutNode *node);

    void addFlexLineIfLastFlexItem(uint32_t childIndex, uint32_t childCount, WXCoreFlexLine *flexLine,
                                   float usedCrossSizeSoFar);

    void addFlexLine(WXCoreFlexLine *flexLine, float usedCrossSizeSoFar);

    void determineMainSize(WXCoreFlexDirection flexDirection, float widthMeasureSpec,
                           float heightMeasureSpec);

    uint32_t expandFlexItems(WXCoreFlexLine *flexLine,
                             WXCoreFlexDirection flexDirection, float maxMainSize,
                             float paddingAlongMainAxis,
                             uint32_t startIndex, bool calledRecursively);

    void setMeasuredDimensionForFlex(WXCoreFlexDirection flexDirection, float width, float height);

    void determineCrossSize(WXCoreFlexDirection flexDirection, float widthMeasureSpec,
                            float heightMeasureSpec, float paddingAlongCrossAxis);

    void stretchViews(WXCoreFlexDirection flexDirection, WXCoreAlignItems alignItems);

    void stretchViewVertically(WXCoreLayoutNode *node, float crossSize);

    void stretchViewHorizontally(WXCoreLayoutNode *node, float crossSize);

    bool isWrapRequired(float mainSize, float currentLength, float childLength);

    bool isMainAxisDirectionHorizontal(WXCoreFlexDirection flexDirection);

    bool
    obeyConstrainsDuringExpandMainAxis(WXCoreFlexLine *flexLine, uint32_t childIndex, float flexGrow) {
      // This means the child can't expand beyond the value of the
      // maxWidth/minWidth/maxHeight/minHeight attribute.
      // To adjust the flex line length to the size of mainSize, remaining
      // positive free space needs to be re-distributed to other flex items
      // (children views). In that case, invoke this method again with the same
      // startIndex.
      mChildrenFrozen[childIndex] = true;
      flexLine->mTotalFlexGrow -= flexGrow;
      return true;
    }

    float getSumOfCrossSize() {
      float sum = 0;
      for (WXCoreFlexLine *flexLine: mFlexLines) {
        sum += flexLine->mCrossSize;
      }
      return sum;
    }

    void measure(float width, float height, bool useMeasureFunc) {

      if (useMeasureFunc && measureFunc != nullptr &&
          (isnan(width) || isnan(height))) {
        WXCoreSize size = measureFunc(this, width, height);
        onMeasure(size.width, size.height);
        if (isnan(width)) {
          width = mLayoutResult->mLayoutSize.width +
                  getPaddingRight() + getBorderWidthRight() + getPaddingLeft() + getBorderWidthLeft();
        }
        if (isnan(height)) {
          height = mLayoutResult->mLayoutSize.height +
                   getPaddingTop() + getBorderWidthTop() + getPaddingBottom() + getBorderWidthBottom();
        }
        mLayoutResult->reset();
      }

      onMeasure(width, height);
    }

    /** ================================ layout =================================== **/

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

    void setFrame(float l, float t, float r, float b) {
      mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Left, l);
      mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Top, t);
      mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Right, r);
      mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Bottom, b);
    }

    void determinePositionRelative(float &left, float &top, float &right, float &bottom);

    void layout(float left, float top, float right, float bottom) {
      determinePositionRelative(left, top, right, bottom);
      setFrame(left, top, right, bottom);
      onLayout(left, top, right, bottom);
    }

  public:
    /**
     * Entry function to calculate layout
     */
    void calculateLayout() {
      mLayoutResult->reset();
      measure(getStyleWidth(), getStyleHeight(), true);
      layout(mCssStyle->mMargin.getMargin(WXCore_Margin_Left) + getLayoutPositionLeft(),
             mCssStyle->mMargin.getMargin(WXCore_Margin_Top) + getLayoutPositionTop(),
             mCssStyle->mMargin.getMargin(WXCore_Margin_Left) + getLayoutWidth(),
             mCssStyle->mMargin.getMargin(WXCore_Margin_Top) + getLayoutHeight());
    }

    /** ================================ tree =================================== **/

    uint32_t getChildCount() {
      return mChildList.size();
    }

    void removeChildAt(uint32_t index) {
      mChildList.erase(mChildList.begin() + index);
    }

    void addChildAt(WXCoreLayoutNode *child, uint32_t index) {
      mChildList.insert(mChildList.begin() + index, child);
      child->mParent = this;
    }

    WXCoreLayoutNode *getChildAt(uint32_t index) {
      return mChildList[index];
    }

    WXCoreLayoutNode *getParent() {
      return mParent;
    }


    /** ================================ margin =================================== **/

    float getMarginTop() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Top);
    }

    float getMarginBottom() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Bottom);
    }

    float getMarginLeft() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Left);
    }

    float getMarginRight() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Right);
    }

    void setMargin(WXCoreMarginEdge edge, float margin) {
      if (mCssStyle->mMargin.setMargin(edge, margin))
        dirty();
    }


    /** ================================ padding =================================== **/

    float getPaddingLeft() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Left);
    }

    float getPaddingRight() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Right);
    }

    float getPaddingTop() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Top);
    }

    float getPaddingBottom() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Bottom);
    }

    void setPadding(WXCorePaddingEdge edge, float padding) {
      if (mCssStyle->mPadding.setPadding(edge, padding))
        dirty();
    }


    /** ================================ border-width =================================== **/

    float getBorderWidthLeft() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Left);
    }

    float getBorderWidthRight() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Right);
    }

    float getBorderWidthTop() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Top);
    }

    float getBorderWidthBottom() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Bottom);
    }

    void setBorderWidth(WXCoreBorderWidthEdge edge, float borderWidth) {
      if (mCssStyle->mBorderWidth.setBorderWidth(edge, borderWidth))
        dirty();
    }


    /** ================================ position-type =================================== **/

    void setStylePositionType(WXCorePositionType positionType) {
      if (mCssStyle->mPositionType != positionType) {
        mCssStyle->mPositionType = positionType;
        dirty();
      }
    }

    WXCorePositionType getStypePositionType() {
      return mCssStyle->mPositionType;
    }


    /** ================================ position =================================== **/

    float getStylePositionTop() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top);
    }

    float getStylePositionBottom() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
    }

    float getStylePositionLeft() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left);
    }

    float getStylePositionRight() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right);
    }

    void setStylePosition(WXCorePositionEdge edge, float positionRight) {
      if (mCssStyle->mStylePosition.setPosition(edge, positionRight))
        dirty();
    }


    /** ================================ dimension =================================== **/

    void setStyleWidth(float width) {
      if (mCssStyle->mStyleWidth != width) {
        mCssStyle->mStyleWidth = width;
        dirty();
      }
    }

    float getStyleWidth() {
      return mCssStyle->mStyleWidth;
    }

    void setStyleHeight(float height) {
      if (mCssStyle->mStyleHeight != height) {
        mCssStyle->mStyleHeight = height;
        dirty();
      }
    }

    float getStyleHeight() {
      return mCssStyle->mStyleHeight;
    }

    void setMinWidth(float minWidth) {
      if (mCssStyle->mMinWidth != minWidth) {
        mCssStyle->mMinWidth = minWidth;
        dirty();
      }
    }

    float getMinWidth() {
      return mCssStyle->mMinWidth;
    }

    void setMaxWidth(float maxWidth) {
      if (mCssStyle->mMaxWidth != maxWidth) {
        mCssStyle->mMaxWidth = maxWidth;
        dirty();
      }
    }

    float getMaxWidth() {
      return mCssStyle->mMaxWidth;
    }

    void setMinHeight(float minHeight) {
      if (mCssStyle->mMinHeight != minHeight) {
        mCssStyle->mMinHeight = minHeight;
        dirty();
      }
    }

    float getMinHeight() {
      return mCssStyle->mMinHeight;
    }

    void setMaxHeight(float maxHeight) {
      if (mCssStyle->mMaxHeight != maxHeight) {
        mCssStyle->mMaxHeight = maxHeight;
        dirty();
      }
    }

    float getMaxHeight() {
      return mCssStyle->mMaxHeight;
    }


    /** ================================ flex-style =================================== **/

    void setFlexDirection(WXCoreFlexDirection flexDirection) {
      if (mCssStyle->mFlexDirection != flexDirection) {
        mCssStyle->mFlexDirection = flexDirection;
        dirty();
      }
    }

    WXCoreFlexDirection getFlexDirection() {
      return mCssStyle->mFlexDirection;
    }

    void setFlexWrap(WXCoreFlexWrap flexWrap) {
      if (mCssStyle->mFlexWrap != flexWrap) {
        mCssStyle->mFlexWrap = flexWrap;
        dirty();
      }
    }

    WXCoreFlexWrap getFlexWrap() {
      return mCssStyle->mFlexWrap;
    }

    void setJustifyContent(WXCoreJustifyContent justifyContent) {
      if (mCssStyle->mJustifyContent != justifyContent) {
        mCssStyle->mJustifyContent = justifyContent;
        dirty();
      }
    }

    WXCoreJustifyContent getJustifyContent() {
      return mCssStyle->mJustifyContent;
    }

    void setAlignItems(WXCoreAlignItems alignItems) {
      if (mCssStyle->mAlignItems != alignItems) {
        mCssStyle->mAlignItems = alignItems;
        dirty();
      }
    }

    WXCoreAlignItems getAlignItems() {
      return mCssStyle->mAlignItems;
    }

    void setAlignSelf(WXCoreAlignSelf alignSelf) {
      if (mCssStyle->mAlignSelf != alignSelf) {
        mCssStyle->mAlignSelf = alignSelf;
        dirty();
      }
    }

    WXCoreAlignSelf getAlignSelf() {
      return mCssStyle->mAlignSelf;
    }

    void setFlex(float flex) {
      if (mCssStyle->mFlexGrow != flex) {
        mCssStyle->mFlexGrow = flex;
        dirty();
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
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Top);
    }

    float getLayoutPositionBottom() {
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Bottom);
    }

    float getLayoutPositionLeft() {
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Left);
    }

    float getLayoutPositionRight() {
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Right);
    }

  private:

    void setMeasuredDimension(float width, float height) {
      mLayoutResult->mLayoutSize.width = width;
      mLayoutResult->mLayoutSize.height = height;
    }


    /** ================================ other =================================== **/

  private:
    bool hasNewLayout() {
      return mHasNewLayout;
    }

    bool isDirty() {
      return mIsDirty;
    }

    void dirty() {
      requestLayout();
    }

    void requestLayout() {
      mIsDirty = true;
      if (getParent() != nullptr) {
        getParent()->requestLayout();
      }
    }

  public:
    bool isVisible() {
      return mVisible;
    }

    void setVisible(bool visible) {
      mVisible = visible;
    }

  private:
    float getLargestMainSize();
  };

}


#endif //WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
