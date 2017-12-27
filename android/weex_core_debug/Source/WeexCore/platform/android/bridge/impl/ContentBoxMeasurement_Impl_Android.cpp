#include <WeexCore/platform/android/base/jni/android_jni.h>
#include "WeexCore/platform/android/bridge/ContentBoxMeasurement_jni.h"
#include "ContentBoxMeasurement_Impl_Android.h"
#include "MeasureMode_Impl_Android.h"

namespace WeexCore {
  bool RegisterJNIContentBoxMeasurement(JNIEnv *env) {
    RegisterNativesImpl(env);
  }

  base::android::ScopedLocalJavaRef<jobject>
  cumsmeasure_Imple_Android(JNIEnv *env, jobject measureFunc_Impl_Android, float width,
                            float height,
                            int widthMeasureMode, int heightMeasureMode) {
    if (measureFunc_Impl_Android == nullptr)
      return base::android::ScopedLocalJavaRef<jobject>(env, nullptr);
    return Java_ContentBoxMeasurement_measure(env, measureFunc_Impl_Android, width, height,
                                              widthMeasureMode, heightMeasureMode);
  }

  void layoutBefore_Impl_Android(JNIEnv *env, jobject measureFunc_Impl_Android) {
    if (measureFunc_Impl_Android != nullptr)
      Java_ContentBoxMeasurement_layoutBefore(env, measureFunc_Impl_Android);
  }

  void
  layoutAfter_Impl_Android(JNIEnv *env, jobject measureFunc_Impl_Android, float width,
                           float height) {
    if (measureFunc_Impl_Android != nullptr)
      Java_ContentBoxMeasurement_layoutAfter(env, measureFunc_Impl_Android, width, height);
  }
}
