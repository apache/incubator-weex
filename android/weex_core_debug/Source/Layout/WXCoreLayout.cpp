#include "WXCoreLayout.h"

using namespace WXCoreFlexLayout;

const float WXCoreCSSStyle::FLEX_DEFAULT = 1;
const float WXCoreCSSStyle::FLEX_GROW_DEFAULT = 0;
const float WXCoreCSSStyle::MAX_SIZE = NAN;
const WXCoreFlexDirection WXCoreCSSStyle::FLEX_DIRECTION_DEFAULT = WXCore_Flex_Direction_Row;
const WXCoreFlexWrap WXCoreCSSStyle::FLEX_WRAP_DEFAULT = WXCore_Wrap_NoWrap;
const WXCoreJustifyContent WXCoreCSSStyle::FLEX_JUSTIFY_CONTENT_DEFAULT = WXCore_Justify_Flex_Start;
const WXCoreAlignItems WXCoreCSSStyle::FLEX_ALIGN_ITEMS_DEFAULT = WXCore_AlignItems_Flex_Start;
const WXCoreAlignSelf WXCoreCSSStyle::FLEX_ALIGN_SELF_DEFAULT = WXCore_AlignSelf_Auto;

template<class T>
static T max_num(T a, T b) {
  return a > b ? a : b;
}

template<class T>
static T min_num(T a, T b) {
  return a < b ? a : b;
}

static float getChildMeasureSpec(float parentDimension, float padding,
                                 float childDimension) {
  if (isnan(parentDimension)) {
    return childDimension;
  } else {
    return isnan(childDimension) ? max_num<float>(0, parentDimension - padding) : childDimension;
  }
}

namespace WXCoreFlexLayout {

