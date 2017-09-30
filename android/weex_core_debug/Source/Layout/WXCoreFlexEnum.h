#ifndef WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H
#define WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H

namespace WXCoreFlexLayout {

/**
 * MainAxis direction
 */
  enum WXCoreFlexDirection {
    WXCore_Flex_Direction_Column,
    WXCore_Flex_Direction_Column_Reverse,
    WXCore_Flex_Direction_Row,
    WXCore_Flex_Direction_Row_Reverse,
  };

/**
 * Controls the position of the element on the MainAxis
 */
  enum WXCoreJustifyContent {
    WXCore_Justify_Flex_Start,
    WXCore_Justify_Center,
    WXCore_Justify_Flex_End,
    WXCore_Justify_Space_Between,
    WXCore_Justify_Space_Around,
  };

/**
 * Controls the position of the element on the CrossAxis and whether Stretch
 */
  enum WXCoreAlignItems {
    WXCore_AlignItems_Flex_Start,
    WXCore_AlignItems_Center,
    WXCore_AlignItems_Flex_End,
    WXCore_AlignItems_Baseline,
    WXCore_AlignItems_Stretch,
  };

/**
 * Controls the count of flexlines
 */
  enum WXCoreFlexWrap {
    WXCore_Wrap_NoWrap,
    WXCore_Wrap_Wrap,
    WXCore_Wrap_WrapReverse,
  };

/**
 * The align-self will overrides the align-items specified by the Flex container.
 * The two attributes have the same range of values.
 */
  enum WXCoreAlignSelf {
    WXCore_AlignSelf_Auto = -1,
    WXCore_AlignSelf_Flex_Start = WXCore_AlignItems_Flex_Start,
    WXCore_AlignSelf_Center = WXCore_AlignItems_Center,
    WXCore_AlignSelf_Flex_End = WXCore_AlignItems_Flex_End,
    WXCore_AlignSelf_Baseline = WXCore_AlignItems_Baseline,
    WXCore_AlignSelf_Stretch = WXCore_AlignItems_Stretch,
  };

  enum WXCorePositionType {
    WXCore_PositionType_Relative,
    WXCore_PositionType_Absolute,
  };

  enum WXCorePositionEdge {
    WXCore_PositionEdge_Top,
    WXCore_PositionEdge_Bottom,
    WXCore_PositionEdge_Left,
    WXCore_PositionEdge_Right,
  };

  enum WXCoreMarginEdge {
    WXCore_Margin_ALL,
    WXCore_Margin_Top,
    WXCore_Margin_Bottom,
    WXCore_Margin_Left,
    WXCore_Margin_Right,
  };

  enum WXCorePaddingEdge {
    WXCore_Padding_ALL,
    WXCore_Padding_Top,
    WXCore_Padding_Bottom,
    WXCore_Padding_Left,
    WXCore_Padding_Right,
  };

  enum WXCoreBorderWidthEdge {
    WXCore_Border_Width_ALL,
    WXCore_Border_Width_Top,
    WXCore_Border_Width_Bottom,
    WXCore_Border_Width_Left,
    WXCore_Border_Width_Right,
  };
}
#endif //WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H
