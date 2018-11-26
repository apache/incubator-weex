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

#include "base/time_point.h"
#include <assert.h>
#include <time.h>

namespace weex {
namespace base {
TimePoint TimePoint::Now() {
  struct timespec ts;
  int res = clock_gettime(CLOCK_MONOTONIC, &ts);
  assert(res == 0);
  return TimePoint(TimeUnit::FromSeconds(ts.tv_sec).ToNanoseconds() +
                   ts.tv_nsec);
}
}  // namespace base
}  // namespace weex