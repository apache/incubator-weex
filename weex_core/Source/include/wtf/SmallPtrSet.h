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
#ifndef SmallPtrSet_h
#define SmallPtrSet_h

#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>
#include <wtf/HashFunctions.h>
#include <wtf/Noncopyable.h>

namespace WTF {

template<typename PtrType, unsigned SmallArraySize = 8>
class SmallPtrSet {
    WTF_MAKE_NONCOPYABLE(SmallPtrSet);
    static_assert(std::is_trivially_destructible<PtrType>::value, "We currently don't support non-trivially destructible pointer types.");
    static_assert(sizeof(PtrType) == sizeof(void*), "Only support pointer sized things.");
    static_assert(!(SmallArraySize & (SmallArraySize - 1)), "Inline size must be a power of two.");

public: 
    SmallPtrSet()
    {
        initialize();
    }

    // We take care to have SmallPtrSet have partial move semantics allowable through
    // memcpy. It's partial move semantics because our destructor should not be called
    // on the SmallPtrObject in the old memory we were moved from (otherwise, we might free m_buffer twice)
    // unless that old memory is reset to be isSmall(). See move constructor below.
    // To maintain these semantics, we determine if we're small by checking our size
    // and not our m_buffer pointer. And when we're small, we don't do operations on
    // m_buffer, instead, we perform operations on m_smallStorage directly. The reason we want
    // these semantics is that it's beneficial to have a Vector that contains SmallPtrSet
    // (or an object with SmallPtrSet as a field) be allowed to use memcpy for its move operation.

    SmallPtrSet(SmallPtrSet&& other)
    {
        memcpy(this, &other, sizeof(SmallPtrSet));
        other.initialize();
    }

    SmallPtrSet& operator=(SmallPtrSet&& other)
    {
        this->~SmallPtrSet();
        new (this) SmallPtrSet(WTFMove(other));
        return *this;
    }

    ~SmallPtrSet()
    {
        if (!isSmall())
            fastFree(m_buffer);
    }

    inline void add(PtrType ptr)
    {
        ASSERT(isValidEntry(ptr));

        if (isSmall()) {
            for (unsigned i = 0; i < m_size; i++) {
                if (m_smallStorage[i] == ptr)
                    return;
            }

            if (m_size < SmallArraySize) {
                m_smallStorage[m_size] = ptr;
                ++m_size;
                return;
            }

            grow(std::max(64u, SmallArraySize * 2));
            // Fall through. We're no longer small :(
        }

        // If we're more than 3/4ths full we grow.
        if (UNLIKELY(m_size * 4 >= m_capacity * 3)) {
            grow(m_capacity * 2);
            ASSERT(!(m_capacity & (m_capacity - 1)));
        }

        void** bucket = this->bucket(ptr);
        if (*bucket != ptr) {
            *bucket = ptr;
            ++m_size;
        }
    }

    inline bool contains(PtrType ptr) const
    {
        ASSERT(isValidEntry(ptr));
        if (isSmall()) {
            for (unsigned i = 0; i < m_size; i++) { // We only need to search up to m_size because we store things linearly inside m_smallStorage.
                if (m_smallStorage[i] == ptr)
                    return true;
            }
            return false;
        }

        void** bucket = this->bucket(ptr);
        return *bucket == ptr;
    }

    class iterator {
    public:
        iterator& operator++()
        {
            m_index++;
            ASSERT(m_index <= m_capacity);
            while (m_index < m_capacity && m_buffer[m_index] == emptyValue())
                m_index++;
            return *this;
        }
        
        PtrType operator*() const { ASSERT(m_index < m_capacity); return static_cast<PtrType>(m_buffer[m_index]); }
        bool operator==(const iterator& other) const { ASSERT(m_buffer == other.m_buffer); return m_index == other.m_index; }
        bool operator!=(const iterator& other) const { ASSERT(m_buffer == other.m_buffer); return !(*this == other); }

    private:
        template<typename U, unsigned S> friend class WTF::SmallPtrSet;
        unsigned m_index;
        unsigned m_capacity;
        void** m_buffer;
    };

    iterator begin() const
    {
        iterator it;
        it.m_index = std::numeric_limits<unsigned>::max();
        it.m_capacity = m_capacity;
        if (isSmall())
            it.m_buffer = const_cast<void**>(m_smallStorage);
        else
            it.m_buffer = m_buffer;

        ++it;

        return it;
    }

    iterator end() const
    {
        iterator it;
        it.m_index = m_capacity;
        it.m_capacity = m_capacity;
        if (isSmall())
            it.m_buffer = const_cast<void**>(m_smallStorage);
        else
            it.m_buffer = m_buffer;

        return it;
    }

    inline unsigned size() const { return m_size; }

private:
    constexpr static void* emptyValue()
    {
        return bitwise_cast<void*>(std::numeric_limits<uintptr_t>::max());
    }

    bool isValidEntry(const PtrType ptr) const
    {
        return ptr != emptyValue();
    }

    inline bool isSmall() const
    {
        return m_capacity == SmallArraySize;
    }

    inline void initialize()
    {
        m_size = 0;
        m_buffer = nullptr;
        m_capacity = SmallArraySize;
        memset(m_smallStorage, -1, sizeof(void*) * SmallArraySize);
        ASSERT(isSmall());
    }

    inline void grow(unsigned size)
    {
        ASSERT(static_cast<int32_t>(bitwise_cast<intptr_t>(emptyValue())) == -1);

        size_t allocationSize = sizeof(void*) * size;
        bool wasSmall = isSmall();
        void** oldBuffer = wasSmall ? m_smallStorage : m_buffer;
        unsigned oldCapacity = m_capacity;
        m_buffer = static_cast<void**>(fastMalloc(allocationSize));
        memset(m_buffer, -1, allocationSize);
        m_capacity = size;

        for (unsigned i = 0; i < oldCapacity; i++) {
            if (oldBuffer[i] != emptyValue()) {
                void** ptr = this->bucket(static_cast<PtrType>(oldBuffer[i]));
                *ptr = oldBuffer[i];
            }
        }

        if (!wasSmall)
            fastFree(oldBuffer);
    }


    inline void** bucket(PtrType target) const
    {
        ASSERT(!(m_capacity & (m_capacity - 1)));
        unsigned bucket = PtrHashBase<PtrType, false /* isSmartPtr */>::hash(target) & (m_capacity - 1);
        unsigned index = 0;
        while (true) {
            void** ptr = m_buffer + bucket;
            if (*ptr == emptyValue())
                return ptr;
            if (*ptr == target)
                return ptr;
            index++;
            bucket = (bucket + index) & (m_capacity - 1);
        }
    }

    unsigned m_size;
    unsigned m_capacity;
    void** m_buffer;
    void* m_smallStorage[SmallArraySize];
};

} // namespace WTF

using WTF::SmallPtrSet;

#endif // SmallPtrSet_h
