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
#ifndef WTF_WeakPtr_h
#define WTF_WeakPtr_h

#include <wtf/Noncopyable.h>
#include <wtf/Ref.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/Threading.h>

#if USE(WEB_THREAD)
#include <wtf/MainThread.h>
#endif

namespace WTF {

template<typename T> class WeakPtr;
template<typename T> class WeakPtrFactory;

// Note: WeakReference is an implementation detail, and should not be used directly.
template<typename T>
class WeakReference : public ThreadSafeRefCounted<WeakReference<T>> {
    WTF_MAKE_NONCOPYABLE(WeakReference<T>);
    WTF_MAKE_FAST_ALLOCATED;
public:
    T* get() const
    {
#if USE(WEB_THREAD)
        ASSERT(canAccessThreadLocalDataForThread(m_boundThread));
#else
        ASSERT(m_boundThread == currentThread());
#endif
        return m_ptr;
    }

    void clear()
    {
#if USE(WEB_THREAD)
        ASSERT(canAccessThreadLocalDataForThread(m_boundThread));
#else
        ASSERT(m_boundThread == currentThread());
#endif
        m_ptr = nullptr;
    }

private:
    friend class WeakPtr<T>;
    friend class WeakPtrFactory<T>;

    static Ref<WeakReference<T>> create(T* ptr) { return adoptRef(*new WeakReference(ptr)); }

    explicit WeakReference(T* ptr)
        : m_ptr(ptr)
#ifndef NDEBUG
        , m_boundThread(currentThread())
#endif
    {
    }

    T* m_ptr;
#ifndef NDEBUG
    ThreadIdentifier m_boundThread;
#endif
};

template<typename T>
class WeakPtr {
    WTF_MAKE_FAST_ALLOCATED;
public:
    WeakPtr() : m_ref(WeakReference<T>::create(nullptr)) { }
    WeakPtr(const WeakPtr& o) : m_ref(o.m_ref.copyRef()) { }
    template<typename U> WeakPtr(const WeakPtr<U>& o) : m_ref(o.m_ref.copyRef()) { }

    T* get() const { return m_ref->get(); }
    operator bool() const { return m_ref->get(); }

    WeakPtr& operator=(const WeakPtr& o) { m_ref = o.m_ref.copyRef(); return *this; }
    WeakPtr& operator=(std::nullptr_t) { m_ref = WeakReference<T>::create(nullptr); return *this; }

    T* operator->() const { return m_ref->get(); }

    void clear() { m_ref = WeakReference<T>::create(nullptr); }

private:
    friend class WeakPtrFactory<T>;
    WeakPtr(Ref<WeakReference<T>>&& ref) : m_ref(std::forward<Ref<WeakReference<T>>>(ref)) { }

    Ref<WeakReference<T>> m_ref;
};

template<typename T>
class WeakPtrFactory {
    WTF_MAKE_NONCOPYABLE(WeakPtrFactory<T>);
    WTF_MAKE_FAST_ALLOCATED;
public:
    explicit WeakPtrFactory(T* ptr) : m_ref(WeakReference<T>::create(ptr)) { }

    ~WeakPtrFactory() { m_ref->clear(); }

    // We should consider having createWeakPtr populate m_ref the first time createWeakPtr is called.
    WeakPtr<T> createWeakPtr() const { return WeakPtr<T>(m_ref.copyRef()); }

    void revokeAll()
    {
        T* ptr = m_ref->get();
        m_ref->clear();
        // We create a new WeakReference so that future calls to createWeakPtr() create nonzero WeakPtrs.
        m_ref = WeakReference<T>::create(ptr);
    }

private:
    Ref<WeakReference<T>> m_ref;
};

template<typename T, typename U> inline bool operator==(const WeakPtr<T>& a, const WeakPtr<U>& b)
{
    return a.get() == b.get();
}

template<typename T, typename U> inline bool operator==(const WeakPtr<T>& a, U* b)
{
    return a.get() == b;
}

template<typename T, typename U> inline bool operator==(T* a, const WeakPtr<U>& b)
{
    return a == b.get();
}

template<typename T, typename U> inline bool operator!=(const WeakPtr<T>& a, const WeakPtr<U>& b)
{
    return a.get() != b.get();
}

template<typename T, typename U> inline bool operator!=(const WeakPtr<T>& a, U* b)
{
    return a.get() != b;
}

template<typename T, typename U> inline bool operator!=(T* a, const WeakPtr<U>& b)
{
    return a != b.get();
}

} // namespace WTF

using WTF::WeakPtr;
using WTF::WeakPtrFactory;
using WTF::WeakReference;

#endif
