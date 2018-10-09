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
#ifndef BASE_MESSAGE_LOOP_MESSAGE_LOOP_H
#define BASE_MESSAGE_LOOP_MESSAGE_LOOP_H

#include <memory>
#include <deque>
#include <mutex>
#include <queue>
#include "base/closure.h"
#include "base/common.h"
#include "base/message_loop/message_pump.h"
#include "base/time_point.h"

namespace weex {
namespace base {
class MessageLoop : public MessagePump::Delegate {
 public:
  enum Type {
    PLATFORM,
    DEFAULT,
    IO,
  };
  enum TaskPriority {
    UI = 10,
    NORMAL = 5,
  };
  MessageLoop(Type type);
  ~MessageLoop();

  static MessageLoop* GetCurrent();

  // The unit of the priority task is milliseconds
  void PostTask(const Closure& closure);
  void PostPriorityTask(TaskPriority p, const Closure& closure);
  void PostDelayedTask(const Closure& closure, int64_t delayed_ms);
  void PostPriorityDelayedTask(TaskPriority p, const Closure& closure,
                               int64_t delayed_ms);

  void Run();
  void Stop();

  inline Type type() { return type_; }

 private:
  void DoWork();

  struct DelayedTask {
    TimePoint time_point;
    size_t priority;
    Closure task;
    DelayedTask(size_t p_priority, Closure p_task, TimePoint p_time_point)
        : priority(p_priority),
          task(std::move(p_task)),
          time_point(p_time_point) {}
  };
  struct DelayedTaskCompare {
    bool operator()(const DelayedTask& a, const DelayedTask& b) {
      return a.time_point == b.time_point
                 ? a.priority < b.priority
                 : a.time_point > b.time_point;
    };
  };
  using DelayedTaskQueue =
      std::priority_queue<DelayedTask, std::deque<DelayedTask>,
                          DelayedTaskCompare>;
  DelayedTaskQueue delayed_tasks_;
  Type type_;
  std::unique_ptr<MessagePump> message_pump_;
  std::mutex delayed_tasks_mutex_;
  DISALLOW_COPY_AND_ASSIGN(MessageLoop);
};
}  // namespace base
}  // namespace weex

#endif  // BASE_MESSAGE_LOOP_MESSAGE_LOOP_H
