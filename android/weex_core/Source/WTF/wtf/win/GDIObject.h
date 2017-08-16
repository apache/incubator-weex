/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#ifndef GDIObject_h
#define GDIObject_h

#include <algorithm>
#include <cstddef>
#include <memory>
#include <windows.h>
#include <wtf/Assertions.h>
#include <wtf/Noncopyable.h>

namespace WTF {

template<typename T> void deleteObject(T);

template<typename T> class GDIObject {
    WTF_MAKE_NONCOPYABLE(GDIObject);
public:
    GDIObject() : m_object(0) { }
    GDIObject(std::nullptr_t) : m_object(0) { }
    ~GDIObject() { deleteObject<T>(m_object); }

    T get() const { return m_object; }

    void clear();
    T leak() WARN_UNUSED_RETURN;

    bool operator!() const { return !m_object; }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef const void* UnspecifiedBoolType;
    operator UnspecifiedBoolType() const { return m_object ? reinterpret_cast<UnspecifiedBoolType>(&m_object) : 0; }

    GDIObject<T>& operator=(std::nullptr_t) { clear(); return *this; }

    GDIObject(GDIObject&&);
    template<typename U> GDIObject(GDIObject<U>&&);

    GDIObject& operator=(GDIObject&&);
    template<typename U> GDIObject& operator=(GDIObject<U>&&);

    void swap(GDIObject& o) { std::swap(m_object, o.m_object); }

private:
    template<typename U> friend GDIObject<U> adoptGDIObject(U);
    GDIObject(T object) : m_object(object) { }

    GDIObject<T>& operator=(T);

    T m_object;
};

template<typename T> inline void GDIObject<T>::clear()
{
    T object = m_object;
    m_object = 0;
    deleteObject(object);
}

template<typename T> inline T GDIObject<T>::leak()
{
    T object = m_object;
    m_object = 0;
    return object;
}

template<typename T> inline GDIObject<T>::GDIObject(GDIObject<T>&& other)
    : m_object(other.leak())
{
}

template<typename T> inline GDIObject<T>& GDIObject<T>::operator=(GDIObject<T>&& other)
{
    auto object = WTFMove(other);
    swap(object);
    return *this;
}

template<typename T> inline GDIObject<T> adoptGDIObject(T object)
{
    return GDIObject<T>(object);
}

template<typename T> inline void swap(GDIObject<T>& a, GDIObject<T>& b)
{
    a.swap(b);
}

// Nearly all GDI types use the same DeleteObject call.
template<typename T> inline void deleteObject(T object)
{
    if (object)
        ::DeleteObject(object);
}

template<> inline void deleteObject<HDC>(HDC hdc)
{
    if (hdc)
        ::DeleteDC(hdc);
}

} // namespace WTF

using WTF::GDIObject;
using WTF::adoptGDIObject;

#endif // GDIObject_h
