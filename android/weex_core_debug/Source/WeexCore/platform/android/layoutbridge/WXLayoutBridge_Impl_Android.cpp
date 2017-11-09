#include <base/android/jni/android_jni.h>
#include <gen/WXLayoutBridge_jni.h>
#include "WXLayoutBridge_Impl_Android.h"
#include "MeasureMode_Impl_Android.h"
#include "ContentBoxMeasurement_Impl_Android.h"

namespace WeexCore {
  bool RegisterJNILayout(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}

static void Layout(JNIEnv *env, jobject jcaller) {
  Java_WXLayoutBridge_measure(env, getEXACTLY(env), getATMOST(env));
  measure(env);
}

