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

#ifndef CORE_BASE_MESSAGE_LOOP_MESSAGE_PUMP_POSIX_H
#define CORE_BASE_MESSAGE_LOOP_MESSAGE_PUMP_POSIX_H

#include <condition_variable>
#include <mutex>
#include "base/common.h"
#include "base/message_loop/message_pump.h"

namespace weex {
namespace base {
class MessagePumpPosix : public MessagePump {
 public:
  MessagePumpPosix();
  ~MessagePumpPosix();
  void Run(Delegate* delegate) override;
  void Stop() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(TimeUnit delayed_time) override;

 private:
  bool stop_request_;
  std::condition_variable condition_;
  std::mutex mutex_;
  TimeUnit delayed_time_;
  DISALLOW_COPY_AND_ASSIGN(MessagePumpPosix);
};
}  // namespace base
}  // namespace weex

#endif  // CORE_BASE_MESSAGE_LOOP_MESSAGE_PUMP_POSIX_H