  void WXCoreLayoutNode::onMeasure(float width, float height) {
    if (mChildrenFrozen == nullptr || mChildrenFrozen_oldlength < getChildCount()) {
      mChildrenFrozen = new bool[getChildCount()];
      mChildrenFrozen_oldlength = getChildCount();
    }

    // Only calculate the children views which are affected from the last measure.
    switch (mCssStyle.mFlexDirection) {
      case WXCore_Flex_Direction_Row:
      case WXCore_Flex_Direction_Row_Reverse:
        measureHorizontal(width, height);
        break;
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
        measureVertical(width, height);
        break;
//    default:
//      throw new IllegalStateException(
//          "Invalid value for the flex direction is set: " + mFlexDirection);
    }

    memset(mChildrenFrozen, false, getChildCount());
  }

/**
 * Sub method for {@link WXCoreLayoutNode #onMeasure(int, int)}, when the main axis direction is horizontal
 *
 * @param width  horizontal space requirements as imposed by the parent
 * @param height vertical space requirements as imposed by the parent
 */
  void WXCoreLayoutNode::measureHorizontal(float width, float height) {

    mFlexLines.clear();

    int childCount = getChildCount();

    float largestHeightInRow = -1.0;
    float totalCrossSize = 0;
    WXCoreFlexLine *flexLine = new WXCoreFlexLine();

    flexLine->mMainSize = getPaddingLeft() + getPaddingRight();

    // Determine how many flex lines are needed in this layout by measuring each child.
    for (int i = 0; i < childCount; i++) {
      WXCoreLayoutNode *child = getChildAt(i);

      if (child == nullptr) {
        addFlexLineIfLastFlexItem(i, childCount, flexLine, totalCrossSize);
        continue;
      }

      // Record stretch child's index
      if (child->mCssStyle.mAlignSelf == WXCore_AlignSelf_Stretch) {
        flexLine->mIndicesAlignSelfStretch.push_back(i);
      }

      // Calculate the MeasureSpec of the child based on its margin and the width/height/padding of its parent
      float childWidthMeasureSpec = getChildMeasureSpec(width,
                                                        getPaddingLeft() + getPaddingRight() + child->getMarginLeft()
                                                        + child->getMarginRight(),
                                                        child->getStyleWidth());
      float childHeightMeasureSpec = getChildMeasureSpec(height,
                                                         getPaddingTop() + getPaddingBottom() + child->getMarginTop()
                                                         + child->getMarginBottom() + totalCrossSize,
                                                         child->getStyleHeight());

      // Measure child based on its MeasureSpec
      child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

      // Checks if the view's mStyleWidth/mStyleHeight don't violate the minimum/maximum size constraints
      checkSizeConstraints(child);

      largestHeightInRow = max_num<float>(largestHeightInRow,
                                          child->mLayoutResult.mComputedHeight + child->getMarginTop() +
                                          child->getMarginBottom());

      // Determine if a wrap is required (add a new flexline)
      if (isWrapRequired(width, flexLine->mMainSize,
                         child->mLayoutResult.mComputedWidth + child->getMarginLeft() + child->getMarginRight()
      )) {
        if (flexLine->getItemCountNotGone() > 0) {
          addFlexLine(flexLine, totalCrossSize);
          totalCrossSize += flexLine->mCrossSize;
        }

        flexLine = new WXCoreFlexLine();
        flexLine->mItemCount = 1;
        flexLine->mMainSize = getPaddingLeft() + getPaddingRight();
        largestHeightInRow = child->mLayoutResult.mComputedHeight + child->getMarginTop() + child->getMarginBottom();
      } else {
        flexLine->mItemCount++;
      }

      flexLine->mMainSize += child->mLayoutResult.mComputedWidth + child->getMarginLeft() + child->getMarginRight();
      flexLine->mTotalFlexGrow += child->mCssStyle.mFlexGrowValue;

      // Temporarily set the cross axis length as the largest child in the row
      // Expand along the cross axis depending on the mAlignContent property if needed later
      flexLine->mCrossSize = max_num<float>(flexLine->mCrossSize, largestHeightInRow);

      flexLine->mMaxBaseline = max_num<float>(flexLine->mMaxBaseline, child->getBaseline() + child->getMarginTop());

      addFlexLineIfLastFlexItem(i, childCount, flexLine, totalCrossSize);
    }

    // Determine the main size by expanding the individual flexGrow attribute.
    determineMainSize(mCssStyle.mFlexDirection, width, height);

    // Determines the cross size (Calculate the length along the cross axis).
    determineCrossSize(mCssStyle.mFlexDirection, width, height, getPaddingTop() + getPaddingBottom());

    // Now cross size for each flex line is determined.

    // Expand the views if alignItems (or alignSelf in each child view) is set to stretch.
    stretchViews(mCssStyle.mFlexDirection, mCssStyle.mAlignItems);

    // Set measure result.
    setMeasuredDimension(width, height);
  }

/**
 * Sub method for {@link WXCoreLayoutNode #onMeasure(int, int)} when the main axis direction is vertical
 * (either from mStyleTop to mStyleBottom or mStyleBottom to mStyleTop).
 *
 * @param width  horizontal space requirements as imposed by the parent
 * @param height vertical space requirements as imposed by the parent
 */
  void WXCoreLayoutNode::measureVertical(float width, float height) {

    mFlexLines.clear();

    int childCount = getChildCount();
    float largestWidthInColumn = -1.0;
    float totalCrossSize = 0;

    WXCoreFlexLine *flexLine = new WXCoreFlexLine();
    flexLine->mMainSize = getPaddingTop() + getPaddingBottom();

    // Determine how many flex lines are needed in this layout by measuring each child.
    for (int i = 0; i < childCount; i++) {
      WXCoreLayoutNode *child = getChildAt(i);
      if (child == nullptr) {
        addFlexLineIfLastFlexItem(i, childCount, flexLine, totalCrossSize);
        continue;
      }

      // Record stretch child's index
      if (child->mCssStyle.mAlignSelf == WXCore_AlignSelf_Stretch) {
        flexLine->mIndicesAlignSelfStretch.push_back(i);
      }

      // Calculate the MeasureSpec of the child based on its margin and the width/height/padding of its parent
      float childWidthMeasureSpec = getChildMeasureSpec(width,
                                                        getPaddingLeft() + getPaddingRight() + child->getMarginLeft()
                                                        + child->getMarginRight() + totalCrossSize,
                                                        child->getStyleWidth());
      float childHeightMeasureSpec = getChildMeasureSpec(height,
                                                         getPaddingTop() + getPaddingBottom() + child->getMarginTop()
                                                         + child->getMarginBottom(), child->getStyleHeight());

      // Measure child based on its MeasureSpec
      child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

      // Checks if the view's mStyleWidth/mStyleHeight don't violate the minimum/maximum size constraints
      checkSizeConstraints(child);

      largestWidthInColumn = max_num<float>(largestWidthInColumn,
                                            child->mLayoutResult.mComputedWidth + child->getMarginLeft() +
                                            child->getMarginRight());

      // Determine if a wrap is required (add a new flexline)
      if (isWrapRequired(height, flexLine->mMainSize,
                         child->mLayoutResult.mComputedHeight + child->getMarginTop() + child->getMarginBottom()
      )) {
        if (flexLine->getItemCountNotGone() > 0) {
          addFlexLine(flexLine, totalCrossSize);
          totalCrossSize += flexLine->mCrossSize;
        }

        flexLine = new WXCoreFlexLine();
        flexLine->mItemCount = 1;
        flexLine->mMainSize = getPaddingTop() + getPaddingBottom();
        largestWidthInColumn = child->mLayoutResult.mComputedWidth + child->getMarginLeft() + child->getMarginRight();
      } else {
        flexLine->mItemCount++;
      }

      flexLine->mMainSize += child->mLayoutResult.mComputedHeight + child->getMarginTop() + child->getMarginBottom();
      flexLine->mTotalFlexGrow += child->mCssStyle.mFlexGrowValue;

      // Temporarily set the cross axis length as the largest child in the row
      // Expand along the cross axis depending on the mAlignContent property if needed later
      flexLine->mCrossSize = max_num<float>(flexLine->mCrossSize, largestWidthInColumn);
      addFlexLineIfLastFlexItem(i, childCount, flexLine, totalCrossSize);
    }

    // Determine the main size by expanding the individual flexGrow attribute.
    determineMainSize(mCssStyle.mFlexDirection, width, height);

    // Determines the cross size (Calculate the length along the cross axis).
    determineCrossSize(mCssStyle.mFlexDirection, width, height, getPaddingLeft() + getPaddingRight());

    // Now cross size for each flex line is determined.

    // Expand the views if alignItems (or alignSelf in each child view) is set to stretch.
    stretchViews(mCssStyle.mFlexDirection, mCssStyle.mAlignItems);

    // Set measure result.
    setMeasuredDimension(width, height);
  }

/**
 * Checks if the view's mStyleWidth/mStyleHeight don't violate the minimum/maximum size constraints
 * imposed by the {@link WXCoreLayoutNode #mMinWidth}, {@link WXCoreLayoutNode #mMinHeight},
 * {@link WXCoreLayoutNode #mMaxWidth} and {@link WXCoreLayoutNode #mMaxHeight} attributes.
 */
  void WXCoreLayoutNode::checkSizeConstraints(WXCoreLayoutNode *node) {
    bool needsMeasure = false;
    float childWidth = node->mLayoutResult.mComputedWidth;
    float childHeight = node->mLayoutResult.mComputedHeight;

    if (node->mLayoutResult.mComputedWidth < node->mCssStyle.mMinWidth) {
      needsMeasure = true;
      childWidth = node->mCssStyle.mMinWidth;
    } else if (node->mLayoutResult.mComputedWidth > node->mCssStyle.mMaxWidth) {
      needsMeasure = true;
      childWidth = node->mCssStyle.mMaxWidth;
    }

    if (childHeight < node->mCssStyle.mMinHeight) {
      needsMeasure = true;
      childHeight = node->mCssStyle.mMinHeight;
    } else if (childHeight > node->mCssStyle.mMaxHeight) {
      needsMeasure = true;
      childHeight = node->mCssStyle.mMaxHeight;
    }

    if (needsMeasure) {
      node->measure(childWidth, childHeight);
    }
  }

