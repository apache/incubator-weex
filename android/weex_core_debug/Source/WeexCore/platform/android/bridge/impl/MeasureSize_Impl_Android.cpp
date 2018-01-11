#include <WeexCore/platform/android/base/jni/android_jni.h>
#include "MeasureSize_Impl_Android.h"
#include "WeexCore/platform/android/jniprebuild/jniheader/MeasureSize_jni.h"

namespace WeexCore {
  bool RegisterJNIMeasureSize(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }

  jfloat GetMeasureHeight(JNIEnv *env, jobject size) {
    return Java_MeasureSize_getHeight(env, size);
  }

  jfloat GetMeasureWidth(JNIEnv *env, jobject size) {
    return Java_MeasureSize_getWidth(env, size);
  }
}
