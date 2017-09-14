
#include "android_jni.h"

namespace {
JavaVM* g_jvm = nullptr;
}

namespace base {
namespace android {

void InitVM(JavaVM* vm) {
    g_jvm = vm;
}

JNIEnv* AttachCurrentThread() {
    JNIEnv* env = nullptr;
    jint ret =  g_jvm->AttachCurrentThread(&env, nullptr);
    return env;
}

void DetachFromVM() {
    if (g_jvm) {
        g_jvm->DetachCurrentThread();
    }
}

ScopedLocalJavaRef<jclass> GetClass(JNIEnv* env, const char* class_name) {
    jclass clazz;
    clazz = env->FindClass(class_name);
    return ScopedLocalJavaRef<jclass >(env, clazz);
}

jclass GetClass(JNIEnv* env, const char* class_name, intptr_t* class_id) {
    if (*class_id) {
        return reinterpret_cast<jclass>(*class_id);
    }
    ScopedGlobalJavaRef<jclass> clazz;
    clazz.Reset(env, GetClass(env, class_name));
    *class_id = reinterpret_cast<intptr_t>(clazz.Release());
}

jmethodID GetMethod(JNIEnv* env, jclass clazz, MethodType type,
    const char* method_name, const char* jni_signature) {
    if (type == STATIC_METHOD) {
        return env->GetStaticMethodID(clazz, method_name, jni_signature);
    } else if (type == INSTANCE_METHOD) {
        return env->GetMethodID(clazz, method_name, jni_signature);
    }
    return 0;
}

jmethodID GetMethod(JNIEnv* env, jclass clazz, MethodType type,
    const char* method_name, const char* jni_signature, intptr_t* method_id) {
    if (*method_id) {
        return reinterpret_cast<jmethodID>(*method_id);
    }
    *method_id = reinterpret_cast<intptr_t>(GetMethod(env,
        clazz, type, method_name, jni_signature));
    return reinterpret_cast<jmethodID>(*method_id);
}

bool HasException(JNIEnv* env) {
    return env->ExceptionCheck() != JNI_FALSE;
}

bool ClearException(JNIEnv* env) {
    if (!HasException(env))
        return false;
    env->ExceptionDescribe();
    env->ExceptionClear();
    return true;
}

void CheckException(JNIEnv* env) {
    if (!HasException(env))
        return;

    // Exception has been found, might as well tell breakpad about it.
    jthrowable java_throwable = env->ExceptionOccurred();
    if (java_throwable) {
        // Clear the pending exception, since a local reference is now held.
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    // Now, feel good about it and die.
    // CHECK(false) << "Please include Java exception stack in crash report";
}
}  // namespace android
}  // namespace base
