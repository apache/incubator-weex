/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_Ref_h
#define WTF_Ref_h

#include <wtf/Assertions.h>
#include <wtf/GetPtr.h>
#include <wtf/Noncopyable.h>
#include <wtf/StdLibExtras.h>
#include <wtf/TypeCasts.h>

#if ASAN_ENABLED
extern "C" void __asan_poison_memory_region(void const volatile *addr, size_t size);
extern "C" void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
extern "C" int __asan_address_is_poisoned(void const volatile *addr);
#endif

namespace WTF {

inline void adopted(const void*) { }

template<typename T> class Ref;
template<typename T> Ref<T> adoptRef(T&);

template<typename T> class Ref {
public:
    static constexpr bool isRef = true;

    ~Ref()
    {
#if ASAN_ENABLED
        if (__asan_address_is_poisoned(this))
            __asan_unpoison_memory_region(this, sizeof(*this));
#endif
        if (m_ptr)
            m_ptr->deref();
    }

    Ref(T& object)
        : m_ptr(&object)
    {
        m_ptr->ref();
    }

    // Use copyRef() instead.
    Ref(const Ref& other) = delete;
    template<typename U> Ref(const Ref<U>& other) = delete;

    Ref(Ref&& other)
        : m_ptr(&other.leakRef())
    {
        ASSERT(m_ptr);
    }

    template<typename U>
    Ref(Ref<U>&& other)
        : m_ptr(&other.leakRef())
    {
        ASSERT(m_ptr);
    }

    Ref& operator=(T& object)
    {
        ASSERT(m_ptr);
        object.ref();
        m_ptr->deref();
        m_ptr = &object;
        ASSERT(m_ptr);
        return *this;
    }

    // Use copyRef() and the move assignment operators instead.
    Ref& operator=(const Ref& reference) = delete;
    template<typename U> Ref& operator=(const Ref<U>& reference) = delete;

    Ref& operator=(Ref&& reference)
    {
        ASSERT(m_ptr);
        m_ptr->deref();
        m_ptr = &reference.leakRef();
        ASSERT(m_ptr);
        return *this;
    }

    template<typename U> Ref& operator=(Ref<U>&& reference)
    {
        ASSERT(m_ptr);
        m_ptr->deref();
        m_ptr = &reference.leakRef();
        ASSERT(m_ptr);
        return *this;
    }

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    Ref(HashTableDeletedValueType) : m_ptr(hashTableDeletedValue()) { }
    bool isHashTableDeletedValue() const { return m_ptr == hashTableDeletedValue(); }
    static T* hashTableDeletedValue() { return reinterpret_cast<T*>(-1); }

    Ref(HashTableEmptyValueType) : m_ptr(hashTableEmptyValue()) { }
    bool isHashTableEmptyValue() const { return m_ptr == hashTableEmptyValue(); }
    static T* hashTableEmptyValue() { return nullptr; }

    const T* ptrAllowingHashTableEmptyValue() const { ASSERT(m_ptr || isHashTableEmptyValue()); return m_ptr; }
    T* ptrAllowingHashTableEmptyValue() { ASSERT(m_ptr || isHashTableEmptyValue()); return m_ptr; }

    void assignToHashTableEmptyValue(Ref&& reference)
    {
        ASSERT(m_ptr == hashTableEmptyValue());
        m_ptr = &reference.leakRef();
        ASSERT(m_ptr);
    }

    T* operator->() const { ASSERT(m_ptr); return m_ptr; }
    T* ptr() const { ASSERT(m_ptr); return m_ptr; }
    T& get() const { ASSERT(m_ptr); return *m_ptr; }
    operator T&() const { ASSERT(m_ptr); return *m_ptr; }
    bool operator!() const { ASSERT(m_ptr); return !*m_ptr; }

    template<typename U> Ref<T> replace(Ref<U>&&) WARN_UNUSED_RETURN;

#if COMPILER_SUPPORTS(CXX_REFERENCE_QUALIFIED_FUNCTIONS)
    Ref copyRef() && = delete;
    Ref copyRef() const & WARN_UNUSED_RETURN { return Ref(*m_ptr); }
#else
    Ref copyRef() const WARN_UNUSED_RETURN { return Ref(*m_ptr); }
#endif

    T& leakRef() WARN_UNUSED_RETURN
    {
        ASSERT(m_ptr);

        T& result = *std::exchange(m_ptr, nullptr);
#if ASAN_ENABLED
        __asan_poison_memory_region(this, sizeof(*this));
#endif
        return result;
    }

private:
    friend Ref adoptRef<T>(T&);

    enum AdoptTag { Adopt };
    Ref(T& object, AdoptTag)
        : m_ptr(&object)
    {
    }

    T* m_ptr;
};

template<typename T> template<typename U> inline Ref<T> Ref<T>::replace(Ref<U>&& reference)
{
    auto oldReference = adoptRef(*m_ptr);
    m_ptr = &reference.leakRef();
    return oldReference;
}

template<typename T, typename U> inline Ref<T> static_reference_cast(Ref<U>& reference)
{
    return Ref<T>(static_cast<T&>(reference.get()));
}

template<typename T, typename U> inline Ref<T> static_reference_cast(Ref<U>&& reference)
{
    return adoptRef(static_cast<T&>(reference.leakRef()));
}

template<typename T, typename U> inline Ref<T> static_reference_cast(const Ref<U>& reference)
{
    return Ref<T>(static_cast<T&>(reference.copyRef().get()));
}

template <typename T>
struct GetPtrHelper<Ref<T>> {
    typedef T* PtrType;
    static T* getPtr(const Ref<T>& p) { return const_cast<T*>(p.ptr()); }
};

template <typename T> 
struct IsSmartPtr<Ref<T>> {
    static const bool value = true;
};

template<typename T>
inline Ref<T> adoptRef(T& reference)
{
    adopted(&reference);
    return Ref<T>(reference, Ref<T>::Adopt);
}

template<typename T>
inline Ref<T> makeRef(T& reference)
{
    return Ref<T>(reference);
}

template<typename ExpectedType, typename ArgType> inline bool is(Ref<ArgType>& source)
{
    return is<ExpectedType>(source.get());
}

template<typename ExpectedType, typename ArgType> inline bool is(const Ref<ArgType>& source)
{
    return is<ExpectedType>(source.get());
}

} // namespace WTF

using WTF::Ref;
using WTF::adoptRef;
using WTF::makeRef;
using WTF::static_reference_cast;

#endif // WTF_Ref_h
