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
#pragma once

#include "JSExportMacros.h"
#include <cstddef>
#include <wtf/HashTraits.h>
#include <wtf/Noncopyable.h>
#include <wtf/VectorTraits.h>

namespace JSC {

class WeakImpl;
class WeakHandleOwner;

// This is a free function rather than a Weak<T> member function so we can put it in Weak.cpp.
JS_EXPORT_PRIVATE void weakClearSlowCase(WeakImpl*&);

template<typename T> class Weak {
    WTF_MAKE_NONCOPYABLE(Weak);
public:
    Weak()
        : m_impl(0)
    {
    }

    Weak(std::nullptr_t)
        : m_impl(0)
    {
    }

    inline Weak(T*, WeakHandleOwner* = 0, void* context = 0);

    enum HashTableDeletedValueTag { HashTableDeletedValue };
    inline bool isHashTableDeletedValue() const;
    inline Weak(HashTableDeletedValueTag);

    inline Weak(Weak&&);

    ~Weak()
    {
        clear();
    }

    inline void swap(Weak&);

    inline Weak& operator=(Weak&&);

    inline bool operator!() const;
    inline T* operator->() const;
    inline T& operator*() const;
    inline T* get() const;

    inline bool was(T*) const;

    inline explicit operator bool() const;

    inline WeakImpl* leakImpl() WARN_UNUSED_RETURN;
    void clear()
    {
        if (!m_impl)
            return;
        weakClearSlowCase(m_impl);
    }
    
private:
    static inline WeakImpl* hashTableDeletedValue();

    WeakImpl* m_impl;
};

} // namespace JSC

namespace WTF {

template<typename T> struct VectorTraits<JSC::Weak<T>> : SimpleClassVectorTraits {
    static const bool canCompareWithMemcmp = false;
};

template<typename T> struct HashTraits<JSC::Weak<T>> : SimpleClassHashTraits<JSC::Weak<T>> {
    typedef JSC::Weak<T> StorageType;

    typedef std::nullptr_t EmptyValueType;
    static EmptyValueType emptyValue() { return nullptr; }

    typedef T* PeekType;
    static PeekType peek(const StorageType& value) { return value.get(); }
    static PeekType peek(EmptyValueType) { return PeekType(); }
};

} // namespace WTF
