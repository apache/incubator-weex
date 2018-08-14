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

#ifndef BASE_THREAD_THREAD_H
#define BASE_THREAD_THREAD_H

#include <pthread.h>
#include <memory>
#if OS_ANDROID
#include "base/thread/thread_impl_android.h"
#elif OS_IOS
#include "base/thread/thread_impl_darwin.h"
#endif

namespace weex {
namespace base {
class Thread {
 public:
  Thread(MessageLoop::Type type) : impl_(CreateImpl(ThreadParams(type))) {}
  Thread(const ThreadParams& params) : impl_(CreateImpl(params)) {}
  ~Thread() {}

  inline void Start() { impl_->Start(); }

  inline void Stop() { impl_->Stop(); }

  inline MessageLoop* message_loop() { return impl_->message_loop(); }

 private:
  inline static ThreadImpl* CreateImpl(const ThreadParams& params) {
#if OS_ANDROID
    return new ThreadImplAndroid(params);
#elif OS_IOS
    return new ThreadImplDarwin(params);
#endif
  }
  std::unique_ptr<ThreadImpl> impl_;
  DISALLOW_COPY_AND_ASSIGN(Thread);
};
}  // namespace base
}  // namespace weex

#endif  // BASE_THREAD_THREAD_H
