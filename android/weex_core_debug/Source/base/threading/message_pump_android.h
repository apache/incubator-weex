// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_THREADING_MESSAGE_PUMP_ANDROID_H_
#define WEEX_BASE_THREADING_MESSAGE_PUMP_ANDROID_H_

#include <jni.h>
#include "../android/android_jni.h"
#include "../android/scoped_java_ref.h"
#include "message_pump.h"

namespace base {
class MessagePumpAndroid : public MessagePump {
 public:
    explicit MessagePumpAndroid(Delegate* delegate);

    virtual ~MessagePumpAndroid();

    virtual void Run(Delegate* delegate) {}

    virtual void Stop();

    virtual void ScheduleWork();

    virtual void ScheduleDelayedWork(Clouse* clouse, int delayed_time) {}

    virtual void ScheduleIntervalWork(Clouse* clouse, int delayed_time) {}

    static bool RegisterJNIUtils(JNIEnv* env);

 private:
    android::ScopedGlobalJavaRef<jobject> system_message_handler_ref_;

    Delegate* delegate_;
};
}  // namespace base

#endif  // WEEX_BASE_THREADING_MESSAGE_PUMP_ANDROID_H_
