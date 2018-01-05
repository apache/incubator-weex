#ifdef __cplusplus

#ifndef WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H
#define WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H

namespace WeexCore {

  /**
   * MainAxis direction
   */
  typedef enum WXCoreFlexDirection {
    kFlexDirectionColumn,
    kFlexDirectionColumnReverse,
    kFlexDirectionRow,
    kFlexDirectionRowReverse,
  } WXCoreFlexDirection;

  /**
   * Controls the position of the element on the MainAxis
   */
  typedef enum WXCoreJustifyContent {
    kJustifyFlexStart,
    kJustifyCenter,
    kJustifyFlexEnd,
    kJustifySpaceBetween,
    kJustifySpaceAround,
  } WXCoreJustifyContent;

  /**
   * Controls the position of the element on the CrossAxis and whether Stretch
   */
  typedef enum WXCoreAlignItems {
    kAlignItemsFlexStart,
    kAlignItemsCenter,
    kAlignItemsFlexEnd,
    kAlignItemsStretch,
  } WXCoreAlignItems;

  /**
   * Controls the count of flexlines
   */
  typedef enum WXCoreFlexWrap {
    kNoWrap,
    kWrap,
    kWrapReverse,
  } WXCoreFlexWrap;

  /**
   * The align-self will overrides the align-items specified by the Flex container.
   * The two attributes have the same range of values.
   */
  typedef enum WXCoreAlignSelf {
    kAlignSelfAuto = -1,
    kAlignSelfFlexStart = kAlignItemsFlexStart,
    kAlignSelfCenter = kAlignItemsCenter,
    kAlignSelfFlexEnd = kAlignItemsFlexEnd,
    kAlignSelfStretch = kAlignItemsStretch,
  } WXCoreAlignSelf;

  typedef enum WXCorePositionType {
    kRelative,
    kAbsolute,
    kFixed = kAbsolute,
  } WXCorePositionType;

  typedef enum WXCorePositionEdge {
    kPositionEdgeTop,
    kPositionEdgeBottom,
    kPositionEdgeLeft,
    kPositionEdgeRight,
  } WXCorePositionEdge;

  typedef enum WXCoreMarginEdge {
    kMarginALL,
    kMarginTop,
    kMarginBottom,
    kMarginLeft,
    kMarginRight,
  } WXCoreMarginEdge;

  typedef enum WXCorePaddingEdge {
    kPaddingALL,
    kPaddingTop,
    kPaddingBottom,
    kPaddingLeft,
    kPaddingRight,
  } WXCorePaddingEdge;

  typedef enum WXCoreBorderWidthEdge {
    kBorderWidthALL,
    kBorderWidthTop,
    kBorderWidthBottom,
    kBorderWidthLeft,
    kBorderWidthRight,
  } WXCoreBorderWidthEdge;

}
#endif //WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H
#endif