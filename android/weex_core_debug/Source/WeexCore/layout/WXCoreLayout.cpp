#include "WXCoreLayout.h"

using namespace WeexCore;

const float WXCoreCSSStyle::FLEX_GROW_DEFAULT = 0;
const WXCoreFlexDirection WXCoreCSSStyle::FLEX_DIRECTION_DEFAULT = WXCore_Flex_Direction_Column;
const WXCoreFlexWrap WXCoreCSSStyle::FLEX_WRAP_DEFAULT = WXCore_Wrap_NoWrap;
const WXCoreJustifyContent WXCoreCSSStyle::FLEX_JUSTIFY_CONTENT_DEFAULT = WXCore_Justify_Flex_Start;
const WXCoreAlignItems WXCoreCSSStyle::FLEX_ALIGN_ITEMS_DEFAULT = WXCore_AlignItems_Stretch;
const WXCoreAlignSelf WXCoreCSSStyle::FLEX_ALIGN_SELF_DEFAULT = WXCore_AlignSelf_Auto;


namespace WeexCore {

  /**
   * Entry function to calculate layout
   */
  void WXCoreLayoutNode::calculateLayout() {
    BFCs.clear();
    initFormatingContext(BFCs);
    auto bfcDimension = calculateBFCDimension();
    if(isDirty()) {
      measure(bfcDimension.first, bfcDimension.second, true);
    }
    layout(mCssStyle->mMargin.getMargin(WXCore_Margin_Left),
           mCssStyle->mMargin.getMargin(WXCore_Margin_Top),
           mCssStyle->mMargin.getMargin(WXCore_Margin_Left) + getLayoutWidth(),
           mCssStyle->mMargin.getMargin(WXCore_Margin_Top) + getLayoutHeight());
    for (Index i = 0; i < getChildCount(BFC); ++i) {
      WXCoreLayoutNode *child = getChildAt(BFC, i);
      child->calculateLayout();
    }
  }

  void WXCoreLayoutNode::initFormatingContext(std::vector<WXCoreLayoutNode *> &BFCs) {
    NonBFCs.clear();
    for (Index i = 0; i < getChildCount(); i++) {
      WXCoreLayoutNode *child = getChildAt(i);
      if (isBFC(child)) {
        BFCs.push_back(child);
      } else {
        NonBFCs.push_back(child);
        child->initFormatingContext(BFCs);
      }
    }
    reset();
  }

