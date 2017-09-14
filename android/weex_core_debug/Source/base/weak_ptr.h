// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_WEAK_PTR_H_
#define WEEX_BASE_WEAK_PTR_H_

#include "ref_counted_ptr.h"

namespace base {

template<class T>
class WeakPtr {
 public:
    using type_name = T;
    class Flag : public RefCountPtr<Flag> {
     public:
        Flag(): is_valid_(true) {
        }
        void Invalidate() { is_valid_ = false; }
        bool IsValid() { return is_valid_; }
     private:
        bool is_valid_;
    };

    WeakPtr() : ptr_(0), invalidate_flag_(0) {}

    WeakPtr(type_name* ptr) :
            ptr_(ptr),
            invalidate_flag_(new Flag) {
            }

    WeakPtr(type_name* ptr, Flag* flag) :
            ptr_(ptr),
            invalidate_flag_(flag) {
            }

    WeakPtr(WeakPtr<type_name>& other) :
            ptr_(other.ptr_),
            invalidate_flag_(other.invalidate_flag_) {
            }

    WeakPtr(const WeakPtr<type_name>& other) :
            ptr_(other.ptr_),
            invalidate_flag_(other.invalidate_flag_) {
            }

    ~WeakPtr() {
    }

    type_name* Get() { return ptr_; }

    void Invalidate() { invalidate_flag_->Invalidate(); }

    bool IsValid() {
        return invalidate_flag_.Get() != NULL
                && invalidate_flag_.Get()->IsValid();
    }

    type_name* operator->() const { return ptr_; }

    WeakPtr<type_name>& operator=(WeakPtr<type_name>& other) {
        ptr_ = other.ptr_;
        invalidate_flag_ = other.invalidate_flag_;
        return *this;
    }

 private:
    type_name* ptr_;
    ScopedRefPtr<Flag> invalidate_flag_;
};
}  // namespace base

#endif  // WEEX_BASE_WEAK_PTR_H_
