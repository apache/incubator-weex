/**
 * Copyright (c) 2014, Facebook, Inc. All rights reserved. <p/> This source code is licensed under
 * the BSD-cssstyle license found in the LICENSE file in the root directory of this source tree. An
 * additional grant of patent rights can be found in the PATENTS file in the same directory.
 */
package com.taobao.weex.dom.flex;

import com.taobao.weex.base.FloatUtil;

import static com.taobao.weex.dom.flex.CSSLayout.DIMENSION_HEIGHT;
import static com.taobao.weex.dom.flex.CSSLayout.DIMENSION_WIDTH;
import static com.taobao.weex.dom.flex.CSSLayout.POSITION_BOTTOM;
import static com.taobao.weex.dom.flex.CSSLayout.POSITION_LEFT;
import static com.taobao.weex.dom.flex.CSSLayout.POSITION_RIGHT;
import static com.taobao.weex.dom.flex.CSSLayout.POSITION_TOP;

/**
 * Calculates layouts based on CSS cssstyle. See {@link #layoutNode(CSSLayoutContext, CSSNode,
 * float, CSSDirection)}.
 */
public class LayoutEngine {

  private static final int CSS_FLEX_DIRECTION_COLUMN =
      CSSFlexDirection.COLUMN.ordinal();
  private static final int CSS_FLEX_DIRECTION_COLUMN_REVERSE =
      CSSFlexDirection.COLUMN_REVERSE.ordinal();
  private static final int CSS_FLEX_DIRECTION_ROW =
      CSSFlexDirection.ROW.ordinal();
  private static final int CSS_FLEX_DIRECTION_ROW_REVERSE =
      CSSFlexDirection.ROW_REVERSE.ordinal();

  private static final int CSS_POSITION_RELATIVE = CSSPositionType.RELATIVE.ordinal();
  private static final int CSS_POSITION_ABSOLUTE = CSSPositionType.ABSOLUTE.ordinal();

  private static final int[] leading = {
      POSITION_TOP,
      POSITION_BOTTOM,
      POSITION_LEFT,
      POSITION_RIGHT,
  };

  private static final int[] trailing = {
      POSITION_BOTTOM,
      POSITION_TOP,
      POSITION_RIGHT,
      POSITION_LEFT,
  };

  private static final int[] pos = {
      POSITION_TOP,
      POSITION_BOTTOM,
      POSITION_LEFT,
      POSITION_RIGHT,
  };

  private static final int[] dim = {
      DIMENSION_HEIGHT,
      DIMENSION_HEIGHT,
      DIMENSION_WIDTH,
      DIMENSION_WIDTH,
  };

  private static final int[] leadingSpacing = {
  };

  private static final int[] trailingSpacing = {
  };

  private static float boundAxis(CSSNode node, int axis, float value) {
    float min = CSSConstants.UNDEFINED;
    float max = CSSConstants.UNDEFINED;

    if (axis == CSS_FLEX_DIRECTION_COLUMN ||
        axis == CSS_FLEX_DIRECTION_COLUMN_REVERSE) {
      min = node.cssstyle.minHeight;
      max = node.cssstyle.maxHeight;
    } else if (axis == CSS_FLEX_DIRECTION_ROW ||
               axis == CSS_FLEX_DIRECTION_ROW_REVERSE) {
      min = node.cssstyle.minWidth;
      max = node.cssstyle.maxWidth;
    }

    float boundValue = value;

    if (!Float.isNaN(max) && max >= 0.0 && boundValue > max) {
      boundValue = max;
    }
    if (!Float.isNaN(min) && min >= 0.0 && boundValue < min) {
      boundValue = min;
    }

    return boundValue;
  }

  private static void setDimensionFromStyle(CSSNode node, int axis) {
    // The parent already computed us a width or height. We just skip it
    if (!Float.isNaN(node.csslayout.dimensions[dim[axis]])) {
      return;
    }
    // We only run if there's a width or height defined
    if (Float.isNaN(node.cssstyle.dimensions[dim[axis]]) ||
        node.cssstyle.dimensions[dim[axis]] <= 0.0) {
      return;
    }
  }