  std::pair<float, float> WXCoreLayoutNode::calculateBFCDimension() {
    float width=mCssStyle->mStyleWidth, height=mCssStyle->mStyleHeight;
    if (mCssStyle->mPositionType == WXCore_PositionType_Absolute) {
      if (isnan(width) &&
          mParent != nullptr &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left)) &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right))) {
        width = mParent->mLayoutResult->mLayoutSize.width -
                mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left) -
                mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right);
        setWidthMeasureMode(EXACTLY);
      }

      if (isnan(mCssStyle->mStyleHeight) &&
          mParent != nullptr &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top)) &&
          !isnan(mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom))) {
        height = mParent->mLayoutResult->mLayoutSize.height -
                 mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top) -
                 mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
        setHeightMeasureMode(EXACTLY);
      }
    }

    return std::make_pair(width, height);
  }

  void WXCoreLayoutNode::measure(const float width, const float height, const bool hypotheticalMeasurment){
    if(hypotheticalMeasurment){
      hypotheticalMeasure(width, height, hypotheticalMeasurment);
    }
    if(getChildCount(NON_BFC) > 0){
      if((isMainAxisHorizontal(this) && widthDirty) || (!isMainAxisHorizontal(this) && heightDirty)){
        measureInternalNode(width, height, false, false);
      }
      determineMainSize(width, height);
      determineCrossSize(width, height, true);
      measureInternalNode(width, height, true, false);
      determineCrossSize(width, height, false);
    }
    else {
      if (widthDirty || heightDirty) {
        measureLeafNode(width, height);
      }
    }
    clearDirty();
  }

  void WXCoreLayoutNode::hypotheticalMeasure(const float width, const float height, const bool hypotheticalMeasurment){
    if (getChildCount(NON_BFC) > 0) {
      measureInternalNode(width, height, true, hypotheticalMeasurment);
    } else {
      measureLeafNode(width, height);
    }
    widthDirty = false;
    heightDirty = false;
  }

    void WXCoreLayoutNode::measureLeafNode(float width, float height) {
      if ((measureFunc != nullptr) &&
          (widthMeasureMode == UNSPECIFIED
           || heightMeasureMode == UNSPECIFIED)) {
        WXCoreSize dimension = measureFunc(this, width, widthMeasureMode, height, heightMeasureMode);
        if(widthMeasureMode == UNSPECIFIED && isnan(width)){
          width = dimension.width + sumPaddingBorderAlongAxis(this, true);
        }
        if(heightMeasureMode == UNSPECIFIED && isnan(height)){
          height = dimension.height + sumPaddingBorderAlongAxis(this, false);
        }
      } else {
        width = widthMeasureMode == UNSPECIFIED ? sumPaddingBorderAlongAxis(this, true)
                                                            : width;
        height = heightMeasureMode == UNSPECIFIED ? sumPaddingBorderAlongAxis(this, false)
                                                              : height;
      }
      setMeasuredDimension(width, height);
    }


  /**
   * Determine the main size by expanding the individual flexGrow attribute.
   */
  void WXCoreLayoutNode::determineMainSize(float width, float height) {
      float maxMainSize;
      bool horizontal = isMainAxisHorizontal(this);
      float paddingAlongMainAxis = sumPaddingBorderAlongAxis(this, horizontal);
      if (horizontal) {
        maxMainSize = widthMeasureMode == UNSPECIFIED ? getLargestMainSize() : width;
      } else {
        maxMainSize = heightMeasureMode == UNSPECIFIED ? getLargestMainSize() : height;
      }

      Index childIndex = 0;
      for (auto flexLine : mFlexLines) {
        if (flexLine->mMainSize < maxMainSize) {
          childIndex = expandItemsInFlexLine(flexLine, maxMainSize, paddingAlongMainAxis, childIndex);
        }
      }
    }


  /**
   * @param flexDirection         the flex direction attribute
   * @param width                 stylewidth by this node
   * @param height                styleheight by this node
   * @param paddingAlongCrossAxis the padding value for this node along the cross axis
   */
    void WXCoreLayoutNode::determineCrossSize(float width, float height, bool stretch) {
      if (mFlexLines.size() == 1 && isCrossExactly()) {
        bool horizontal = isMainAxisHorizontal(this);
        float size = mFlexLines[0]->mCrossSize;
        float paddingAlongCrossAxis = sumPaddingBorderAlongAxis(this, !horizontal);
        if (horizontal) {
          if (heightMeasureMode == EXACTLY) {
            size = height - paddingAlongCrossAxis;
          }
        } else {
          if (widthMeasureMode == EXACTLY) {
            size = width - paddingAlongCrossAxis;
          }
        }
        mFlexLines[0]->mCrossSize = size;
      }
      if (stretch) {
        stretchViewCrossSize();
      }
    }


    void WXCoreLayoutNode::measureInternalNode(float width, float height, bool needMeasure, bool hypotheticalMeasurment) {
      mFlexLines.clear();
      Index childCount = getChildCount(NON_BFC);
      auto *flexLine = new WXCoreFlexLine();
      flexLine->mMainSize = sumPaddingBorderAlongAxis(this, isMainAxisHorizontal(this));

      // The index of the view in a same flex line.
      for (Index i = 0; i < childCount; i++) {
        WXCoreLayoutNode *child = getChildAt(NON_BFC, i);
        if (child->mCssStyle->mAlignSelf == WXCore_AlignSelf_Stretch) {
          flexLine->mIndicesAlignSelfStretch.push_back(i);
        }
        measureChild(child, width, height, needMeasure, hypotheticalMeasurment);
        checkSizeConstraints(child, hypotheticalMeasurment);
        if (isWrapRequired(isMainAxisHorizontal(this) ? width : height, flexLine->mMainSize,
                           calcItemSizeAlongAxis(child, isMainAxisHorizontal(this)))) {
          if (flexLine->mItemCount > 0) {
            mFlexLines.push_back(flexLine);
          }
          flexLine = new WXCoreFlexLine();
          flexLine->mItemCount = 1;
          flexLine->mMainSize = sumPaddingBorderAlongAxis(this, isMainAxisHorizontal(this));
        } else {
          flexLine->mItemCount++;
        }
        updateCurrentFlexline(childCount, flexLine, i, child);
      }
      setMeasuredDimensionForFlex(width, widthMeasureMode, height, heightMeasureMode);
    }

    void WXCoreLayoutNode::updateCurrentFlexline(const Index childCount, WXCoreFlexLine* flexLine, const Index i, const WXCoreLayoutNode* child){
      flexLine->mMainSize = calcItemSizeAlongAxis(child, isMainAxisHorizontal(this));
      sumFlexGrow(child, flexLine, i, isMainAxisHorizontal(this));
      flexLine->mCrossSize = std::max(flexLine->mCrossSize, calcItemSizeAlongAxis(child, !isMainAxisHorizontal(this)));
      if (i == childCount - 1 && flexLine->mItemCount != 0) {
        mFlexLines.push_back(flexLine);
      }
    }

    void WXCoreLayoutNode::measureChild(WXCoreLayoutNode* child, const float parentWidth, const float parentHeight,
                                     bool needMeasure, bool hypotheticalMeasurment) {
      if (needMeasure && child->isDirty()) {
        if (hypotheticalMeasurment) {
          float childWidth = child->mCssStyle->mStyleWidth;
          float childHeight = child->mCssStyle->mStyleHeight;
          if (isSingleFlexLine(isMainAxisHorizontal(this) ? parentWidth : parentHeight)
              && mCssStyle->mAlignItems == WXCore_AlignItems_Stretch) {
            if (isMainAxisHorizontal(this)) {
              if(!isnan(parentHeight) && isnan(child->mCssStyle->mStyleHeight)){
                childHeight = (parentHeight-sumPaddingBorderAlongAxis(this, false));
              }
            } else {
              if(!isnan(parentWidth) && isnan(child->mCssStyle->mStyleWidth)){
                childWidth = (parentWidth - sumPaddingBorderAlongAxis(this, true));
              }
            }
          }
          child->hypotheticalMeasure(childWidth, childHeight, hypotheticalMeasurment);
        } else {
          child->measure(child->mLayoutResult->mLayoutSize.width,
                         child->mLayoutResult->mLayoutSize.height, hypotheticalMeasurment);
        }
      }
    }

    void WXCoreLayoutNode::checkSizeConstraints(WXCoreLayoutNode *node, const bool hypotheticalMeasurment) {
      bool widthRemeasure = false, heightRemeasure = false;
      float nodeWidth = node->mLayoutResult->mLayoutSize.width;
      float nodeHeight = node->mLayoutResult->mLayoutSize.height;

      if (node->widthMeasureMode != UNSPECIFIED) {
        if (nodeWidth < node->mCssStyle->mMinWidth) {
          widthRemeasure = true;
          nodeWidth = node->mCssStyle->mMinWidth;
        } else if (nodeWidth > node->mCssStyle->mMaxWidth) {
          widthRemeasure = true;
          nodeWidth = node->mCssStyle->mMaxWidth;
        }
      }

      if (node->heightMeasureMode != UNSPECIFIED) {
        if (nodeHeight < node->mCssStyle->mMinHeight) {
          heightRemeasure = true;
          nodeHeight = node->mCssStyle->mMinHeight;
        } else if (nodeHeight > node->mCssStyle->mMaxHeight) {
          heightRemeasure = true;
          nodeHeight = node->mCssStyle->mMaxHeight;
        }
      }

      node->setWidthMeasureMode(widthRemeasure ? EXACTLY : node->widthMeasureMode);
      node->setHeightMeasureMode(heightRemeasure ? EXACTLY : node->heightMeasureMode);

      if (hypotheticalMeasurment) {
        if (widthRemeasure) {
          node->setLayoutWidth(nodeWidth);
        }
        if (heightRemeasure) {
          node->setLayoutHeight(nodeHeight);
        }
      } else {
        if (widthRemeasure || heightRemeasure) {
          node->measure(nodeWidth, nodeHeight, hypotheticalMeasurment);
        }
      }
    }

    Index WXCoreLayoutNode::expandItemsInFlexLine(WXCoreFlexLine *flexLine,
                                      const float maxMainSize, const float paddingBorderAlongMainAxis,
                                      const Index startIndex) {
      Index childIndex = startIndex;
      if (flexLine->mTotalFlexGrow <= 0) {
        childIndex += flexLine->mItemCount;
      } else {
        bool needsReexpand = false;
        const float unitSpace = (maxMainSize - flexLine->mMainSize + flexLine->mTotalFlexibleSize) /
                                (flexLine->mTotalFlexGrow > 1 ? flexLine->mTotalFlexGrow : 1);
        float sizeBeforeExpand = flexLine->mMainSize;
        flexLine->mMainSize = paddingBorderAlongMainAxis;

        for (Index i = 0; i < flexLine->mItemCount; i++) {
          WXCoreLayoutNode *child = getChildAt(NON_BFC, childIndex);
          if (!mChildrenFrozen[childIndex]) {
            float childSizeAlongMainAxis = unitSpace * child->mCssStyle->mFlexGrow;
            std::pair<bool, float> limitSize = limitChildMainSize(flexLine, child,
                                                                childSizeAlongMainAxis, childIndex);
            needsReexpand = limitSize.first;
            adjustChildSize(child, limitSize.second);
          }
          flexLine->mMainSize += calcItemSizeAlongAxis(child, isMainAxisHorizontal(this));
          childIndex++;
        }

        if (needsReexpand && sizeBeforeExpand != flexLine->mMainSize) {
          // Re-invoke the method with the same startIndex to distribute the positive free space
          // that wasn't fully distributed (because of maximum/minimum length constraint)
          expandItemsInFlexLine(flexLine, maxMainSize, paddingBorderAlongMainAxis, startIndex);
        }
      }
      return childIndex;
    }

    inline std::pair<bool, float> WXCoreLayoutNode::limitChildMainSize(WXCoreFlexLine* flexLine, const WXCoreLayoutNode* child,
                                                           float childSizeAlongMainAxis, const Index childIndex){
      bool needsReexpand = false;
      if (isMainAxisHorizontal(this)) {
        if (childSizeAlongMainAxis > child->mCssStyle->mMaxWidth) {
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMaxWidth;
        } else if (childSizeAlongMainAxis < child->mCssStyle->mMinWidth) {
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMinWidth;
        }
      } else {
        if (childSizeAlongMainAxis > child->mCssStyle->mMaxHeight) {
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMaxHeight;
        } else if (childSizeAlongMainAxis < child->mCssStyle->mMinHeight) {
          needsReexpand = limitMainSizeForFlexGrow(flexLine, childIndex, child->mCssStyle->mFlexGrow);
          childSizeAlongMainAxis = child->mCssStyle->mMinHeight;
        }
      }
      return std::make_pair(needsReexpand, childSizeAlongMainAxis);
    }

    void WXCoreLayoutNode::adjustChildSize(WXCoreLayoutNode* child, float childMainSize) {
      if (isMainAxisHorizontal(this)) {
        child->setWidthMeasureMode(EXACTLY);
        child->setLayoutWidth(childMainSize);
      } else {
        child->setHeightMeasureMode(EXACTLY);
        child->setLayoutHeight(childMainSize);
      }
    }

    void WXCoreLayoutNode::stretchViewCrossSize(){
      if (mCssStyle->mAlignItems == WXCore_AlignItems_Stretch) {
        Index viewIndex = 0;
        for (auto flexLine : mFlexLines) {
          for (Index i = 0; i < flexLine->mItemCount; i++, viewIndex++) {
            auto child = getChildAt(NON_BFC, viewIndex);
            if (child->mCssStyle->mAlignSelf == WXCore_AlignSelf_Auto ||
                child->mCssStyle->mAlignSelf == WXCore_AlignSelf_Stretch) {
              stretchViewCrossSize(child, flexLine->mCrossSize);
            }
          }
        }
      } else {
        for (auto flexLine : mFlexLines) {
          for (auto index : flexLine->mIndicesAlignSelfStretch) {
            stretchViewCrossSize(getChildAt(NON_BFC, index), flexLine->mCrossSize);
          }
        }
      }
    }

    void WXCoreLayoutNode::stretchViewCrossSize(WXCoreLayoutNode* child, float crossSize){
      if (isMainAxisHorizontal(this)) {
        if (child->heightMeasureMode != EXACTLY) {
          child->setHeightMeasureMode(EXACTLY);
          child->setLayoutHeight(std::max(0.f, crossSize));
        }
      } else {
        if (child->widthMeasureMode != EXACTLY) {
          child->setWidthMeasureMode(EXACTLY);
          child->setLayoutWidth(std::max(0.f, crossSize));
        }
      }
    }

    void WXCoreLayoutNode::setFrame(const float l, const float t, const float r, const float b) {
      if (mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Left) != l
          || mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Top) != t
          || mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Right) != r
          || mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Bottom) != b) {
        setHasNewLayout(true);
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Left, l);
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Top, t);
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Right, r);
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Bottom, b);
      }
    }

    float WXCoreLayoutNode::getTotalMainSize() {
      float sum = 0;
      for (WXCoreFlexLine *flexLine : mFlexLines) {
        sum += flexLine->mMainSize;
      }
      return sum;
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
        layoutVertical(mCssStyle->mFlexWrap == WXCore_Wrap_WrapReverse, true, left, top, right,
                       bottom);
        break;
      case WXCore_Flex_Direction_Column:
      default:
        layoutVertical(mCssStyle->mFlexWrap == WXCore_Wrap_WrapReverse, false, left, top, right,
                       bottom);
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
    Index currentViewIndex = 0;

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
    for (auto flexLine: mFlexLines) {
      float spaceBetweenItem = 0.f;

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
          childLeft = getPaddingLeft() + getBorderWidthLeft() + (width - flexLine->mMainSize) / 2.f;
          childRight = width - getPaddingRight() - getBorderWidthRight() - (width - flexLine->mMainSize) / 2.f;
          break;
        case WXCore_Justify_Space_Around:
          Index visibleCount;
          visibleCount = flexLine->mItemCount;
          if (visibleCount != 0) {
            spaceBetweenItem = (width - flexLine->mMainSize)
                               / (float) visibleCount;
          }
          childLeft = getPaddingLeft() + getBorderWidthLeft() + spaceBetweenItem / 2.f;
          childRight = width - getPaddingRight() - getBorderWidthRight() - spaceBetweenItem / 2.f;
          break;
        case WXCore_Justify_Space_Between:
          childLeft = getPaddingLeft() + getBorderWidthLeft();
          Index visibleItem;
          visibleItem = flexLine->mItemCount;
          float denominator;
          denominator = visibleItem != 1 ? visibleItem - 1 : 1.f;
          spaceBetweenItem = (width - flexLine->mMainSize) / denominator;
          childRight = width - getPaddingRight() - getBorderWidthRight();
          break;
        default:
          childLeft = getPaddingLeft() + getBorderWidthLeft();
          childRight = width - getPaddingRight() - getBorderWidthRight();
          break;
      }

      spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

      for (Index j = 0; j < flexLine->mItemCount; j++) {

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
    Index currentViewIndex = 0;

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

    for (auto flexLine:mFlexLines) {
      float spaceBetweenItem = 0.f;

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
          Index visibleCount;
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
          Index visibleItem;
          visibleItem = flexLine->mItemCount;
          float denominator;
          denominator = visibleItem != 1 ? visibleItem - 1 : 1.f;
          spaceBetweenItem = (height - flexLine->mMainSize) / denominator;
          childBottom = height - getPaddingBottom() - getBorderWidthBottom();
          break;
        default:
          childTop = getPaddingTop() + getBorderWidthTop();
          childBottom = height - getPaddingBottom() - getBorderWidthBottom();
          break;
      }

      spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

      for (Index j = 0; j < flexLine->mItemCount; j++) {
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
                                   - node->getMarginRight()) / 2.f;
        if (!isRtl) {
          node->layout(left + leftFromCrossAxis, top, right + leftFromCrossAxis, bottom);
        } else {
          node->layout(left - leftFromCrossAxis, top, right - leftFromCrossAxis, bottom);
        }
        break;
    }
  }
}


