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

#ifndef BASE_MAKE_COPYABLE_H_
#define BASE_MAKE_COPYABLE_H_

#include <memory>
#include <utility>

namespace weex {
namespace base {

template <typename T>
class CopyableLambda {
 public:
  explicit CopyableLambda(T func)
      : impl_(std::make_shared<Impl>(std::move(func))) {}

  template <typename... ArgType>
  void operator()(ArgType&&... args) const {
    impl_->func_(std::forward<ArgType>(args)...);
  }

 private:
  class Impl {
   public:
    explicit Impl(T func) : func_(std::move(func)) {}
    T func_;
  };

  std::shared_ptr<Impl> impl_;
};

// Sample:
// std::unique_ptr<Foo> foo = ...
// std::function<void()> func =
//     MakeCopyable([bar = std::move(foo)]() { return bar->count(); });
template <typename T>
CopyableLambda<T> MakeCopyable(T lambda) {
  return CopyableLambda<T>(std::move(lambda));
}

}  // namespace base

}  // namespace weex

#endif  // BASE_MAKE_COPYABLE_H_
