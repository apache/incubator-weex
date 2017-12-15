#ifndef WEEXV8_RENDERACTION_H
#define WEEXV8_RENDERACTION_H

#include <Layout/WXCoreLayout.h>
#include <WeexCore/render/RenderObject.h>
#include <WeexCore/render/RenderManager.h>
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/platform/android/BridgeAndroid.h>
#include <string>
#include <base/TimeUtils.h>

namespace WeexCore {

  class RenderAction {
  public:
    virtual void ExecuteAction() {}
  };

}

#endif //WEEXV8_RENDERACTION_H
