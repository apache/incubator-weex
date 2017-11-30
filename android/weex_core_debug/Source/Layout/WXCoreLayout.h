#ifdef __cplusplus

#ifndef WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#define WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H

#include "WXCoreStyle.h"
#include "WXCoreFlexEnum.h"
#include <vector>
#include <iostream>
#include <string>

namespace WXCoreFlexLayout {

  class WXCoreLayoutNode;

  class WXCoreFlexLine;

  typedef enum FormatingContext {
    BFC,
    NON_BFC,
  } FormatingContext;

  typedef enum MeasureMode {
    AT_MOST,
    EXACTLY,
  } MeasureMode;

  typedef struct WXCoreSize {
    float width;
    float height;

    void reset() {
      width = 0;
      height = 0;
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

  typedef struct WXBFCDimension {
    float width;
    MeasureMode widthMeasureMode;
    float height;
    MeasureMode heightMeasureMode;

    void reset() {
      width = 0;
      height = 0;
    }
  } WXBFCDimension;

  typedef WXCoreSize(*WXCoreMeasureFunc)(WXCoreLayoutNode *node, float width,
                                         MeasureMode widthMeasureMode,
                                         float height, MeasureMode heightMeasureMode);

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

  private:

    /**
     * Holds the 'frozen' state of children during measure. If a view is frozen it will no longer
     * expand regardless of mFlexGrow. Items are indexed by the child's
     * reordered index.
     */
    bool *mChildrenFrozen;

    uint32_t mChildrenFrozen_oldlength;

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

    bool mIsDirty;

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
      node->dirty();
      return node;
    }

    WXCoreLayoutNode() :
        mChildrenFrozen(nullptr),
        mChildrenFrozen_oldlength(0),
        mParent(nullptr),
        mHasNewLayout(true),
        mIsDirty(true),
        mVisible(true),
        measureFunc(nullptr) {
      mCssStyle = new WXCoreCSSStyle();
      mLayoutResult = new WXCorelayoutResult();
      mLastAvailableSize = nullptr;
    }

    inline void freeWXCoreNode() {
      mHasNewLayout = true;
      mIsDirty = true;
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

//      if (mCssStyle != nullptr) {
//        delete mCssStyle;
//        mCssStyle = nullptr;
//      }

      if (mLayoutResult != nullptr) {
        delete mLayoutResult;
        mLayoutResult = nullptr;
      }

      if (mLastAvailableSize != nullptr) {
        delete mLastAvailableSize;
        mLastAvailableSize = nullptr;
      }
    }

    inline void reset() {
      mLayoutResult->reset();
      dirty();
      for (int i = 0; i < getChildCount(NON_BFC); i++) {
        WXCoreLayoutNode *child = getChildAt(NON_BFC, i);
        child->reset();
      }
    }

    inline void resetLayoutResult() {
      mLayoutResult->reset();
      dirty();
    }

    inline void copyStyle(WXCoreLayoutNode *srcNode) {
      if (memcmp(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle)) != 0) {
        memcpy(mCssStyle, srcNode->mCssStyle, sizeof(WXCoreCSSStyle));
        dirty();
      }
    }

