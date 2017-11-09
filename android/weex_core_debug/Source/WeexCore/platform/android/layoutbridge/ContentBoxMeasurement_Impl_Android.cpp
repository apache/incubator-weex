#include <base/android/jni/android_jni.h>
#include <gen/ContentBoxMeasurement_jni.h>
#include "ContentBoxMeasurement_Impl_Android.h"
#include "MeasureMode_Impl_Android.h"

namespace WeexCore {
  bool RegisterJNIContentBoxMeasurement(JNIEnv *env) {
    RegisterNativesImpl(env);
  }
}

void measure(JNIEnv *env) {
  Java_ContentBoxMeasurement_measure(env, Java_ContentBoxMeasurement_Generate(env).Get(), 1.1, 2.2,
                                     getEXACTLY(env), getATMOST(env));
}
