#include <jni.h>
#include <android/bridge/impl/bridge_impl_android.h>
#include <android/base/jni/android_jni.h>
#include <android/bridge/impl/measure_mode_impl_android.h>
#include <android/bridge/impl/content_box_measurement_impl_android.h>

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
