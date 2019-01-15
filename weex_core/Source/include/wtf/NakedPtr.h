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
