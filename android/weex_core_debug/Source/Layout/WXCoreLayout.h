#ifndef WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#define WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H

#include "WXCoreStyle.h"
#include <limits.h>
#include <vector>
#include <memory.h>
#include <math.h>

class WXCoreFlexLine {
public:

  float mMainSize;

  /**
   * The sum of the lengths of dividers along the main axis. This value should be lower or
   * than than the value of {@link #mMainSize}.
   */
  float mDividerLengthInMainSize;

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

  WXCoreFlexLine() : mMainSize(0),
                     mDividerLengthInMainSize(0),
                     mCrossSize(0),
                     mItemCount(0),
                     mGoneItemCount(0),
                     mTotalFlexGrow(0),
                     mMaxBaseline(0),
                     mSumCrossSizeBefore(0) {
  }

  /**
   * @return the count of the views whose visibilities are not gone in this flex line.
   */
  int getItemCountNotGone() {
    return mItemCount - mGoneItemCount;
  }
};


class WXCoreLayoutNode {
private:
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

  /**
   * Holds the 'frozen' state of children during measure. If a view is frozen it will no longer
   * expand regardless of mFlexGrow. Items are indexed by the child's
   * reordered index.
   */
  bool *mChildrenFrozen;

  int mChildrenFrozen_oldlength;

  std::vector<WXCoreFlexLine *> mFlexLines;

  std::vector<WXCoreLayoutNode *> mChildren;

  WXCoreLayoutNode *mParent;

  float mStyleWidth;
  float mStyleHeight;
  float mComputedHeight;
  float mComputedWidth;

  WXCoreMargin mMargin;

  WXCorePadding mPadding;

  WXCoreBorderWidth mBorderWidth;

  WXCorePosition mStylePosition;

  WXCorePosition mLayoutPosition;

  WXCorePositionType mPositionType;

  bool mHasNewLayout;

  bool mIsDirty;

  bool mVisible;

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
    mLayoutPosition.setPosition(WXCore_PositionEdge_Left, l);
    mLayoutPosition.setPosition(WXCore_PositionEdge_Top, t);
    mLayoutPosition.setPosition(WXCore_PositionEdge_Right, r);
    mLayoutPosition.setPosition(WXCore_PositionEdge_Bottom, b);
  }


