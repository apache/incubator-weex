#include <base/android/jni/android_jni.h>
#include <gen/ContentBoxMeasurement_jni.h>
#include "ContentBoxMeasurement_Impl_Android.h"
#include "MeasureMode_Impl_Android.h"

namespace WeexCore {
  bool RegisterJNIContentBoxMeasurement(JNIEnv *env) {
    RegisterNativesImpl(env);
  }
}

base::android::ScopedLocalJavaRef<jobject>
cumsmeasure_Imple_Android(JNIEnv *env, jobject measureFunc_Impl_Android, float width, float height,
                          int widthMeasureMode, int heightMeasureMode) {
  return Java_ContentBoxMeasurement_measure(env, measureFunc_Impl_Android, width, height,
                                            widthMeasureMode, heightMeasureMode);
}

void layoutBefore_Impl_Android(JNIEnv *env, jobject measureFunc_Impl_Android) {
  Java_ContentBoxMeasurement_layoutBefore(env, measureFunc_Impl_Android);
}

void
layoutAfter_Impl_Android(JNIEnv *env, jobject measureFunc_Impl_Android, float width, float height) {
  Java_ContentBoxMeasurement_layoutAfter(env, measureFunc_Impl_Android, width, height);
}