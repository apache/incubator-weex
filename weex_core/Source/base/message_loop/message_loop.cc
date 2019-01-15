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

#include "base/message_loop/message_loop.h"
#include "base/thread/thread_local.h"
#ifdef __ANDROID__
#include "base/message_loop/message_pump_android.h"
#elif OS_IOS
#include "base/message_loop/message_pump_darwin.h"
#endif
#include "base/message_loop/message_pump_posix.h"

namespace weex {
namespace base {

ThreadLocal<MessageLoop> s_tl_message_loop;

MessageLoop::MessageLoop(Type type)
    : delayed_tasks_(), type_(type), delayed_tasks_mutex_() {
  switch (type) {
    case PLATFORM:
#ifdef __ANDROID__ 
      message_pump_ = std::unique_ptr<MessagePump>(new MessagePumpAndroid());
#elif OS_IOS
      message_pump_ = std::unique_ptr<MessagePump>(new MessagePumpDarwin());
#endif
      break;
    case DEFAULT:
      message_pump_ = std::unique_ptr<MessagePump>(new MessagePumpPosix());
      break;
    case IO:
      break;
  }
}
MessageLoop::~MessageLoop() {}

MessageLoop* MessageLoop::GetCurrent() { return s_tl_message_loop.Get(); }

void MessageLoop::Run() {
  // Store in thread local stack
  s_tl_message_loop.Set(this);
  message_pump_->Run(this);
}

void MessageLoop::Stop() { message_pump_->Stop(); }

void MessageLoop::DoWork() {
  std::vector<Closure> closure_list;
  {
    std::lock_guard<std::mutex> lock(delayed_tasks_mutex_);

    if (delayed_tasks_.empty()) {
      return;
    }

    auto now = TimePoint::Now();
    while (!delayed_tasks_.empty()) {
      const auto& top = delayed_tasks_.top();
      if (top.time_point > now) {
        break;
      }
      closure_list.emplace_back(std::move(top.task));
      delayed_tasks_.pop();
    }

    if (!delayed_tasks_.empty()) {
      message_pump_->ScheduleDelayedWork(delayed_tasks_.top().time_point - now);
    }
  }

  for (const auto& closure : closure_list) {
    closure();
  }
}

void MessageLoop::PostTask(const Closure& closure) {
  PostDelayedTask(closure, 0);
}

void MessageLoop::PostPriorityTask(TaskPriority p, const Closure& closure) {
  PostPriorityDelayedTask(p, closure, 0);
}

void MessageLoop::PostDelayedTask(const Closure& closure, int64_t delayed_ms) {
  PostPriorityDelayedTask(TaskPriority::NORMAL, closure, delayed_ms);
}

void MessageLoop::PostPriorityDelayedTask(TaskPriority p,
                                          const Closure& closure,
                                          int64_t delayed_ms) {
  TimePoint time_point =
      TimePoint::Now() + TimeUnit::FromMilliseconds(delayed_ms);
  std::lock_guard<std::mutex> lock(delayed_tasks_mutex_);
  // If the time of top task don't change, we don't need to reschedule.
  if (!delayed_tasks_.empty()) {
    auto old_top_time = delayed_tasks_.top().time_point;
    delayed_tasks_.push({p, std::move(closure), std::move(time_point)});
    if (old_top_time > delayed_tasks_.top().time_point) {
      message_pump_->ScheduleWork();
    }
  } else {
    delayed_tasks_.push({p, std::move(closure), std::move(time_point)});
    message_pump_->ScheduleWork();
  }
}
}  // namespace base
}  // namespace weex
