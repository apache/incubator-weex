#ifndef WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#define WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H

#include "WXCoreStyle.h"
#include "WXCoreFlexEnum.h"
#include <limits.h>
#include <vector>
#include <memory.h>
#include <math.h>

namespace WXCoreFlexLayout {

  /**
   * layout-result：layout-height、layout-width、position（left、right、top、bottom）
   */
  typedef struct WXCorelayoutResult {
    float mComputedHeight;
    float mComputedWidth;
    WXCorePosition mLayoutPosition;
  } WXCorelayoutResult;


  /**
   * flie line
   */
  class WXCoreFlexLine {
  public:
    float mMainSize;

    float mCrossSize;

    int mItemCount;

    int mGoneItemCount;

    float mTotalFlexGrow;

    /**
     * The largest value of the individual child's baseline
     * if the {@link WXCoreLayoutNode #mAlignItems} value is not
     * {@link WXCoreAlignSelf # WXCore_AlignSelf_Baseline}
     * or the flex direction is vertical, this value is not used.
     * If the alignment direction is from the bottom to top,
     * (e.g. mFlexWrap == WXCore_Wrap_WrapReverse and flexDirection == WXCore_Flex_Direction_Row)
     * store this value from the distance from the bottom of the view minus baseline.
     */
    float mMaxBaseline;

    /**
     * The sum of the cross size used before this flex line.
     */
    float mSumCrossSizeBefore;

    /**
     * Store the indices of the children views whose mAlignSelf property is stretch.
     * The stored indices are the absolute indices including all children in the Flexbox,
     * not the relative indices in this flex line.
     */
    std::vector<int> mIndicesAlignSelfStretch;

    /**
     * @return the count of the views whose visibilities are not gone in this flex line.
     */
    int getItemCountNotGone() {
      return mItemCount - mGoneItemCount;
    }

    WXCoreFlexLine() : mMainSize(0),
                       mCrossSize(0),
                       mItemCount(0),
                       mGoneItemCount(0),
                       mTotalFlexGrow(0),
                       mMaxBaseline(0),
                       mSumCrossSizeBefore(0) {
    }
  };


  class WXCoreLayoutNode;

  typedef std::vector<WXCoreFlexLine *>::iterator FLEXLINTS_IT;
  typedef std::vector<WXCoreLayoutNode *>::iterator CHILD_LIST_IT;

  /**
   * Layout node
   */
  class WXCoreLayoutNode {
  public:
    WXCoreLayoutNode() :
        mChildrenFrozen(nullptr),
        mChildrenFrozen_oldlength(0),
        mParent(nullptr),
        mHasNewLayout(true),
        mIsDirty(false),
        mVisible(true) {
    }

    ~WXCoreLayoutNode() {
      if (mChildrenFrozen != nullptr) {
        delete mChildrenFrozen;
        mChildrenFrozen = nullptr;
      }

      if (mParent != nullptr) {
        delete mParent;
        mParent = nullptr;
      }

      for (FLEXLINTS_IT it = mFlexLines.begin(); it != mFlexLines.end(); ++it) {
        if (*it != nullptr) {
          delete *it;
          *it = nullptr;
        }
      }

      for (CHILD_LIST_IT it = mChildList.begin(); it != mChildList.end(); ++it) {
        if (*it != nullptr) {
          delete *it;
          *it = nullptr;
        }
      }
    }

  private:
    /**
     * Holds the 'frozen' state of children during measure. If a view is frozen it will no longer
     * expand regardless of mFlexGrow. Items are indexed by the child's
     * reordered index.
     */
    bool *mChildrenFrozen;

    int mChildrenFrozen_oldlength;

    std::vector<WXCoreFlexLine *> mFlexLines;

    std::vector<WXCoreLayoutNode *> mChildList;

    WXCoreLayoutNode *mParent;

    WXCoreCSSStyle mCssStyle;

    WXCorelayoutResult mLayoutResult;

    bool mHasNewLayout;

    bool mIsDirty;

    bool mVisible;

