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
#ifndef MallocPtr_h
#define MallocPtr_h

// MallocPtr is a smart pointer class that calls fastFree in its destructor.
// It is intended to be used for pointers where the C++ lifetime semantics
// (calling constructors and destructors) is not desired. 

namespace WTF {

template<typename T> class MallocPtr {
public:
    MallocPtr()
        : m_ptr(nullptr)
    {
    }

    MallocPtr(std::nullptr_t)
        : m_ptr(nullptr)
    {
    }

    MallocPtr(MallocPtr&& other)
        : m_ptr(other.leakPtr())
    {
    }

    ~MallocPtr()
    {
        fastFree(m_ptr);
    }

    T* get() const
    {
        return m_ptr;
    }

    T *leakPtr() WARN_UNUSED_RETURN
    {
        return std::exchange(m_ptr, nullptr);
    }

    bool operator!() const
    {
        return !m_ptr;
    }

    MallocPtr& operator=(MallocPtr&& other)
    {
        MallocPtr ptr = WTFMove(other);
        swap(ptr);

        return *this;
    }

    void swap(MallocPtr& other)
    {
        std::swap(m_ptr, other.m_ptr);
    }

    template<typename U> friend MallocPtr<U> adoptMallocPtr(U*);

    static MallocPtr malloc(size_t size)
    {
        MallocPtr mallocPtr;
        mallocPtr.m_ptr = static_cast<T*>(fastMalloc(size));

        return mallocPtr;
    }

    void realloc(size_t newSize)
    {
        m_ptr = static_cast<T*>(fastRealloc(m_ptr, newSize));
    }

private:
    explicit MallocPtr(T* ptr)
        : m_ptr(ptr)
    {
    }

    T* m_ptr;
};

template<typename U> MallocPtr<U> adoptMallocPtr(U* ptr)
{
    return MallocPtr<U>(ptr);
}

} // namespace WTF

using WTF::MallocPtr;
using WTF::adoptMallocPtr;

#endif // MallocPtr_h
