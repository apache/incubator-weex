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
#ifndef BloomFilter_h
#define BloomFilter_h

#include <array>
#include <wtf/text/AtomicString.h>

namespace WTF {

// Bloom filter with k=2. Uses 2^keyBits/8 bytes of memory.
// False positive rate is approximately (1-e^(-2n/m))^2, where n is the number of unique 
// keys and m is the table size (==2^keyBits).
// See http://en.wikipedia.org/wiki/Bloom_filter
template <unsigned keyBits>
class BloomFilter {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static const size_t tableSize = 1 << keyBits;

    BloomFilter();

    void add(unsigned hash);
    // For example SHA1::Digest.
    template <size_t hashSize> void add(const std::array<uint8_t, hashSize>&);

    void add(const BloomFilter&);

    // The filter may give false positives (claim it may contain a key it doesn't)
    // but never false negatives (claim it doesn't contain a key it does).
    bool mayContain(unsigned hash) const;
    template <size_t hashSize> bool mayContain(const std::array<uint8_t, hashSize>&) const;
    
    void clear();

    void add(const AtomicString& string) { add(string.impl()->existingHash()); }
    void add(const String& string) { add(string.impl()->hash()); }
    bool mayContain(const AtomicString& string) const { return mayContain(string.impl()->existingHash()); }
    bool mayContain(const String& string) const { return mayContain(string.impl()->hash()); }

private:
    static const unsigned bitsPerPosition = 8 * sizeof(unsigned);
    static const unsigned keyMask = (1 << keyBits) - 1;
    static unsigned arrayIndex(unsigned key) { return key / bitsPerPosition; }
    static unsigned bitMask(unsigned key) { return 1 << (key % bitsPerPosition); }
    template <size_t hashSize> static std::pair<unsigned, unsigned> keysFromHash(const std::array<uint8_t, hashSize>&);

    bool isBitSet(unsigned key) const;
    void setBit(unsigned key);

    std::array<unsigned, tableSize / bitsPerPosition> m_bitArray;
};

template <unsigned keyBits>
inline BloomFilter<keyBits>::BloomFilter()
    : m_bitArray()
{
}

template <unsigned keyBits>
inline bool BloomFilter<keyBits>::mayContain(unsigned hash) const
{
    // The top and bottom bits of the incoming hash are treated as independent bloom filter hash functions.
    // This works well as long as the filter size is not much above 2^16.
    return isBitSet(hash) && isBitSet(hash >> 16);
}

template <unsigned keyBits>
inline void BloomFilter<keyBits>::add(unsigned hash)
{
    setBit(hash);
    setBit(hash >> 16);
}

template <unsigned keyBits>
template <size_t hashSize>
inline std::pair<unsigned, unsigned> BloomFilter<keyBits>::keysFromHash(const std::array<uint8_t, hashSize>& hash)
{
    // We could use larger k value than 2 for long hashes.
    static_assert(hashSize >= 2 * sizeof(unsigned), "Hash array too short");
    return {
        *reinterpret_cast<const unsigned*>(hash.data()),
        *reinterpret_cast<const unsigned*>(hash.data() + sizeof(unsigned))
    };
}

template <unsigned keyBits>
template <size_t hashSize>
inline bool BloomFilter<keyBits>::mayContain(const std::array<uint8_t, hashSize>& hash) const
{
    auto keys = keysFromHash(hash);
    return isBitSet(keys.first) && isBitSet(keys.second);
}

template <unsigned keyBits>
template <size_t hashSize>
inline void BloomFilter<keyBits>::add(const std::array<uint8_t, hashSize>& hash)
{
    auto keys = keysFromHash(hash);
    setBit(keys.first);
    setBit(keys.second);
}

template <unsigned keyBits>
inline void BloomFilter<keyBits>::add(const BloomFilter& other)
{
    for (size_t i = 0; i < m_bitArray.size(); ++i)
        m_bitArray[i] |= other.m_bitArray[i];
}

template <unsigned keyBits>
bool BloomFilter<keyBits>::isBitSet(unsigned key) const
{
    unsigned maskedKey = key & keyMask;
    ASSERT(arrayIndex(maskedKey) < m_bitArray.size());
    return m_bitArray[arrayIndex(maskedKey)] & bitMask(maskedKey);
}

template <unsigned keyBits>
void BloomFilter<keyBits>::setBit(unsigned key)
{
    unsigned maskedKey = key & keyMask;
    ASSERT(arrayIndex(maskedKey) < m_bitArray.size());
    m_bitArray[arrayIndex(maskedKey)] |= bitMask(maskedKey);
}

template <unsigned keyBits>
inline void BloomFilter<keyBits>::clear()
{
    m_bitArray.fill(0);
}

// Counting bloom filter with 8 bit counters. Uses 2^keyBits bytes of memory. Error rates as above.
// See http://en.wikipedia.org/wiki/Bloom_filter#Counting_filters
template <unsigned keyBits>
class CountingBloomFilter {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static const size_t tableSize = 1 << keyBits;
    static unsigned maximumCount() { return std::numeric_limits<uint8_t>::max(); }
    