  void WXCoreLayoutNode::addFlexLineIfLastFlexItem(int childIndex, int childCount, WXCoreFlexLine *flexLine,
                                                   float usedCrossSizeSoFar) {
    if (childIndex == childCount - 1 && flexLine->getItemCountNotGone() != 0) {
      // Add the flex line if this item is the last item
      addFlexLine(flexLine, usedCrossSizeSoFar);
    }
  }

  void WXCoreLayoutNode::addFlexLine(WXCoreFlexLine *flexLine, float usedCrossSizeSoFar) {
    flexLine->mSumCrossSizeBefore = usedCrossSizeSoFar;
    // The size of the end divider isn't added until the flexLine is added to the flex container
    // take the divider mStyleWidth (or mStyleHeight) into account when adding the flex line.
    mFlexLines.push_back(flexLine);
  }

/**
 * Determine the main size by expanding the individual flexGrow attribute.
 *
 * @param flexDirection     the value of the flex direction
 * @param widthMeasureSpec  horizontal space requirements as imposed by the parent
 * @param heightMeasureSpec vertical space requirements as imposed by the parent
 */
  void WXCoreLayoutNode::determineMainSize(WXCoreFlexDirection flexDirection, float widthMeasureSpec,
                                           float heightMeasureSpec) {
    float mainSize;
    float paddingAlongMainAxis;

    switch (flexDirection) {
      case WXCore_Flex_Direction_Row:
      case WXCore_Flex_Direction_Row_Reverse:
        if (isnan(widthMeasureSpec)) {
          mainSize = getLargestMainSize();
        } else {
          mainSize = widthMeasureSpec;
        }
        paddingAlongMainAxis = getPaddingLeft() + getPaddingRight();
        break;
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
        if (isnan(heightMeasureSpec)) {
          mainSize = getLargestMainSize();
        } else {
          mainSize = heightMeasureSpec;
        }
        paddingAlongMainAxis = getPaddingTop() + getPaddingBottom();
        break;
//    default:
//      throw new IllegalArgumentException("Invalid flex direction: " + flexDirection);
    }

    int childIndex = 0;
    for (WXCoreFlexLine *flexLine : mFlexLines) {
      if (flexLine->mMainSize < mainSize) {
        childIndex = expandFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine,
                                     flexDirection, mainSize, paddingAlongMainAxis, childIndex, false);
      }
    }
  }

