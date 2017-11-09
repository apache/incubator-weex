#include "WXCoreLayout.h"

using namespace WXCoreFlexLayout;

const float WXCoreCSSStyle::FLEX_GROW_DEFAULT = 0;
const WXCoreFlexDirection WXCoreCSSStyle::FLEX_DIRECTION_DEFAULT = WXCore_Flex_Direction_Column;
const WXCoreFlexWrap WXCoreCSSStyle::FLEX_WRAP_DEFAULT = WXCore_Wrap_NoWrap;
const WXCoreJustifyContent WXCoreCSSStyle::FLEX_JUSTIFY_CONTENT_DEFAULT = WXCore_Justify_Flex_Start;
const WXCoreAlignItems WXCoreCSSStyle::FLEX_ALIGN_ITEMS_DEFAULT = WXCore_AlignItems_Stretch;
const WXCoreAlignSelf WXCoreCSSStyle::FLEX_ALIGN_SELF_DEFAULT = WXCore_AlignSelf_Auto;

template<class T>
static T max_num(T a, T b) {
  return a > b ? a : b;
}

template<class T>
static T min_num(T a, T b) {
  return a < b ? a : b;
}

namespace WXCoreFlexLayout {

  /**
   * Entry function to calculate layout
   */
  void WXCoreLayoutNode::calculateLayout() {
    BFCs.clear();
    initFormatingContext(BFCs);
    initMeasureMode();
    reset();
    WXBFCDimension bfcDimension = calculateBFCDimension();
    measure(bfcDimension.width, bfcDimension.widthMeasureMode, bfcDimension.height, bfcDimension.heightMeasureMode,
            true);
    layout(mCssStyle->mMargin.getMargin(WXCore_Margin_Left),
           mCssStyle->mMargin.getMargin(WXCore_Margin_Top),
           mCssStyle->mMargin.getMargin(WXCore_Margin_Left) + getLayoutWidth(),
           mCssStyle->mMargin.getMargin(WXCore_Margin_Top) + getLayoutHeight());
    for (int i = 0; i < getChildCount(BFC); ++i) {
      WXCoreLayoutNode *child = getChildAt(BFC, i);
      child->calculateLayout();
    }
  }

  void WXCoreLayoutNode::initFormatingContext(std::vector<WXCoreLayoutNode *> &BFCs) {
    NonBFCs.clear();
    for (int i = 0; i < getChildCount(ALL); i++) {
      WXCoreLayoutNode *child = getChildAt(ALL, i);
      if (isBFC(child)) {
        BFCs.push_back(child);
      } else {
        NonBFCs.push_back(child);
        child->initFormatingContext(BFCs);
      }
    }
  }

