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

#include "base/message_loop/message_pump_posix.h"

namespace weex {
namespace base {

MessagePumpPosix::MessagePumpPosix()
    : stop_request_(false), condition_(), delayed_time_() {}

MessagePumpPosix::~MessagePumpPosix() {}

void MessagePumpPosix::Run(Delegate* delegate) {
  TimeUnit zero;
  for (;;) {
    if (stop_request_) break;
    std::unique_lock<std::mutex> lock(mutex_);
    if (delayed_time_ == zero) {
      condition_.wait(lock);
    } else {
      condition_.wait_for(
          lock, std::chrono::nanoseconds(delayed_time_.ToNanoseconds()));
    }
    delayed_time_ = zero;
    delegate->DoWork();
  }
}

void MessagePumpPosix::Stop() { stop_request_ = true; }

void MessagePumpPosix::ScheduleWork() { condition_.notify_one(); }

void MessagePumpPosix::ScheduleDelayedWork(TimeUnit delayed_time) {
  // This method is only used by the same threads as MessageLoop::DoWork, so we
  // only update the delayed time instead of blocking and waiting.
  delayed_time_ = std::move(delayed_time);
}
}  // namespace base
}  // namespace weex