    /** ================================ measure =================================== **/

    void onMeasure(float width, float height);

    void measureHorizontal(float width, float height);

    void measureVertical(float width, float height);

    void checkSizeConstraints(WXCoreLayoutNode *node);

    void addFlexLineIfLastFlexItem(int childIndex, int childCount, WXCoreFlexLine *flexLine,
                                   float usedCrossSizeSoFar);

    void addFlexLine(WXCoreFlexLine *flexLine, float usedCrossSizeSoFar);

    void determineMainSize(WXCoreFlexDirection flexDirection, float widthMeasureSpec,
                           float heightMeasureSpec);

    int expandFlexItems(float widthMeasureSpec, float heightMeasureSpec, WXCoreFlexLine *flexLine,
                        WXCoreFlexDirection flexDirection, float maxMainSize,
                        float paddingAlongMainAxis,
                        int startIndex, bool calledRecursively);

    float getChildWidthMeasureSpec(float widthMeasureSpec, WXCoreLayoutNode *node, float padding);

    float getChildHeightMeasureSpec(float heightMeasureSpec, WXCoreLayoutNode *node, float padding);

    void determineCrossSize(WXCoreFlexDirection flexDirection, float widthMeasureSpec,
                            float heightMeasureSpec, float paddingAlongCrossAxis);

    void stretchViews(WXCoreFlexDirection flexDirection, WXCoreAlignItems alignItems);

    void stretchViewVertically(WXCoreLayoutNode *node, float crossSize);

    void stretchViewHorizontally(WXCoreLayoutNode *node, float crossSize);

    bool isWrapRequired(float maxSize, float currentLength, float childLength);

    float getSumOfCrossSize();

    bool isMainAxisDirectionHorizontal(WXCoreFlexDirection flexDirection);


    /** ================================ layout =================================== **/

    void onLayout(float left, float top, float right, float bottom);

    void layoutHorizontal(bool isRtl, float left, float top, float right, float bottom);

    void layoutSingleChildHorizontal(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine,
                                     WXCoreFlexWrap flexWrap,
                                     WXCoreAlignItems alignItems, float left, float top, float right,
                                     float bottom);

    void layoutVertical(bool isRtl, bool fromBottomToTop, float left, float top,
                        float right, float bottom);

    void layoutSingleChildVertical(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine, bool isRtl,
                                   WXCoreAlignItems alignItems, float left, float top, float right,
                                   float bottom);

    void setFrame(float l, float t, float r, float b) {
      mLayoutResult.mLayoutPosition.setPosition(WXCore_PositionEdge_Left, l);
      mLayoutResult.mLayoutPosition.setPosition(WXCore_PositionEdge_Top, t);
      mLayoutResult.mLayoutPosition.setPosition(WXCore_PositionEdge_Right, r);
      mLayoutResult.mLayoutPosition.setPosition(WXCore_PositionEdge_Bottom, b);
    }

  public:
    /**
     * Entry function to calculate layout
     */
    void calculateLayout() {
      measure(getStyleWidth(), getStyleHeight());

      layout(getLayoutPositionLeft(), getLayoutPositionTop(),
             getLayoutWidth(), getLayoutHeight());
    }

    void measure(float width, float height) {
      onMeasure(width, height);
    }

    void layout(float left, float top, float right, float bottom) {
      setFrame(left, top, right, bottom);
      onLayout(left, top, right, bottom);
    }

    int getChildCount() {
      return mChildList.size();
    }

    void removeChildAt(int index) {
      mChildList.erase(mChildList.begin() + index);
    }

    void addChildAt(WXCoreLayoutNode *child, int index) {
      mChildList.insert(mChildList.begin() + index, child);
    }

    WXCoreLayoutNode *getChildAt(int index) {
      return mChildList[index];
    }


    /** ================================ margin =================================== **/

    float getMarginTop() {
      return mCssStyle.mMargin.getMargin(WXCore_Margin_Top);
    }

    float getMarginBottom() {
      return mCssStyle.mMargin.getMargin(WXCore_Margin_Bottom);
    }

