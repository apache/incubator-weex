/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_NakedPtr_h
#define WTF_NakedPtr_h

#include <wtf/FastMalloc.h>

namespace WTF {

// The purpose of this class is to ensure that the wrapped pointer will never be
// used uninitialized.

template <typename T> class NakedPtr {
    WTF_MAKE_FAST_ALLOCATED;
public:
    ALWAYS_INLINE NakedPtr() : m_ptr(nullptr) { }
    ALWAYS_INLINE NakedPtr(T* ptr) : m_ptr(ptr) { }
    ALWAYS_INLINE NakedPtr(const NakedPtr& o) : m_ptr(o.m_ptr) { }
    template<typename U> NakedPtr(const NakedPtr<U>& o) : m_ptr(o.get()) { }

    ALWAYS_INLINE NakedPtr(NakedPtr&& o) : m_ptr(o.get()) { }
    template<typename U> NakedPtr(NakedPtr<U>&& o) : m_ptr(o.get()) { }

    T* get() const { return m_ptr; }

    void clear() { m_ptr = nullptr; }

    T& operator*() const { ASSERT(m_ptr); return *m_ptr; }
    ALWAYS_INLINE T* operator->() const { return m_ptr; }

    operator T*() { return m_ptr; }

    bool operator!() const { return !m_ptr; }
    
    explicit operator bool() const { return !!m_ptr; }

    NakedPtr& operator=(const NakedPtr&);
    NakedPtr& operator=(T*);
    template<typename U> NakedPtr& operator=(const NakedPtr<U>&);
    NakedPtr& operator=(NakedPtr&&);
    template<typename U> NakedPtr& operator=(NakedPtr<U>&&);

    void swap(NakedPtr&);

private:
    T* m_ptr;
};

template<typename T> inline NakedPtr<T>& NakedPtr<T>::operator=(const NakedPtr& o)
{
    m_ptr = o.m_ptr;
    return *this;
}

template<typename T> inline NakedPtr<T>& NakedPtr<T>::operator=(T* optr)
{
    m_ptr = optr;
    return *this;
}

template<typename T> template<typename U> inline NakedPtr<T>& NakedPtr<T>::operator=(const NakedPtr<U>& o)
{
    NakedPtr ptr = o;
    swap(ptr);
    return *this;
}

template<typename T> inline NakedPtr<T>& NakedPtr<T>::operator=(NakedPtr&& o)
{
    NakedPtr ptr = WTFMove(o);
    swap(ptr);
    return *this;
}

template<typename T> template<typename U> inline NakedPtr<T>& NakedPtr<T>::operator=(NakedPtr<U>&& o)
{
    NakedPtr ptr = WTFMove(o);
    swap(ptr);
    return *this;
}

template<class T> inline void NakedPtr<T>::swap(NakedPtr& o)
{
    std::swap(m_ptr, o.m_ptr);
}

template<class T> inline void swap(NakedPtr<T>& a, NakedPtr<T>& b)
{
    a.swap(b);
}

} // namespace WTF

using WTF::NakedPtr;

#endif // WTF_NakedPtr_h
