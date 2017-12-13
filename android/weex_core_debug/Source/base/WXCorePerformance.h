#ifndef WEEX_PROJECT_WXPERFORMANCE_H
#define WEEX_PROJECT_WXPERFORMANCE_H

#include <base/android/LogUtils.h>

namespace WeexCore {

  typedef enum PerformanceStage {
    onFirstScreen, onRenderSuccess,
  } PerformanceStage;

  class WXCorePerformance {

  public:

    long long jniCallTime;

    long long cssLayoutTime;

    long long addElementActionJNITime;

    long long layoutActionJniTime;

    WXCorePerformance() : jniCallTime(0), cssLayoutTime(0), addElementActionJNITime(0),
                          layoutActionJniTime(0) {}

    inline void printPerformanceLog(PerformanceStage performanceStage) {
      printPerformanceLog_Impl_Android(performanceStage);
    }

  private:

    inline void printPerformanceLog_Impl_Android(PerformanceStage performanceStage) {
      if (performanceStage == onFirstScreen) {
        LOGD("[WeexCore render time] onFirstScreen");
      } else {
        LOGD("[WeexCore render time] onRenderSuccess");
      }
      LOGD("[WeexCore render time]      jniCallTime: %lld", jniCallTime);
      LOGD("[WeexCore render time]      addElementActionJNITime: %lld", addElementActionJNITime);
      LOGD("[WeexCore render time]      layoutActionJniTime: %lld", layoutActionJniTime);
      LOGD("[WeexCore render time]      cssLayoutTime: %lld", cssLayoutTime);
    }

    inline void printPerformanceLog_Impl_iOS(PerformanceStage performanceStage) {

    }
  };
}

#endif //WEEX_PROJECT_WXPERFORMANCE_H
