#include <base/android/jni/android_jni.h>
#include "MeasureSize_Impl_Android.h"
#include <gen/MeasureSize_jni.h>

namespace WeexCore {
  bool RegisterJNIMeasureSize(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}
