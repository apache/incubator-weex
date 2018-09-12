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
#ifndef BASE_THREAD_PLATFORM_THREAD_H
#define BASE_THREAD_PLATFORM_THREAD_H

#include "base/common.h"
#include "base/message_loop/message_loop.h"

namespace weex {
namespace base {
struct ThreadParams {
  int priority;
  std::string name;
  MessageLoop::Type type;

  ThreadParams(const std::string& p_name, int p_priority,
               MessageLoop::Type p_type)
      : name(p_name), type(p_type), priority(p_priority) {}

  ThreadParams(MessageLoop::Type p_type) : ThreadParams("", 0, p_type) {}
};
class ThreadImpl {
 public:
  ThreadImpl(const ThreadParams& params)
      : name_(params.name), message_loop_(new MessageLoop(params.type)) {}
  virtual ~ThreadImpl() {}
  virtual void Start() = 0;
  virtual void Stop() = 0;
  virtual void SetName(const std::string& name) { name_ = name; };
  inline const std::string& name() const { return name_; }

  inline MessageLoop* message_loop() { return message_loop_.get(); }

 protected:
  std::unique_ptr<MessageLoop> message_loop_;
  std::string name_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ThreadImpl);
};
}  // namespace base
}  // namespace weex

#endif  // BASE_THREAD_PLATFORM_THREAD_H