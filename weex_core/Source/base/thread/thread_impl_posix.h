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

#ifndef WEEX_PROJECT_THREAD_IMPL_POSIX_H
#define WEEX_PROJECT_THREAD_IMPL_POSIX_H

#include <pthread.h>
#include <string>
#include "base/thread/thread_impl.h"

namespace weex {
namespace base {
class ThreadImplPosix : public ThreadImpl {
 public:
  ThreadImplPosix(const ThreadParams& params);
  virtual ~ThreadImplPosix();
  virtual void Start() override;
  virtual void Stop() override;

  inline pthread_t handle() const { return handle_; }

 private:
  pthread_t handle_;
  DISALLOW_COPY_AND_ASSIGN(ThreadImplPosix);
};
}  // namespace base
}  // namespace weex

#endif  // WEEX_PROJECT_THREAD_IMPL_POSIX_H