    inline void copyMeasureFunc(WXCoreLayoutNode *srcNode) {
      if (memcmp(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc)) != 0) {
        memcpy(&measureFunc, &srcNode->measureFunc, sizeof(WXCoreMeasureFunc));
        dirty();
      }
    }

    inline void copyNode(WXCoreLayoutNode *srcNode) {
      copyStyle(srcNode);
      copyMeasureFunc(srcNode);
      for (WXCoreLayoutNode *node : mChildList) {
        WXCoreLayoutNode *temp = newWXCoreNode();
        memcpy(&node, &temp, sizeof(WXCoreLayoutNode));
        srcNode->appendChild(temp);
      }
      dirty();
    }

    /** ================================ measureFunc =================================== **/

    inline void setMeasureFunc(WXCoreMeasureFunc measure) {
      measureFunc = measure;
      dirty();
    }

    inline WXCoreMeasureFunc getMeasureFunc() {
      return measureFunc;
    }

    inline bool haveMeasureFunc() {
      return measureFunc == nullptr ? false : true;
    }

    /** ================================ context =================================== **/


    inline void *getContext() {
      return context;
    }

    inline void setContext(void *context) {
      this->context = context;
    }

  private:

    /** ================================ measure =================================== **/

    void initMeasureMode();

    inline void setLayoutWidth(float width) {
      if (mLayoutResult->mLayoutSize.width != width) {
        mLayoutResult->mLayoutSize.width = width;
        setHasNewLayout(true);
      }
    }

    inline void setLayoutHeight(float height) {
      if (mLayoutResult->mLayoutSize.height != height) {
        mLayoutResult->mLayoutSize.height = height;
        setHasNewLayout(true);
      }
    }

    void
    measure(float width, MeasureMode widthMeasureMode, float height, MeasureMode heightMeasureMode,
            bool useMeasureFunc);

    void onMeasure(float width, MeasureMode widthMeasureMode, float height,
                   MeasureMode heightMeasureMode);

    void measureHorizontal(float width, MeasureMode widthMeasureMode, float height,
                           MeasureMode heightMeasureMode);

    void measureVertical(float width, MeasureMode widthMeasureMode, float height,
                         MeasureMode heightMeasureMode);

    void checkSizeConstraints(WXCoreLayoutNode *node);

    void
    determineMainSize(WXCoreFlexDirection flexDirection, float width, MeasureMode widthMeasureMode,
                      float height, MeasureMode heightMeasureMode);

    uint32_t expandFlexItems(WXCoreFlexLine *flexLine,
                             WXCoreFlexDirection flexDirection, float maxMainSize,
                             float paddingAlongMainAxis,
                             uint32_t startIndex, bool calledRecursively);

    void setMeasuredDimensionForFlex(WXCoreFlexDirection flexDirection,
                                     float width, MeasureMode widthMeasureMode,
                                     float height, MeasureMode heightMeasureMode);

    void
    determineCrossSize(WXCoreFlexDirection flexDirection, float width, MeasureMode widthMeasureMode,
                       float height, MeasureMode heightMeasureMode, float paddingAlongCrossAxis);

    void stretchViews(WXCoreFlexDirection flexDirection, WXCoreAlignItems alignItems);

    void stretchViewVertically(WXCoreLayoutNode *node, float crossSize);

    void stretchViewHorizontally(WXCoreLayoutNode *node, float crossSize);

    WXCoreSize getChildDimension(float width, float height, WXCoreLayoutNode *child,
                                 float totalCrossSize);

    bool isWrapRequired(float mainSize, float currentLength, float childLength);

    bool isMainAxisDirectionHorizontal(WXCoreFlexDirection flexDirection);

    void
    addFlexLineIfLastFlexItem(uint32_t childIndex, uint32_t childCount, WXCoreFlexLine *flexLine,
                              float usedCrossSizeSoFar);

    void addFlexLine(WXCoreFlexLine *flexLine, float usedCrossSizeSoFar);

    inline bool
    obeyConstrainsDuringExpandMainAxis(WXCoreFlexLine *flexLine, uint32_t childIndex,
                                       float flexGrow) {
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

    inline float getSumOfCrossSize() {
      float sum = 0;
      for (WXCoreFlexLine *flexLine: mFlexLines) {
        sum += flexLine->mCrossSize;
      }
      return sum;
    }

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

    inline void setFrame(float l, float t, float r, float b) {
      if (mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Left) != l
          || mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Top) != t
          || mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Right) != r
          || mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Bottom) != b) {
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Left, l);
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Top, t);
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Right, r);
        mLayoutResult->mLayoutPosition.setPosition(WXCore_PositionEdge_Bottom, b);
        setHasNewLayout(true);
      }
    }

    virtual void onLayoutBefore() {

    }

    virtual void onLayoutAfter(float width, float height) {

    }


  public:

    /** ================================ tree =================================== **/

    inline uint32_t getChildCount(FormatingContext formatingContext) {
      switch (formatingContext) {
        case NON_BFC:
          return NonBFCs.size();
        case BFC:
          return BFCs.size();
        default:
          return mChildList.size();
      }
    }

    inline uint32_t getChildCount() {
      return mChildList.size();
    }

    inline void removeChildAt(uint32_t index) {
      mChildList.erase(mChildList.begin() + index);
      dirty();
    }

    inline void removeChild(WXCoreLayoutNode *child) {
      for (int index = 0; index < mChildList.size(); index++) {
        if (child == mChildList[index]) {
          mChildList.erase(mChildList.begin() + index);
          break;
        }
      }
      dirty();
    }

    inline void addChildAt(WXCoreLayoutNode *child, uint32_t index) {
      mChildList.insert(mChildList.begin() + index, child);
      child->mParent = this;
      dirty();
    }

    inline void appendChild(WXCoreLayoutNode *child) {
      mChildList.push_back(child);
      child->mParent = this;
      dirty();
    }

    inline WXCoreLayoutNode *getChildAt(FormatingContext formatingContext, uint32_t index) {
      switch (formatingContext) {
        case NON_BFC:
          return NonBFCs[index];
        case BFC:
          return BFCs[index];
        default:
          return mChildList[index];
      }
    }

    inline WXCoreLayoutNode *getChildAt(uint32_t index) {
      return mChildList[index];
    }

    inline WXCoreLayoutNode *getParent() {
      return mParent;
    }

    void initFormatingContext(std::vector<WXCoreLayoutNode *> &BFCs);

    WXBFCDimension calculateBFCDimension();

    inline bool isBFC(WXCoreLayoutNode *node) {
      return node->mCssStyle->mPositionType == WXCore_PositionType_Absolute;
    }


    /** ================================ margin =================================== **/

    inline float getMarginTop() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Top);
    }

    inline float getMarginBottom() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Bottom);
    }

    inline float getMarginLeft() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Left);
    }

    inline float getMarginRight() {
      return mCssStyle->mMargin.getMargin(WXCore_Margin_Right);
    }

    inline void setMargin(WXCoreMarginEdge edge, float margin) {
      if (mCssStyle->mMargin.setMargin(edge, margin)) {
        dirty();
        setHasNewLayout(true);
      }
    }

    /** ================================ padding =================================== **/

    inline float getPaddingLeft() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Left);
    }

    inline float getPaddingRight() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Right);
    }

    inline float getPaddingTop() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Top);
    }

    inline float getPaddingBottom() {
      return mCssStyle->mPadding.getPadding(WXCore_Padding_Bottom);
    }

    inline void setPadding(WXCorePaddingEdge edge, float padding) {
      if (mCssStyle->mPadding.setPadding(edge, padding)) {
        dirty();
        setHasNewLayout(true);
      }
    }


    /** ================================ border-width =================================== **/

    inline float getBorderWidthLeft() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Left);
    }

    inline float getBorderWidthRight() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Right);
    }

    inline float getBorderWidthTop() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Top);
    }

    inline float getBorderWidthBottom() {
      return mCssStyle->mBorderWidth.getBorderWidth(WXCore_Border_Width_Bottom);
    }

    inline void setBorderWidth(WXCoreBorderWidthEdge edge, float borderWidth) {
      if (mCssStyle->mBorderWidth.setBorderWidth(edge, borderWidth)) {
        dirty();
        setHasNewLayout(true);
      }
    }


    /** ================================ position-type =================================== **/

    inline void setStylePositionType(WXCorePositionType positionType) {
      if (mCssStyle->mPositionType != positionType) {
        mCssStyle->mPositionType = positionType;
        dirty();
      }
    }

    inline WXCorePositionType getStypePositionType() {
      return mCssStyle->mPositionType;
    }


    /** ================================ position =================================== **/

    inline float getStylePositionTop() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Top);
    }

    inline float getStylePositionBottom() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Bottom);
    }

    inline float getStylePositionLeft() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Left);
    }

    inline float getStylePositionRight() {
      return mCssStyle->mStylePosition.getPosition(WXCore_PositionEdge_Right);
    }

    inline void setStylePosition(WXCorePositionEdge edge, float positionRight) {
      if (mCssStyle->mStylePosition.setPosition(edge, positionRight))
        dirty();
    }


    /** ================================ dimension =================================== **/

    inline void setStyleWidth(float width) {
      if (mCssStyle->mStyleWidth != width) {
        mCssStyle->mStyleWidth = width;
        dirty();
      }
    }

    inline float getStyleWidth() {
      return mCssStyle->mStyleWidth;
    }

    inline void setStyleHeight(float height) {
      if (mCssStyle->mStyleHeight != height) {
        mCssStyle->mStyleHeight = height;
        dirty();
      }
    }

    inline float getStyleHeight() {
      return mCssStyle->mStyleHeight;
    }

    inline void setMinWidth(float minWidth) {
      if (mCssStyle->mMinWidth != minWidth) {
        mCssStyle->mMinWidth = minWidth;
        dirty();
      }
    }

    inline float getMinWidth() {
      return mCssStyle->mMinWidth;
    }

    inline void setMaxWidth(float maxWidth) {
      if (mCssStyle->mMaxWidth != maxWidth) {
        mCssStyle->mMaxWidth = maxWidth;
        dirty();
      }
    }

    inline float getMaxWidth() {
      return mCssStyle->mMaxWidth;
    }

    inline void setMinHeight(float minHeight) {
      if (mCssStyle->mMinHeight != minHeight) {
        mCssStyle->mMinHeight = minHeight;
        dirty();
      }
    }

    inline float getMinHeight() {
      return mCssStyle->mMinHeight;
    }

    inline void setMaxHeight(float maxHeight) {
      if (mCssStyle->mMaxHeight != maxHeight) {
        mCssStyle->mMaxHeight = maxHeight;
        dirty();
      }
    }

    inline float getMaxHeight() {
      return mCssStyle->mMaxHeight;
    }


    /** ================================ flex-style =================================== **/

    inline void setFlexDirection(WXCoreFlexDirection flexDirection) {
      if (mCssStyle->mFlexDirection != flexDirection) {
        mCssStyle->mFlexDirection = flexDirection;
        dirty();
      }
    }

    inline WXCoreFlexDirection getFlexDirection() {
      return mCssStyle->mFlexDirection;
    }

    inline void setFlexWrap(WXCoreFlexWrap flexWrap) {
      if (mCssStyle->mFlexWrap != flexWrap) {
        mCssStyle->mFlexWrap = flexWrap;
        dirty();
      }
    }

    inline WXCoreFlexWrap getFlexWrap() {
      return mCssStyle->mFlexWrap;
    }

    inline void setJustifyContent(WXCoreJustifyContent justifyContent) {
      if (mCssStyle->mJustifyContent != justifyContent) {
        mCssStyle->mJustifyContent = justifyContent;
        dirty();
      }
    }

    inline WXCoreJustifyContent getJustifyContent() {
      return mCssStyle->mJustifyContent;
    }

    inline void setAlignItems(WXCoreAlignItems alignItems) {
      if (mCssStyle->mAlignItems != alignItems) {
        mCssStyle->mAlignItems = alignItems;
        dirty();
      }
    }

    inline WXCoreAlignItems getAlignItems() {
      return mCssStyle->mAlignItems;
    }

    inline void setAlignSelf(WXCoreAlignSelf alignSelf) {
      if (mCssStyle->mAlignSelf != alignSelf) {
        mCssStyle->mAlignSelf = alignSelf;
        dirty();
      }
    }

    inline WXCoreAlignSelf getAlignSelf() {
      return mCssStyle->mAlignSelf;
    }

    inline void setFlex(float flex) {
      if (mCssStyle->mFlexGrow != flex) {
        mCssStyle->mFlexGrow = flex;
        dirty();
      }
    }

    inline float getFlex() {
      return mCssStyle->mFlexGrow;
    }

    /** ================================ layout-result =================================== **/

  public:

    inline float getLayoutWidth() {
      return mLayoutResult->mLayoutSize.width;
    }

    inline float getLayoutHeight() {
      return mLayoutResult->mLayoutSize.height;
    }

    inline float getLayoutPositionTop() {
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Top);
    }

    inline float getLayoutPositionBottom() {
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Bottom);
    }

    inline float getLayoutPositionLeft() {
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Left);
    }

    inline float getLayoutPositionRight() {
      return mLayoutResult->mLayoutPosition.getPosition(WXCore_PositionEdge_Right);
    }

  private:

    inline void setMeasuredDimension(float width, float height) {
      setLayoutWidth(width);
      setLayoutHeight(height);
    }


    /** ================================ other =================================== **/

  private:

    inline void resetDirty() {
      mIsDirty = false;
    }

  public:

    inline bool hasNewLayout() {
      return mHasNewLayout;
    }

    inline bool isDirty() {
      return mIsDirty;
    }

    inline void dirty() {
      if (!isDirty()) {
        mIsDirty = true;
        if (getParent() != nullptr) {
          getParent()->dirty();
        }
      }
    }

    inline void setHasNewLayout(bool hasNewLayout) {
      this->mHasNewLayout = hasNewLayout;
    }

    inline bool isVisible() {
      return mVisible;
    }

    inline void setVisible(bool visible) {
      mVisible = visible;
    }

    inline bool isUndefined(float value) {
      return isnan(value);
    }

  private:
    float getLargestMainSize();
  };
}
#endif //WEEXCORE_FLEXLAYOUT_WXCORELAYOUTNODE_H
#endif