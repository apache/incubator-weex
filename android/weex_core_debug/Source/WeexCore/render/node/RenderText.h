#ifndef WEEX_PROJECT_RENDERTEXT_H
#define WEEX_PROJECT_RENDERTEXT_H

#include <WeexCore/render/node/RenderObject.h>

namespace WeexCore {
  class RenderText : public RenderObject {
  public:
    RenderText();

  private:
    void
    measure(float width, MeasureMode widthMeasureMode, float height, MeasureMode heightMeasureMode,
            bool useMeasureFunc);
  };
}

#endif //WEEX_PROJECT_RENDERTEXT_H
