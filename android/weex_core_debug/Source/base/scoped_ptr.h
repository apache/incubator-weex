
#ifndef WEEX_BASE_SCOPED_PTR_H_
#define WEEX_BASE_SCOPED_PTR_H_

#include <cstddef>

namespace base {

template<class T>
class ScopedPtr {
 public:
    using element_type = T;

    ScopedPtr(const ScopedPtr<element_type>& other) {
        ptr_ = other.ptr_;
        const_cast<ScopedPtr<element_type> *>(&other)->ptr_ = NULL;
    }

    ScopedPtr(ScopedPtr<element_type>& other) {
        ptr_ = other.ptr_;
        other.ptr_ = NULL;
    }

    explicit ScopedPtr(element_type* p) : ptr_(p) {}

    ScopedPtr() : ptr_(nullptr) {}

    ~ScopedPtr() {
        if (ptr_) {
            delete static_cast<element_type*>(ptr_);
        }
    }

    ScopedPtr& operator=(ScopedPtr&& scoped_ptr) {
        ptr_ = scoped_ptr.ptr_;
        scoped_ptr.ptr_ = NULL;
        return *this;
    }

    ScopedPtr& operator=(ScopedPtr& scoped_ptr) {
        ptr_ = scoped_ptr.ptr_;
        scoped_ptr.ptr_ = NULL;
        return *this;
    }

    void Reset(element_type* p = nullptr) {
        if (ptr_) {
            delete static_cast<element_type*>(ptr_);
        }
        ptr_ = p;
    }

    element_type* Get() const { return ptr_; }

    element_type* operator->() const { return ptr_; }

 private:
    element_type* ptr_;
};
}  // namespace base

#endif  // WEEX_BASE_SCOPED_PTR_H_
