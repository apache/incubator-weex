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

    WXCorePerformance() : jniCallTime(0), cssLayoutTime(0) {}

    inline void printPerformanceLog(PerformanceStage performanceStage) {
      printPerformanceLog_Impl_Android(performanceStage);
    }

  private:

    inline void printPerformanceLog_Impl_Android(PerformanceStage performanceStage) {
      if (performanceStage == onFirstScreen) {
        LOGD("[render time] onFirstScreen");
      } else {
        LOGD("[render time] onRenderSuccess");
      }
      LOGD("[render time]      jniCallTime: %lld", jniCallTime);
      LOGD("[render time]      cssLayoutTime: %lld", cssLayoutTime);
    }

    inline void printPerformanceLog_Impl_iOS(PerformanceStage performanceStage) {

    }
  };
}

#endif //WEEX_PROJECT_WXPERFORMANCE_H
