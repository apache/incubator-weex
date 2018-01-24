#ifdef __cplusplus

#ifndef WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H
#define WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H

namespace WeexCore {

  /**
   * MainAxis direction
   */
  enum WXCoreFlexDirection {
    kFlexDirectionColumn,
    kFlexDirectionColumnReverse,
    kFlexDirectionRow,
    kFlexDirectionRowReverse,
  };

  /**
   * Controls the position of the element on the MainAxis
   */
  enum WXCoreJustifyContent {
    kJustifyFlexStart,
    kJustifyCenter,
    kJustifyFlexEnd,
    kJustifySpaceBetween,
    kJustifySpaceAround,
  };

  /**
   * Controls the position of the element on the CrossAxis and whether Stretch
   */
  enum WXCoreAlignItems {
    kAlignItemsFlexStart,
    kAlignItemsCenter,
    kAlignItemsFlexEnd,
    kAlignItemsStretch,
  };

  /**
   * Controls the count of flexlines
   */
  enum WXCoreFlexWrap {
    kNoWrap,
    kWrap,
    kWrapReverse,
  };

  /**
   * The align-self will overrides the align-items specified by the Flex container.
   * The two attributes have the same range of values.
   */
  enum WXCoreAlignSelf {
    kAlignSelfAuto = -1,
    kAlignSelfFlexStart = kAlignItemsFlexStart,
    kAlignSelfCenter = kAlignItemsCenter,
    kAlignSelfFlexEnd = kAlignItemsFlexEnd,
    kAlignSelfStretch = kAlignItemsStretch,
  };

  enum WXCorePositionType {
    kRelative,
    kAbsolute,
    kFixed = kAbsolute,
    kSticky = kRelative
  };

  enum WXCorePositionEdge {
    kPositionEdgeTop,
    kPositionEdgeBottom,
    kPositionEdgeLeft,
    kPositionEdgeRight,
  };

  enum WXCoreMarginEdge {
    kMarginALL,
    kMarginTop,
    kMarginBottom,
    kMarginLeft,
    kMarginRight,
  };

  enum WXCorePaddingEdge {
    kPaddingALL,
    kPaddingTop,
    kPaddingBottom,
    kPaddingLeft,
    kPaddingRight,
  };

  enum WXCoreBorderWidthEdge {
    kBorderWidthALL,
    kBorderWidthTop,
    kBorderWidthBottom,
    kBorderWidthLeft,
    kBorderWidthRight,
  };

}
#endif //WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H
#endif