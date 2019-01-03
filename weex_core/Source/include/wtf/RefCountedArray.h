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
#ifndef RefCountedArray_h
#define RefCountedArray_h

#include <wtf/FastMalloc.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

// This implements a reference counted array for POD** values, which is optimized for:
// - An empty array only uses one word.
// - A copy of the array only uses one word (i.e. assignment means aliasing).
// - The vector can't grow beyond 2^32-1 elements.
// - In all other regards this has similar space usage to a Vector.
//
// ** This could be modified to support non-POD values quite easily. It just
//    hasn't been, so far, because there has been no need. Moreover, even now,
//    it's used for things that aren't quite POD according to the official
//    defintion, such as JSC::Instruction.

namespace WTF {

template<typename T>
class RefCountedArray {
public:
    RefCountedArray()
        : m_data(0)
    {
    }
    
    RefCountedArray(const RefCountedArray& other)
        : m_data(other.m_data)
    {
        if (m_data)
            Header::fromPayload(m_data)->refCount++;
    }

    explicit RefCountedArray(size_t size)
    {
        if (!size) {
            m_data = 0;
            return;
        }

        m_data = (static_cast<Header*>(fastMalloc(Header::size() + sizeof(T) * size)))->payload();
        Header::fromPayload(m_data)->refCount = 1;
        Header::fromPayload(m_data)->length = size;
        ASSERT(Header::fromPayload(m_data)->length == size);
        VectorTypeOperations<T>::initialize(begin(), end());
    }

    RefCountedArray clone() const
    {
        RefCountedArray result(size());
        for (unsigned i = size(); i--;)
            result[i] = at(i);
        return result;
    }

    template<size_t inlineCapacity, typename OverflowHandler>
    explicit RefCountedArray(const Vector<T, inlineCapacity, OverflowHandler>& other)
    {
        if (other.isEmpty()) {
            m_data = 0;
            return;
        }
        
        m_data = (static_cast<Header*>(fastMalloc(Header::size() + sizeof(T) * other.size())))->payload();
        Header::fromPayload(m_data)->refCount = 1;
        Header::fromPayload(m_data)->length = other.size();
        ASSERT(Header::fromPayload(m_data)->length == other.size());
        VectorTypeOperations<T>::uninitializedCopy(other.begin(), other.end(), m_data);
    }
    
    RefCountedArray& operator=(const RefCountedArray& other)
    {
        T* oldData = m_data;
        m_data = other.m_data;
        if (m_data)
            Header::fromPayload(m_data)->refCount++;
        
        if (!oldData)
            return *this;
        if (--Header::fromPayload(oldData)->refCount)
            return *this;
        VectorTypeOperations<T>::destruct(oldData, oldData + Header::fromPayload(oldData)->length);
        fastFree(Header::fromPayload(oldData));
        return *this;
    }
    
    ~RefCountedArray()
    {
        if (!m_data)
            return;
        if (--Header::fromPayload(m_data)->refCount)
            return;
        VectorTypeOperations<T>::destruct(begin(), end());
        fastFree(Header::fromPayload(m_data));
    }
    
    unsigned refCount() const
    {
        if (!m_data)
            return 0;
        return Header::fromPayload(m_data)->refCount;
    }
    
    size_t size() const
    {
        if (!m_data)
            return 0;
        return Header::fromPayload(m_data)->length;
    }
    
    size_t byteSize() const { return size() * sizeof(T); }
    
    T* data() { return m_data; }
    T* begin() { return m_data; }
    T* end()
    {
        if (!m_data)
            return 0;
        return m_data + Header::fromPayload(m_data)->length;
    }
    
    const T* data() const { return m_data; }
    const T* begin() const { return m_data; }
    const T* end() const { return const_cast<RefCountedArray*>(this)->end(); }
    
    T& at(size_t i)
    {
        ASSERT_WITH_SECURITY_IMPLICATION(i < size());
        return begin()[i];
    }
    
    const T& at(size_t i) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(i < size());
        return begin()[i];
    }
    
    T& operator[](size_t i) { return at(i); }
    const T& operator[](size_t i) const { return at(i); }

    bool operator==(const RefCountedArray& other) const
    {
        if (m_data == other.m_data)
            return true;
        if (!m_data || !other.m_data)
            return false;
        unsigned length = Header::fromPayload(m_data)->length;
        if (length != Header::fromPayload(other.m_data)->length)
            return false;
        for (unsigned i = 0; i < length; ++i) {
            if (m_data[i] != other.m_data[i])
                return false;
        }
        return true;
    }
    
private:
    struct Header {
        unsigned refCount;
        unsigned length;
        
        static size_t size()
        {
            return (sizeof(Header) + 7) & ~7;
        }
        
        T* payload()
        {
            char* result = reinterpret_cast<char*>(this) + size();
            ASSERT(!(bitwise_cast<uintptr_t>(result) & 7));
            return reinterpret_cast_ptr<T*>(result);
        }
        
        static Header* fromPayload(T* payload)
        {
            return reinterpret_cast_ptr<Header*>(reinterpret_cast<char*>(payload) - size());
        }
    };
    
    T* m_data;
};

} // namespace WTF

using WTF::RefCountedArray;

#endif // RefCountedArray_h

