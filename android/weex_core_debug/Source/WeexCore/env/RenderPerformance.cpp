#include "RenderPerformance.h"
#include <WeexCore/platform/android/base/LogUtils.h>

namespace WeexCore {
  void
  RenderPerformance::PrintPerformanceLogImplAndroid(const PerformanceStage &performanceStage) {
    if (performanceStage == onFirstScreen) {
      LOGD("[WeexCore render time]onFirstScreen");
    } else {
      LOGD("[WeexCore render time]onRenderSuccess");
    }
    LOGD("[WeexCore render time]      jniCallTime: %lld", jniCallTime);
    LOGD("[WeexCore render time]        -addElementActionJNITime: %lld", addElementActionJNITime);
    LOGD("[WeexCore render time]         layoutActionJNITime: %lld", layoutActionJniTime);
    LOGD("[WeexCore render time]        -jniCallBridgeTime: %lld", jniCallBridgeTime);
    LOGD("[WeexCore render time]         createJMapJNITime: %lld", createJMapJNITime);
    LOGD("[WeexCore render time]      cssLayoutTime: %lld", cssLayoutTime);
    LOGD("[WeexCore render time]      parseJsonTime: %lld", parseJsonTime);
    LOGD("[WeexCore render time]      buildRenderObjectTime: %lld", buildRenderObjectTime);
  }

  void RenderPerformance::PrintPerformanceLogImplIOS(const PerformanceStage &performanceStage) {

  }
}
