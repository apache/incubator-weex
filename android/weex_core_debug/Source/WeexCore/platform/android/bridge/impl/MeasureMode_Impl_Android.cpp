#include <base/android/jni/android_jni.h>
#include "MeasureMode_Impl_Android.h"
#include "WeexCore/platform/android/bridge/MeasureMode_jni.h"

namespace WeexCore {
  bool RegisterJNIMeasureMode(JNIEnv *env) {
    RegisterNativesImpl(env);
  }

  jint getEXACTLY(JNIEnv *env) {
    return Java_MeasureMode_getEXACTLY(env);
  }

  jint getATMOST(JNIEnv *env) {
    return Java_MeasureMode_getATMOST(env);
  }
}

