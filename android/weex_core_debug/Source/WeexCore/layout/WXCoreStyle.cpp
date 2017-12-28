#include "WXCoreStyle.h"

namespace WeexCore {

  bool WXCoreMargin::setMargin(const WXCoreMarginEdge &edge, const float &margin) {
    bool dirty = false;
    switch (edge) {
      case WXCore_Margin_ALL:
        if (mMarginLeft != margin
            || mMarginTop != margin
            || mMarginRight != margin
            || mMarginBottom != margin) {
          mMarginLeft = margin;
          mMarginTop = margin;
          mMarginRight = margin;
          mMarginBottom = margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Left:
        if (mMarginLeft != margin) {
          mMarginLeft = margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Top:
        if (mMarginTop != margin) {
          mMarginTop = margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Right:
        if (mMarginRight != margin) {
          mMarginRight = margin;
          dirty = true;
        }
        break;
      case WXCore_Margin_Bottom:
        if (mMarginBottom != margin) {
          mMarginBottom = margin;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float WXCoreMargin::getMargin(const WXCoreMarginEdge &edge) {
    float margin = 0;
    switch (edge) {
      case WXCore_Margin_Left:
        margin = mMarginLeft;
        break;
      case WXCore_Margin_Top:
        margin = mMarginTop;
        break;
      case WXCore_Margin_Right:
        margin = mMarginRight;
        break;
      case WXCore_Margin_Bottom:
        margin = mMarginBottom;
        break;
      default:
        break;
    }
    return margin;
  }

  bool WXCorePadding::setPadding(const WXCorePaddingEdge &edge, const float &padding) {
    bool dirty = false;
    switch (edge) {
      case WXCore_Padding_ALL:
        if (mPaddingLeft != padding
            || mPaddingTop != padding
            || mPaddingRight != padding
            || mPaddingBottom != padding) {
          mPaddingLeft = padding;
          mPaddingTop = padding;
          mPaddingRight = padding;
          mPaddingBottom = padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Left:
        if (mPaddingLeft != padding) {
          mPaddingLeft = padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Top:
        if (mPaddingTop != padding) {
          mPaddingTop = padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Right:
        if (mPaddingRight != padding) {
          mPaddingRight = padding;
          dirty = true;
        }
        break;
      case WXCore_Padding_Bottom:
        if (mPaddingBottom != padding) {
          mPaddingBottom = padding;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float WXCorePadding::getPadding(const WXCorePaddingEdge &edge) {
    float padding = 0;
    switch (edge) {
      case WXCore_Padding_Left:
        padding = mPaddingLeft;
        break;
      case WXCore_Padding_Top:
        padding = mPaddingTop;
        break;
      case WXCore_Padding_Right:
        padding = mPaddingRight;
        break;
      case WXCore_Padding_Bottom:
        padding = mPaddingBottom;
        break;
      default:
        break;
    }
    return padding;
  }

  bool WXCoreBorderWidth::setBorderWidth(const WXCoreBorderWidthEdge &edge, const float &borderWidth) {
    bool dirty = false;
    switch (edge) {
      case WXCore_Border_Width_ALL:
        if (mBorderWidthLeft != borderWidth
            || mBorderWidthTop != borderWidth
            || mBorderWidthRight != borderWidth
            || mBorderWidthBottom != borderWidth) {
          mBorderWidthLeft = borderWidth;
          mBorderWidthTop = borderWidth;
          mBorderWidthRight = borderWidth;
          mBorderWidthBottom = borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Left:
        if (mBorderWidthLeft != borderWidth) {
          mBorderWidthLeft = borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Top:
        if (mBorderWidthTop != borderWidth) {
          mBorderWidthTop = borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Right:
        if (mBorderWidthRight != borderWidth) {
          mBorderWidthRight = borderWidth;
          dirty = true;
        }
        break;
      case WXCore_Border_Width_Bottom:
        if (mBorderWidthBottom != borderWidth) {
          mBorderWidthBottom = borderWidth;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float WXCoreBorderWidth::getBorderWidth(const WXCoreBorderWidthEdge &edge) {
    float borderWidth = 0;
    switch (edge) {
      case WXCore_Border_Width_Left:
        borderWidth = mBorderWidthLeft;
        break;
      case WXCore_Border_Width_Top:
        borderWidth = mBorderWidthTop;
        break;
      case WXCore_Border_Width_Right:
        borderWidth = mBorderWidthRight;
        break;
      case WXCore_Border_Width_Bottom:
        borderWidth = mBorderWidthBottom;
        break;
      default:
        break;
    }
    return borderWidth;
  }

  bool WXCorePosition::setPosition(const WXCorePositionEdge &edge, const float &position) {
    bool dirty = false;
    switch (edge) {
      case WXCore_PositionEdge_Left:
        if (mLeft != position) {
          mLeft = position;
          dirty = true;
        }
        break;
      case WXCore_PositionEdge_Top:
        if (mTop != position) {
          mTop = position;
          dirty = true;
        }
        break;
      case WXCore_PositionEdge_Right:
        if (mRight != position) {
          mRight = position;
          dirty = true;
        }
        break;
      case WXCore_PositionEdge_Bottom:
        if (mBottom != position) {
          mBottom = position;
          dirty = true;
        }
        break;
    }
    return dirty;
  }

  float WXCorePosition::getPosition(const WXCorePositionEdge &edge) {
    float position = 0;
    switch (edge) {
      case WXCore_PositionEdge_Left:
        position = mLeft;
        break;
      case WXCore_PositionEdge_Top:
        position = mTop;
        break;
      case WXCore_PositionEdge_Right:
        position = mRight;
        break;
      case WXCore_PositionEdge_Bottom:
        position = mBottom;
        break;
    }
    return position;
  }

}