  WXBFCDimension WXCoreLayoutNode::calculateBFCDimension() {
    MeasureMode bfcWidthMeasureMode, bfcHeightMeasureMode;
    float width, height;
    width = isnan(mCssStyle->mStyleWidth) ? MAXFLOAT / 2.0f : mCssStyle->mStyleWidth;
    height = isnan(mCssStyle->mStyleHeight) ? MAXFLOAT / 2.0f : mCssStyle->mStyleHeight;
    bfcWidthMeasureMode =
            isnan(mCssStyle->mStyleWidth) ? AT_MOST : EXACTLY;
    bfcHeightMeasureMode =
            isnan(mCssStyle->mStyleHeight) ? AT_MOST : EXACTLY;

    if (mCssStyle->mPositionType == WXCore_PositionType_Absolute) {
      if (isnan(mCssStyle->mStyleWidth) &&
          mParent != nullptr &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left)) &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right))) {
        width = mParent->mLayoutResult->mLayoutSize.width -
                mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left) -
                mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right);
        bfcWidthMeasureMode = EXACTLY;
      }

      if (isnan(mCssStyle->mStyleHeight) &&
          mParent != nullptr &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top)) &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom))) {
        height = mParent->mLayoutResult->mLayoutSize.height -
                 mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top) -
                 mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
        bfcHeightMeasureMode = EXACTLY;
      }
    }

    WXBFCDimension bfcDimension;
    bfcDimension.width = width;
    bfcDimension.height = height;
    bfcDimension.widthMeasureMode = bfcWidthMeasureMode;
    bfcDimension.heightMeasureMode = bfcHeightMeasureMode;
    return bfcDimension;
  }

  void WXCoreLayoutNode::initMeasureMode() {
    widthMeasureMode =
            isnan(mCssStyle->mStyleWidth) ? AT_MOST : EXACTLY;
    heightMeasureMode =
            isnan(mCssStyle->mStyleHeight) ? AT_MOST : EXACTLY;
    for (int i = 0; i < getChildCount(NON_BFC); i++) {
      WXCoreLayoutNode *child = getChildAt(NON_BFC, i);
      child->initMeasureMode();
    }
  }

  void WXCoreLayoutNode::measure(float width, MeasureMode widthMeasureMode, float height, MeasureMode heightMeasureMode,
                                 bool useMeasureFunc) {
    if (getChildCount(NON_BFC) > 0) {
      onMeasure(width, widthMeasureMode, height, heightMeasureMode);
    } else {
      if (useMeasureFunc && measureFunc != nullptr &&
          (isnan(mCssStyle->mStyleWidth) || isnan(mCssStyle->mStyleHeight))) {
        WXCoreSize dimension = measureFunc(this, width, widthMeasureMode, height, heightMeasureMode);
//          onMeasure(dimension.width, dimension.height);
        width = widthMeasureMode == EXACTLY ? dimension.width :
                min_num<float>(dimension.width + mCssStyle->mPadding.getPadding(WXCore_Padding_Right) +
                               mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Right) +
                               mCssStyle->mPadding.getPadding(WXCore_Padding_Left) +
                               mCssStyle->mBorderWidth.getBorderWidth(
                                   WXCore_Border_Width_Left),
                               width);
        height = heightMeasureMode == EXACTLY ? dimension.height :
                 min_num<float>(dimension.height + mCssStyle->mPadding.getPadding(WXCore_Padding_Top) +
                                mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Top) +
                                mCssStyle->mPadding.getPadding(WXCore_Padding_Bottom) +
                                mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Bottom),
                                height);
      } else {
        if (widthMeasureMode == AT_MOST) {
          width = 0;
        }
        if (heightMeasureMode == AT_MOST) {
          height = 0;
        }
      }
      setMeasuredDimension(width, height);
    }
  }

  void
  WXCoreLayoutNode::onMeasure(float width, MeasureMode widthMeasureMode, float height, MeasureMode heightMeasureMode) {
    if (mChildrenFrozen == nullptr || mChildrenFrozen_oldlength < getChildCount(NON_BFC)) {
      mChildrenFrozen = new bool[getChildCount(NON_BFC)];
      mChildrenFrozen_oldlength = getChildCount(NON_BFC);
    }

    // Only calculate the children views which are affected from the last measure.
    switch (mCssStyle->mFlexDirection) {
      case WXCore_Flex_Direction_Row:
      case WXCore_Flex_Direction_Row_Reverse:
        measureHorizontal(width, widthMeasureMode, height, heightMeasureMode);
        break;
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
      default:
        measureVertical(width, widthMeasureMode, height, heightMeasureMode);
        break;
    }

    memset(mChildrenFrozen, false, getChildCount(NON_BFC));
  }

  /**
   * Sub method for {@link WXCoreLayoutNode #onMeasure(int, int)}, when the main axis direction is horizontal
   *
   * @param width  horizontal space requirements as imposed by the parent
   * @param height vertical space requirements as imposed by the parent
   */
  void WXCoreLayoutNode::measureHorizontal(float width, MeasureMode widthMeasureMode, float height,
                                           MeasureMode heightMeasureMode) {

    mFlexLines.clear();

    uint32_t childCount = getChildCount(NON_BFC);

    float largestHeightInRow = 0.0f;
    float totalCrossSize = 0.0f;
    WXCoreFlexLine *flexLine = new WXCoreFlexLine();

    flexLine->mMainSize = getPaddingLeft() + getPaddingRight() + getBorderWidthLeft() + getBorderWidthRight();

    // Determine how many flex lines are needed in this layout by measuring each child.
    for (uint32_t i = 0; i < childCount; i++) {
      WXCoreLayoutNode *child = getChildAt(NON_BFC, i);

      // Record stretch child's index
      if (child->mCssStyle->mAlignSelf == WXCore_AlignSelf_Stretch ||
          (mCssStyle->mAlignItems == WXCore_AlignItems_Stretch &&
           child->mCssStyle->mAlignSelf == WXCore_AlignSelf_Auto)) {
        flexLine->mIndicesAlignSelfStretch.push_back(i);
      }

      WXCoreSize measureDimension = getChildDimension(width, height, child,
                                                      totalCrossSize);

      // Measure child based on its MeasureSpec
      child->measure(measureDimension.width, child->widthMeasureMode,
                     measureDimension.height, child->heightMeasureMode, true);

      // Checks if the view's mStyleWidth/mStyleHeight don't violate the minimum/maximum size constraints
      checkSizeConstraints(child);

      largestHeightInRow = max_num<float>(largestHeightInRow,
                                          child->mLayoutResult->mLayoutSize.height + child->getMarginTop() +
                                          child->getMarginBottom());

      // Determine if a wrap is required (add a new flexline)
      if (isWrapRequired(width, flexLine->mMainSize,
                         child->mLayoutResult->mLayoutSize.width + child->getMarginLeft() + child->getMarginRight()
      )) {
        if (flexLine->mItemCount > 0) {
          addFlexLine(flexLine, totalCrossSize);
          totalCrossSize += flexLine->mCrossSize;
        }

        flexLine = new WXCoreFlexLine();
        flexLine->mItemCount = 1;
        flexLine->mMainSize = getPaddingLeft() + getPaddingRight() + getBorderWidthLeft() + getBorderWidthRight();
        largestHeightInRow =
            child->mLayoutResult->mLayoutSize.height + child->getMarginTop() + child->getMarginBottom();
      } else {
        flexLine->mItemCount++;
      }

      flexLine->mMainSize += child->mLayoutResult->mLayoutSize.width + child->getMarginLeft() + child->getMarginRight();

      if (child->mCssStyle->mFlexGrow > 0) {
        flexLine->mTotalFlexGrow += child->mCssStyle->mFlexGrow;
        flexLine->mTotalFlexibleSize += child->mLayoutResult->mLayoutSize.width;
      } else {
        mChildrenFrozen[i] = true;
      }

      // Temporarily set the cross axis length as the largest child in the row
      // Expand along the cross axis depending on the mAlignContent property if needed later
      flexLine->mCrossSize = max_num<float>(flexLine->mCrossSize, largestHeightInRow);

      addFlexLineIfLastFlexItem(i, childCount, flexLine, totalCrossSize);
    }

    // Determine the main size by expanding the individual flexGrow attribute.
    determineMainSize(mCssStyle->mFlexDirection, width, widthMeasureMode, height, heightMeasureMode);

    // Determines the cross size (Calculate the length along the cross axis).
    determineCrossSize(mCssStyle->mFlexDirection, width, widthMeasureMode, height, heightMeasureMode,
                       getPaddingTop() + getPaddingBottom() + getBorderWidthTop() + getBorderWidthBottom());

    // Expand the views if alignItems (or alignSelf in each child view) is set to stretch.
    stretchViews(mCssStyle->mFlexDirection, mCssStyle->mAlignItems);

    // Set measure result.
    setMeasuredDimensionForFlex(mCssStyle->mFlexDirection, width, widthMeasureMode, height, heightMeasureMode);
  }

  /**
   * Sub method for {@link WXCoreLayoutNode #onMeasure(int, int)} when the main axis direction is vertical
   * (either from mStyleTop to mStyleBottom or mStyleBottom to mStyleTop).
   *
   * @param width  horizontal space requirements as imposed by the parent
   * @param height vertical space requirements as imposed by the parent
   */
  void WXCoreLayoutNode::measureVertical(float width, MeasureMode widthMeasureMode, float height,
                                         MeasureMode heightMeasureMode) {

    mFlexLines.clear();

    uint32_t childCount = getChildCount(NON_BFC);
    float largestWidthInColumn = 0.0f;
    float totalCrossSize = 0.0f;

    WXCoreFlexLine *flexLine = new WXCoreFlexLine();
    flexLine->mMainSize = getPaddingTop() + getPaddingBottom() + getBorderWidthTop() + getBorderWidthBottom();

    // Determine how many flex lines are needed in this layout by measuring each child.
    for (uint32_t i = 0; i < childCount; i++) {
      WXCoreLayoutNode *child = getChildAt(NON_BFC, i);

      // Record stretch child's index
      if (child->mCssStyle->mAlignSelf == WXCore_AlignSelf_Stretch ||
          (mCssStyle->mAlignItems == WXCore_AlignItems_Stretch &&
           child->mCssStyle->mAlignSelf == WXCore_AlignSelf_Auto)) {
        flexLine->mIndicesAlignSelfStretch.push_back(i);
      }

      WXCoreSize measureDimension = getChildDimension(width, height, child,
                                                      totalCrossSize);
      // Measure child based on its MeasureSpec
      child->measure(measureDimension.width, child->widthMeasureMode,
                     measureDimension.height, child->heightMeasureMode, true);

      // Checks if the view's mStyleWidth/mStyleHeight don't violate the minimum/maximum size constraints
      checkSizeConstraints(child);

      largestWidthInColumn = max_num<float>(largestWidthInColumn,
                                            child->mLayoutResult->mLayoutSize.width + child->getMarginLeft() +
                                            child->getMarginRight());

      // Determine if a wrap is required (add a new flexline)
      if (isWrapRequired(height, flexLine->mMainSize,
                         child->mLayoutResult->mLayoutSize.height + child->getMarginTop() + child->getMarginBottom()
      )) {
        if (flexLine->mItemCount > 0) {
          addFlexLine(flexLine, totalCrossSize);
          totalCrossSize += flexLine->mCrossSize;
        }

        flexLine = new WXCoreFlexLine();
        flexLine->mItemCount = 1;
        flexLine->mMainSize = getPaddingTop() + getPaddingBottom() + getBorderWidthTop() + getBorderWidthBottom();
        largestWidthInColumn =
            child->mLayoutResult->mLayoutSize.width + child->getMarginLeft() + child->getMarginRight();
      } else {
        flexLine->mItemCount++;
      }

      flexLine->mMainSize +=
          child->mLayoutResult->mLayoutSize.height + child->getMarginTop() + child->getMarginBottom();

      if (child->mCssStyle->mFlexGrow > 0) {
        flexLine->mTotalFlexGrow += child->mCssStyle->mFlexGrow;
        flexLine->mTotalFlexibleSize += child->mLayoutResult->mLayoutSize.height;
      } else {
        mChildrenFrozen[i] = true;
      }

      // Temporarily set the cross axis length as the largest child in the row
      // Expand along the cross axis depending on the mAlignContent property if needed later
      flexLine->mCrossSize = max_num<float>(flexLine->mCrossSize, largestWidthInColumn);
      addFlexLineIfLastFlexItem(i, childCount, flexLine, totalCrossSize);
    }

    // Determine the main size by expanding the individual flexGrow attribute.
    determineMainSize(mCssStyle->mFlexDirection, width, widthMeasureMode, height, heightMeasureMode);

    // Determines the cross size (Calculate the length along the cross axis).
    determineCrossSize(mCssStyle->mFlexDirection, width, widthMeasureMode, height, heightMeasureMode,
                       getPaddingLeft() + getPaddingRight() + getBorderWidthLeft() + getBorderWidthRight());

    // Expand the views if alignItems (or alignSelf in each child view) is set to stretch.
    stretchViews(mCssStyle->mFlexDirection, mCssStyle->mAlignItems);

    // Set measure result.
    setMeasuredDimensionForFlex(mCssStyle->mFlexDirection, width, widthMeasureMode, height, heightMeasureMode);
  }

  /**
   * Checks if the view's mStyleWidth/mStyleHeight don't violate the minimum/maximum size constraints
   * imposed by the {@link WXCoreLayoutNode #mMinWidth}, {@link WXCoreLayoutNode #mMinHeight},
   * {@link WXCoreLayoutNode #mMaxWidth} and {@link WXCoreLayoutNode #mMaxHeight} attributes.
   */
  void WXCoreLayoutNode::checkSizeConstraints(WXCoreLayoutNode *node) {
    bool needReMeasure = false;
    float childWidth = node->mLayoutResult->mLayoutSize.width;
    float childHeight = node->mLayoutResult->mLayoutSize.height;

    if (childWidth < node->mCssStyle->mMinWidth) {
      needReMeasure = true;
      childWidth = node->mCssStyle->mMinWidth;
    } else if (childWidth > node->mCssStyle->mMaxWidth) {
      needReMeasure = true;
      childWidth = node->mCssStyle->mMaxWidth;
    }

    if (childHeight < node->mCssStyle->mMinHeight) {
      needReMeasure = true;
      childHeight = node->mCssStyle->mMinHeight;
    } else if (childHeight > node->mCssStyle->mMaxHeight) {
      needReMeasure = true;
      childHeight = node->mCssStyle->mMaxHeight;
    }

    if (needReMeasure) {
      node->measure(childWidth, EXACTLY, childHeight, EXACTLY, false);
    }
  }

  /**
   * Determine the main size by expanding the individual flexGrow attribute.
   */
  void WXCoreLayoutNode::determineMainSize(WXCoreFlexDirection flexDirection,
                                           float width, MeasureMode widthMeasureMode,
                                           float height, MeasureMode heightMeasureMode) {
    float maxMainSize;
    float paddingAlongMainAxis;

    switch (flexDirection) {
      case WXCore_Flex_Direction_Row:
      case WXCore_Flex_Direction_Row_Reverse:
        maxMainSize = widthMeasureMode == AT_MOST ? getLargestMainSize() : width;
        paddingAlongMainAxis = getPaddingLeft() + getPaddingRight() + getBorderWidthLeft() + getBorderWidthRight();
        break;
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
      default:
        maxMainSize = heightMeasureMode == AT_MOST ? getLargestMainSize() : height;
        paddingAlongMainAxis = getPaddingTop() + getPaddingBottom() + getBorderWidthTop() + getBorderWidthBottom();
        break;
    }

    uint32_t childIndex = 0;
    for (WXCoreFlexLine *flexLine : mFlexLines) {
      if (flexLine->mMainSize < maxMainSize) {
        childIndex = expandFlexItems(flexLine,
                                     flexDirection, maxMainSize, paddingAlongMainAxis, childIndex, false);
      }
    }
  }

  /**
   * Expand the flex items along the main axis based on the individual flexGrow attribute.
   *
   * @param flexLine             the flex line to which flex items belong
   * @param flexDirection        the flexDirection value for this item
   * @param maxMainSize          the maximum main size. Expanded main size will be this size
   * @param paddingAlongMainAxis the padding value along the main axis
   * @param startIndex           the start index of the children views to be expanded. This index
   *                             needs to be an absolute index in the flex container,
   *                             not the relative index in the flex line.
   * @param calledRecursively    true if this method is called recursively, false otherwise
   * @return the next startIndex, the next flex line's first flex item starts from the returned index
   */
  uint32_t WXCoreLayoutNode::expandFlexItems(WXCoreFlexLine *flexLine,
                                             WXCoreFlexDirection flexDirection, float maxMainSize,
                                             float paddingAlongMainAxis,
                                             uint32_t startIndex, bool calledRecursively) {
    uint32_t childIndex = startIndex;
    if (flexLine->mTotalFlexGrow <= 0) {
      childIndex += flexLine->mItemCount;
      return childIndex;
    }

    float sizeBeforeExpand = flexLine->mMainSize;
    bool needReExpand = false;
    float unitSpace = (maxMainSize - flexLine->mMainSize + flexLine->mTotalFlexibleSize) /
                      (flexLine->mTotalFlexGrow > 1 ? flexLine->mTotalFlexGrow : 1);
    flexLine->mMainSize = paddingAlongMainAxis;

    if (!calledRecursively) {
      flexLine->mCrossSize = 0.0f;
    }

    float largestCrossSize = 0.0f;

    for (uint32_t i = 0; i < flexLine->mItemCount; i++) {
      WXCoreLayoutNode *child = getChildAt(NON_BFC, childIndex);

      if (isMainAxisDirectionHorizontal(flexDirection)) {
        // The direction of the main axis is horizontal
        if (!mChildrenFrozen[childIndex]) {
          float rawCalculatedWidth = unitSpace * child->mCssStyle->mFlexGrow;

          if (rawCalculatedWidth > child->mCssStyle->mMaxWidth) {
            needReExpand = obeyConstrainsDuringExpandMainAxis(flexLine, childIndex,
                                                              child->mCssStyle->mFlexGrow);
            rawCalculatedWidth = child->mCssStyle->mMaxWidth;
          } else if (rawCalculatedWidth < child->mCssStyle->mMinWidth) {
            needReExpand = obeyConstrainsDuringExpandMainAxis(flexLine, childIndex,
                                                              child->mCssStyle->mFlexGrow);
            rawCalculatedWidth = child->mCssStyle->mMinWidth;
          }

          child->measure(rawCalculatedWidth, EXACTLY, child->mLayoutResult->mLayoutSize.height, EXACTLY, false);
          largestCrossSize = max_num<float>(largestCrossSize, child->mLayoutResult->mLayoutSize.height
                                                              + child->getMarginTop() + child->getMarginBottom());
        }

        flexLine->mMainSize +=
            child->mLayoutResult->mLayoutSize.width + child->getMarginLeft() + child->getMarginRight();
      } else {
        // The direction of the main axis is vertical
        if (!mChildrenFrozen[childIndex]) {
          float rawCalculatedHeight = unitSpace * child->mCssStyle->mFlexGrow;
          if (rawCalculatedHeight > child->mCssStyle->mMaxHeight) {
            needReExpand = obeyConstrainsDuringExpandMainAxis(flexLine, childIndex,
                                                              child->mCssStyle->mFlexGrow);
            rawCalculatedHeight = child->mCssStyle->mMaxHeight;
          } else if (rawCalculatedHeight < child->mCssStyle->mMinHeight) {
            needReExpand = obeyConstrainsDuringExpandMainAxis(flexLine, childIndex,
                                                              child->mCssStyle->mFlexGrow);
            rawCalculatedHeight = child->mCssStyle->mMinHeight;
          }

          child->measure(child->mLayoutResult->mLayoutSize.width, EXACTLY, rawCalculatedHeight, EXACTLY, false);
          largestCrossSize = max_num<float>(largestCrossSize, child->mLayoutResult->mLayoutSize.width
                                                              + child->getMarginLeft() + child->getMarginRight());
        }
        flexLine->mMainSize +=
            child->mLayoutResult->mLayoutSize.height + child->getMarginTop() + child->getMarginBottom();
      }

      flexLine->mCrossSize = max_num<float>(flexLine->mCrossSize, largestCrossSize);
      childIndex++;
    }

    if (needReExpand && sizeBeforeExpand != flexLine->mMainSize) {
      // Re-invoke the method with the same startIndex to distribute the positive free space
      // that wasn't fully distributed (because of maximum length constraint)
      expandFlexItems(flexLine, flexDirection,
                      maxMainSize, paddingAlongMainAxis, startIndex, true);
    }

    return childIndex;
  }

  /**
   * @param flexDirection         the flex direction attribute
   * @param width                 stylewidth by this node
   * @param height                styleheight by this node
   * @param paddingAlongCrossAxis the padding value for this node along the cross axis
   */
  void
  WXCoreLayoutNode::determineCrossSize(WXCoreFlexDirection flexDirection, float width, MeasureMode widthMeasureMode,
                                       float height, MeasureMode heightMeasureMode, float paddingAlongCrossAxis) {
    float size;
    bool crossMeasureModeIsExactly;
    switch (flexDirection) {
      case WXCore_Flex_Direction_Row:
      case WXCore_Flex_Direction_Row_Reverse:
        if (heightMeasureMode == EXACTLY) {
          size = height;
          crossMeasureModeIsExactly = true;
        } else {
          size = getSumOfCrossSize();
          crossMeasureModeIsExactly = false;
        }
        break;
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
      default:
        if (widthMeasureMode == EXACTLY) {
          size = width;
          crossMeasureModeIsExactly = true;
        } else {
          size = getSumOfCrossSize();
          crossMeasureModeIsExactly = false;
        }
        break;
    }

    if (mFlexLines.size() == 1 && crossMeasureModeIsExactly) {
      mFlexLines[0]->mCrossSize = size - paddingAlongCrossAxis;
    }
  }

  /**
   * Expand the view if the {@link #WXCoreAlignItems} attribute is set to {@link WXCoreAlignItems #STRETCH}
   * or {@link WXCoreAlignItems #STRETCH} is set to an individual child view.
   *
   * @param flexDirection the flex direction attribute
   * @param alignItems    the align items attribute
   */
  void WXCoreLayoutNode::stretchViews(WXCoreFlexDirection flexDirection, WXCoreAlignItems alignItems) {
    if (alignItems == WXCore_AlignItems_Stretch) {
      // WXCoreAlignItems is STRETCH
      uint32_t viewIndex = 0;
      for (WXCoreFlexLine *flexLine : mFlexLines) {
        for (uint32_t i = 0; i < flexLine->mItemCount; i++, viewIndex++) {
          WXCoreLayoutNode *child = getChildAt(NON_BFC, viewIndex);

          if (child->mCssStyle->mAlignSelf != WXCore_AlignSelf_Auto &&
              child->mCssStyle->mAlignSelf != WXCore_AlignSelf_Stretch) {
            continue;
          }

          switch (flexDirection) {
            case WXCore_Flex_Direction_Row:
            case WXCore_Flex_Direction_Row_Reverse:
              stretchViewVertically(child, flexLine->mCrossSize);
              break;
            case WXCore_Flex_Direction_Column:
            case WXCore_Flex_Direction_Column_Reverse:
            default:
              stretchViewHorizontally(child, flexLine->mCrossSize);
              break;
          }
        }
      }
    } else {
      // WXCoreAlignItems is STRETCH
      for (WXCoreFlexLine *flexLine : mFlexLines) {
        for (uint32_t index : flexLine->mIndicesAlignSelfStretch) {
          WXCoreLayoutNode *child = getChildAt(NON_BFC, index);
          switch (flexDirection) {
            case WXCore_Flex_Direction_Row:
            case WXCore_Flex_Direction_Row_Reverse:
              stretchViewVertically(child, flexLine->mCrossSize);
              break;
            case WXCore_Flex_Direction_Column:
            case WXCore_Flex_Direction_Column_Reverse:
            default:
              stretchViewHorizontally(child, flexLine->mCrossSize);
              break;
          }
        }
      }
    }
  }

  /**
   * Expand the view vertically to the size of the crossSize (considering the view margins)
   *
   * @param node      the item to be stretched
   * @param crossSize the cross size
   */
  void WXCoreLayoutNode::stretchViewVertically(WXCoreLayoutNode *node, float crossSize) {
    if (node->heightMeasureMode != EXACTLY) {
      float newHeight = crossSize - node->getMarginTop() - node->getMarginBottom();
      newHeight = max_num<float>(newHeight, 0);
      node->measure(node->mLayoutResult->mLayoutSize.width, EXACTLY, newHeight, EXACTLY, false);
    }
  }

  /**
   * Expand the view horizontally to the size of the crossSize (considering the view margins)
   *
   * @param node      the item to be stretched
   * @param crossSize the cross size
   */
  void WXCoreLayoutNode::stretchViewHorizontally(WXCoreLayoutNode *node, float crossSize) {
    if (node->widthMeasureMode != EXACTLY) {
      float newWidth = crossSize - node->getMarginLeft() - node->getMarginRight();
      newWidth = max_num<float>(newWidth, 0);
      node->measure(newWidth, EXACTLY, node->mLayoutResult->mLayoutSize.height, EXACTLY, false);
    }
  }

  void WXCoreLayoutNode::setMeasuredDimensionForFlex(WXCoreFlexDirection flexDirection, float width,
                                                     MeasureMode widthMeasureMode,
                                                     float height, MeasureMode heightMeasureMode) {
    float calculatedMaxHeight, calculatedMaxWidth;
    switch (flexDirection) {
      case WXCore_Flex_Direction_Row: // Intentional fall through
      case WXCore_Flex_Direction_Row_Reverse:
        calculatedMaxHeight =
            getSumOfCrossSize() + getPaddingTop() + getPaddingBottom() + getBorderWidthTop() + getBorderWidthBottom();
        calculatedMaxWidth = getLargestMainSize();
        break;
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
      default:
        calculatedMaxHeight = getLargestMainSize();
        calculatedMaxWidth =
            getSumOfCrossSize() + getPaddingLeft() + getPaddingRight() + getBorderWidthLeft() + getBorderWidthRight();
        break;
    }

    float actualWidth, actualHeight;
    actualWidth = widthMeasureMode == EXACTLY ? width : calculatedMaxWidth;
    actualHeight = heightMeasureMode == EXACTLY ? height : calculatedMaxHeight;
    setMeasuredDimension(actualWidth, actualHeight);
  }

  void WXCoreLayoutNode::layout(float left, float top, float right, float bottom) {
    switch (mCssStyle->mPositionType) {
      case WXCore_PositionType_Absolute:
        calcAbsoluteOffset(left, top, right, bottom);
        break;
      default:
      case WXCore_PositionType_Relative:
        calcRelativeOffset(left, top, right, bottom);
        break;
    }
    setFrame(left, top, right, bottom);
    onLayout(left, top, right, bottom);
  }

  void WXCoreLayoutNode::calcRelativeOffset(float &left, float &top, float &right, float &bottom) {
    if (!isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left))) {
      left += mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left);
      right += mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left);
    } else if (!isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right))) {
      left -= mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right);
      right -= mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right);
    }

    if (!isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top))) {
      top += mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top);
      bottom += mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top);
    } else if (!isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom))) {
      top -= mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
      bottom -= mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
    }
  }

  void WXCoreLayoutNode::calcAbsoluteOffset(float &left, float &top, float &right, float &bottom) {
    WXCorePadding parentPadding;
    WXCoreBorderWidth parentBorder;
    WXCoreSize parentSize;

    if (mParent != nullptr) {
      parentPadding = mParent->mCssStyle->mPadding;
      parentBorder = mParent->mCssStyle->mBorderWidth;
      parentSize = mParent->mLayoutResult->mLayoutSize;
    }

    if (isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left))) {
      if (isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right))) {
        left += parentBorder.getBorderWidth(WXCore_Border_Width_Left) + parentPadding.getPadding(WXCore_Padding_Left);
        right += parentBorder.getBorderWidth(WXCore_Border_Width_Left) + parentPadding.getPadding(WXCore_Padding_Left);
      } else {
        right += parentSize.width -
                 (parentBorder.getBorderWidth(WXCore_Border_Width_Right) +
                  mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right)
                  + mLayoutResult->mLayoutSize.width);
        left += parentSize.width -
                (parentBorder.getBorderWidth(WXCore_Border_Width_Right) +
                 mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right)
                 + mLayoutResult->mLayoutSize.width);
      }
    } else {
      left += parentBorder.getBorderWidth(WXCore_Border_Width_Left) +
              mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left);
      right += parentBorder.getBorderWidth(WXCore_Border_Width_Left) +
               mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left);
    }

    if (isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top))) {
      if (isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom))) {
        top += parentBorder.getBorderWidth(WXCore_Border_Width_Top) + parentPadding.getPadding(WXCore_Padding_Top);
        bottom += parentBorder.getBorderWidth(WXCore_Border_Width_Top) + parentPadding.getPadding(WXCore_Padding_Top);
      } else {
        top += parentSize.height -
               (parentBorder.getBorderWidth(WXCore_Border_Width_Bottom) +
                mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom)
                + mLayoutResult->mLayoutSize.height);
        bottom += parentSize.height -
                  (parentBorder.getBorderWidth(WXCore_Border_Width_Bottom) +
                   mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom)
                   + mLayoutResult->mLayoutSize.height);
      }
    } else {
      top += parentBorder.getBorderWidth(WXCore_Border_Width_Top) +
             mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top);
      bottom += parentBorder.getBorderWidth(WXCore_Border_Width_Top) +
                mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top);
    }
  }

  void WXCoreLayoutNode::onLayout(float left, float top, float right, float bottom) {
    switch (mCssStyle->mFlexDirection) {
      case WXCore_Flex_Direction_Row:
        layoutHorizontal(false, left, top, right, bottom);
        break;
      case WXCore_Flex_Direction_Row_Reverse:
        layoutHorizontal(true, left, top, right, bottom);
        break;
      case WXCore_Flex_Direction_Column_Reverse:
        layoutVertical(mCssStyle->mFlexWrap == WXCore_Wrap_WrapReverse, true, left, top, right, bottom);
        break;
      case WXCore_Flex_Direction_Column:
      default:
        layoutVertical(mCssStyle->mFlexWrap == WXCore_Wrap_WrapReverse, false, left, top, right, bottom);
        break;
    }
  }

  /**
   * Sub method for {@link WXCoreLayoutNode #onLayout(int, int, int, int)} when the
   * {@link #mFlexDirection} is either {@link WXCoreFlexDirection #WXCore_Flex_Direction_Row} or
   * {@link WXCoreFlexDirection #WXCore_Flex_Direction_Row_REVERSE}.
   *
   * @param isRtl  {@code true} if the horizontal layout direction is mStyleRight to mStyleLeft, {@code
   *               false} otherwise.
   * @param left   the mStyleLeft position of this View
   * @param top    the mStyleTop position of this View
   * @param right  the mStyleRight position of this View
   * @param bottom the mStyleBottom position of this View
   */
  void WXCoreLayoutNode::layoutHorizontal(bool isRtl, float left, float top, float right, float bottom) {

    float childLeft;
    uint32_t currentViewIndex = 0;

    float height = bottom - top;
    float width = right - left;

    // childBottom is used if the mFlexWrap is FLEX_WRAP_WRAP_REVERSE otherwise
    // childTop is used to align the vertical position of the children views.
    float childBottom = height - getPaddingBottom() - getBorderWidthBottom();
    float childTop = getPaddingTop() + getBorderWidthTop();

    // Used only for RTL layout
    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childRight;

    for (uint32_t i = 0, size = (uint32_t) mFlexLines.size(); i < size; i++) {
      WXCoreFlexLine *flexLine = mFlexLines[i];

      float spaceBetweenItem = 0.0f;

      switch (mCssStyle->mJustifyContent) {
        case WXCore_Justify_Flex_Start:
          childLeft = getPaddingLeft() + getBorderWidthLeft();
          childRight = width - getPaddingRight() - getBorderWidthRight();
          break;
        case WXCore_Justify_Flex_End:
          childLeft = width - flexLine->mMainSize + getPaddingRight() + getBorderWidthRight();
          childRight = flexLine->mMainSize - getPaddingLeft() - getBorderWidthLeft();
          break;
        case WXCore_Justify_Center:
          childLeft = getPaddingLeft() + getBorderWidthLeft() + (width - flexLine->mMainSize) / 2.0f;
          childRight = width - getPaddingRight() - getBorderWidthRight() - (width - flexLine->mMainSize) / 2.0f;
          break;
        case WXCore_Justify_Space_Around:
          uint32_t visibleCount;
          visibleCount = flexLine->mItemCount;
          if (visibleCount != 0) {
            spaceBetweenItem = (width - flexLine->mMainSize)
                               / (float) visibleCount;
          }
          childLeft = getPaddingLeft() + getBorderWidthLeft() + spaceBetweenItem / 2.0f;
          childRight = width - getPaddingRight() - getBorderWidthRight() - spaceBetweenItem / 2.0f;
          break;
        case WXCore_Justify_Space_Between:
          childLeft = getPaddingLeft() + getBorderWidthLeft();
          uint32_t visibleItem;
          visibleItem = flexLine->mItemCount;
          float denominator;
          denominator = visibleItem != 1 ? visibleItem - 1 : 1.0f;
          spaceBetweenItem = (width - flexLine->mMainSize) / denominator;
          childRight = width - getPaddingRight() - getBorderWidthRight();
          break;
        default:
          childLeft = getPaddingLeft() + getBorderWidthLeft();
          childRight = width - getPaddingRight() - getBorderWidthRight();
          break;
      }

      spaceBetweenItem = max_num<float>(spaceBetweenItem, 0);

      for (uint32_t j = 0; j < flexLine->mItemCount; j++) {

        WXCoreLayoutNode *child = getChildAt(NON_BFC, currentViewIndex);

        if (child == nullptr) {
          continue;
        }

        childLeft += child->getMarginLeft();
        childRight -= child->getMarginRight();

        if (mCssStyle->mFlexWrap == WXCore_Wrap_WrapReverse) {
          if (isRtl) {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle->mFlexWrap, mCssStyle->mAlignItems,
                                        childRight - child->mLayoutResult->mLayoutSize.width,
                                        childBottom - child->mLayoutResult->mLayoutSize.height, childRight,
                                        childBottom);
          } else {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle->mFlexWrap, mCssStyle->mAlignItems,
                                        childLeft, childBottom - child->mLayoutResult->mLayoutSize.height,
                                        childLeft + child->mLayoutResult->mLayoutSize.width,
                                        childBottom);
          }
        } else {
          if (isRtl) {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle->mFlexWrap, mCssStyle->mAlignItems,
                                        childRight - child->mLayoutResult->mLayoutSize.width, childTop,
                                        childRight, childTop + child->mLayoutResult->mLayoutSize.height);
          } else {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle->mFlexWrap, mCssStyle->mAlignItems,
                                        childLeft, childTop,
                                        childLeft + child->mLayoutResult->mLayoutSize.width,
                                        childTop + child->mLayoutResult->mLayoutSize.height);
          }
        }

        childLeft += child->mLayoutResult->mLayoutSize.width + spaceBetweenItem + child->getMarginRight();
        childRight -= child->mLayoutResult->mLayoutSize.width + spaceBetweenItem + child->getMarginLeft();
        currentViewIndex++;
      }

      childTop += flexLine->mCrossSize;
      childBottom -= flexLine->mCrossSize;
    }
  }

  /**
   * Place a single View when the layout direction is horizontal ({@link #mFlexDirection} is
   * either {@link WXCoreFlexDirection #WXCore_Flex_Direction_Row} or
   * {@link WXCoreFlexDirection #WXCore_Flex_Direction_Row_REVERSE}).
   *
   * @param node       the View to be placed
   * @param flexLine   the {@link WXCoreFlexLine} where the View belongs to
   * @param flexWrap   the flex wrap attribute of this BasicLayoutNode
   * @param alignItems the align items attribute of this BasicLayoutNode
   * @param left       the mStyleLeft position of the View, which the View's margin is already taken
   *                   into account
   * @param top        the mStyleTop position of the flex line where the View belongs to. The actual
   *                   View's mStyleTop position is shifted depending on the flexWrap and alignItems
   *                   attributes
   * @param right      the mStyleRight position of the View, which the View's margin is already taken
   *                   into account
   * @param bottom     the mStyleBottom position of the flex line where the View belongs to. The actual
   *                   View's mStyleBottom position is shifted depending on the flexWrap and alignItems
   *                   attributes
   */
  void WXCoreLayoutNode::layoutSingleChildHorizontal(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine,
                                                     WXCoreFlexWrap flexWrap, WXCoreAlignItems alignItems,
                                                     float left, float top, float right, float bottom) {
    if (node->mCssStyle->mAlignSelf != WXCore_AlignSelf_Auto) {
      // Expecting the values for alignItems and alignSelf match except for ALIGN_SELF_AUTO.
      // Assigning the alignSelf value as alignItems should work.
      alignItems = static_cast<WXCoreAlignItems>(node->mCssStyle->mAlignSelf);
    }

    float crossSize = flexLine->mCrossSize;

    switch (alignItems) {
      case WXCore_AlignItems_Flex_Start:
      case WXCore_AlignItems_Stretch:
        if (flexWrap != WXCore_Wrap_WrapReverse) {
          node->layout(left, top + node->getMarginTop(), right, bottom + node->getMarginTop());
        } else {
          node->layout(left, top - node->getMarginBottom(), right, bottom - node->getMarginBottom());
        }
        break;
      case WXCore_AlignItems_Flex_End:
        if (flexWrap != WXCore_Wrap_WrapReverse) {
          node->layout(left,
                       top + crossSize - node->mLayoutResult->mLayoutSize.height - node->getMarginBottom(),
                       right, top + crossSize - node->getMarginBottom());
        } else {
          // If the flexWrap == FLEX_WRAP_WRAP_REVERSE, the direction of the
          // flexEnd is flipped (from mStyleTop to mStyleBottom).
          node->layout(left, top - crossSize + node->mLayoutResult->mLayoutSize.height + node->getMarginTop(),
                       right, bottom - crossSize + node->mLayoutResult->mLayoutSize.height + node->getMarginTop());
        }
        break;
      case WXCore_AlignItems_Center:
        float topFromCrossAxis = (crossSize - node->mLayoutResult->mLayoutSize.height
                                  + node->getMarginTop() - node->getMarginBottom()) / 2;
        if (flexWrap != WXCore_Wrap_WrapReverse) {
          node->layout(left, top + topFromCrossAxis,
                       right, top + topFromCrossAxis + node->mLayoutResult->mLayoutSize.height);
        } else {
          node->layout(left, top - topFromCrossAxis,
                       right, top - topFromCrossAxis + node->mLayoutResult->mLayoutSize.height);
        }
        break;
    }
  }

  /**
   * Sub method for {@link WXCoreLayoutNode #onLayout(int, int, int, int)} when the
   * {@link #mFlexDirection} is either {@link WXCoreFlexDirection #WXCore_Flex_Direction_Column} or
   * {@link WXCoreFlexDirection #WXCore_Flex_Direction_Column_Reverse}.
   *
   * @param isRtl           {@code true} if the horizontal layout direction is mStyleRight to mStyleLeft,
   *                        {@code false}
   *                        otherwise
   * @param fromBottomToTop {@code true} if the layout direction is mStyleBottom to mStyleTop, {@code false}
   *                        otherwise
   * @param left            the mStyleLeft position of this View
   * @param top             the mStyleTop position of this View
   * @param right           the mStyleRight position of this View
   * @param bottom          the mStyleBottom position of this View
   */
  void
  WXCoreLayoutNode::layoutVertical(bool isRtl, bool fromBottomToTop, float left, float top, float right, float bottom) {
    float childLeft = getPaddingLeft();
    uint32_t currentViewIndex = 0;

    float width = right - left;
    float height = bottom - top;

    // childRight is used if the mFlexWrap is FLEX_WRAP_WRAP_REVERSE otherwise
    // childLeft is used to align the horizontal position of the children views.
    float childRight = width - getPaddingRight() - getBorderWidthRight();

    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childTop;

    // Used only for if the direction is from mStyleBottom to mStyleTop
    float childBottom;

    for (uint32_t i = 0, size = (uint32_t) mFlexLines.size(); i < size; i++) {
      WXCoreFlexLine *flexLine = mFlexLines[i];
      float spaceBetweenItem = 0.0f;

      switch (mCssStyle->mJustifyContent) {
        case WXCore_Justify_Flex_Start:
          childTop = getPaddingTop() + getBorderWidthTop();
          childBottom = height - getPaddingBottom() - getBorderWidthBottom();
          break;
        case WXCore_Justify_Flex_End:
          childTop = height - flexLine->mMainSize + getPaddingBottom() + getBorderWidthBottom();
          childBottom = flexLine->mMainSize - getPaddingTop() - getBorderWidthTop();
          break;
        case WXCore_Justify_Center:
          childTop = getPaddingTop() + getBorderWidthTop() + (height - flexLine->mMainSize) / 2;
          childBottom = height - getPaddingBottom() - getBorderWidthBottom() - (height - flexLine->mMainSize) / 2;
          break;
        case WXCore_Justify_Space_Around:
          uint32_t visibleCount;
          visibleCount = flexLine->mItemCount;
          if (visibleCount != 0) {
            spaceBetweenItem = (height - flexLine->mMainSize)
                               / (float) visibleCount;
          }
          childTop = getPaddingTop() + getBorderWidthTop() + spaceBetweenItem / 2;
          childBottom = height - getPaddingBottom() - getBorderWidthBottom() - spaceBetweenItem / 2;
          break;
        case WXCore_Justify_Space_Between:
          childTop = getPaddingTop() + getBorderWidthTop();
          uint32_t visibleItem;
          visibleItem = flexLine->mItemCount;
          float denominator;
          denominator = visibleItem != 1 ? visibleItem - 1 : 1.0f;
          spaceBetweenItem = (height - flexLine->mMainSize) / denominator;
          childBottom = height - getPaddingBottom() - getBorderWidthBottom();
          break;
        default:
          childTop = getPaddingTop() + getBorderWidthTop();
          childBottom = height - getPaddingBottom() - getBorderWidthBottom();
          break;
      }

      spaceBetweenItem = max_num<float>(spaceBetweenItem, 0);

      for (uint32_t j = 0; j < flexLine->mItemCount; j++) {
        WXCoreLayoutNode *child = getChildAt(NON_BFC, currentViewIndex);
        if (child == nullptr) {
          continue;
        }

        childTop += child->getMarginTop();
        childBottom -= child->getMarginBottom();

        if (isRtl) {
          if (fromBottomToTop) {
            layoutSingleChildVertical(child, flexLine, true, mCssStyle->mAlignItems,
                                      childRight - child->mLayoutResult->mLayoutSize.width,
                                      childBottom - child->mLayoutResult->mLayoutSize.height, childRight,
                                      childBottom);
          } else {
            layoutSingleChildVertical(child, flexLine, true, mCssStyle->mAlignItems,
                                      childRight - child->mLayoutResult->mLayoutSize.width, childTop,
                                      childRight, childTop + child->mLayoutResult->mLayoutSize.height);
          }
        } else {
          if (fromBottomToTop) {
            layoutSingleChildVertical(child, flexLine, false, mCssStyle->mAlignItems,
                                      childLeft, childBottom - child->mLayoutResult->mLayoutSize.height,
                                      childLeft + child->mLayoutResult->mLayoutSize.width, childBottom);
          } else {
            layoutSingleChildVertical(child, flexLine, false, mCssStyle->mAlignItems,
                                      childLeft, childTop,
                                      childLeft + child->mLayoutResult->mLayoutSize.width,
                                      childTop + child->mLayoutResult->mLayoutSize.height);
          }
        }

        childTop += child->mLayoutResult->mLayoutSize.height + spaceBetweenItem + child->getMarginBottom();
        childBottom -= child->mLayoutResult->mLayoutSize.height + spaceBetweenItem + child->getMarginTop();
        currentViewIndex++;
      }

      childLeft += flexLine->mCrossSize;
      childRight -= flexLine->mCrossSize;
    }
  }


  /**
   * Place a single View when the layout direction is vertical ({@link #mFlexDirection} is
   * either {@link WXCoreFlexDirection #WXCore_Flex_Direction_Column} or
   * {@link WXCoreFlexDirection #WXCore_Flex_Direction_Column_Reverse}).
   *
   * @param node       the View to be placed
   * @param flexLine   the {@link FlexLine} where the View belongs to
   * @param isRtl      {@code true} if the layout direction is mStyleRight to mStyleLeft, {@code false}
   *                   otherwise
   * @param alignItems the align items attribute of this BasicLayoutNode
   * @param left       the mStyleLeft position of the flex line where the View belongs to. The actual
   *                   View's mStyleLeft position is shifted depending on the isRtl and alignItems
   *                   attributes
   * @param top        the mStyleTop position of the View, which the View's margin is already taken
   *                   into account
   * @param right      the mStyleRight position of the flex line where the View belongs to. The actual
   *                   View's mStyleRight position is shifted depending on the isRtl and alignItems
   *                   attributes
   * @param bottom     the mStyleBottom position of the View, which the View's margin is already taken
   *                   into account
   */
  void WXCoreLayoutNode::layoutSingleChildVertical(WXCoreLayoutNode *node, WXCoreFlexLine *flexLine, bool isRtl,
                                                   WXCoreAlignItems alignItems, float left, float top, float right,
                                                   float bottom) {
    if (node->mCssStyle->mAlignSelf != WXCore_AlignSelf_Auto) {
      // Expecting the values for alignItems and alignSelf match except for ALIGN_SELF_AUTO.
      // Assigning the alignSelf value as alignItems should work.
      alignItems = static_cast<WXCoreAlignItems>(node->mCssStyle->mAlignSelf);
    }

    float crossSize = flexLine->mCrossSize;

    switch (alignItems) {
      case WXCore_AlignItems_Flex_Start:
      case WXCore_AlignItems_Stretch:
        if (!isRtl) {
          node->layout(left + node->getMarginLeft(), top, right + node->getMarginLeft(), bottom);
        } else {
          node->layout(left - node->getMarginRight(), top, right - node->getMarginRight(), bottom);
        }
        break;
      case WXCore_AlignItems_Flex_End:
        if (!isRtl) {
          node->layout(left + crossSize - node->mLayoutResult->mLayoutSize.width - node->getMarginRight(),
                       top, right + crossSize - node->mLayoutResult->mLayoutSize.width - node->getMarginRight(),
                       bottom);
        } else {
          // If the flexWrap == FLEX_WRAP_WRAP_REVERSE, the direction of the
          // flexEnd is flipped (from mStyleLeft to mStyleRight).
          node->layout(left - crossSize + node->mLayoutResult->mLayoutSize.width + node->getMarginLeft(), top,
                       right - crossSize + node->mLayoutResult->mLayoutSize.width + node->getMarginLeft(),
                       bottom);
        }
        break;
      case WXCore_AlignItems_Center:
        float leftFromCrossAxis = (crossSize - node->mLayoutResult->mLayoutSize.width
                                   + node->getMarginLeft()
                                   - node->getMarginRight()) / 2.0f;
        if (!isRtl) {
          node->layout(left + leftFromCrossAxis, top, right + leftFromCrossAxis, bottom);
        } else {
          node->layout(left - leftFromCrossAxis, top, right - leftFromCrossAxis, bottom);
        }
        break;
    }
  }

  WXCoreSize WXCoreLayoutNode::getChildDimension(float width, float height, WXCoreLayoutNode *child,
                                                 float totalCrossSize) {
    float availableWidth, availableHeight;
    switch (child->widthMeasureMode) {
      case AT_MOST:
        availableWidth =
            width - (mCssStyle->mPadding.getPadding(WXCore_Padding_Left) +
                     mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Left) +
                     child->mCssStyle->mMargin
                         .getMargin(WXCore_Margin_Left) +
                     mCssStyle->mPadding.getPadding(WXCore_Padding_Right) +
                     mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Right) +
                     child->mCssStyle->mMargin
                         .getMargin(WXCore_Margin_Right));
        if (!isMainAxisDirectionHorizontal(mCssStyle->mFlexDirection)) {
          availableWidth -= totalCrossSize;
        }
        break;
      default:
      case EXACTLY:
        availableWidth = child->mCssStyle->mStyleWidth;
        break;
    }

    switch (child->heightMeasureMode) {
      case AT_MOST:
        availableHeight =
            height - (mCssStyle->mPadding.getPadding(WXCore_Padding_Top) +
                      mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Top) +
                      child->mCssStyle->mMargin
                          .getMargin(WXCore_Margin_Top) +
                      mCssStyle->mPadding.getPadding(WXCore_Padding_Bottom) +
                      mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Bottom) +
                      child->mCssStyle->mMargin
                          .getMargin(WXCore_Margin_Bottom));
        if (isMainAxisDirectionHorizontal(mCssStyle->mFlexDirection)) {
          availableHeight -= totalCrossSize;
        }
        break;
      default:
      case EXACTLY:
        availableHeight = child->mCssStyle->mStyleHeight;
        break;
    }
    WXCoreSize dimension;
    dimension.width = availableWidth;
    dimension.height = availableHeight;
    return dimension;
  }

  float WXCoreLayoutNode::getLargestMainSize() {
    float largestSize = 0;
    for (WXCoreFlexLine *flexLine : mFlexLines) {
      largestSize = max_num<float>(largestSize, flexLine->mMainSize);
    }
    return largestSize;
  }

  void WXCoreLayoutNode::addFlexLineIfLastFlexItem(uint32_t childIndex, uint32_t childCount, WXCoreFlexLine *flexLine,
                                                   float usedCrossSizeSoFar) {
    if (childIndex == childCount - 1 && flexLine->mItemCount != 0) {
      // Add the flex line if this item is the last item
      addFlexLine(flexLine, usedCrossSizeSoFar);
    }
  }

  void WXCoreLayoutNode::addFlexLine(WXCoreFlexLine *flexLine, float usedCrossSizeSoFar) {
    flexLine->mSumCrossSizeBefore = usedCrossSizeSoFar;
    mFlexLines.push_back(flexLine);
  }

  /**
   * Determine if a wrap is required (add a new flexline).
   *
   * @param maxSize       the max size along the main axis direction
   * @param currentLength the accumulated current length
   * @param childLength   the length of a child view which is to be collected to the flex line
   * @return {@code true} if a wrap is required, {@code false} otherwise
   */
  bool WXCoreLayoutNode::isWrapRequired(float mainSize, float currentLength, float childLength) {
    return mCssStyle->mFlexWrap != WXCore_Wrap_NoWrap && mainSize < currentLength + childLength;
  }

  bool WXCoreLayoutNode::isMainAxisDirectionHorizontal(WXCoreFlexDirection flexDirection) {
    return flexDirection == WXCore_Flex_Direction_Row
           || flexDirection == WXCore_Flex_Direction_Row_Reverse;
  }
}


