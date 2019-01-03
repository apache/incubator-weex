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
