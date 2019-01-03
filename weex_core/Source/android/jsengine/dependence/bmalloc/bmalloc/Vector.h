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
#ifndef Vector_h
#define Vector_h

#include "Inline.h"
#include "VMAllocate.h"
#include <cstddef>
#include <cstring>

namespace bmalloc {

// A replacement for std::vector that allocates using vmAllocate instead of
// malloc, shrinks automatically, and supports "popping" from the middle.

template<typename T>
class Vector {
    static_assert(std::is_trivially_destructible<T>::value, "Vector must have a trivial destructor.");
public:
    typedef T* iterator;
    typedef const T* const_iterator;

    Vector();
    Vector(Vector&&);
    ~Vector();

    iterator begin() { return m_buffer; }
    iterator end() { return m_buffer + m_size; }

    size_t size() { return m_size; }
    size_t capacity() { return m_capacity; }
    
    T& operator[](size_t);
    T& last() { return m_buffer[m_size - 1]; }

    void push(const T&);

    T pop();
    T pop(size_t);
    T pop(const_iterator it) { return pop(it - begin()); }
    
    void insert(iterator, const T&);
    T remove(iterator);

    void grow(size_t);
    void shrink(size_t);
    void resize(size_t);

    void shrinkToFit();

private:
    static const size_t growFactor = 2;
    static const size_t shrinkFactor = 4;
    static size_t initialCapacity() { return vmPageSize() / sizeof(T); }

    void growCapacity();
    void shrinkCapacity();
    void reallocateBuffer(size_t);

    T* m_buffer;
    size_t m_size;
    size_t m_capacity;
};

template<typename T>
inline Vector<T>::Vector()
    : m_buffer(nullptr)
    , m_size(0)
    , m_capacity(0)
{
}

template<typename T>
inline Vector<T>::Vector(Vector&& other)
    : m_buffer(other.m_buffer)
    , m_size(other.m_size)
    , m_capacity(other.m_capacity)
{
    other.m_buffer = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
}

template<typename T>
Vector<T>::~Vector()
{
    if (m_buffer)
        vmDeallocate(m_buffer, vmSize(m_capacity * sizeof(T)));
}

template<typename T>
inline T& Vector<T>::operator[](size_t i)
{
    BASSERT(i < m_size);
    return m_buffer[i];
}

template<typename T>
INLINE void Vector<T>::push(const T& value)
{
    if (m_size == m_capacity)
        growCapacity();
    m_buffer[m_size++] = value;
}

template<typename T>
inline T Vector<T>::pop()
{
    BASSERT(m_size);
    T value = m_buffer[m_size - 1];
    shrink(m_size - 1);
    return value;
}

template<typename T>
inline T Vector<T>::pop(size_t i)
{
    BASSERT(i < m_size);
    std::swap(m_buffer[i], last());
    return pop();
}

template<typename T>
void Vector<T>::insert(iterator it, const T& value)
{
    size_t index = it - begin();
    size_t moveCount = end() - it;

    grow(m_size + 1);
    std::memmove(&m_buffer[index + 1], &m_buffer[index], moveCount * sizeof(T));

    m_buffer[index] = value;
}

template<typename T>
T Vector<T>::remove(iterator it)
{
    size_t index = it - begin();
    size_t moveCount = end() - it - 1;

    T result = *it;

    std::memmove(&m_buffer[index], &m_buffer[index + 1], moveCount * sizeof(T));
    shrink(m_size - 1);
    
    return result;
}

template<typename T>
inline void Vector<T>::grow(size_t size)
{
    BASSERT(size >= m_size);
    while (m_size < size)
        push(T());
}

template<typename T>
inline void Vector<T>::shrink(size_t size)
{
    BASSERT(size <= m_size);
    m_size = size;
    if (m_size < m_capacity / shrinkFactor && m_capacity > initialCapacity())
        shrinkCapacity();
}

template<typename T>
inline void Vector<T>::resize(size_t size)
{
    if (size <= m_size)
        shrink(size);
    else
        grow(size);
}

template<typename T>
void Vector<T>::reallocateBuffer(size_t newCapacity)
{
    RELEASE_BASSERT(newCapacity < std::numeric_limits<size_t>::max() / sizeof(T));

    size_t vmSize = bmalloc::vmSize(newCapacity * sizeof(T));
    T* newBuffer = vmSize ? static_cast<T*>(vmAllocate(vmSize)) : nullptr;
    if (m_buffer) {
        std::memcpy(newBuffer, m_buffer, m_size * sizeof(T));
        vmDeallocate(m_buffer, bmalloc::vmSize(m_capacity * sizeof(T)));
    }

    m_buffer = newBuffer;
    m_capacity = vmSize / sizeof(T);
}

template<typename T>
NO_INLINE void Vector<T>::shrinkCapacity()
{
    size_t newCapacity = max(initialCapacity(), m_capacity / shrinkFactor);
    reallocateBuffer(newCapacity);
}

template<typename T>
NO_INLINE void Vector<T>::growCapacity()
{
    size_t newCapacity = max(initialCapacity(), m_size * growFactor);
    reallocateBuffer(newCapacity);
}

template<typename T>
void Vector<T>::shrinkToFit()
{
    if (m_size < m_capacity)
        reallocateBuffer(m_size);
}

} // namespace bmalloc

#endif // Vector_h
