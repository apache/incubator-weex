#include "WXCoreLayout.h"

using namespace WeexCore;

const float WXCoreCSSStyle::kFlexGrowDefault = 0;
const WXCoreFlexDirection WXCoreCSSStyle::kFlexDirectionDefault = kFlexDirectionColumn;
const WXCoreFlexWrap WXCoreCSSStyle::kFlexWrapDefault = kNoWrap;
const WXCoreJustifyContent WXCoreCSSStyle::kFlexJustifyContentDefault = kJustifyFlexStart;
const WXCoreAlignItems WXCoreCSSStyle::kFlexAlignItemsDefault = kAlignItemsStretch;
const WXCoreAlignSelf WXCoreCSSStyle::kFlexAlignSelfDefault = kAlignSelfAuto;


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
    layout(mCssStyle->mMargin.getMargin(kMarginLeft),
           mCssStyle->mMargin.getMargin(kMarginTop),
           mCssStyle->mMargin.getMargin(kMarginLeft) + getLayoutWidth(),
           mCssStyle->mMargin.getMargin(kMarginTop) + getLayoutHeight());
    for (Index i = 0; i < getChildCount(kBFC); ++i) {
      WXCoreLayoutNode *child = getChildAt(kBFC, i);
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
    if (mCssStyle->mPositionType == kAbsolute) {
      if (isnan(width) &&
          mParent != nullptr &&
          !isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft)) &&
          !isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeRight))) {
        width = mParent->mLayoutResult->mLayoutSize.width -
                mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft) -
                mCssStyle->mStylePosition.getPosition(kPositionEdgeRight);
        setWidthMeasureMode(kExactly);
      }

      if (isnan(mCssStyle->mStyleHeight) &&
          mParent != nullptr &&
          !isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeTop)) &&
          !isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom))) {
        height = mParent->mLayoutResult->mLayoutSize.height -
                 mCssStyle->mStylePosition.getPosition(kPositionEdgeTop) -
                 mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom);
        setHeightMeasureMode(kExactly);
      }
    }

    return std::make_pair(width, height);
  }

  void WXCoreLayoutNode::measure(const float width, const float height, const bool hypotheticalMeasurment){
    if(hypotheticalMeasurment){
      //Only BFC will enter this case.
      hypotheticalMeasure(width, height, hypotheticalMeasurment);
      syncSizeFromHypothesis();
    }

    if(getChildCount(kNonBFC) > 0){
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
        measureLeafNode(width, height, hypotheticalMeasurment);
      }
    }
    clearDirty();
  }

  void WXCoreLayoutNode::hypotheticalMeasure(const float width, const float height, const bool hypotheticalMeasurment){
    if (getChildCount(kNonBFC) > 0) {
      measureInternalNode(width, height, true, hypotheticalMeasurment);
    } else {
      measureLeafNode(width, height, hypotheticalMeasurment);
    }
    widthDirty = false;
    heightDirty = false;
    mLayoutResult->mLayoutSize.width = mLayoutResult->mLayoutSize.hypotheticalWidth;
    mLayoutResult->mLayoutSize.height = mLayoutResult->mLayoutSize.hypotheticalHeight;
  }

    void WXCoreLayoutNode::measureLeafNode(float width, float height, bool hypothetical) {
      if ((measureFunc != nullptr) &&
          (widthMeasureMode == kUnspecified
           || heightMeasureMode == kUnspecified)) {
        WXCoreSize dimension = measureFunc(this, width, widthMeasureMode, height, heightMeasureMode);
        if(widthMeasureMode == kUnspecified && isnan(width)){
          width = dimension.width + sumPaddingBorderAlongAxis(this, true);
        }
        if(heightMeasureMode == kUnspecified && isnan(height)){
          height = dimension.height + sumPaddingBorderAlongAxis(this, false);
        }
      } else {
        width = widthMeasureMode == kUnspecified ? sumPaddingBorderAlongAxis(this, true)
                                                            : width;
        height = heightMeasureMode == kUnspecified ? sumPaddingBorderAlongAxis(this, false)
                                                              : height;
      }
      setMeasuredDimension(width, height, hypothetical);
    }


  /**
   * Determine the main size by expanding the individual flexGrow attribute.
   */
  void WXCoreLayoutNode::determineMainSize(float width, float height) {
      float maxMainSize;
      bool horizontal = isMainAxisHorizontal(this);
      float paddingAlongMainAxis = sumPaddingBorderAlongAxis(this, horizontal);
      if (horizontal) {
        maxMainSize = widthMeasureMode == kUnspecified ? getLargestMainSize() : width;
      } else {
        maxMainSize = heightMeasureMode == kUnspecified ? getLargestMainSize() : height;
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
          if (heightMeasureMode == kExactly) {
            size = height - paddingAlongCrossAxis;
          }
        } else {
          if (widthMeasureMode == kExactly) {
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
      Index childCount = getChildCount(kNonBFC);
      auto *flexLine = new WXCoreFlexLine();
      flexLine->mMainSize = sumPaddingBorderAlongAxis(this, isMainAxisHorizontal(this));

      // The index of the view in a same flex line.
      for (Index i = 0; i < childCount; i++) {
        WXCoreLayoutNode *child = getChildAt(kNonBFC, i);
        if (child->mCssStyle->mAlignSelf == kAlignSelfStretch) {
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
      setMeasuredDimensionForFlex(width, widthMeasureMode, height, heightMeasureMode, hypotheticalMeasurment);
    }

    void WXCoreLayoutNode::updateCurrentFlexline(const Index childCount, WXCoreFlexLine* flexLine, const Index i, const WXCoreLayoutNode* child){
      flexLine->mMainSize += calcItemSizeAlongAxis(child, isMainAxisHorizontal(this));
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
              && mCssStyle->mAlignItems == kAlignItemsStretch) {
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

      if (node->widthMeasureMode != kUnspecified) {
        if (nodeWidth < node->mCssStyle->mMinWidth) {
          widthRemeasure = true;
          nodeWidth = node->mCssStyle->mMinWidth;
        } else if (nodeWidth > node->mCssStyle->mMaxWidth) {
          widthRemeasure = true;
          nodeWidth = node->mCssStyle->mMaxWidth;
        }
      }

      if (node->heightMeasureMode != kUnspecified) {
        if (nodeHeight < node->mCssStyle->mMinHeight) {
          heightRemeasure = true;
          nodeHeight = node->mCssStyle->mMinHeight;
        } else if (nodeHeight > node->mCssStyle->mMaxHeight) {
          heightRemeasure = true;
          nodeHeight = node->mCssStyle->mMaxHeight;
        }
      }

      node->setWidthMeasureMode(widthRemeasure ? kExactly : node->widthMeasureMode);
      node->setHeightMeasureMode(heightRemeasure ? kExactly : node->heightMeasureMode);

      if (hypotheticalMeasurment) {
        if (widthRemeasure) {
          node->mLayoutResult->mLayoutSize.hypotheticalWidth = nodeWidth;
          node->markDirty(false);
          node->widthDirty = true;
        }
        if (heightRemeasure) {
          node->mLayoutResult->mLayoutSize.hypotheticalHeight = nodeHeight;
          node->markDirty(false);
          node->heightDirty = true;
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
          WXCoreLayoutNode *child = getChildAt(kNonBFC, childIndex);
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
        child->setWidthMeasureMode(kExactly);
        child->setLayoutWidth(childMainSize);
      } else {
        child->setHeightMeasureMode(kExactly);
        child->setLayoutHeight(childMainSize);
      }
    }

    void WXCoreLayoutNode::stretchViewCrossSize(){
      if (mCssStyle->mAlignItems == kAlignItemsStretch) {
        Index viewIndex = 0;
        for (Index i = 0; i< mFlexLines.size(); i++ ) {
            auto flexLine = mFlexLines.at(i);
            for (Index j = 0; j < flexLine->mItemCount; j++, viewIndex++) {
                WXCoreLayoutNode* child = getChildAt(kNonBFC, viewIndex);
              if (child->mCssStyle->mAlignSelf == kAlignSelfAuto ||
                  child->mCssStyle->mAlignSelf == kAlignSelfStretch) {
                stretchViewCrossSize(child, flexLine->mCrossSize);
              }
          }
        }
      } else {
        for (auto flexLine : mFlexLines) {
          for (auto index : flexLine->mIndicesAlignSelfStretch) {
            stretchViewCrossSize(getChildAt(kNonBFC, index), flexLine->mCrossSize);
          }
        }
      }
    }

    void WXCoreLayoutNode::stretchViewCrossSize(WXCoreLayoutNode* child, float crossSize){
      if (isMainAxisHorizontal(this)) {
        if (child->heightMeasureMode != kExactly) {
          child->setHeightMeasureMode(kExactly);
          child->setLayoutHeight(std::max(0.f, crossSize));
        }
      } else {
        if (child->widthMeasureMode != kExactly) {
          child->setWidthMeasureMode(kExactly);
          child->setLayoutWidth(std::max(0.f, crossSize));
        }
      }
    }

    void WXCoreLayoutNode::setFrame(const float l, const float t, const float r, const float b) {
      if (mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeLeft) != l
          || mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeTop) != t
          || mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeRight) != r
          || mLayoutResult->mLayoutPosition.getPosition(kPositionEdgeBottom) != b) {
        setHasNewLayout(true);
        mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeLeft, l);
        mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeTop, t);
        mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeRight, r);
        mLayoutResult->mLayoutPosition.setPosition(kPositionEdgeBottom, b);
      }
    }

  void WXCoreLayoutNode::layout(float left, float top, float right, float bottom) {
    switch (mCssStyle->mPositionType) {
      case kAbsolute:
        calcAbsoluteOffset(left, top, right, bottom);
        break;
      default:
      case kRelative:
        calcRelativeOffset(left, top, right, bottom);
        break;
    }
    setFrame(left, top, right, bottom);
    onLayout(left, top, right, bottom);
  }

  void WXCoreLayoutNode::calcRelativeOffset(float &left, float &top, float &right, float &bottom) {
    if (!isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft))) {
      left += mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft);
      right += mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft);
    } else if (!isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeRight))) {
      left -= mCssStyle->mStylePosition.getPosition(kPositionEdgeRight);
      right -= mCssStyle->mStylePosition.getPosition(kPositionEdgeRight);
    }

    if (!isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeTop))) {
      top += mCssStyle->mStylePosition.getPosition(kPositionEdgeTop);
      bottom += mCssStyle->mStylePosition.getPosition(kPositionEdgeTop);
    } else if (!isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom))) {
      top -= mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom);
      bottom -= mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom);
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

    if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeLeft))) {
      if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeRight))) {
        left += parentBorder.getBorderWidth(kBorderWidthLeft) + parentPadding.getPadding(kPaddingLeft);
        right += parentBorder.getBorderWidth(kBorderWidthLeft) + parentPadding.getPadding(kPaddingLeft);
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

    if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeTop))) {
      if (isnan(mCssStyle->mStylePosition.getPosition(kPositionEdgeBottom))) {
        top += parentBorder.getBorderWidth(kBorderWidthTop) + parentPadding.getPadding(kPaddingTop);
        bottom += parentBorder.getBorderWidth(kBorderWidthTop) + parentPadding.getPadding(kPaddingTop);
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

  void WXCoreLayoutNode::onLayout(float left, float top, float right, float bottom) {
    switch (mCssStyle->mFlexDirection) {
      case kFlexDirectionRow:
        layoutHorizontal(false, left, top, right, bottom);
        break;
      case kFlexDirectionRowReverse:
        layoutHorizontal(true, left, top, right, bottom);
        break;
      case kFlexDirectionColumnReverse:
        layoutVertical(mCssStyle->mFlexWrap == kWrapReverse, true, left, top, right,
                       bottom);
        break;
      case kFlexDirectionColumn:
      default:
        layoutVertical(mCssStyle->mFlexWrap == kWrapReverse, false, left, top, right,
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
        case kJustifyFlexStart:
          childLeft = getPaddingLeft() + getBorderWidthLeft();
          childRight = width - getPaddingRight() - getBorderWidthRight();
          break;
        case kJustifyFlexEnd:
          childLeft = width - flexLine->mMainSize + getPaddingRight() + getBorderWidthRight();
          childRight = flexLine->mMainSize - getPaddingLeft() - getBorderWidthLeft();
          break;
        case kJustifyCenter:
          childLeft = getPaddingLeft() + getBorderWidthLeft() + (width - flexLine->mMainSize) / 2.f;
          childRight = width - getPaddingRight() - getBorderWidthRight() - (width - flexLine->mMainSize) / 2.f;
          break;
        case kJustifySpaceAround:
          Index visibleCount;
          visibleCount = flexLine->mItemCount;
          if (visibleCount != 0) {
            spaceBetweenItem = (width - flexLine->mMainSize)
                               / (float) visibleCount;
          }
          childLeft = getPaddingLeft() + getBorderWidthLeft() + spaceBetweenItem / 2.f;
          childRight = width - getPaddingRight() - getBorderWidthRight() - spaceBetweenItem / 2.f;
          break;
        case kJustifySpaceBetween:
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

        WXCoreLayoutNode *child = getChildAt(kNonBFC, currentViewIndex);

        if (child == nullptr) {
          continue;
        }

        childLeft += child->getMarginLeft();
        childRight -= child->getMarginRight();

        if (mCssStyle->mFlexWrap == kWrapReverse) {
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
    if (node->mCssStyle->mAlignSelf != kAlignSelfAuto) {
      // Expecting the values for alignItems and alignSelf match except for ALIGN_SELF_AUTO.
      // Assigning the alignSelf value as alignItems should work.
      alignItems = static_cast<WXCoreAlignItems>(node->mCssStyle->mAlignSelf);
    }

    float crossSize = flexLine->mCrossSize;

    switch (alignItems) {
      case kAlignItemsFlexStart:
      case kAlignItemsStretch:
        if (flexWrap != kWrapReverse) {
          node->layout(left, top + node->getMarginTop(), right, bottom + node->getMarginTop());
        } else {
          node->layout(left, top - node->getMarginBottom(), right, bottom - node->getMarginBottom());
        }
        break;
      case kAlignItemsFlexEnd:
        if (flexWrap != kWrapReverse) {
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
      case kAlignItemsCenter:
        float topFromCrossAxis = (crossSize - node->mLayoutResult->mLayoutSize.height
                                  + node->getMarginTop() - node->getMarginBottom()) / 2;
        if (flexWrap != kWrapReverse) {
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
        case kJustifyFlexStart:
          childTop = getPaddingTop() + getBorderWidthTop();
          childBottom = height - getPaddingBottom() - getBorderWidthBottom();
          break;
        case kJustifyFlexEnd:
          childTop = height - flexLine->mMainSize + getPaddingBottom() + getBorderWidthBottom();
          childBottom = flexLine->mMainSize - getPaddingTop() - getBorderWidthTop();
          break;
        case kJustifyCenter:
          childTop = getPaddingTop() + getBorderWidthTop() + (height - flexLine->mMainSize) / 2;
          childBottom = height - getPaddingBottom() - getBorderWidthBottom() - (height - flexLine->mMainSize) / 2;
          break;
        case kJustifySpaceAround:
          Index visibleCount;
          visibleCount = flexLine->mItemCount;
          if (visibleCount != 0) {
            spaceBetweenItem = (height - flexLine->mMainSize)
                               / (float) visibleCount;
          }
          childTop = getPaddingTop() + getBorderWidthTop() + spaceBetweenItem / 2;
          childBottom = height - getPaddingBottom() - getBorderWidthBottom() - spaceBetweenItem / 2;
          break;
        case kJustifySpaceBetween:
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
        WXCoreLayoutNode *child = getChildAt(kNonBFC, currentViewIndex);

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
    if (node->mCssStyle->mAlignSelf != kAlignSelfAuto) {
      // Expecting the values for alignItems and alignSelf match except for ALIGN_SELF_AUTO.
      // Assigning the alignSelf value as alignItems should work.
      alignItems = static_cast<WXCoreAlignItems>(node->mCssStyle->mAlignSelf);
    }

    float crossSize = flexLine->mCrossSize;

    switch (alignItems) {
      case kAlignItemsFlexStart:
      case kAlignItemsStretch:
        if (!isRtl) {
          node->layout(left + node->getMarginLeft(), top, right + node->getMarginLeft(), bottom);
        } else {
          node->layout(left - node->getMarginRight(), top, right - node->getMarginRight(), bottom);
        }
        break;
      case kAlignItemsFlexEnd:
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
      case kAlignItemsCenter:
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