    CountingBloomFilter();

    void add(unsigned hash);
    void remove(unsigned hash);

    // The filter may give false positives (claim it may contain a key it doesn't)
    // but never false negatives (claim it doesn't contain a key it does).
    bool mayContain(unsigned hash) const { return firstBucket(hash) && secondBucket(hash); }
    
    // The filter must be cleared before reuse even if all keys are removed.
    // Otherwise overflowed keys will stick around.
    void clear();

    void add(const AtomicString& string) { add(string.impl()->existingHash()); }
    void add(const String& string) { add(string.impl()->hash()); }
    void remove(const AtomicString& string) { remove(string.impl()->existingHash()); }
    void remove(const String& string) { remove(string.impl()->hash()); }

    bool mayContain(const AtomicString& string) const { return mayContain(string.impl()->existingHash()); }
    bool mayContain(const String& string) const { return mayContain(string.impl()->hash()); }

#if !ASSERT_DISABLED
    // Slow.
    bool likelyEmpty() const;
    bool isClear() const;
#endif

private:
    static const unsigned keyMask = (1 << keyBits) - 1;

    uint8_t& firstBucket(unsigned hash) { return m_buckets[hash & keyMask]; }
    uint8_t& secondBucket(unsigned hash) { return m_buckets[(hash >> 16) & keyMask]; }
    const uint8_t& firstBucket(unsigned hash) const { return m_buckets[hash & keyMask]; }
    const uint8_t& secondBucket(unsigned hash) const { return m_buckets[(hash >> 16) & keyMask]; }

    std::array<uint8_t, tableSize> m_buckets;
};

template <unsigned keyBits>
inline CountingBloomFilter<keyBits>::CountingBloomFilter()
    : m_buckets()
{
}

template <unsigned keyBits>
inline void CountingBloomFilter<keyBits>::add(unsigned hash)
{
    auto& first = firstBucket(hash);
    auto& second = secondBucket(hash);
    if (LIKELY(first < maximumCount()))
        ++first;
    if (LIKELY(second < maximumCount()))
        ++second;
}

template <unsigned keyBits>
inline void CountingBloomFilter<keyBits>::remove(unsigned hash)
{
    auto& first = firstBucket(hash);
    auto& second = secondBucket(hash);
    ASSERT(first);
    ASSERT(second);
    // In case of an overflow, the bucket sticks in the table until clear().
    if (LIKELY(first < maximumCount()))
        --first;
    if (LIKELY(second < maximumCount()))
        --second;
}
    
template <unsigned keyBits>
inline void CountingBloomFilter<keyBits>::clear()
{
    m_buckets.fill(0);
}

#if !ASSERT_DISABLED
template <unsigned keyBits>
bool CountingBloomFilter<keyBits>::likelyEmpty() const
{
    for (auto& bucket : m_buckets) {
        if (bucket && bucket != maximumCount())
            return false;
    }
    return true;
}

template <unsigned keyBits>
bool CountingBloomFilter<keyBits>::isClear() const
{
    for (auto& bucket : m_buckets) {
        if (bucket)
            return false;
    }
    return true;
}
#endif

}

using WTF::BloomFilter;
using WTF::CountingBloomFilter;

#endif