public:

  /**
   * Entry function to calculate layout
   */
  void calculateLayout() {
    measure(getStyleWidth(), getStyleHeight());

    layout(getLayoutLeft(), getLayoutTop(),
           getLayoutWidth(), getLayoutHeight());
  }

  const static float FLEX_DEFAULT;

  const static float FLEX_GROW_DEFAULT;

  const static float MAX_SIZE;

  float mFlexValue;

  /**
   * This attribute determines how much this child will grow if positive free space is
   * distributed relative to the rest of other flex items included in the same flex line.
   * If not specified, {@link #FLEX_GROW_DEFAULT} is set as a default value.
   */
  float mFlexGrowValue;

  /**
   * This attribute determines the minimum mStyleWidth the child can shrink to.
   */
  float mMinWidth;

  /**
   * This attribute determines the minimum mStyleHeight the child can shrink to.
   */
  float mMinHeight;

  /**
   * This attribute determines the maximum mStyleWidth the child can expand to.
   */
  float mMaxWidth;

  /**
   * This attribute determines the maximum mStyleHeight the child can expand to.
   */
  float mMaxHeight;

  WXCoreLayoutNode() : mFlexDirection(WXCore_Flex_Direction_Row),
                       mFlexWrap(WXCore_Wrap_NoWrap),
                       mJustifyContent(WXCore_Justify_Flex_Start),
                       mAlignItems(WXCore_AlignItems_Stretch),
                       mAlignSelf(WXCore_AlignSelf_Auto),
                       mChildrenFrozen(nullptr),
                       mChildrenFrozen_oldlength(0),
                       mParent(nullptr),
                       mStyleWidth(NAN), mStyleHeight(NAN),
                       mComputedHeight(0), mComputedWidth(0),
                       mHasNewLayout(true),
                       mIsDirty(true),
                       mVisible(true),
                       mFlexValue(FLEX_DEFAULT),
                       mFlexGrowValue(FLEX_GROW_DEFAULT),
                       mMaxWidth(MAX_SIZE), mMaxHeight(MAX_SIZE) {
  }

  ~WXCoreLayoutNode() {
    delete mChildrenFrozen;
    mChildrenFrozen = nullptr;
  }

  void measure(float width, float height) {
    onMeasure(width, height);
  }

  void layout(float left, float top, float right, float bottom) {
    setFrame(left, top, right, bottom);
    onLayout(left, top, right, bottom);
  }

  int getChildCount() {
    return mChildren.size();
  }

  void removeChildAt(int index) {
    mChildren.erase(mChildren.begin() + index);
  }

  void addChildAt(WXCoreLayoutNode *child, int index) {
    mChildren.insert(mChildren.begin() + index, child);
  }

  WXCoreLayoutNode *getChildAt(int index) {
    return mChildren[index];
  }

  float getMarginTop() {
    return mMargin.getMargin(WXCore_Margin_Top);
  }

  float getMarginBottom() {
    return mMargin.getMargin(WXCore_Margin_Bottom);
  }

  float getMarginLeft() {
    return mMargin.getMargin(WXCore_Margin_Left);
  }

  float getMarginRight() {
    return mMargin.getMargin(WXCore_Margin_Right);
  }

  float getPaddingLeft() {
    return mPadding.getPadding(WXCore_Padding_Left);
  }

  float getPaddingRight() {
    return mPadding.getPadding(WXCore_Padding_Right);
  }

  float getPaddingTop() {
    return mPadding.getPadding(WXCore_Padding_Top);
  }

  float getPaddingBottom() {
    return mPadding.getPadding(WXCore_Padding_Bottom);
  }

  float getBorderWidthLeft() {
    return mBorderWidth.getBorderWidth(WXCore_Border_Width_Left);
  }

  float getBorderWidthRight() {
    return mBorderWidth.getBorderWidth(WXCore_Border_Width_Right);
  }

  float getBorderWidthTop() {
    return mBorderWidth.getBorderWidth(WXCore_Border_Width_Top);
  }

  float getBorderWidthBottom() {
    return mBorderWidth.getBorderWidth(WXCore_Border_Width_Bottom);
  }

  void setMargin(WXCoreMarginEdge edge, float margin) {
    mMargin.setMargin(edge, margin);
  }

  void setPadding(WXCorePaddingEdge edge, float padding) {
    mPadding.setPadding(edge, padding);
  }

  void setBorderWidth(WXCoreBorderWidthEdge edge, float borderWidth) {
    mBorderWidth.setBorderWidth(edge, borderWidth);
  }

  float getStylePositionTop() {
    return mStylePosition.getPosition(WXCore_PositionEdge_Top);
  }

  void setStylePositionTop(float positionTop) {
    mStylePosition.setPosition(WXCore_PositionEdge_Top, positionTop);
  }

  float getStylePositionBottom() {
    return mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
  }

  void setStylePositionBottom(float positionBottom) {
    mStylePosition.setPosition(WXCore_PositionEdge_Bottom, positionBottom);
  }

  float getStylePositionLeft() {
    return mStylePosition.getPosition(WXCore_PositionEdge_Left);
  }

  void setStylePositionLeft(float positionLeft) {
    mStylePosition.setPosition(WXCore_PositionEdge_Left, positionLeft);
  }

  float getStylePositionRight() {
    return mStylePosition.getPosition(WXCore_PositionEdge_Right);
  }

  void setStylePositionRight(float positionRight) {
    mStylePosition.setPosition(WXCore_PositionEdge_Right, positionRight);
  }

  void setStylePositionType(WXCorePositionType positionType) {
    mPositionType = positionType;
  }

  WXCorePositionType getStypePositionType() {
    return mPositionType;
  }

  void setMinWidth(float minWidth) {
    if (mMinWidth != minWidth) {
      mMinWidth = minWidth;
      dirty();
    }
  }

  void setMaxWidth(float maxWidth) {
    if (mMaxWidth != maxWidth) {
      mMaxWidth = maxWidth;
      dirty();
    }
  }

  void setMinHeight(float minHeight) {
    if (mMinHeight != minHeight) {
      mMinHeight = minHeight;
      dirty();
    }
  }

  void setMaxHeight(float maxHeight) {
    if (mMaxHeight != maxHeight) {
      mMaxHeight = maxHeight;
      dirty();
    }
  }

  /**
   * Get this node's mStyleWidth, as defined in the cssstyle.
   */
  float getStyleWidth() {
    return mStyleWidth;
  }

  void setStyleWidth(float width) {
    if (mStyleWidth != width) {
      mStyleWidth = width;
      dirty();
    }
  }

  /**
   * Get this node's mStyleHeight, as defined in the cssstyle.
   */
  float getStyleHeight() {
    return mStyleHeight;
  }

  void setStyleHeight(float height) {
    if (mStyleHeight != height) {
      mStyleHeight = height;
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

  float getLayoutWidth() {
    return mComputedWidth;
  }

  float getLayoutHeight() {
    return mComputedHeight;
  }

  float getLayoutX() {
    return getLayoutLeft();
  }

  float getLayoutY() {
    return getLayoutTop();
  }

  float getLayoutTop() {
    return mLayoutPosition.getPosition(WXCore_PositionEdge_Top);
  }

  float getLayoutBottom() {
    return mLayoutPosition.getPosition(WXCore_PositionEdge_Bottom);
  }

  float getLayoutLeft() {
    return mLayoutPosition.getPosition(WXCore_PositionEdge_Left);
  }

  float getLayoutRight() {
    return mLayoutPosition.getPosition(WXCore_PositionEdge_Right);
  }

  float getBaseline() {
    return 1;
  }

  void setMeasuredDimension(float width, float height) {
    mComputedWidth = width;
    mComputedHeight = height;
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

  WXCoreFlexDirection getFlexDirection() {
    return mFlexDirection;
  }

  void setFlexDirection(WXCoreFlexDirection flexDirection) {
    if (mFlexDirection != flexDirection) {
      mFlexDirection = flexDirection;
      requestLayout();
    }
  }

  WXCoreFlexWrap getFlexWrap() {
    return mFlexWrap;
  }

  void setFlexWrap(WXCoreFlexWrap flexWrap) {
    if (mFlexWrap != flexWrap) {
      mFlexWrap = flexWrap;
      requestLayout();
    }
  }

  WXCoreJustifyContent getJustifyContent() {
    return mJustifyContent;
  }

  void setJustifyContent(WXCoreJustifyContent justifyContent) {
    if (mJustifyContent != justifyContent) {
      mJustifyContent = justifyContent;
      requestLayout();
    }
  }

  WXCoreAlignItems getAlignItems() {
    return mAlignItems;
  }

  void setAlignItems(WXCoreAlignItems alignItems) {
    if (mAlignItems != alignItems) {
      mAlignItems = alignItems;
      requestLayout();
    }
  }

  WXCoreAlignSelf getAlignSelf() {
    return mAlignSelf;
  }

  void setAlignSelf(WXCoreAlignSelf alignSelf) {
    if (mAlignSelf != alignSelf) {
      mAlignSelf = alignSelf;
      requestLayout();
    }
  }

  /**
   * Get this node's flex value, as defined by cssstyle.
   */
  float getFlexValue() {
    return mFlexValue;
  }

  void setFlex(float flex) {
    if (mFlexValue != flex) {
      mFlexValue = flex;
      dirty();
    }
  }

  /**
   * Per child parameters for children views of the {@link WXCoreLayoutNode}.
   */
  static float getChildMeasureSpec(float spec, float padding, float childDimension) {
    return childDimension;
  }
};


#endif //WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
