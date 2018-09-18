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

#ifndef CORE_BASE_TIME_UNIT_H
#define CORE_BASE_TIME_UNIT_H

#include <cstdint>
#include <limits>

namespace weex {
namespace base {
class TimeUnit {
 public:
  TimeUnit() {}

  static constexpr TimeUnit Zero(int64_t nanos) { return TimeUnit(0); }

  static constexpr TimeUnit Min() {
    return TimeUnit(std::numeric_limits<int64_t>::min());
  }
  static constexpr TimeUnit Max() {
    return TimeUnit(std::numeric_limits<int64_t>::max());
  }

  static constexpr TimeUnit FromNanoseconds(int64_t nanos) {
    return TimeUnit(nanos);
  }

  static constexpr TimeUnit FromMicroseconds(int64_t micros) {
    return FromNanoseconds(micros * 1000);
  }

  static constexpr TimeUnit FromMilliseconds(int64_t millis) {
    return FromMicroseconds(millis * 1000);
  }

  static constexpr TimeUnit FromSeconds(int64_t seconds) {
    return FromMilliseconds(seconds * 1000);
  }

  constexpr TimeUnit operator-(TimeUnit other) const {
    return TimeUnit::FromNanoseconds(nanos_ - other.nanos_);
  }

  constexpr TimeUnit operator+(TimeUnit other) const {
    return TimeUnit::FromNanoseconds(nanos_ + other.nanos_);
  }

  constexpr TimeUnit operator/(int64_t divisor) const {
    return TimeUnit::FromNanoseconds(nanos_ / divisor);
  }

  constexpr int64_t operator/(TimeUnit other) const {
    return nanos_ / other.nanos_;
  }

  constexpr TimeUnit operator*(int64_t multiplier) const {
    return TimeUnit::FromNanoseconds(nanos_ * multiplier);
  }

  constexpr TimeUnit operator%(TimeUnit other) const {
    return TimeUnit::FromNanoseconds(nanos_ % other.nanos_);
  }

  bool operator==(TimeUnit other) const { return nanos_ == other.nanos_; }
  bool operator!=(TimeUnit other) const { return nanos_ != other.nanos_; }
  bool operator<(TimeUnit other) const { return nanos_ < other.nanos_; }
  bool operator<=(TimeUnit other) const { return nanos_ <= other.nanos_; }
  bool operator>(TimeUnit other) const { return nanos_ > other.nanos_; }
  bool operator>=(TimeUnit other) const { return nanos_ >= other.nanos_; }

  constexpr int64_t ToNanoseconds() const { return nanos_; }
  constexpr int64_t ToMicroseconds() const { return ToNanoseconds() / 1000; }
  constexpr int64_t ToMilliseconds() const { return ToMicroseconds() / 1000; }
  constexpr int64_t ToSeconds() const { return ToMilliseconds() / 1000; }

 private:
  explicit constexpr TimeUnit(int64_t nanos) : nanos_(nanos) {}
  int64_t nanos_ = 0;
};
}  // namespace base
}  // namespace weex

#endif  // CORE_BASE_TIME_UNIT_H
