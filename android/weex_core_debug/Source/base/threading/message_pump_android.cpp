// Copyright 2017 The Weex Authors. All rights reserved.

#include "message_pump_android.h"

#include "SystemMessageHandler_jni.h"

void RunWork(JNIEnv* env, jobject jcaller, jlong delegateNative) {
    base::MessagePump::Delegate* delegate =
        reinterpret_cast<base::MessagePump::Delegate*>(delegateNative);
    if (delegate != NULL) {
        delegate->DoWork();
    }
}

namespace base {


bool MessagePumpAndroid::RegisterJNIUtils(JNIEnv* env) {
    return RegisterNativesImpl(env);
}

MessagePumpAndroid::MessagePumpAndroid(Delegate* delegate):
    delegate_(delegate) {
    JNIEnv* env = base::android::AttachCurrentThread();
    system_message_handler_ref_.Reset(env,
                                      Java_SystemMessageHandler_create(
                                          env,
                                          reinterpret_cast<jlong>(delegate)));
}

MessagePumpAndroid::~MessagePumpAndroid() {
}

void MessagePumpAndroid::ScheduleWork() {
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_SystemMessageHandler_scheduleWork(
        env, system_message_handler_ref_.Get());
}

    void MessagePumpAndroid::Stop() {
        JNIEnv* env = base::android::AttachCurrentThread();
        Java_SystemMessageHandler_stop(
                env, system_message_handler_ref_.Get());
    }

}  // namespace base
