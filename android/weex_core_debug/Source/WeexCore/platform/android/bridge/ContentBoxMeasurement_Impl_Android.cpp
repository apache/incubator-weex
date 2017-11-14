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

}
