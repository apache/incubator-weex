#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include <core/layout/layout.h>
#include <core/render/node/render_object.h>
#include <core/render/manager/render_manager.h>
#include <core/render/page/render_page.h>
#include <android/bridge/impl/bridge_impl_android.h>
#include <string>
#include <base/TimeUtils.h>

namespace WeexCore {

  class render_action {
  public:
    virtual void ExecuteAction() = 0;
  };
}

#endif //WEEXV8_RENDERACTION_H
