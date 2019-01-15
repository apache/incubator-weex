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
#ifndef PackedIntVector_h
#define PackedIntVector_h

#include <wtf/BitVector.h>

namespace WTF {

// This class allows you to create an array of integers, where those
// integers have only a handful of bits each. It is not meant to be
// efficient in time, but only in space. (Though making it efficient
// in time for power-of-2 values of bitCount would not be difficult.)
// Note that this does not work as expected for signed types, if you
// are relying on the sign being preserved.

template<typename T, unsigned bitCount>
class PackedIntVector {
public:
    static_assert(bitCount, "bitCount must not be zero!");
    static_assert(bitCount < sizeof(void*) * 8, "bitCount must not exceed the address space limit!");

    PackedIntVector()
    {
    }
    
    PackedIntVector(const PackedIntVector& other)
        : m_bits(other.m_bits)
    {
    }
    
    PackedIntVector& operator=(const PackedIntVector& other)
    {
        m_bits = other.m_bits;
        return *this;
    }
    
    size_t size() const
    {
        return m_bits.size() / bitCount;
    }
    
    void ensureSize(size_t numInts)
    {
        m_bits.ensureSize(numInts * bitCount);
    }
    
    void resize(size_t numInts)
    {
        m_bits.resize(numInts * bitCount);
    }
    
    void clearAll()
    {
        m_bits.clearAll();
    }
    
    T get(size_t index) const
    {
        uintptr_t result = 0;
        for (unsigned subIndex = 0; subIndex < bitCount; ++subIndex) {
            result <<= 1;
            result |= (m_bits.quickGet(index * bitCount + subIndex) ? 1 : 0);
        }
        return static_cast<T>(result);
    }
    
    void set(size_t index, T value)
    {
        // Do arithmetic using uintptr_t, because (1) we know what it is
        // (T might be an enum) and (2) it's the largest integer type that
        // is likely to perform decently well.
        uintptr_t myValue = static_cast<uintptr_t>(value);
        
        // Preliminary sanity check that the value is not out of range.
        ASSERT((myValue & mask()) == myValue);
        
        for (unsigned subIndex = bitCount; subIndex-- > 0;) {
            m_bits.quickSet(index * bitCount + subIndex, !!(myValue & 1));
            myValue >>= 1;
        }
        
        // Final sanity check that we stored what the user thought we
        // stored.
        ASSERT(get(index) == value);
    }
private:
    // This returns the mask, and is careful to not step on the wrap-around
    // semantics of the shift amount (1 << 32 is 1 since 32 wraps to 0). There
    // is the separate question of why you would ever use this to store 32-bit
    // or 64-bit values, but it's probably better to have this work as expected
    // in such situations regardless.
    static uintptr_t mask() { return (static_cast<uintptr_t>(2) << (bitCount - 1)) - 1; }
               
    // Stores integers bit by bit in big endian.
    BitVector m_bits;
};

} // namespace WTF

using WTF::PackedIntVector;

#endif // PackedIntVector_h

