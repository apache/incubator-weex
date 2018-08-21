/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

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
    kFixed,
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

  enum WXCoreEdge{
    kTop,
    kRight,
    kBottom,
    kLeft,
  };

}
#endif //WEEXCORE_FLEXLAYOUT_WXCOREFLEXENUM_H
#endif