/**
 * Expand the flex items along the main axis based on the individual flexGrow attribute.
 *
 * @param widthMeasureSpec     the horizontal space requirements as imposed by the parent
 * @param heightMeasureSpec    the vertical space requirements as imposed by the parent
 * @param flexLine             the flex line to which flex items belong
 * @param flexDirection        the flexDirection value for this WXCoreLayoutNode
 * @param maxMainSize          the maximum main size. Expanded main size will be this size
 * @param paddingAlongMainAxis the padding value along the main axis
 * @param startIndex           the start index of the children views to be expanded. This index
 *                             needs to
 *                             be an absolute index in the flex container (WXCoreLayoutNode),
 *                             not the relative index in the flex line.
 * @param calledRecursively    true if this method is called recursively, false otherwise
 * @return the next index, the next flex line's first flex item starts from the returned index
 */
  int WXCoreLayoutNode::expandFlexItems(float widthMeasureSpec, float heightMeasureSpec, WXCoreFlexLine *flexLine,
                                        WXCoreFlexDirection flexDirection, float maxMainSize,
                                        float paddingAlongMainAxis,
                                        int startIndex, bool calledRecursively) {
    int childIndex = startIndex;
    if (flexLine->mTotalFlexGrow <= 0 || maxMainSize < flexLine->mMainSize) {
      childIndex += flexLine->mItemCount;
      return childIndex;
    }

    float sizeBeforeExpand = flexLine->mMainSize;
    bool needsReexpand = false;
    float unitSpace = (maxMainSize - flexLine->mMainSize) / flexLine->mTotalFlexGrow;
    flexLine->mMainSize = paddingAlongMainAxis;

    // Setting the cross size of the flex line as the temporal value since the cross size of
    // each flex item may be changed from the initial calculation
    // (in the measureHorizontal/measureVertical method) even this method is part of the main
    // size determination.
    // E.g. If a TextView's layout_width is set to 0dp, layout_height is set to wrap_content,
    // and layout_flexGrow is set to 1, the TextView is trying to expand to the vertical
    // direction to enclose its content (in the measureHorizontal method), but
    // the mStyleWidth will be expanded in this method. In that case, the mStyleHeight needs to be measured
    // again with the expanded mStyleWidth.
    if (!calledRecursively) {
      flexLine->mCrossSize = -1.0;
    }

    float largestCrossSize = 0;
    float accumulatedRoundError = 0;

    for (int i = 0; i < flexLine->mItemCount; i++) {
      WXCoreLayoutNode *child = getChildAt(childIndex);
      if (child == nullptr) {
        continue;
      }

      if (isMainAxisDirectionHorizontal(flexDirection)) {
        // The direction of the main axis is horizontal
        if (!mChildrenFrozen[childIndex]) {
          float rawCalculatedWidth = child->mLayoutResult.mComputedWidth + unitSpace * child->mCssStyle.mFlexGrowValue;
          if (i == flexLine->mItemCount - 1) {
            rawCalculatedWidth += accumulatedRoundError;
            accumulatedRoundError = 0;
          }

          float newWidth = rawCalculatedWidth;
          if (newWidth > child->mCssStyle.mMaxWidth) {
            // This means the child can't expand beyond the value of the maxWidth
            // attribute.
            // To adjust the flex line length to the size of maxMainSize, remaining
            // positive free space needs to be re-distributed to other flex items
            // (children views). In that case, invoke this method again with the same
            // startIndex.
            needsReexpand = true;
            newWidth = child->mCssStyle.mMaxWidth;
            mChildrenFrozen[childIndex] = true;
            flexLine->mTotalFlexGrow -= child->mCssStyle.mFlexGrowValue;
          } else {
            accumulatedRoundError += (rawCalculatedWidth - newWidth);
            if (accumulatedRoundError > 1.0) {
              newWidth += 1;
              accumulatedRoundError -= 1.0;
            } else if (accumulatedRoundError < -1.0) {
              newWidth -= 1;
              accumulatedRoundError += 1.0;
            }
          }

          float childHeightMeasureSpec = getChildHeightMeasureSpec(heightMeasureSpec, child,
                                                                   flexLine->mSumCrossSizeBefore);
          child->measure(newWidth, childHeightMeasureSpec);
          largestCrossSize = max_num<float>(largestCrossSize, child->mLayoutResult.mComputedHeight
                                                              + child->getMarginTop() + child->getMarginBottom());
        }

        flexLine->mMainSize += child->mLayoutResult.mComputedWidth + child->getMarginLeft() + child->getMarginRight();
      } else {
        // The direction of the main axis is vertical
        if (!mChildrenFrozen[childIndex]) {
          float rawCalculatedHeight =
              child->mLayoutResult.mComputedHeight + unitSpace * child->mCssStyle.mFlexGrowValue;

          if (i == flexLine->mItemCount - 1) {
            rawCalculatedHeight += accumulatedRoundError;
            accumulatedRoundError = 0;
          }

          float newHeight = rawCalculatedHeight;

          if (newHeight > child->mCssStyle.mMaxHeight) {
            // This means the child can't expand beyond the value of the maxHeight
            // attribute.
            // To adjust the flex line length to the size of maxMainSize, remaining
            // positive free space needs to be re-distributed to other flex items
            // (children views). In that case, invoke this method again with the same
            // startIndex.
            needsReexpand = true;
            newHeight = child->mCssStyle.mMaxHeight;
            mChildrenFrozen[childIndex] = true;
            flexLine->mTotalFlexGrow -= child->mCssStyle.mFlexGrowValue;
          } else {
            accumulatedRoundError += (rawCalculatedHeight - newHeight);
            if (accumulatedRoundError > 1.0) {
              newHeight += 1;
              accumulatedRoundError -= 1.0;
            } else if (accumulatedRoundError < -1.0) {
              newHeight -= 1;
              accumulatedRoundError += 1.0;
            }
          }

          float childWidthMeasureSpec = getChildWidthMeasureSpec(widthMeasureSpec, child,
                                                                 flexLine->mSumCrossSizeBefore);
          child->measure(childWidthMeasureSpec, newHeight);
          largestCrossSize = max_num<float>(largestCrossSize, child->mLayoutResult.mComputedWidth
                                                              + child->getMarginLeft() + child->getMarginRight());
        }
        flexLine->mMainSize += child->mLayoutResult.mComputedHeight + child->getMarginTop() + child->getMarginBottom();
      }

      flexLine->mCrossSize = max_num<float>(flexLine->mCrossSize, largestCrossSize);
      childIndex++;
    }

    if (needsReexpand && sizeBeforeExpand != flexLine->mMainSize) {
      // Re-invoke the method with the same startIndex to distribute the positive free space
      // that wasn't fully distributed (because of maximum length constraint)
      expandFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine, flexDirection,
                      maxMainSize, paddingAlongMainAxis, startIndex, true);
    }

    return childIndex;
  }

  float WXCoreLayoutNode::getChildWidthMeasureSpec(float widthMeasureSpec, WXCoreLayoutNode *node, float padding) {
    float childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec,
                                                      getPaddingLeft() + getPaddingRight() + node->getMarginLeft()
                                                      + node->getMarginRight() + padding, node->getStyleWidth());
    float childWidth = childWidthMeasureSpec;
    if (childWidth > node->mCssStyle.mMaxWidth) {
      childWidthMeasureSpec = node->mCssStyle.mMaxWidth;
    } else if (childWidth < node->mCssStyle.mMinWidth) {
      childWidthMeasureSpec = node->mCssStyle.mMinWidth;
    }

    return childWidthMeasureSpec;
  }

  float WXCoreLayoutNode::getChildHeightMeasureSpec(float heightMeasureSpec, WXCoreLayoutNode *node, float padding) {
    float childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                                                       getPaddingTop() + getPaddingBottom() + node->getMarginTop()
                                                       + node->getMarginBottom() + padding, node->getStyleHeight());
    float childHeight = childHeightMeasureSpec;

    if (childHeight > node->mCssStyle.mMaxHeight) {
      childHeightMeasureSpec = node->mCssStyle.mMaxHeight;
    } else if (childHeight < node->mCssStyle.mMinHeight) {
      childHeightMeasureSpec = node->mCssStyle.mMinHeight;
    }

    return childHeightMeasureSpec;
  }

