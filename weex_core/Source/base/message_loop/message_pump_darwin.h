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

#ifndef WEEX_PROJECT_MESSAGE_PUMP_IOS_H
#define WEEX_PROJECT_MESSAGE_PUMP_IOS_H

#include "base/message_loop/message_pump.h"

namespace weex {
namespace base {
class MessagePumpDarwin : public MessagePump {
 public:
  MessagePumpDarwin();
  ~MessagePumpDarwin();
  void Run(Delegate* delegate) override;
  void Stop() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(TimeUnit delayed_time) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(MessagePumpDarwin);
};
}  // namespace base
}  // namespace weex

#endif  // WEEX_PROJECT_MESSAGE_PUMP_IOS_H
