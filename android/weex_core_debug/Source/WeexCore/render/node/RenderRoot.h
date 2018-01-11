#ifndef WEEX_PROJECT_RENDERROOT_H
#define WEEX_PROJECT_RENDERROOT_H

#include <WeexCore/render/node/RenderObject.h>

namespace WeexCore {
  class RenderRoot : public RenderObject {
  public:

    void SetDefaultWidth(const float &defaultWidth);

    void SetDefaultHeight(const float &defaultHeight);

    bool ViewInit();
  };
}

#endif //WEEX_PROJECT_RENDERROOT_H
