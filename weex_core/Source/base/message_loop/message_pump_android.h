/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CORE_BASE_MESSAGE_LOOP_MESSAGE_PUMP_ANDROID_H
#define CORE_BASE_MESSAGE_LOOP_MESSAGE_PUMP_ANDROID_H
#ifdef __ANDROID__

#include <jni.h>
#include "base/android/jni/scoped_java_ref.h"
#include "base/android/jni/jni_object_wrap.h"
#include "base/message_loop/message_pump.h"

namespace weex {
namespace base {
class MessagePumpAndroid : public MessagePump, public WeexCore::JNIObjectWrap {
 public:
  MessagePumpAndroid();
  ~MessagePumpAndroid();
  void Run(Delegate* delegate) override;
  void Stop() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(TimeUnit delayed_time) override;

  static bool RegisterJNIUtils(JNIEnv* env);

 private:
  DISALLOW_COPY_AND_ASSIGN(MessagePumpAndroid);
};
}  // namespace base
}  // namespace weex

#endif
#endif  // CORE_BASE_MESSAGE_LOOP_MESSAGE_PUMP_ANDROID_H
