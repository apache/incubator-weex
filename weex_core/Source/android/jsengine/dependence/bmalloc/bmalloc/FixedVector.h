/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#ifndef FixedVector_h
#define FixedVector_h

#include "BAssert.h"
#include <array>
#include <cstddef>
#include <type_traits>

namespace bmalloc {

// A replacement for std::vector that uses a fixed-sized inline backing store.

template<typename T, size_t Capacity>
class FixedVector {
    static_assert(std::is_trivially_destructible<T>::value, "FixedVector must have a trivial destructor.");
public:
    FixedVector(const FixedVector&) = delete;
    FixedVector& operator=(const FixedVector&) = delete;

    FixedVector();

    const T* begin() const { return &m_buffer[0]; }
    const T* end() const { return begin() + size(); }

    size_t size() const { return m_size; }
    size_t capacity() const { return Capacity; }
    
    T& operator[](size_t);

    void push(const T&);
    void push(const T*, const T*);
    T pop();

    void shrink(T*);
    void shrink(size_t);

    void clear() { shrink(static_cast<size_t>(0)); }
    bool isEmpty() { return !m_size; }

private:
    size_t m_size;
    std::array<T, Capacity> m_buffer;
};

template<typename T, size_t Capacity>
inline FixedVector<T, Capacity>::FixedVector()
    : m_size(0)
{
}

template<typename T, size_t Capacity>
inline T& FixedVector<T, Capacity>::operator[](size_t i)
{
    BASSERT(i < m_size);
    return m_buffer[i];
}

template<typename T, size_t Capacity>
inline void FixedVector<T, Capacity>::push(const T& value)
{
    BASSERT(m_size < Capacity);
    m_buffer[m_size++] = value;
}

template<typename T, size_t Capacity>
inline void FixedVector<T, Capacity>::push(const T* begin, const T* end)
{
    for (const T* it = begin; it != end; ++it)
        push(*it);
}

template<typename T, size_t Capacity>
inline T FixedVector<T, Capacity>::pop()
{
    BASSERT(m_size);
    return m_buffer[--m_size];
}

template<typename T, size_t Capacity>
inline void FixedVector<T, Capacity>::shrink(size_t size)
{
    BASSERT(size <= m_size);
    m_size = size;
}

template<typename T, size_t Capacity>
inline void FixedVector<T, Capacity>::shrink(T* end)
{
    shrink(end - begin());
}

} // namespace bmalloc

#endif // FixedVector_h
