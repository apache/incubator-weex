#ifndef WEEX_PROJECT_RENDERCELL_H
#define WEEX_PROJECT_RENDERCELL_H

#include <WeexCore/render/node/RenderObject.h>

namespace WeexCore {
  class RenderCell : public RenderObject {
  public:
    RenderCell();

  private:
    void
    measure(float width, MeasureMode widthMeasureMode, float height, MeasureMode heightMeasureMode,
            bool useMeasureFunc);
  };
}

#endif //WEEX_PROJECT_RENDERCELL_H