    float getMarginLeft() {
      return mCssStyle.mMargin.getMargin(WXCore_Margin_Left);
    }

    float getMarginRight() {
      return mCssStyle.mMargin.getMargin(WXCore_Margin_Right);
    }

    void setMargin(WXCoreMarginEdge edge, float margin) {
      if (mCssStyle.mMargin.setMargin(edge, margin))
        dirty();
    }


    /** ================================ padding =================================== **/

    float getPaddingLeft() {
      return mCssStyle.mPadding.getPadding(WXCore_Padding_Left);
    }

    float getPaddingRight() {
      return mCssStyle.mPadding.getPadding(WXCore_Padding_Right);
    }

    float getPaddingTop() {
      return mCssStyle.mPadding.getPadding(WXCore_Padding_Top);
    }

    float getPaddingBottom() {
      return mCssStyle.mPadding.getPadding(WXCore_Padding_Bottom);
    }

    void setPadding(WXCorePaddingEdge edge, float padding) {
      if (mCssStyle.mPadding.setPadding(edge, padding))
        dirty();
    }


    /** ================================ border-width =================================== **/

    float getBorderWidthLeft() {
      return mCssStyle.mBorderWidth.getBorderWidth(WXCore_Border_Width_Left);
    }

    float getBorderWidthRight() {
      return mCssStyle.mBorderWidth.getBorderWidth(WXCore_Border_Width_Right);
    }

    float getBorderWidthTop() {
      return mCssStyle.mBorderWidth.getBorderWidth(WXCore_Border_Width_Top);
    }

    float getBorderWidthBottom() {
      return mCssStyle.mBorderWidth.getBorderWidth(WXCore_Border_Width_Bottom);
    }

    void setBorderWidth(WXCoreBorderWidthEdge edge, float borderWidth) {
      if (mCssStyle.mBorderWidth.setBorderWidth(edge, borderWidth))
        dirty();
    }


    /** ================================ position =================================== **/

    float getStylePositionTop() {
      return mCssStyle.mStylePosition.getPosition(WXCore_PositionEdge_Top);
    }

    float getStylePositionBottom() {
      return mCssStyle.mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
    }

    float getStylePositionLeft() {
      return mCssStyle.mStylePosition.getPosition(WXCore_PositionEdge_Left);
    }

    float getStylePositionRight() {
      return mCssStyle.mStylePosition.getPosition(WXCore_PositionEdge_Right);
    }

    void setStylePosition(WXCorePositionEdge edge, float positionRight) {
      if (mCssStyle.mStylePosition.setPosition(edge, positionRight))
        dirty();
    }


    /** ================================ position-type =================================== **/

    void setStylePositionType(WXCorePositionType positionType) {
      if (mCssStyle.mPositionType != positionType) {
        mCssStyle.mPositionType = positionType;
        dirty();
      }
    }

    WXCorePositionType getStypePositionType() {
      return mCssStyle.mPositionType;
    }


    /** ================================ dimension =================================== **/

    void setMinWidth(float minWidth) {
      if (mCssStyle.mMinWidth != minWidth) {
        mCssStyle.mMinWidth = minWidth;
        dirty();
      }
    }

    void setMaxWidth(float maxWidth) {
      if (mCssStyle.mMaxWidth != maxWidth) {
        mCssStyle.mMaxWidth = maxWidth;
        dirty();
      }
    }

    void setMinHeight(float minHeight) {
      if (mCssStyle.mMinHeight != minHeight) {
        mCssStyle.mMinHeight = minHeight;
        dirty();
      }
    }

    void setMaxHeight(float maxHeight) {
      if (mCssStyle.mMaxHeight != maxHeight) {
        mCssStyle.mMaxHeight = maxHeight;
        dirty();
      }
    }

    float getStyleWidth() {
      return mCssStyle.mStyleWidth;
    }

    void setStyleWidth(float width) {
      if (mCssStyle.mStyleWidth != width) {
        mCssStyle.mStyleWidth = width;
        dirty();
      }
    }

