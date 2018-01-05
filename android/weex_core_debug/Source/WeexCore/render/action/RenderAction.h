#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include <WeexCore/layout/WXCoreLayout.h>
#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/platform/android/bridge/impl/WXBridge_Impl_Android.h>
#include <string>
#include <base/TimeUtils.h>

namespace WeexCore {

  class RenderAction {
  public:
    virtual void ExecuteAction() = 0;
  };
}

#endif //WEEXV8_RENDERACTION_H
