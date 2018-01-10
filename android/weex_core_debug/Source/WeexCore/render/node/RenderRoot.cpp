#include "RenderRoot.h"

namespace WeexCore {
  void RenderRoot::setDefaultWidth(const float &defaultWidth) {
    if (isnan(getStyleWidth())) {
      setStyleWidth(defaultWidth);
    }
  }

  void RenderRoot::setDefaultHeight(const float &defaultHeight) {
    if (isnan(getStyleHeight())) {
      setStyleHeight(defaultHeight);
    }
  }

  bool RenderRoot::ViewInit() {
    return !isnan(getStyleWidth()) && !isnan(getStyleHeight()) && getStyleWidth() > 0 &&
           getStyleHeight() > 0;
  }
}