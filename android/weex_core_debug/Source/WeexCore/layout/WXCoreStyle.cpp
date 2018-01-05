#include "WXCoreStyle.h"

namespace WeexCore {

  bool WXCoreMargin::setMargin(const WXCoreMarginEdge &edge, const float &margin) {
    bool dirty = false;
    switch (edge) {
      case kMarginALL:
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
      case kMarginLeft:
        if (mMarginLeft != margin) {
          mMarginLeft = margin;
          dirty = true;
        }
        break;
      case kMarginTop:
        if (mMarginTop != margin) {
          mMarginTop = margin;
          dirty = true;
        }
        break;
      case kMarginRight:
        if (mMarginRight != margin) {
          mMarginRight = margin;
          dirty = true;
        }
        break;
      case kMarginBottom:
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
      case kMarginLeft:
        margin = mMarginLeft;
        break;
      case kMarginTop:
        margin = mMarginTop;
        break;
      case kMarginRight:
        margin = mMarginRight;
        break;
      case kMarginBottom:
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
      case kPaddingALL:
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
      case kPaddingLeft:
        if (mPaddingLeft != padding) {
          mPaddingLeft = padding;
          dirty = true;
        }
        break;
      case kPaddingTop:
        if (mPaddingTop != padding) {
          mPaddingTop = padding;
          dirty = true;
        }
        break;
      case kPaddingRight:
        if (mPaddingRight != padding) {
          mPaddingRight = padding;
          dirty = true;
        }
        break;
      case kPaddingBottom:
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
      case kPaddingLeft:
        padding = mPaddingLeft;
        break;
      case kPaddingTop:
        padding = mPaddingTop;
        break;
      case kPaddingRight:
        padding = mPaddingRight;
        break;
      case kPaddingBottom:
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
      case kBorderWidthALL:
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
      case kBorderWidthLeft:
        if (mBorderWidthLeft != borderWidth) {
          mBorderWidthLeft = borderWidth;
          dirty = true;
        }
        break;
      case kBorderWidthTop:
        if (mBorderWidthTop != borderWidth) {
          mBorderWidthTop = borderWidth;
          dirty = true;
        }
        break;
      case kBorderWidthRight:
        if (mBorderWidthRight != borderWidth) {
          mBorderWidthRight = borderWidth;
          dirty = true;
        }
        break;
      case kBorderWidthBottom:
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
      case kBorderWidthLeft:
        borderWidth = mBorderWidthLeft;
        break;
      case kBorderWidthTop:
        borderWidth = mBorderWidthTop;
        break;
      case kBorderWidthRight:
        borderWidth = mBorderWidthRight;
        break;
      case kBorderWidthBottom:
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
      case kPositionEdgeLeft:
        if (mLeft != position) {
          mLeft = position;
          dirty = true;
        }
        break;
      case kPositionEdgeTop:
        if (mTop != position) {
          mTop = position;
          dirty = true;
        }
        break;
      case kPositionEdgeRight:
        if (mRight != position) {
          mRight = position;
          dirty = true;
        }
        break;
      case kPositionEdgeBottom:
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
      case kPositionEdgeLeft:
        position = mLeft;
        break;
      case kPositionEdgeTop:
        position = mTop;
        break;
      case kPositionEdgeRight:
        position = mRight;
        break;
      case kPositionEdgeBottom:
        position = mBottom;
        break;
    }
    return position;
  }

}
