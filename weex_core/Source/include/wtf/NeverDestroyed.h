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
#ifndef NeverDestroyed_h
#define NeverDestroyed_h

#include <type_traits>
#include <utility>
#include <wtf/Noncopyable.h>
#include <wtf/RefCounted.h>

// NeverDestroyed is a smart pointer like class who ensures that the destructor
// for the given object is never called, but doesn't use the heap to allocate it.
// It's useful for static local variables, and can be used like so:
//
// MySharedGlobal& mySharedGlobal()
// {
//   static NeverDestroyed<MySharedGlobal> myGlobal("Hello", 42);
//   return myGlobal;
// }

namespace WTF {

template<typename T> class NeverDestroyed {
    WTF_MAKE_NONCOPYABLE(NeverDestroyed);

public:
    template<typename... Args>
    NeverDestroyed(Args&&... args)
    {
        MaybeRelax<T>(new (asPtr()) T(std::forward<Args>(args)...));
    }

    operator T&() { return *asPtr(); }
    T& get() { return *asPtr(); }

private:
    typedef typename std::remove_const<T>::type* PointerType;

    PointerType asPtr() { return reinterpret_cast<PointerType>(&m_storage); }

    // FIXME: Investigate whether we should allocate a hunk of virtual memory
    // and hand out chunks of it to NeverDestroyed instead, to reduce fragmentation.
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type m_storage;

    template <typename PtrType, bool ShouldRelax = std::is_base_of<RefCountedBase, PtrType>::value> struct MaybeRelax {
        explicit MaybeRelax(PtrType*) { }
    };
    template <typename PtrType> struct MaybeRelax<PtrType, true> {
        explicit MaybeRelax(PtrType* ptr) { ptr->relaxAdoptionRequirement(); }
    };
};

template<typename T> class LazyNeverDestroyed {
    WTF_MAKE_NONCOPYABLE(LazyNeverDestroyed);

public:
    LazyNeverDestroyed() = default;

    template<typename... Args>
    void construct(Args&&... args)
    {
        ASSERT(!m_isConstructed);

#if !ASSERT_DISABLED
        m_isConstructed = true;
#endif

        MaybeRelax<T>(new (asPtr()) T(std::forward<Args>(args)...));
    }

    operator T&() { return *asPtr(); }
    T& get() { return *asPtr(); }

    T* operator->() { return asPtr(); }

private:
    typedef typename std::remove_const<T>::type* PointerType;

    PointerType asPtr()
    {
        ASSERT(m_isConstructed);

        return reinterpret_cast<PointerType>(&m_storage);
    }

    // FIXME: Investigate whether we should allocate a hunk of virtual memory
    // and hand out chunks of it to NeverDestroyed instead, to reduce fragmentation.
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type m_storage;

    template <typename PtrType, bool ShouldRelax = std::is_base_of<RefCountedBase, PtrType>::value> struct MaybeRelax {
        explicit MaybeRelax(PtrType*) { }
    };
    template <typename PtrType> struct MaybeRelax<PtrType, true> {
        explicit MaybeRelax(PtrType* ptr) { ptr->relaxAdoptionRequirement(); }
    };

#if !ASSERT_DISABLED
    // LazyNeverDestroyed objects are always static, so this variable is initialized to false.
    // It must not be initialized dynamically, because that would not be thread safe.
    bool m_isConstructed;
#endif
};

} // namespace WTF;

using WTF::LazyNeverDestroyed;
using WTF::NeverDestroyed;

#endif // NeverDestroyed_h
