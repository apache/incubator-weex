#include <android/base/jni/android_jni.h>
#include "measure_mode_impl_android.h"
#include <android/jniprebuild/jniheader/MeasureMode_jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env) {
    RegisterNativesImpl(env);
  }

  jint Exactly(JNIEnv *env) {
    return Java_MeasureMode_getEXACTLY(env);
  }

  jint Unspecified(JNIEnv *env) {
    return Java_MeasureMode_getUNSPECIFIED(env);
  }
}

