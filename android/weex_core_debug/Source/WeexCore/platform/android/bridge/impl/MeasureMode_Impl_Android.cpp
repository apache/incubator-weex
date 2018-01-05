#include <WeexCore/platform/android/base/jni/android_jni.h>
#include "MeasureMode_Impl_Android.h"
#include "WeexCore/platform/android/jniprebuild/jniheader/MeasureMode_jni.h"

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env) {
    RegisterNativesImpl(env);
  }

  jint getExactly(JNIEnv *env) {
    return Java_MeasureMode_getEXACTLY(env);
  }

  jint getUnspecified(JNIEnv *env) {
    return Java_MeasureMode_getUNSPECIFIED(env);
  }
}

