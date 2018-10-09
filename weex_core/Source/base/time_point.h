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

#ifndef CORE_BASE_TIME_POINT_H
#define CORE_BASE_TIME_POINT_H

#include <cstdint>
#include "base/time_unit.h"

namespace weex {
namespace base {
class TimePoint {
 public:
  static TimePoint Now();

  TimePoint operator+(TimeUnit unit) const {
    return TimePoint(nanos_ + unit.ToNanoseconds());
  }
  TimePoint operator-(TimeUnit unit) const {
    return TimePoint(nanos_ - unit.ToNanoseconds());
  }
  TimeUnit operator-(TimePoint other) const {
    return TimeUnit::FromNanoseconds(nanos_ - other.nanos_);
  }

  bool operator==(TimePoint other) const { return nanos_ == other.nanos_; }
  bool operator!=(TimePoint other) const { return nanos_ != other.nanos_; }
  bool operator<(TimePoint other) const { return nanos_ < other.nanos_; }
  bool operator<=(TimePoint other) const { return nanos_ <= other.nanos_; }
  bool operator>(TimePoint other) const { return nanos_ > other.nanos_; }
  bool operator>=(TimePoint other) const { return nanos_ >= other.nanos_; }

  constexpr TimeUnit ToTimeUnit() const {
    return TimeUnit::FromNanoseconds(nanos_);
  }

 private:
  explicit constexpr TimePoint(int64_t ns) : nanos_(ns) {}
  int64_t nanos_;
};
}  // namespace base
}  // namespace weex

#endif  // CORE_BASE_TIME_POINT_H
