#include <android/base/jni/android_jni.h>
#include <android/jniprebuild/jniheader/ContentBoxMeasurement_jni.h>
#include "content_box_measurement_impl_android.h"
#include "measure_mode_impl_android.h"

namespace WeexCore {
  bool RegisterJNIContentBoxMeasurement(JNIEnv *env) {
    RegisterNativesImpl(env);
  }

  void cumsmeasure_Imple_Android(JNIEnv *env, jobject measureFunc_Impl_Android,
                                 float width, float height, int widthMeasureMode, int heightMeasureMode) {
    if (measureFunc_Impl_Android != nullptr)
      Java_ContentBoxMeasurement_measure(env, measureFunc_Impl_Android, width, height, widthMeasureMode, heightMeasureMode);
  }

  void LayoutBeforeImplAndroid(JNIEnv *env, jobject measureFunc_impl_android) {
    if (measureFunc_impl_android != nullptr)
      Java_ContentBoxMeasurement_layoutBefore(env, measureFunc_impl_android);
  }

  void LayoutAfterImplAndroid(JNIEnv *env, jobject measureFunc_impl_android, float width, float height) {
    if (measureFunc_impl_android != nullptr)
      Java_ContentBoxMeasurement_layoutAfter(env, measureFunc_impl_android, width, height);
  }

  float GetLayoutWidth(JNIEnv *env, jobject measureFunc_impl_android) {
    if (measureFunc_impl_android != nullptr)
      Java_ContentBoxMeasurement_getWidth(env, measureFunc_impl_android);
  }

  float GetLayoutHeight(JNIEnv *env, jobject measureFunc_impl_android) {
    if (measureFunc_impl_android != nullptr)
      Java_ContentBoxMeasurement_getHeight(env, measureFunc_impl_android);
  }
}