/**
 * Determines the cross size (Calculate the length along the cross axis).
 * Expand the cross size only if the mStyleHeight mode is MeasureSpec.EXACTLY, otherwise
 * use the sum of cross sizes of all flex lines.
 *
 * @param flexDirection         the flex direction attribute
 * @param widthMeasureSpec      horizontal space requirements as imposed by the parent
 * @param heightMeasureSpec     vertical space requirements as imposed by the parent
 * @param paddingAlongCrossAxis the padding value for the WXCoreLayoutNode along the cross axis
 */
  void WXCoreLayoutNode::determineCrossSize(WXCoreFlexDirection flexDirection, float widthMeasureSpec,
                                            float heightMeasureSpec, float paddingAlongCrossAxis) {
    // The MeasureSpec mode along the cross axis
    float size;
    switch (flexDirection) {
      case WXCore_Flex_Direction_Row:
      case WXCore_Flex_Direction_Row_Reverse:
        size = heightMeasureSpec;
        break;
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
        size = widthMeasureSpec;
        break;
//    default:
//      throw new IllegalArgumentException("Invalid flex direction: " + flexDirection);
    }

    float totalCrossSize = getSumOfCrossSize() + paddingAlongCrossAxis;

    if (mFlexLines.size() == 1) {
      mFlexLines[0]->mCrossSize = size - paddingAlongCrossAxis;
      // alignContent property is valid only if the Flexbox has at least two lines
    } else if (mFlexLines.size() >= 2) {
      if (totalCrossSize < size) {
        float freeSpaceUnit = (size - totalCrossSize) / (float) mFlexLines.size();
        float accumulatedError = 0;

        for (int i = 0, flexLinesSize = mFlexLines.size(); i < flexLinesSize; i++) {
          WXCoreFlexLine *flexLine = mFlexLines[i];
          float newCrossSizeAsFloat = flexLine->mCrossSize + freeSpaceUnit;

          if (i == mFlexLines.size() - 1) {
            newCrossSizeAsFloat += accumulatedError;
            accumulatedError = 0;
          }

          float newCrossSize = newCrossSizeAsFloat;
          accumulatedError += (newCrossSizeAsFloat - newCrossSize);

          if (accumulatedError > 1) {
            newCrossSize += 1;
            accumulatedError -= 1;
          } else if (accumulatedError < -1) {
            newCrossSize -= 1;
            accumulatedError += 1;
          }

          flexLine->mCrossSize = newCrossSize;
        }
      }
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
      int viewIndex = 0;
      for (WXCoreFlexLine *flexLine : mFlexLines) {
        for (int i = 0; i < flexLine->mItemCount; i++, viewIndex++) {
          WXCoreLayoutNode *child = getChildAt(viewIndex);

          if (child->mCssStyle.mAlignSelf != WXCore_AlignSelf_Auto &&
              child->mCssStyle.mAlignSelf != WXCore_AlignSelf_Stretch) {
            continue;
          }

          switch (flexDirection) {
            case WXCore_Flex_Direction_Row:
            case WXCore_Flex_Direction_Row_Reverse:
              stretchViewVertically(child, flexLine->mCrossSize);
              break;
            case WXCore_Flex_Direction_Column:
            case WXCore_Flex_Direction_Column_Reverse:
              stretchViewHorizontally(child, flexLine->mCrossSize);
              break;
//          default:
//            throw new IllegalArgumentException(
//                "Invalid flex direction: " + flexDirection);
          }
        }
      }
    } else {
      for (WXCoreFlexLine *flexLine : mFlexLines) {
        for (int index : flexLine->mIndicesAlignSelfStretch) {
          WXCoreLayoutNode *child = getChildAt(index);

          switch (flexDirection) {
            case WXCore_Flex_Direction_Row:
            case WXCore_Flex_Direction_Row_Reverse:
              stretchViewVertically(child, flexLine->mCrossSize);
              break;
            case WXCore_Flex_Direction_Column:
            case WXCore_Flex_Direction_Column_Reverse:
              stretchViewHorizontally(child, flexLine->mCrossSize);
              break;
//          default:
//            throw new IllegalArgumentException(
//                "Invalid flex direction: " + flexDirection);
          }
        }
      }
    }
  }

