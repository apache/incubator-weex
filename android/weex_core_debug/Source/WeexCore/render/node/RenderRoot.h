#ifndef WEEX_PROJECT_RENDERROOT_H
#define WEEX_PROJECT_RENDERROOT_H

#include <WeexCore/render/node/RenderObject.h>

namespace WeexCore {
  class RenderRoot : public RenderObject {
  public:

    inline void SetDefaultWidth(const float &defaultWidth) {
      if (isnan(getStyleWidth())) {
        setStyleWidth(defaultWidth);
      }
    }

    inline void SetDefaultHeight(const float &defaultHeight) {
      if (isnan(getStyleHeight())) {
        setStyleHeight(defaultHeight);
      }
    }

    inline bool ViewInit() {
      return !isnan(getStyleWidth()) && !isnan(getStyleHeight()) && getStyleWidth() > 0 &&
             getStyleHeight() > 0;
    }
  };
}

#endif //WEEX_PROJECT_RENDERROOT_H
