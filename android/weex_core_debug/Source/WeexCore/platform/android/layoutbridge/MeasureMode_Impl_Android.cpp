#include <base/android/jni/android_jni.h>
#include "MeasureMode_Impl_Android.h"
#include <gen/MeasureMode_jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env) {
    RegisterNativesImpl(env);
  }
}

jint getEXACTLY(JNIEnv *env) {
  return Java_MeasureMode_getEXACTLY(env,Java_MeasureMode_GET(env).Get());
}

jint getATMOST(JNIEnv *env) {
  return Java_MeasureMode_getATMOST(env, Java_MeasureMode_GET(env).Get());
}
