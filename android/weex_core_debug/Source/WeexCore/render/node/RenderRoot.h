#ifndef WEEX_PROJECT_RENDERROOT_H
#define WEEX_PROJECT_RENDERROOT_H

#include <WeexCore/render/node/RenderObject.h>

namespace WeexCore {
  class RenderRoot : public RenderObject {
  public:

    void setDefaultWidth(const float &defaultWidth);

    void setDefaultHeight(const float &defaultHeight);

    bool ViewInit();
  };
}

#endif //WEEX_PROJECT_RENDERROOT_H