/**
 * Expand the view vertically to the size of the crossSize (considering the view margins)
 *
 * @param node      the View to be stretched
 * @param crossSize the cross size
 */
  void WXCoreLayoutNode::stretchViewVertically(WXCoreLayoutNode *node, float crossSize) {
    float newHeight = crossSize - node->getMarginTop() - node->getMarginBottom();
    newHeight = max_num<float>(newHeight, 0);
    node->measure(node->mLayoutResult.mComputedWidth, newHeight);
  }

/**
 * Expand the view horizontally to the size of the crossSize (considering the view margins)
 *
 * @param node      the View to be stretched
 * @param crossSize the cross size
 */
  void WXCoreLayoutNode::stretchViewHorizontally(WXCoreLayoutNode *node, float crossSize) {
    float newWidth = crossSize - node->getMarginLeft() - node->getMarginRight();
    newWidth = max_num<float>(newWidth, 0);
    node->measure(newWidth, node->mLayoutResult.mComputedHeight);
  }

/**
 * Determine if a wrap is required (add a new flexline).
 *
 * @param maxSize       the max size along the main axis direction
 * @param currentLength the accumulated current length
 * @param childLength   the length of a child view which is to be collected to the flex line
 * @return {@code true} if a wrap is required, {@code false} otherwise
 */
  bool WXCoreLayoutNode::isWrapRequired(float maxSize, float currentLength, float childLength) {
    if (mCssStyle.mFlexWrap == WXCore_Wrap_NoWrap) {
      return false;
    }
    return maxSize < currentLength + childLength;
  }