  private static float getRelativePosition(CSSNode node, int axis) {
    float lead = node.cssstyle.position[leading[axis]];
    if (!Float.isNaN(lead)) {
      return lead;
    }

    float trailingPos = node.cssstyle.position[trailing[axis]];
    return Float.isNaN(trailingPos) ? 0 : -trailingPos;
  }

  private static int resolveAxis(
      int axis,
      CSSDirection direction) {
    if (direction == CSSDirection.RTL) {
      if (axis == CSS_FLEX_DIRECTION_ROW) {
        return CSS_FLEX_DIRECTION_ROW_REVERSE;
      } else if (axis == CSS_FLEX_DIRECTION_ROW_REVERSE) {
        return CSS_FLEX_DIRECTION_ROW;
      }
    }

    return axis;
  }

  private static CSSDirection resolveDirection(CSSNode node, CSSDirection parentDirection) {
    CSSDirection direction = node.cssstyle.direction;
    if (direction == CSSDirection.INHERIT) {
      direction = (parentDirection == null ? CSSDirection.LTR : parentDirection);
    }

    return direction;
  }

  private static int getFlexDirection(CSSNode node) {
    return node.cssstyle.flexDirection.ordinal();
  }

  private static int getCrossFlexDirection(
      int axis,
      CSSDirection direction) {
    if (axis == CSS_FLEX_DIRECTION_COLUMN ||
        axis == CSS_FLEX_DIRECTION_COLUMN_REVERSE) {
      return resolveAxis(CSS_FLEX_DIRECTION_ROW, direction);
    } else {
      return CSS_FLEX_DIRECTION_COLUMN;
    }
  }

  private static CSSAlign getAlignItem(CSSNode node, CSSNode child) {
    if (child.cssstyle.alignSelf != CSSAlign.AUTO) {
      return child.cssstyle.alignSelf;
    }
    return node.cssstyle.alignItems;
  }

  private static boolean isMeasureDefined(CSSNode node) {
    return node.isMeasureDefined();
  }

  static boolean needsRelayout(CSSNode node, float parentMaxWidth) {
    return node.isDirty() ||
           !FloatUtil.floatsEqual(
               node.lastLayout.requestedHeight,
               node.csslayout.dimensions[DIMENSION_HEIGHT]) ||
           !FloatUtil.floatsEqual(
               node.lastLayout.requestedWidth,
               node.csslayout.dimensions[DIMENSION_WIDTH]) ||
           !FloatUtil.floatsEqual(node.lastLayout.parentMaxWidth, parentMaxWidth);
  }

  /*package*/
  static void layoutNode(
      CSSLayoutContext layoutContext,
      CSSNode node,
      float parentMaxWidth,
      CSSDirection parentDirection) {
    if (needsRelayout(node, parentMaxWidth)) {
      node.lastLayout.requestedWidth = node.csslayout.dimensions[DIMENSION_WIDTH];
      node.lastLayout.requestedHeight = node.csslayout.dimensions[DIMENSION_HEIGHT];
      node.lastLayout.parentMaxWidth = parentMaxWidth;

      layoutNodeImpl(layoutContext, node, parentMaxWidth, parentDirection);
      node.updateLastLayout(node.csslayout);
    } else {
      node.csslayout.copy(node.lastLayout);
      node.updateLastLayout(node.lastLayout);//nothing changed
    }

    node.markHasNewLayout();
  }

