#include <jni.h>
#include <WeexCore/platform/android/bridge/impl/WeexCore_Impl_Android.h>
#include <WeexCore/platform/android/base/jni/android_jni.h>
#include <WeexCore/platform/android/bridge/impl/MeasureMode_Impl_Android.h>
#include <WeexCore/platform/android/bridge/impl/ContentBoxMeasurement_Impl_Android.h>

jint JNI_OnLoad (JavaVM *vm, void *reserved)
{
    //base::android::InitVM(vm);
    // JNIEnv* env = base::android::AttachCurrentThread();
    JNIEnv *env;
    /* Get environment */
    if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
      return JNI_FALSE;
    }
    WeexCore::RegisterJNIUtils(env);
    WeexCore::RegisterJNIMeasureMode(env);
    WeexCore::RegisterJNIContentBoxMeasurement(env);
    return WeexCore::OnLoad(vm, reserved);
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
    WeexCore::Unload(vm, reserved);
}