/**
 * Retrieve the sum of the cross sizes of all flex lines including divider lengths.
 *
 * @return the sum of the cross sizes
 */
  float WXCoreLayoutNode::getSumOfCrossSize() {
    float sum = 0;
    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
      WXCoreFlexLine *flexLine = mFlexLines[i];
      sum += flexLine->mCrossSize;
    }
    return sum;
  }

  bool WXCoreLayoutNode::isMainAxisDirectionHorizontal(WXCoreFlexDirection flexDirection) {
    return flexDirection == WXCore_Flex_Direction_Row
           || flexDirection == WXCore_Flex_Direction_Row_Reverse;
  }

  void WXCoreLayoutNode::onLayout(float left, float top, float right, float bottom) {
    switch (mCssStyle.mFlexDirection) {
      case WXCore_Flex_Direction_Row:
        layoutHorizontal(false, left, top, right, bottom);
        break;
      case WXCore_Flex_Direction_Row_Reverse:
        layoutHorizontal(true, left, top, right, bottom);
        break;
      case WXCore_Flex_Direction_Column:
        layoutVertical(mCssStyle.mFlexWrap == WXCore_Wrap_WrapReverse, false, left, top, right, bottom);
        break;
      case WXCore_Flex_Direction_Column_Reverse:
        layoutVertical(mCssStyle.mFlexWrap == WXCore_Wrap_WrapReverse, true, left, top, right, bottom);
        break;
//    default:
//      *(int *) (uintptr_t) 0xbbadbeef = 0;
//      throw new IllegalStateException("Invalid flex direction is set: " + mFlexDirection);
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
    int currentViewIndex = 0;

    float height = bottom - top;
    float width = right - left;

    // childBottom is used if the mFlexWrap is FLEX_WRAP_WRAP_REVERSE otherwise
    // childTop is used to align the vertical position of the children views.
    float childBottom = height - getPaddingBottom();
    float childTop = getPaddingTop();

    // Used only for RTL layout
    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childRight;

    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
      WXCoreFlexLine *flexLine = mFlexLines[i];

      float spaceBetweenItem = 0;

      switch (mCssStyle.mJustifyContent) {
        case WXCore_Justify_Flex_Start:
          childLeft = getPaddingLeft();
          childRight = width - getPaddingRight();
          break;
        case WXCore_Justify_Flex_End:
          childLeft = width - flexLine->mMainSize + getPaddingRight();
          childRight = flexLine->mMainSize - getPaddingLeft();
          break;
        case WXCore_Justify_Center:
          childLeft = getPaddingLeft() + (width - flexLine->mMainSize) / 2;
          childRight = width - getPaddingRight() - (width - flexLine->mMainSize) / 2;
          break;
        case WXCore_Justify_Space_Around:
          int visibleCount;
          visibleCount = flexLine->getItemCountNotGone();
          if (visibleCount != 0) {
            spaceBetweenItem = (width - flexLine->mMainSize)
                               / (float) visibleCount;
          }
          childLeft = getPaddingLeft() + spaceBetweenItem / 2;
          childRight = width - getPaddingRight() - spaceBetweenItem / 2;
          break;
        case WXCore_Justify_Space_Between:
          childLeft = getPaddingLeft();
          int visibleItem = flexLine->getItemCountNotGone();
          float denominator = visibleItem != 1 ? visibleItem - 1 : 1;
          spaceBetweenItem = (width - flexLine->mMainSize) / denominator;
          childRight = width - getPaddingRight();
          break;
//      default:
//        throw new IllegalStateException(
//            "Invalid justifyContent is set: " + mJustifyContent);
      }

      spaceBetweenItem = max_num<float>(spaceBetweenItem, 0);

      for (int j = 0; j < flexLine->mItemCount; j++) {

        WXCoreLayoutNode *child = getChildAt(currentViewIndex);

        if (child == nullptr) {
          continue;
        }

        childLeft += child->getMarginLeft();
        childRight -= child->getMarginRight();

        if (mCssStyle.mFlexWrap == WXCore_Wrap_WrapReverse) {
          if (isRtl) {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle.mFlexWrap, mCssStyle.mAlignItems,
                                        childRight - child->mLayoutResult.mComputedWidth,
                                        childBottom - child->mLayoutResult.mComputedHeight, childRight,
                                        childBottom);
          } else {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle.mFlexWrap, mCssStyle.mAlignItems,
                                        childLeft, childBottom - child->mLayoutResult.mComputedHeight,
                                        childLeft + child->mLayoutResult.mComputedWidth,
                                        childBottom);
          }
        } else {
          if (isRtl) {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle.mFlexWrap, mCssStyle.mAlignItems,
                                        childRight - child->mLayoutResult.mComputedWidth, childTop,
                                        childRight, childTop + child->mLayoutResult.mComputedHeight);
          } else {
            layoutSingleChildHorizontal(child, flexLine, mCssStyle.mFlexWrap, mCssStyle.mAlignItems,
                                        childLeft, childTop,
                                        childLeft + child->mLayoutResult.mComputedWidth,
                                        childTop + child->mLayoutResult.mComputedHeight);
          }
        }

        childLeft += child->mLayoutResult.mComputedWidth + spaceBetweenItem + child->getMarginRight();
        childRight -= child->mLayoutResult.mComputedWidth + spaceBetweenItem + child->getMarginLeft();
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
    if (node->mCssStyle.mAlignSelf != WXCore_AlignSelf_Auto) {
      // Expecting the values for alignItems and alignSelf match except for ALIGN_SELF_AUTO.
      // Assigning the alignSelf value as alignItems should work.
      alignItems = static_cast<WXCoreAlignItems>(node->mCssStyle.mAlignSelf);
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
      case WXCore_AlignItems_Baseline:
        if (flexWrap != WXCore_Wrap_WrapReverse) {
          float marginTop = flexLine->mMaxBaseline - node->getBaseline();
          marginTop = max_num<float>(marginTop, node->getMarginTop());
          node->layout(left, top + marginTop, right, bottom + marginTop);
        } else {
          float marginBottom = flexLine->mMaxBaseline - node->mLayoutResult.mComputedHeight + node->getBaseline();
          marginBottom = max_num<float>(marginBottom, node->getMarginBottom());
          node->layout(left, top - marginBottom, right, bottom - marginBottom);
        }
        break;
      case WXCore_AlignItems_Flex_End:
        if (flexWrap != WXCore_Wrap_WrapReverse) {
          node->layout(left,
                       top + crossSize - node->mLayoutResult.mComputedHeight - node->getMarginBottom(),
                       right, top + crossSize - node->getMarginBottom());
        } else {
          // If the flexWrap == FLEX_WRAP_WRAP_REVERSE, the direction of the
          // flexEnd is flipped (from mStyleTop to mStyleBottom).
          node->layout(left, top - crossSize + node->mLayoutResult.mComputedHeight + node->getMarginTop(),
                       right, bottom - crossSize + node->mLayoutResult.mComputedHeight + node->getMarginTop());
        }
        break;
      case WXCore_AlignItems_Center:
        float topFromCrossAxis = (crossSize - node->mLayoutResult.mComputedHeight
                                  + node->getMarginTop() - node->getMarginBottom()) / 2;
        if (flexWrap != WXCore_Wrap_WrapReverse) {
          node->layout(left, top + topFromCrossAxis,
                       right, top + topFromCrossAxis + node->mLayoutResult.mComputedHeight);
        } else {
          node->layout(left, top - topFromCrossAxis,
                       right, top - topFromCrossAxis + node->mLayoutResult.mComputedHeight);
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
    int currentViewIndex = 0;

    float width = right - left;
    float height = bottom - top;

    // childRight is used if the mFlexWrap is FLEX_WRAP_WRAP_REVERSE otherwise
    // childLeft is used to align the horizontal position of the children views.
    float childRight = width - getPaddingRight();

    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childTop;

    // Used only for if the direction is from mStyleBottom to mStyleTop
    float childBottom;

    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
      WXCoreFlexLine *flexLine = mFlexLines[i];
      float spaceBetweenItem = 0;

      switch (mCssStyle.mJustifyContent) {
        case WXCore_Justify_Flex_Start:
          childTop = getPaddingTop();
          childBottom = height - getPaddingBottom();
          break;
        case WXCore_Justify_Flex_End:
          childTop = height - flexLine->mMainSize + getPaddingBottom();
          childBottom = flexLine->mMainSize - getPaddingTop();
          break;
        case WXCore_Justify_Center:
          childTop = getPaddingTop() + (height - flexLine->mMainSize) / 2;
          childBottom = height - getPaddingBottom() - (height - flexLine->mMainSize) / 2;
          break;
        case WXCore_Justify_Space_Around:
          int visibleCount;
          visibleCount = flexLine->getItemCountNotGone();
          if (visibleCount != 0) {
            spaceBetweenItem = (height - flexLine->mMainSize)
                               / (float) visibleCount;
          }
          childTop = getPaddingTop() + spaceBetweenItem / 2;
          childBottom = height - getPaddingBottom() - spaceBetweenItem / 2;
          break;
        case WXCore_Justify_Space_Between:
          childTop = getPaddingTop();
          int visibleItem = flexLine->getItemCountNotGone();
          float denominator = visibleItem != 1 ? visibleItem - 1 : 1;
          spaceBetweenItem = (height - flexLine->mMainSize) / denominator;
          childBottom = height - getPaddingBottom();
          break;
//      default:
//        throw new IllegalStateException(
//            "Invalid justifyContent is set: " + mJustifyContent);
      }

      spaceBetweenItem = max_num<float>(spaceBetweenItem, 0);

      for (int j = 0; j < flexLine->mItemCount; j++) {
        WXCoreLayoutNode *child = getChildAt(currentViewIndex);
        if (child == nullptr) {
          continue;
        }

        childTop += child->getMarginTop();
        childBottom -= child->getMarginBottom();

        if (isRtl) {
          if (fromBottomToTop) {
            layoutSingleChildVertical(child, flexLine, true, mCssStyle.mAlignItems,
                                      childRight - child->mLayoutResult.mComputedWidth,
                                      childBottom - child->mLayoutResult.mComputedHeight, childRight,
                                      childBottom);
          } else {
            layoutSingleChildVertical(child, flexLine, true, mCssStyle.mAlignItems,
                                      childRight - child->mLayoutResult.mComputedWidth, childTop,
                                      childRight, childTop + child->mLayoutResult.mComputedHeight);
          }
        } else {
          if (fromBottomToTop) {
            layoutSingleChildVertical(child, flexLine, false, mCssStyle.mAlignItems,
                                      childLeft, childBottom - child->mLayoutResult.mComputedHeight,
                                      childLeft + child->mLayoutResult.mComputedWidth, childBottom);
          } else {
            layoutSingleChildVertical(child, flexLine, false, mCssStyle.mAlignItems,
                                      childLeft, childTop,
                                      childLeft + child->mLayoutResult.mComputedWidth,
                                      childTop + child->mLayoutResult.mComputedHeight);
          }
        }

        childTop += child->mLayoutResult.mComputedHeight + spaceBetweenItem + child->getMarginBottom();
        childBottom -= child->mLayoutResult.mComputedHeight + spaceBetweenItem + child->getMarginTop();
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
    if (node->mCssStyle.mAlignSelf != WXCore_AlignSelf_Auto) {
      // Expecting the values for alignItems and alignSelf match except for ALIGN_SELF_AUTO.
      // Assigning the alignSelf value as alignItems should work.
      alignItems = static_cast<WXCoreAlignItems>(node->mCssStyle.mAlignSelf);
    }

    float crossSize = flexLine->mCrossSize;

    switch (alignItems) {
      case WXCore_AlignItems_Flex_Start:
      case WXCore_AlignItems_Stretch:
      case WXCore_AlignItems_Baseline:
        if (!isRtl) {
          node->layout(left + node->getMarginLeft(), top, right + node->getMarginLeft(), bottom);
        } else {
          node->layout(left - node->getMarginRight(), top, right - node->getMarginRight(), bottom);
        }
        break;
      case WXCore_AlignItems_Flex_End:
        if (!isRtl) {
          node->layout(left + crossSize - node->mLayoutResult.mComputedWidth - node->getMarginRight(),
                       top, right + crossSize - node->mLayoutResult.mComputedWidth - node->getMarginRight(),
                       bottom);
        } else {
          // If the flexWrap == FLEX_WRAP_WRAP_REVERSE, the direction of the
          // flexEnd is flipped (from mStyleLeft to mStyleRight).
          node->layout(left - crossSize + node->mLayoutResult.mComputedWidth + node->getMarginLeft(), top,
                       right - crossSize + node->mLayoutResult.mComputedWidth + node->getMarginLeft(),
                       bottom);
        }
        break;
      case WXCore_AlignItems_Center:
        float leftFromCrossAxis = (crossSize - node->mLayoutResult.mComputedWidth
                                   + node->getMarginLeft()
                                   - node->getMarginRight()) / 2;
        if (!isRtl) {
          node->layout(left + leftFromCrossAxis, top, right + leftFromCrossAxis, bottom);
        } else {
          node->layout(left - leftFromCrossAxis, top, right - leftFromCrossAxis, bottom);
        }
        break;
    }
  }

  float WXCoreLayoutNode::getLargestMainSize() {
    float largestSize = -1.0;
    for (WXCoreFlexLine *flexLine : mFlexLines) {
      largestSize = max_num<float>(largestSize, flexLine->mMainSize);
    }
    return largestSize;
  }
}


