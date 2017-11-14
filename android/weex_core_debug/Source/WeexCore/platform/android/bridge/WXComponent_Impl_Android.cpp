#include <base/android/jni/android_jni.h>
#include "WXComponent_Impl_Android.h"
#include <gen/WXComponent_jni.h>

namespace WeexCore {
  bool RegisterJNIWXComponent(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}
