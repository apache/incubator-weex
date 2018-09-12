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

#ifndef BASE_THREAD_WAITABLE_EVENT_H
#define BASE_THREAD_WAITABLE_EVENT_H

#include <condition_variable>
#include <mutex>

namespace weex {
namespace base {
class WaitableEvent {
 public:
  WaitableEvent() : mutex_(), condition_(), signaled_(false) {}
  ~WaitableEvent() {}

  void Signal() {
    std::lock_guard<std::mutex> locker(mutex_);
    signaled_ = true;
    condition_.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> locker(mutex_);
    while (!signaled_) condition_.wait(locker);
    signaled_ = false;
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  bool signaled_;
  DISALLOW_COPY_AND_ASSIGN(WaitableEvent);
};
}  // namespace base
}  // namespace weex
#endif  // BASE_THREAD_WAITABLE_EVENT_H
