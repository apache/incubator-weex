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
#ifndef CORE_DATA_RENDER_HANDLE_H
#define CORE_DATA_RENDER_HANDLE_H

#include <memory>
#include <type_traits>
#include "base/common.h"

namespace weex {
namespace core {
namespace data_render {

// Thread unsafe
class RefCountObject {
 public:
  explicit RefCountObject() : reference_count_(0) {}

  DISALLOW_COPY_AND_ASSIGN(RefCountObject);

  virtual ~RefCountObject() = default;

  inline int decrement() { return --reference_count_; }

  inline int increment() { return ++reference_count_; }

  inline int GetNumReferences() const { return reference_count_; }

 private:
  int reference_count_;
};

// class T should be a subtype of ReferenceCount class.
template <class T>
class Ref {
  // using Enable = typename std::enable_if<std::is_base_of<RefCountObject,
  // T>::value>::type;
 public:
  Ref(T *ptr = nullptr) : ptr_(ptr) {
    if (ptr_ == nullptr) return;
    ptr_->increment();  // defined in ReferenceCount
  }

  // Copy Constructor
  Ref(const Ref &ref) : ptr_{ref.ptr_} {
    if (ptr_ != nullptr) ptr_->increment();  // defined in ReferenceCount
  }

  // Destructor
  ~Ref() { clear(); }

  Ref<T> &operator=(const Ref<T> &ref) {
    clear();
    ptr_ = ref.ptr_;
    if (ptr_ != nullptr) {
      ptr_->increment();
    }
    return *this;
  }

  // Ref<T> &operator=(T *ptr) {
  //   clear();
  //   if (ptr != nullptr) {
  //     ptr_ = ptr;
  //   }

  //   if (ptr_ != nullptr) {
  //     ptr_->increment();
  //   }
  // }

  inline void clear() {
    if (ptr_ != nullptr) {
      ptr_->decrement();
      if (ptr_->GetNumReferences() <= 0) {
        delete ptr_;
      }
      ptr_ = nullptr;
    }
  }

  inline T &operator*() const noexcept { return *ptr_; }

  inline T *operator->() const noexcept { return ptr_; }

  // Check whether it holds an object.
  inline explicit operator bool() const noexcept { return ptr_ != nullptr; }

  inline bool operator<(const Ref<T> &ref) const noexcept {
    return GetPtr() < ref.GetPtr();
  }

  inline bool operator<=(const Ref<T> &ref) const noexcept {
    return GetPtr() <= ref.GetPtr();
  }

  inline bool operator==(const Ref<T> &ref) const {
    return ptr_ == ref.GetPtr();
  }

  inline bool operator!=(const Ref<T> &ref) const {
    return ptr_ != ref.GetPtr();
  }

  // preferably use this function over T *get()
  inline T *GetPtr() const { return ptr_; }

  // redundant func: mimics the library function for std::unique_ptr
  inline T *get() { return ptr_; }

  template <class B>
  inline operator Ref<B>() {
    // typename std::enable_if<std::is_base_of<B, T>::value>::type;
    return Ref<B>(reinterpret_cast<B *>(ptr_));
  }

 private:
  T *ptr_;
};

template <typename T>
using Handle = Ref<T>;

template <typename T, typename... Args>
inline Handle<T> MakeHandle(Args &&... args) {
  return Handle<T>(new T(std::forward<Args>(args)...));
}
    
template <typename T>
inline Handle<T> MakeHandle() {
    return Handle<T>(new T());
}

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif // CORE_DATA_RENDER_HANDLE_H