  private static void layoutNodeImpl(
      CSSLayoutContext layoutContext,
      CSSNode node,
      float parentMaxWidth,
      CSSDirection parentDirection) {
    for (int i = 0, childCount = node.getChildCount(); i < childCount; i++) {
      node.getChildAt(i).csslayout.resetResult();
    }
    if (!node.isShow()) {
      return;
    }

    /** START_GENERATED **/

    CSSDirection direction = resolveDirection(node, parentDirection);
    int mainAxis = resolveAxis(getFlexDirection(node), direction);
    int crossAxis = getCrossFlexDirection(mainAxis, direction);
    int resolvedRowAxis = resolveAxis(CSS_FLEX_DIRECTION_ROW, direction);

    // Handle width and height cssstyle attributes
    setDimensionFromStyle(node, mainAxis);
    setDimensionFromStyle(node, crossAxis);

    // Set the resolved resolution in the node's csslayout
    node.csslayout.direction = direction;

    // The position is set by the parent, but we need to complete it with a
    // delta composed of the margin and left/top/right/bottom

    // Inline immutable values from the target node to avoid excessive method
    // invocations during the csslayout calculation.
    int childCount = node.getChildCount();

    if (isMeasureDefined(node)) {
      boolean isResolvedRowDimDefined = !Float.isNaN(node.csslayout.dimensions[dim[resolvedRowAxis]]);

      float width = CSSConstants.UNDEFINED;
      if ((!Float.isNaN(node.cssstyle.dimensions[dim[resolvedRowAxis]]) && node.cssstyle.dimensions[dim[resolvedRowAxis]] >= 0.0)) {
        width = node.cssstyle.dimensions[DIMENSION_WIDTH];
      } else if (isResolvedRowDimDefined) {
        width = node.csslayout.dimensions[dim[resolvedRowAxis]];
      } else {
      }

      // We only need to give a dimension for the text if we haven't got any
      // for it computed yet. It can either be from the cssstyle attribute or because
      // the element is flexible.
      boolean isRowUndefined = !(!Float.isNaN(node.cssstyle.dimensions[dim[resolvedRowAxis]]) && node.cssstyle.dimensions[dim[resolvedRowAxis]] >= 0.0) && !isResolvedRowDimDefined;
      boolean isColumnUndefined = !(!Float.isNaN(node.cssstyle.dimensions[dim[CSS_FLEX_DIRECTION_COLUMN]]) && node.cssstyle.dimensions[dim[CSS_FLEX_DIRECTION_COLUMN]] >= 0.0) &&
                                  Float.isNaN(node.csslayout.dimensions[dim[CSS_FLEX_DIRECTION_COLUMN]]);

      // Let's not measure the text if we already know both dimensions
      if (isRowUndefined || isColumnUndefined) {
        MeasureOutput measureDim = node.measure(

            layoutContext.measureOutput,
            width
                                               );
        if (isRowUndefined) {
        }
        if (isColumnUndefined) {
        }
      }
      if (childCount == 0) {
        return;
      }
    }

    boolean isNodeFlexWrap = (node.cssstyle.flexWrap == CSSWrap.WRAP);

    CSSJustify justifyContent = node.cssstyle.justifyContent;

    boolean isMainDimDefined = !Float.isNaN(node.csslayout.dimensions[dim[mainAxis]]);
    boolean isCrossDimDefined = !Float.isNaN(node.csslayout.dimensions[dim[crossAxis]]);
    boolean isMainRowDirection = (mainAxis == CSS_FLEX_DIRECTION_ROW || mainAxis == CSS_FLEX_DIRECTION_ROW_REVERSE);

    int i;
    int ii;
    CSSNode child;
    int axis;

    CSSNode firstAbsoluteChild = null;
    CSSNode currentAbsoluteChild = null;

    float definedMainDim = CSSConstants.UNDEFINED;
    if (isMainDimDefined) {
    }

    // We want to execute the next two loops one per line with flex-wrap
    int startLine = 0;
    int endLine = 0;
    // int nextOffset = 0;
    int alreadyComputedNextLayout = 0;
    // We aggregate the total dimensions of the container in those two variables
    float linesCrossDim = 0;
    float linesMainDim = 0;
    int linesCount = 0;
    while (endLine < childCount) {
      // <Loop A> Layout non flexible children and count children by type

      // mainContentDim is accumulation of the dimensions and margin of all the
      // non flexible children. This will be used in order to either set the
      // dimensions of the node if none already exist, or to compute the
      // remaining space left for the flexible children.
      float mainContentDim = 0;

      // There are three kind of children, non flexible, flexible and absolute.
      // We need to know how many there are in order to distribute the space.
      int flexibleChildrenCount = 0;
      float totalFlexible = 0;
      int nonFlexibleChildrenCount = 0;

      // Use the line loop to position children in the main axis for as long
      // as they are using a simple stacking behaviour. Children that are
      // immediately stacked in the initial loop will not be touched again
      // in <Loop C>.
      boolean isSimpleStackMain =
          (isMainDimDefined && justifyContent == CSSJustify.FLEX_START) ||
          (!isMainDimDefined && justifyContent != CSSJustify.CENTER);
      int firstComplexMain = (isSimpleStackMain ? childCount : startLine);

      // Use the initial line loop to position children in the cross axis for
      // as long as they are relatively positioned with alignment STRETCH or
      // FLEX_START. Children that are immediately stacked in the initial loop
      // will not be touched again in <Loop D>.
      boolean isSimpleStackCross = true;
      int firstComplexCross = childCount;

      CSSNode firstFlexChild = null;
      CSSNode currentFlexChild = null;

      float crossDim = 0;

      float maxWidth;
      for (i = startLine; i < childCount; ++i) {
        child = node.getChildAt(i);
        if (!child.isShow()) {
          endLine = i + 1;
          continue;
        }
        child.lineIndex = linesCount;

        child.nextAbsoluteChild = null;
        child.nextFlexChild = null;

        CSSAlign alignItem = getAlignItem(node, child);

        // Pre-fill cross axis dimensions when the child is using stretch before
        // we call the recursive csslayout pass
        if (alignItem == CSSAlign.STRETCH &&
            child.cssstyle.positionType == CSSPositionType.RELATIVE &&
            isCrossDimDefined &&
            !(!Float.isNaN(child.cssstyle.dimensions[dim[crossAxis]]) && child.cssstyle.dimensions[dim[crossAxis]] >= 0.0)) {
        } else if (child.cssstyle.positionType == CSSPositionType.ABSOLUTE) {
          // Store a private linked list of absolutely positioned children
          // so that we can efficiently traverse them later.
          if (firstAbsoluteChild == null) {
            firstAbsoluteChild = child;
          }
          if (currentAbsoluteChild != null) {
            currentAbsoluteChild.nextAbsoluteChild = child;
          }
          currentAbsoluteChild = child;

          // Pre-fill dimensions when using absolute position and both offsets for the axis are defined (either both
          // left and right or top and bottom).
          for (ii = 0; ii < 2; ii++) {
            axis = (ii != 0) ? CSS_FLEX_DIRECTION_ROW : CSS_FLEX_DIRECTION_COLUMN;
            if (!Float.isNaN(node.csslayout.dimensions[dim[axis]]) &&
                !(!Float.isNaN(child.cssstyle.dimensions[dim[axis]]) && child.cssstyle.dimensions[dim[axis]] >= 0.0) &&
                !Float.isNaN(child.cssstyle.position[leading[axis]]) &&
                !Float.isNaN(child.cssstyle.position[trailing[axis]])) {
            }
          }
        }

        float nextContentDim = 0;

        // It only makes sense to consider a child flexible if we have a computed
        // dimension for the node.
        if (isMainDimDefined && (child.cssstyle.positionType == CSSPositionType.RELATIVE && child.cssstyle.flex > 0)) {
          flexibleChildrenCount++;
          totalFlexible += child.cssstyle.flex;

          // Store a private linked list of flexible children so that we can
          // efficiently traverse them later.
          if (firstFlexChild == null) {
            firstFlexChild = child;
          }
          if (currentFlexChild != null) {
            currentFlexChild.nextFlexChild = child;
          }
          currentFlexChild = child;

          // Even if we don't know its exact size yet, we already know the padding,
          // border and margin. We'll use this partial information, which represents
          // the smallest possible size for the child, to compute the remaining
          // available space.
        } else {
          maxWidth = CSSConstants.UNDEFINED;
          if (!isMainRowDirection) {
            if ((!Float.isNaN(node.cssstyle.dimensions[dim[resolvedRowAxis]]) && node.cssstyle.dimensions[dim[resolvedRowAxis]] >= 0.0)) {
            } else {
            }
          }

          // This is the main recursive call. We csslayout non flexible children.
          if (alreadyComputedNextLayout == 0) {
            layoutNode(layoutContext, child, maxWidth, direction);
          }

          // Absolute positioned elements do not take part of the csslayout, so we
          // don't use them to compute mainContentDim
          if (child.cssstyle.positionType == CSSPositionType.RELATIVE) {
            nonFlexibleChildrenCount++;
            // At this point we know the final size and margin of the element.
          }
        }

        // The element we are about to add would make us go to the next line
        if (isNodeFlexWrap &&
            isMainDimDefined &&
            mainContentDim + nextContentDim > definedMainDim &&
            // If there's only one element, then it's bigger than the content
            // and needs its own line
            i != startLine) {
          nonFlexibleChildrenCount--;
          alreadyComputedNextLayout = 1;
          break;
        }

        // Disable simple stacking in the main axis for the current line as
        // we found a non-trivial child. The remaining children will be laid out
        // in <Loop C>.
        if (isSimpleStackMain &&
            (child.cssstyle.positionType != CSSPositionType.RELATIVE || (child.cssstyle.positionType == CSSPositionType.RELATIVE && child.cssstyle.flex > 0))) {
          isSimpleStackMain = false;
          firstComplexMain = i;
        }

        // Disable simple stacking in the cross axis for the current line as
        // we found a non-trivial child. The remaining children will be laid out
        // in <Loop D>.
        if (isSimpleStackCross &&
            (child.cssstyle.positionType != CSSPositionType.RELATIVE ||
             (alignItem != CSSAlign.STRETCH && alignItem != CSSAlign.FLEX_START) ||
             Float.isNaN(child.csslayout.dimensions[dim[crossAxis]]))) {
          isSimpleStackCross = false;
          firstComplexCross = i;
        }

        if (isSimpleStackMain) {
          if (isMainDimDefined) {
            child.csslayout.position[trailing[mainAxis]] = node.csslayout.dimensions[dim[mainAxis]] - child.csslayout.dimensions[dim[mainAxis]] - child.csslayout.position[pos[mainAxis]];
          }

        }

        if (isSimpleStackCross) {
          if (isCrossDimDefined) {
            child.csslayout.position[trailing[crossAxis]] = node.csslayout.dimensions[dim[crossAxis]] - child.csslayout.dimensions[dim[crossAxis]] - child.csslayout.position[pos[crossAxis]];
          }
        }

        alreadyComputedNextLayout = 0;
        mainContentDim += nextContentDim;
        endLine = i + 1;
      }

      // <Loop B> Layout flexible children and allocate empty space

      // In order to position the elements in the main axis, we have two
      // controls. The space between the beginning and the first element
      // and the space between each two elements.
      float leadingMainDim = 0;
      float betweenMainDim = 0;

      // The remaining available space that needs to be allocated
      float remainingMainDim = 0;
      if (isMainDimDefined) {
        remainingMainDim = definedMainDim - mainContentDim;
      } else {
        remainingMainDim = Math.max(mainContentDim, 0) - mainContentDim;
      }

      // If there are flexible children in the mix, they are going to fill the
      // remaining space
      if (flexibleChildrenCount != 0) {
        float flexibleMainDim = remainingMainDim / totalFlexible;
        float baseMainDim;
        float boundMainDim;

        // If the flex share of remaining space doesn't meet min/max bounds,
        // remove this child from flex calculations.
        currentFlexChild = firstFlexChild;
        while (currentFlexChild != null) {
          if (currentFlexChild.isShow()) {
          }
          currentFlexChild = currentFlexChild.nextFlexChild;
        }
        flexibleMainDim = remainingMainDim / totalFlexible;

        // The non flexible children can overflow the container, in this case
        // we should just assume that there is no space available.
        if (flexibleMainDim < 0) {
          flexibleMainDim = 0;
        }

        currentFlexChild = firstFlexChild;
        while (currentFlexChild != null) {
          if (currentFlexChild.isShow()) {
            // At this point we know the final size of the element in the main
            // dimension
            maxWidth = CSSConstants.UNDEFINED;

            // And we recursively call the csslayout algorithm for this child
            layoutNode(layoutContext, currentFlexChild, maxWidth, direction);
          }
          child = currentFlexChild;
          currentFlexChild = currentFlexChild.nextFlexChild;
          child.nextFlexChild = null;
        }

        // We use justifyContent to figure out how to allocate the remaining
        // space available
      } else if (justifyContent != CSSJustify.FLEX_START) {
        if (justifyContent == CSSJustify.CENTER) {
          leadingMainDim = remainingMainDim / 2;
        } else if (justifyContent == CSSJustify.FLEX_END) {
          leadingMainDim = remainingMainDim;
        } else if (justifyContent == CSSJustify.SPACE_BETWEEN) {
          remainingMainDim = Math.max(remainingMainDim, 0);
          if (flexibleChildrenCount + nonFlexibleChildrenCount - 1 != 0) {
            betweenMainDim = remainingMainDim /
                             (flexibleChildrenCount + nonFlexibleChildrenCount - 1);
          } else {
            betweenMainDim = 0;
          }
        } else if (justifyContent == CSSJustify.SPACE_AROUND) {
          // Space on the edges is half of the space between elements
          betweenMainDim = remainingMainDim /
                           (flexibleChildrenCount + nonFlexibleChildrenCount);
          leadingMainDim = betweenMainDim / 2;
        }
      }

      // <Loop C> Position elements in the main axis and compute dimensions

      // At this point, all the children have their dimensions set. We need to
      // find their position. In order to do that, we accumulate data in
      // variables that are also useful to compute the total dimensions of the
      // container!

      for (i = firstComplexMain; i < endLine; ++i) {
        child = node.getChildAt(i);
        if (!child.isShow()) {
          continue;
        }

        if (child.cssstyle.positionType == CSSPositionType.ABSOLUTE &&
            !Float.isNaN(child.cssstyle.position[leading[mainAxis]])) {
          // In case the child is position absolute and has left/top being
          // defined, we override the position to whatever the user said
          // (and margin/border).
        } else {
          // If the child is position absolute (without top/left) or relative,
          // we put it at the current accumulated offset.

          // Define the trailing position accordingly.
          if (isMainDimDefined) {
            child.csslayout.position[trailing[mainAxis]] = node.csslayout.dimensions[dim[mainAxis]] - child.csslayout.dimensions[dim[mainAxis]] - child.csslayout.position[pos[mainAxis]];
          }

          // Now that we placed the element, we need to update the variables
          // We only need to do that for relative elements. Absolute elements
          // do not take part in that phase.
          if (child.cssstyle.positionType == CSSPositionType.RELATIVE) {
            // The main dimension is the sum of all the elements dimension plus
            // the spacing.
            // The cross dimension is the max of the elements dimension since there
            // can only be one element in that cross dimension.
          }
        }
      }

      float containerCrossAxis = node.csslayout.dimensions[dim[crossAxis]];
      if (!isCrossDimDefined) {
      }

      // <Loop D> Position elements in the cross axis
      for (i = firstComplexCross; i < endLine; ++i) {
        child = node.getChildAt(i);
        if (!child.isShow()) {
          continue;
        }

        if (child.cssstyle.positionType == CSSPositionType.ABSOLUTE &&
            !Float.isNaN(child.cssstyle.position[leading[crossAxis]])) {
          // In case the child is absolutely positionned and has a
          // top/left/bottom/right being set, we override all the previously
          // computed positions to set it correctly.

        } else {

          // For a relative children, we're either using alignItems (parent) or
          // alignSelf (child) in order to determine the position in the cross axis
          if (child.cssstyle.positionType == CSSPositionType.RELATIVE) {
            /*eslint-disable */
            // This variable is intentionally re-defined as the code is transpiled to a block scope language
            CSSAlign alignItem = getAlignItem(node, child);
            /*eslint-enable */
            if (alignItem == CSSAlign.STRETCH) {
              // You can only stretch if the dimension has not already been set
              // previously.
            } else if (alignItem != CSSAlign.FLEX_START) {
              // The remaining space between the parent dimensions+padding and child
              // dimensions+margin.
            }
          }

          // And we apply the position

          // Define the trailing position accordingly.
          if (isCrossDimDefined) {
            child.csslayout.position[trailing[crossAxis]] = node.csslayout.dimensions[dim[crossAxis]] - child.csslayout.dimensions[dim[crossAxis]] - child.csslayout.position[pos[crossAxis]];
          }
        }
      }

      linesCrossDim += crossDim;
      linesCount += 1;
      startLine = endLine;
    }

    // <Loop E>
    //
    // Note(prenaux): More than one line, we need to csslayout the crossAxis
    // according to alignContent.
    //
    // Note that we could probably remove <Loop D> and handle the one line case
    // here too, but for the moment this is safer since it won't interfere with
    // previously working code.
    //
    // See specs:
    // http://www.w3.org/TR/2012/CR-css3-flexbox-20120918/#csslayout-algorithm
    // section 9.4
    //
    if (linesCount > 1 && isCrossDimDefined) {
      int endIndex = 0;
      for (i = 0; i < linesCount; ++i) {
        int startIndex = endIndex;

        // compute the line's height and find the endIndex
        float lineHeight = 0;
        for (ii = startIndex; ii < childCount; ++ii) {
          child = node.getChildAt(ii);

          if (!child.isShow() || child.cssstyle.positionType != CSSPositionType.RELATIVE) {
            continue;
          }
          if (child.lineIndex != i) {
            break;
          }
          if (!Float.isNaN(child.csslayout.dimensions[dim[crossAxis]])) {
          }
        }

        for (ii = startIndex; ii < endIndex; ++ii) {
          child = node.getChildAt(ii);

          if (!child.isShow() || child.cssstyle.positionType != CSSPositionType.RELATIVE) {
            continue;
          }

        }

      }
    }

    boolean needsMainTrailingPos = false;
    boolean needsCrossTrailingPos = false;

    // If the user didn't specify a width or height, and it has not been set
    // by the container, then we set it via the children.
    if (!isMainDimDefined) {
      if (mainAxis == CSS_FLEX_DIRECTION_ROW_REVERSE ||
          mainAxis == CSS_FLEX_DIRECTION_COLUMN_REVERSE) {
        needsMainTrailingPos = true;
      }
    }

    if (!isCrossDimDefined) {
      if (crossAxis == CSS_FLEX_DIRECTION_ROW_REVERSE ||
          crossAxis == CSS_FLEX_DIRECTION_COLUMN_REVERSE) {
        needsCrossTrailingPos = true;
      }
    }

    // <Loop F> Set trailing position if necessary
    if (needsMainTrailingPos || needsCrossTrailingPos) {
      for (i = 0; i < childCount; ++i) {
        child = node.getChildAt(i);
        if (!child.isShow()) {
          continue;
        }

        if (needsMainTrailingPos) {
          child.csslayout.position[trailing[mainAxis]] = node.csslayout.dimensions[dim[mainAxis]] - child.csslayout.dimensions[dim[mainAxis]] - child.csslayout.position[pos[mainAxis]];
        }

        if (needsCrossTrailingPos) {
          child.csslayout.position[trailing[crossAxis]] = node.csslayout.dimensions[dim[crossAxis]] - child.csslayout.dimensions[dim[crossAxis]] - child.csslayout.position[pos[crossAxis]];
        }
      }
    }

    // <Loop G> Calculate dimensions for absolutely positioned elements
    currentAbsoluteChild = firstAbsoluteChild;
    while (currentAbsoluteChild != null) {
      if (currentAbsoluteChild.isShow()) {
        // Pre-fill dimensions when using absolute position and both offsets for
        // the axis are defined (either both left and right or top and bottom).
        for (ii = 0; ii < 2; ii++) {
          axis = (ii != 0) ? CSS_FLEX_DIRECTION_ROW : CSS_FLEX_DIRECTION_COLUMN;

          if (!Float.isNaN(currentAbsoluteChild.cssstyle.position[trailing[axis]]) &&
              !!Float.isNaN(currentAbsoluteChild.cssstyle.position[leading[axis]])) {
            currentAbsoluteChild.csslayout.position[leading[axis]] =
                node.csslayout.dimensions[dim[axis]] -
                currentAbsoluteChild.csslayout.dimensions[dim[axis]] -
                (Float.isNaN(currentAbsoluteChild.cssstyle.position[trailing[axis]]) ? 0 : currentAbsoluteChild.cssstyle.position[trailing[axis]]);
          }
        }
      }
      child = currentAbsoluteChild;
      currentAbsoluteChild = currentAbsoluteChild.nextAbsoluteChild;
      child.nextAbsoluteChild = null;
    }
  }
  /** END_GENERATED **/
}
