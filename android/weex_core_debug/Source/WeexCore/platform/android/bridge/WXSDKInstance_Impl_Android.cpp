#include <base/android/jni/android_jni.h>
#include "WXSDKInstance_Impl_Android.h"
#include <gen/WXSDKInstance_jni.h>

namespace WeexCore {
  bool RegisterJNISDKInstance(JNIEnv *env) {
    return RegisterNativesImpl(env);
  }
}
