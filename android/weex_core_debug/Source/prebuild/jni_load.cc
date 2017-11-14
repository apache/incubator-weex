#include <jni.h>
#include <WeexCore/platform/android/WeexCore.h>
#include <base/android/jni/android_jni.h>
#include <android/log.h>
#include <WeexCore/platform/android/bridge/MeasureMode_Impl_Android.h>
#include <WeexCore/platform/android/bridge/ContentBoxMeasurement_Impl_Android.h>
#include <WeexCore/platform/android/bridge/WXSDKInstance_Impl_Android.h>
#include <WeexCore/platform/android/bridge/WXComponent_Impl_Android.h>
#include <WeexCore/platform/android/bridge/MeasureSize_Impl_Android.h>

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
    WeexCore::RegisterJNISDKInstance(env);
    WeexCore::RegisterJNIWXComponent(env);
    WeexCore::RegisterJNIMeasureSize(env);
    __android_log_print(ANDROID_LOG_ERROR, "shiwentao", "jni_onload on jni_load.cc");
    return WeexCore::OnLoad(vm, reserved);
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
    WeexCore::Unload(vm, reserved);
}
