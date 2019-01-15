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
#ifndef WTF_PassRefPtr_h
#define WTF_PassRefPtr_h

#include <wtf/Ref.h>

namespace WTF {

    template<typename T> class RefPtr;
    template<typename T> class PassRefPtr;

    template<typename T> ALWAYS_INLINE void refIfNotNull(T* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->ref();
    }

    template<typename T> ALWAYS_INLINE void derefIfNotNull(T* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->deref();
    }

    template<typename T> class PassRefPtr {
    public:
        typedef T ValueType;
        typedef ValueType* PtrType;

        PassRefPtr() : m_ptr(nullptr) { }
        PassRefPtr(T* ptr) : m_ptr(ptr) { refIfNotNull(ptr); }
        // It somewhat breaks the type system to allow transfer of ownership out of
        // a const PassRefPtr. However, it makes it much easier to work with PassRefPtr
        // temporaries, and we don't have a need to use real const PassRefPtrs anyway.
        PassRefPtr(const PassRefPtr& o) : m_ptr(o.leakRef()) { }
        template<typename U> PassRefPtr(const PassRefPtr<U>& o) : m_ptr(o.leakRef()) { }

        ALWAYS_INLINE ~PassRefPtr() { derefIfNotNull(std::exchange(m_ptr, nullptr)); }

        template<typename U> PassRefPtr(const RefPtr<U>&);
        template<typename U> PassRefPtr(Ref<U>&& reference) : m_ptr(&reference.leakRef()) { }
        template<typename U> PassRefPtr(RefPtr<U>&& reference) : m_ptr(reference.leakRef()) { }

        T* get() const { return m_ptr; }

        T* leakRef() const WARN_UNUSED_RETURN;

        T& operator*() const { return *m_ptr; }
        T* operator->() const { return m_ptr; }

        bool operator!() const { return !m_ptr; }

        // This conversion operator allows implicit conversion to bool but not to other integer types.
        typedef T* (PassRefPtr::*UnspecifiedBoolType);
        operator UnspecifiedBoolType() const { return m_ptr ? &PassRefPtr::m_ptr : nullptr; }

        template<typename V, typename U> friend PassRefPtr<V> static_pointer_cast(const PassRefPtr<U>&);

    private:
        PassRefPtr& operator=(const PassRefPtr&) = delete;

        enum AdoptTag { Adopt };
        PassRefPtr(T* ptr, AdoptTag) : m_ptr(ptr) { }

        mutable T* m_ptr;
    };
    
    template<typename T> template<typename U> inline PassRefPtr<T>::PassRefPtr(const RefPtr<U>& o)
        : m_ptr(o.get())
    {
        T* ptr = m_ptr;
        refIfNotNull(ptr);
    }

    template<typename T> inline T* PassRefPtr<T>::leakRef() const
    {
        return std::exchange(m_ptr, nullptr);
    }

    template<typename T, typename U> inline bool operator==(const PassRefPtr<T>& a, const PassRefPtr<U>& b) 
    { 
        return a.get() == b.get(); 
    }

    template<typename T, typename U> inline bool operator==(const PassRefPtr<T>& a, const RefPtr<U>& b) 
    { 
        return a.get() == b.get(); 
    }

    template<typename T, typename U> inline bool operator==(const RefPtr<T>& a, const PassRefPtr<U>& b) 
    { 
        return a.get() == b.get(); 
    }

    template<typename T, typename U> inline bool operator==(const PassRefPtr<T>& a, U* b) 
    { 
        return a.get() == b; 
    }
    
    template<typename T, typename U> inline bool operator==(T* a, const PassRefPtr<U>& b) 
    {
        return a == b.get(); 
    }
    
    template<typename T, typename U> inline bool operator!=(const PassRefPtr<T>& a, const PassRefPtr<U>& b) 
    { 
        return a.get() != b.get(); 
    }

    template<typename T, typename U> inline bool operator!=(const PassRefPtr<T>& a, const RefPtr<U>& b) 
    { 
        return a.get() != b.get(); 
    }

    template<typename T, typename U> inline bool operator!=(const RefPtr<T>& a, const PassRefPtr<U>& b) 
    { 
        return a.get() != b.get(); 
    }

    template<typename T, typename U> inline bool operator!=(const PassRefPtr<T>& a, U* b)
    {
        return a.get() != b; 
    }

    template<typename T, typename U> inline bool operator!=(T* a, const PassRefPtr<U>& b) 
    { 
        return a != b.get(); 
    }

    template<typename V, typename U> inline PassRefPtr<V> static_pointer_cast(const PassRefPtr<U>& p)
    {
        return PassRefPtr<V>(static_cast<V*>(p.leakRef()), PassRefPtr<V>::Adopt);
    }

    template <typename T> struct IsSmartPtr<PassRefPtr<T>> {
        static const bool value = true;
    };

} // namespace WTF

using WTF::PassRefPtr;
using WTF::static_pointer_cast;

#endif // WTF_PassRefPtr_h
