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

#include "base/thread/thread_impl_posix.h"
#include "base/message_loop/message_loop.h"
#include "base/thread/waitable_event.h"

namespace weex {
namespace base {

struct StartupData {
  WaitableEvent event;
  MessageLoop* message_loop;

  explicit StartupData(MessageLoop* p_message_loop)
      : event(), message_loop(p_message_loop) {}
};

void* ThreadFunc(void* params) {
  StartupData* data = static_cast<StartupData*>(params);
  MessageLoop* message_loop = data->message_loop;
  data->event.Signal();
  message_loop->Run();
  return NULL;
}

ThreadImplPosix::ThreadImplPosix(const ThreadParams& params)
    : ThreadImpl(params) {}

ThreadImplPosix::~ThreadImplPosix() {}

void ThreadImplPosix::Start() {
  if (message_loop()->type() == MessageLoop::Type::DEFAULT) {
    StartupData params(message_loop());
    int error = pthread_create(&handle_, NULL, ThreadFunc, &params);
    if (!error) {
      params.event.Wait();
    }
  } else {
    message_loop()->Run();
  }
}

void ThreadImplPosix::Stop() { message_loop_->Stop(); }

}  // namespace base
}  // namespace weex