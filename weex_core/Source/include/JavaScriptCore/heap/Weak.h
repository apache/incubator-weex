/*
 * Copyright (C) 2009, 2012, 2013, 2016 Apple Inc. All rights reserved.
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
