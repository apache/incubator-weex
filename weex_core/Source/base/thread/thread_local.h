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

#ifndef BASE_THREAD_THREAD_LOCAL_H
#define BASE_THREAD_THREAD_LOCAL_H

#include <pthread.h>
#include "base/common.h"

namespace weex {
namespace base {
typedef pthread_key_t ThreadLocalKey;

inline ThreadLocalKey ThreadLocalAlloc() {
  ThreadLocalKey key;
  pthread_key_create(&key, NULL);
  return key;
}

inline void ThreadLocalFree(ThreadLocalKey key) { pthread_key_delete(key); }

inline void ThreadLocalSetValue(ThreadLocalKey key, void* value) {
  pthread_setspecific(key, value);
}

inline void* ThreadLocalGetValue(ThreadLocalKey key) {
  return pthread_getspecific(key);
}

template <typename Type>
class ThreadLocal {
 public:
  ThreadLocal() : key_(ThreadLocalAlloc()) {}

  ~ThreadLocal() { ThreadLocalFree(key_); }

  Type* Get() { return reinterpret_cast<Type*>(ThreadLocalGetValue(key_)); }

  void Set(Type* ptr) { ThreadLocalSetValue(key_, ptr); }

 private:
  ThreadLocalKey key_;
  DISALLOW_COPY_AND_ASSIGN(ThreadLocal<Type>);
};
}  // namespace base
}  // namespace weex
#endif  // BASE_THREAD_THREAD_LOCAL_H