    float getStyleHeight() {
      return mCssStyle.mStyleHeight;
    }

    void setStyleHeight(float height) {
      if (mCssStyle.mStyleHeight != height) {
        mCssStyle.mStyleHeight = height;
        dirty();
      }
    }

    /** ================================ layout-result =================================== **/

    float getLayoutWidth() {
      return mLayoutResult.mComputedWidth;
    }

    float getLayoutHeight() {
      return mLayoutResult.mComputedHeight;
    }

    float getLayoutX() {
      return getLayoutPositionLeft();
    }

    float getLayoutY() {
      return getLayoutPositionTop();
    }

    float getLayoutPositionTop() {
      return mLayoutResult.mLayoutPosition.getPosition(WXCore_PositionEdge_Top);
    }

    float getLayoutPositionBottom() {
      return mLayoutResult.mLayoutPosition.getPosition(WXCore_PositionEdge_Bottom);
    }

    float getLayoutPositionLeft() {
      return mLayoutResult.mLayoutPosition.getPosition(WXCore_PositionEdge_Left);
    }

    float getLayoutPositionRight() {
      return mLayoutResult.mLayoutPosition.getPosition(WXCore_PositionEdge_Right);
    }

    void setMeasuredDimension(float width, float height) {
      mLayoutResult.mComputedWidth = width;
      mLayoutResult.mComputedHeight = height;
    }


    /** ================================ flex-style =================================== **/

    WXCoreFlexDirection getFlexDirection() {
      return mCssStyle.mFlexDirection;
    }

    void setFlexDirection(WXCoreFlexDirection flexDirection) {
      if (mCssStyle.mFlexDirection != flexDirection) {
        mCssStyle.mFlexDirection = flexDirection;
        dirty();
      }
    }

    WXCoreFlexWrap getFlexWrap() {
      return mCssStyle.mFlexWrap;
    }

    void setFlexWrap(WXCoreFlexWrap flexWrap) {
      if (mCssStyle.mFlexWrap != flexWrap) {
        mCssStyle.mFlexWrap = flexWrap;
        dirty();
      }
    }

    WXCoreJustifyContent getJustifyContent() {
      return mCssStyle.mJustifyContent;
    }

    void setJustifyContent(WXCoreJustifyContent justifyContent) {
      if (mCssStyle.mJustifyContent != justifyContent) {
        mCssStyle.mJustifyContent = justifyContent;
        dirty();
      }
    }

    WXCoreAlignItems getAlignItems() {
      return mCssStyle.mAlignItems;
    }

    void setAlignItems(WXCoreAlignItems alignItems) {
      if (mCssStyle.mAlignItems != alignItems) {
        mCssStyle.mAlignItems = alignItems;
        dirty();
      }
    }

    WXCoreAlignSelf getAlignSelf() {
      return mCssStyle.mAlignSelf;
    }

    void setAlignSelf(WXCoreAlignSelf alignSelf) {
      if (mCssStyle.mAlignSelf != alignSelf) {
        mCssStyle.mAlignSelf = alignSelf;
        dirty();
      }
    }

    float getFlexValue() {
      return mCssStyle.mFlexValue;
    }

    void setFlex(float flex) {
      if (mCssStyle.mFlexValue != flex) {
        mCssStyle.mFlexValue = flex;
        dirty();
      }
    }


    bool hasNewLayout() {
      return mHasNewLayout;
    }

    bool isDirty() {
      return mIsDirty;
    }

    void dirty() {
      requestLayout();
    }

    bool isVisible() {
      return mVisible;
    }

    void setVisible(bool visible) {
      mVisible = visible;
    }

    float getBaseline() {
      return 1;
    }

    void requestLayout() {
      mIsDirty = true;
      if (getParent() != nullptr) {
        getParent()->requestLayout();
      }
    }

    WXCoreLayoutNode *getParent() {
      return mParent;
    }

  private:
    float getLargestMainSize();
  };

}


#endif //WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
