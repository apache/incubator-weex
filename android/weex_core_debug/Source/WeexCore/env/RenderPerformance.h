#ifndef WEEX_PROJECT_WXPERFORMANCE_H
#define WEEX_PROJECT_WXPERFORMANCE_H

namespace WeexCore {

  typedef enum PerformanceStage {
    onFirstScreen, onRenderSuccess,
  } PerformanceStage;

  class RenderPerformance {

  public:

    long long jniCallTime;

    long long jniCallBridgeTime;

    long long cssLayoutTime;

    long long addElementActionJNITime;

    long long layoutActionJniTime;

    long long parseJsonTime;

    long long buildRenderObjectTime;

    long long createJMapJNITime;

    RenderPerformance() : jniCallTime(0), jniCallBridgeTime(0), cssLayoutTime(0),
                          addElementActionJNITime(0),
                          layoutActionJniTime(0), parseJsonTime(0), buildRenderObjectTime(0),
                          createJMapJNITime(0) {}

    inline void printPerformanceLog(PerformanceStage performanceStage) {
      printPerformanceLog_Impl_Android(performanceStage);
    }

  private:

    void printPerformanceLog_Impl_Android(const PerformanceStage &performanceStage);

    void printPerformanceLog_Impl_iOS(const PerformanceStage &performanceStage);
  };
}

#endif //WEEX_PROJECT_WXPERFORMANCE_H
