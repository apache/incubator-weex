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
#pragma once

#include <atomic>
#include <iterator>
#include <mutex>
#include <string.h>
#include <type_traits>
#include <utility>
#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>
#include <wtf/HashTraits.h>
#include <wtf/Lock.h>
#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>
#include <wtf/ValueCheck.h>

#define DUMP_HASHTABLE_STATS 0
#define DUMP_HASHTABLE_STATS_PER_TABLE 0

#if DUMP_HASHTABLE_STATS_PER_TABLE
#include <wtf/DataLog.h>
#endif

namespace WTF {

// Enables internal WTF consistency checks that are invoked automatically. Non-WTF callers can call checkTableConsistency() even if internal checks are disabled.
#define CHECK_HASHTABLE_CONSISTENCY 0

#ifdef NDEBUG
#define CHECK_HASHTABLE_ITERATORS 0
#define CHECK_HASHTABLE_USE_AFTER_DESTRUCTION 0
#else
#define CHECK_HASHTABLE_ITERATORS 1
#define CHECK_HASHTABLE_USE_AFTER_DESTRUCTION 1
#endif

#if DUMP_HASHTABLE_STATS

    struct HashTableStats {
        // The following variables are all atomically incremented when modified.
        WTF_EXPORTDATA static std::atomic<unsigned> numAccesses;
        WTF_EXPORTDATA static std::atomic<unsigned> numRehashes;
        WTF_EXPORTDATA static std::atomic<unsigned> numRemoves;
        WTF_EXPORTDATA static std::atomic<unsigned> numReinserts;

        // The following variables are only modified in the recordCollisionAtCount method within a mutex.
        WTF_EXPORTDATA static unsigned maxCollisions;
        WTF_EXPORTDATA static unsigned numCollisions;
        WTF_EXPORTDATA static unsigned collisionGraph[4096];

        WTF_EXPORT_PRIVATE static void recordCollisionAtCount(unsigned count);
        WTF_EXPORT_PRIVATE static void dumpStats();
    };

#endif

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    class HashTable;
    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    class HashTableIterator;
    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    class HashTableConstIterator;

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void addIterator(const HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>*,
        HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>*);

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void removeIterator(HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>*);

#if !CHECK_HASHTABLE_ITERATORS

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline void addIterator(const HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>*,
        HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>*) { }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline void removeIterator(HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>*) { }

#endif

    typedef enum { HashItemKnownGood } HashItemKnownGoodTag;

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    class HashTableConstIterator : public std::iterator<std::forward_iterator_tag, Value, std::ptrdiff_t, const Value*, const Value&> {
    private:
        typedef HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> HashTableType;
        typedef HashTableIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> iterator;
        typedef HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> const_iterator;
        typedef Value ValueType;
        typedef const ValueType& ReferenceType;
        typedef const ValueType* PointerType;

        friend class HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>;
        friend class HashTableIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>;

        void skipEmptyBuckets()
        {
            while (m_position != m_endPosition && HashTableType::isEmptyOrDeletedBucket(*m_position))
                ++m_position;
        }

        HashTableConstIterator(const HashTableType* table, PointerType position, PointerType endPosition)
            : m_position(position), m_endPosition(endPosition)
        {
            addIterator(table, this);
            skipEmptyBuckets();
        }

        HashTableConstIterator(const HashTableType* table, PointerType position, PointerType endPosition, HashItemKnownGoodTag)
            : m_position(position), m_endPosition(endPosition)
        {
            addIterator(table, this);
        }

    public:
        HashTableConstIterator()
        {
            addIterator(static_cast<const HashTableType*>(0), this);
        }

        // default copy, assignment and destructor are OK if CHECK_HASHTABLE_ITERATORS is 0

#if CHECK_HASHTABLE_ITERATORS
        ~HashTableConstIterator()
        {
            removeIterator(this);
        }

        HashTableConstIterator(const const_iterator& other)
            : m_position(other.m_position), m_endPosition(other.m_endPosition)
        {
            addIterator(other.m_table, this);
        }

        const_iterator& operator=(const const_iterator& other)
        {
            m_position = other.m_position;
            m_endPosition = other.m_endPosition;

            removeIterator(this);
            addIterator(other.m_table, this);

            return *this;
        }
#endif

        PointerType get() const
        {
            checkValidity();
            return m_position;
        }
        ReferenceType operator*() const { return *get(); }
        PointerType operator->() const { return get(); }

        const_iterator& operator++()
        {
            checkValidity();
            ASSERT(m_position != m_endPosition);
            ++m_position;
            skipEmptyBuckets();
            return *this;
        }

        // postfix ++ intentionally omitted

        // Comparison.
        bool operator==(const const_iterator& other) const
        {
            checkValidity(other);
            return m_position == other.m_position;
        }
        bool operator!=(const const_iterator& other) const
        {
            checkValidity(other);
            return m_position != other.m_position;
        }
        bool operator==(const iterator& other) const
        {
            return *this == static_cast<const_iterator>(other);
        }
        bool operator!=(const iterator& other) const
        {
            return *this != static_cast<const_iterator>(other);
        }

    private:
        void checkValidity() const
        {
#if CHECK_HASHTABLE_ITERATORS
            ASSERT(m_table);
#endif
        }


#if CHECK_HASHTABLE_ITERATORS
        void checkValidity(const const_iterator& other) const
        {
            ASSERT(m_table);
            ASSERT_UNUSED(other, other.m_table);
            ASSERT(m_table == other.m_table);
        }
#else
        void checkValidity(const const_iterator&) const { }
#endif

        PointerType m_position;
        PointerType m_endPosition;

#if CHECK_HASHTABLE_ITERATORS
    public:
        // Any modifications of the m_next or m_previous of an iterator that is in a linked list of a HashTable::m_iterator,
        // should be guarded with m_table->m_mutex.
        mutable const HashTableType* m_table;
        mutable const_iterator* m_next;
        mutable const_iterator* m_previous;
#endif
    };

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    class HashTableIterator : public std::iterator<std::forward_iterator_tag, Value, std::ptrdiff_t, Value*, Value&> {
    private:
        typedef HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> HashTableType;
        typedef HashTableIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> iterator;
        typedef HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> const_iterator;
        typedef Value ValueType;
        typedef ValueType& ReferenceType;
        typedef ValueType* PointerType;

        friend class HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>;

        HashTableIterator(HashTableType* table, PointerType pos, PointerType end) : m_iterator(table, pos, end) { }
        HashTableIterator(HashTableType* table, PointerType pos, PointerType end, HashItemKnownGoodTag tag) : m_iterator(table, pos, end, tag) { }

    public:
        HashTableIterator() { }

        // default copy, assignment and destructor are OK

        PointerType get() const { return const_cast<PointerType>(m_iterator.get()); }
        ReferenceType operator*() const { return *get(); }
        PointerType operator->() const { return get(); }

        iterator& operator++() { ++m_iterator; return *this; }

        // postfix ++ intentionally omitted

        // Comparison.
        bool operator==(const iterator& other) const { return m_iterator == other.m_iterator; }
        bool operator!=(const iterator& other) const { return m_iterator != other.m_iterator; }
        bool operator==(const const_iterator& other) const { return m_iterator == other; }
        bool operator!=(const const_iterator& other) const { return m_iterator != other; }

        operator const_iterator() const { return m_iterator; }

    private:
        const_iterator m_iterator;
    };

    template<typename ValueTraits, typename HashFunctions> class IdentityHashTranslator {
    public:
        template<typename T> static unsigned hash(const T& key) { return HashFunctions::hash(key); }
        template<typename T, typename U> static bool equal(const T& a, const U& b) { return HashFunctions::equal(a, b); }
        template<typename T, typename U, typename V> static void translate(T& location, const U&, V&& value)
        { 
            ValueTraits::assignToEmpty(location, std::forward<V>(value)); 
        }
    };

    template<typename IteratorType> struct HashTableAddResult {
        HashTableAddResult() : isNewEntry(false) { }
        HashTableAddResult(IteratorType iter, bool isNewEntry) : iterator(iter), isNewEntry(isNewEntry) { }
        IteratorType iterator;
        bool isNewEntry;

        explicit operator bool() const { return isNewEntry; }
    };

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    class HashTable {
    public:
        typedef HashTableIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> iterator;
        typedef HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits> const_iterator;
        typedef Traits ValueTraits;
        typedef Key KeyType;
        typedef Value ValueType;
        typedef IdentityHashTranslator<ValueTraits, HashFunctions> IdentityTranslatorType;
        typedef HashTableAddResult<iterator> AddResult;

#if DUMP_HASHTABLE_STATS_PER_TABLE
        struct Stats {
            Stats()
                : numAccesses(0)
                , numRehashes(0)
                , numRemoves(0)
                , numReinserts(0)
                , maxCollisions(0)
                , numCollisions(0)
                , collisionGraph()
            {
            }

            unsigned numAccesses;
            unsigned numRehashes;
            unsigned numRemoves;
            unsigned numReinserts;

            unsigned maxCollisions;
            unsigned numCollisions;
            unsigned collisionGraph[4096];

            void recordCollisionAtCount(unsigned count)
            {
                if (count > maxCollisions)
                    maxCollisions = count;
                numCollisions++;
                collisionGraph[count]++;
            }

            void dumpStats()
            {
                dataLogF("\nWTF::HashTable::Stats dump\n\n");
                dataLogF("%d accesses\n", numAccesses);
                dataLogF("%d total collisions, average %.2f probes per access\n", numCollisions, 1.0 * (numAccesses + numCollisions) / numAccesses);
                dataLogF("longest collision chain: %d\n", maxCollisions);
                for (unsigned i = 1; i <= maxCollisions; i++) {
                    dataLogF("  %d lookups with exactly %d collisions (%.2f%% , %.2f%% with this many or more)\n", collisionGraph[i], i, 100.0 * (collisionGraph[i] - collisionGraph[i+1]) / numAccesses, 100.0 * collisionGraph[i] / numAccesses);
                }
                dataLogF("%d rehashes\n", numRehashes);
                dataLogF("%d reinserts\n", numReinserts);
            }
        };
#endif

        HashTable();
        ~HashTable() 
        {
            invalidateIterators(); 
            if (m_table)
                deallocateTable(m_table, m_tableSize);
#if CHECK_HASHTABLE_USE_AFTER_DESTRUCTION
            m_table = (ValueType*)(uintptr_t)0xbbadbeef;
#endif
        }

        HashTable(const HashTable&);
        void swap(HashTable&);
        HashTable& operator=(const HashTable&);

        HashTable(HashTable&&);
        HashTable& operator=(HashTable&&);

        // When the hash table is empty, just return the same iterator for end as for begin.
        // This is more efficient because we don't have to skip all the empty and deleted
        // buckets, and iterating an empty table is a common case that's worth optimizing.
        iterator begin() { return isEmpty() ? end() : makeIterator(m_table); }
        iterator end() { return makeKnownGoodIterator(m_table + m_tableSize); }
        const_iterator begin() const { return isEmpty() ? end() : makeConstIterator(m_table); }
        const_iterator end() const { return makeKnownGoodConstIterator(m_table + m_tableSize); }

        unsigned size() const { return m_keyCount; }
        unsigned capacity() const { return m_tableSize; }
        bool isEmpty() const { return !m_keyCount; }

        AddResult add(const ValueType& value) { return add<IdentityTranslatorType>(Extractor::extract(value), value); }
        AddResult add(ValueType&& value) { return add<IdentityTranslatorType>(Extractor::extract(value), WTFMove(value)); }

        // A special version of add() that finds the object by hashing and comparing
        // with some other type, to avoid the cost of type conversion if the object is already
        // in the table.
        template<typename HashTranslator, typename T, typename Extra> AddResult add(T&& key, Extra&&);
        template<typename HashTranslator, typename T, typename Extra> AddResult addPassingHashCode(T&& key, Extra&&);

        iterator find(const KeyType& key) { return find<IdentityTranslatorType>(key); }
        const_iterator find(const KeyType& key) const { return find<IdentityTranslatorType>(key); }
        bool contains(const KeyType& key) const { return contains<IdentityTranslatorType>(key); }

        template<typename HashTranslator, typename T> iterator find(const T&);
        template<typename HashTranslator, typename T> const_iterator find(const T&) const;
        template<typename HashTranslator, typename T> bool contains(const T&) const;

        void remove(const KeyType&);
        void remove(iterator);
        void removeWithoutEntryConsistencyCheck(iterator);
        void removeWithoutEntryConsistencyCheck(const_iterator);
        template<typename Functor>
        void removeIf(const Functor&);
        void clear();

        static bool isEmptyBucket(const ValueType& value) { return isHashTraitsEmptyValue<KeyTraits>(Extractor::extract(value)); }
        static bool isDeletedBucket(const ValueType& value) { return KeyTraits::isDeletedValue(Extractor::extract(value)); }
        static bool isEmptyOrDeletedBucket(const ValueType& value) { return isEmptyBucket(value) || isDeletedBucket(value); }

        ValueType* lookup(const Key& key) { return lookup<IdentityTranslatorType>(key); }
        template<typename HashTranslator, typename T> ValueType* lookup(const T&);
        template<typename HashTranslator, typename T> ValueType* inlineLookup(const T&);

#if !ASSERT_DISABLED
        void checkTableConsistency() const;
#else
        static void checkTableConsistency() { }
#endif
#if CHECK_HASHTABLE_CONSISTENCY
        void internalCheckTableConsistency() const { checkTableConsistency(); }
        void internalCheckTableConsistencyExceptSize() const { checkTableConsistencyExceptSize(); }
#else
        static void internalCheckTableConsistencyExceptSize() { }
        static void internalCheckTableConsistency() { }
#endif

    private:
        static ValueType* allocateTable(unsigned size);
        static void deallocateTable(ValueType* table, unsigned size);

        typedef std::pair<ValueType*, bool> LookupType;
        typedef std::pair<LookupType, unsigned> FullLookupType;

        LookupType lookupForWriting(const Key& key) { return lookupForWriting<IdentityTranslatorType>(key); };
        template<typename HashTranslator, typename T> FullLookupType fullLookupForWriting(const T&);
        template<typename HashTranslator, typename T> LookupType lookupForWriting(const T&);

        template<typename HashTranslator, typename T, typename Extra> void addUniqueForInitialization(T&& key, Extra&&);

        template<typename HashTranslator, typename T> void checkKey(const T&);

        void removeAndInvalidateWithoutEntryConsistencyCheck(ValueType*);
        void removeAndInvalidate(ValueType*);
        void remove(ValueType*);

        bool shouldExpand() const { return (m_keyCount + m_deletedCount) * m_maxLoad >= m_tableSize; }
        bool mustRehashInPlace() const { return m_keyCount * m_minLoad < m_tableSize * 2; }
        bool shouldShrink() const { return m_keyCount * m_minLoad < m_tableSize && m_tableSize > KeyTraits::minimumTableSize; }
        ValueType* expand(ValueType* entry = nullptr);
        void shrink() { rehash(m_tableSize / 2, nullptr); }

        ValueType* rehash(unsigned newTableSize, ValueType* entry);
        ValueType* reinsert(ValueType&&);

        static void initializeBucket(ValueType& bucket);
        static void deleteBucket(ValueType& bucket) { hashTraitsDeleteBucket<Traits>(bucket); }

        FullLookupType makeLookupResult(ValueType* position, bool found, unsigned hash)
            { return FullLookupType(LookupType(position, found), hash); }

        iterator makeIterator(ValueType* pos) { return iterator(this, pos, m_table + m_tableSize); }
        const_iterator makeConstIterator(ValueType* pos) const { return const_iterator(this, pos, m_table + m_tableSize); }
        iterator makeKnownGoodIterator(ValueType* pos) { return iterator(this, pos, m_table + m_tableSize, HashItemKnownGood); }
        const_iterator makeKnownGoodConstIterator(ValueType* pos) const { return const_iterator(this, pos, m_table + m_tableSize, HashItemKnownGood); }

#if !ASSERT_DISABLED
        void checkTableConsistencyExceptSize() const;
#else
        static void checkTableConsistencyExceptSize() { }
#endif

#if CHECK_HASHTABLE_ITERATORS
        void invalidateIterators();
#else
        static void invalidateIterators() { }
#endif

        static const unsigned m_maxLoad = 2;
        static const unsigned m_minLoad = 6;

        ValueType* m_table;
        unsigned m_tableSize;
        unsigned m_tableSizeMask;
        unsigned m_keyCount;
        unsigned m_deletedCount;

#if CHECK_HASHTABLE_ITERATORS
    public:
        // All access to m_iterators should be guarded with m_mutex.
        mutable const_iterator* m_iterators;
        // Use std::unique_ptr so HashTable can still be memmove'd or memcpy'ed.
        mutable std::unique_ptr<Lock> m_mutex;
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
    public:
        mutable std::unique_ptr<Stats> m_stats;
#endif
    };

    // Set all the bits to one after the most significant bit: 00110101010 -> 00111111111.
    template<unsigned size> struct OneifyLowBits;
    template<>
    struct OneifyLowBits<0> {
        static const unsigned value = 0;
    };
    template<unsigned number>
    struct OneifyLowBits {
        static const unsigned value = number | OneifyLowBits<(number >> 1)>::value;
    };
    // Compute the first power of two integer that is an upper bound of the parameter 'number'.
    template<unsigned number>
    struct UpperPowerOfTwoBound {
        static const unsigned value = (OneifyLowBits<number - 1>::value + 1) * 2;
    };

    // Because power of two numbers are the limit of maxLoad, their capacity is twice the
    // UpperPowerOfTwoBound, or 4 times their values.
    template<unsigned size, bool isPowerOfTwo> struct HashTableCapacityForSizeSplitter;
    template<unsigned size>
    struct HashTableCapacityForSizeSplitter<size, true> {
        static const unsigned value = size * 4;
    };
    template<unsigned size>
    struct HashTableCapacityForSizeSplitter<size, false> {
        static const unsigned value = UpperPowerOfTwoBound<size>::value;
    };

    // HashTableCapacityForSize computes the upper power of two capacity to hold the size parameter.
    // This is done at compile time to initialize the HashTraits.
    template<unsigned size>
    struct HashTableCapacityForSize {
        static const unsigned value = HashTableCapacityForSizeSplitter<size, !(size & (size - 1))>::value;
        COMPILE_ASSERT(size > 0, HashTableNonZeroMinimumCapacity);
        COMPILE_ASSERT(!static_cast<unsigned>(value >> 31), HashTableNoCapacityOverflow);
        COMPILE_ASSERT(value > (2 * size), HashTableCapacityHoldsContentSize);
    };

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::HashTable()
        : m_table(0)
        , m_tableSize(0)
        , m_tableSizeMask(0)
        , m_keyCount(0)
        , m_deletedCount(0)
#if CHECK_HASHTABLE_ITERATORS
        , m_iterators(0)
        , m_mutex(std::make_unique<Lock>())
#endif
#if DUMP_HASHTABLE_STATS_PER_TABLE
        , m_stats(std::make_unique<Stats>())
#endif
    {
    }

    inline unsigned doubleHash(unsigned key)
    {
        key = ~key + (key >> 23);
        key ^= (key << 12);
        key ^= (key >> 7);
        key ^= (key << 2);
        key ^= (key >> 20);
        return key;
    }

#if ASSERT_DISABLED

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T>
    inline void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::checkKey(const T&)
    {
    }

#else

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::checkKey(const T& key)
    {
        if (!HashFunctions::safeToCompareToEmptyOrDeleted)
            return;
        ASSERT(!HashTranslator::equal(KeyTraits::emptyValue(), key));
        typename std::aligned_storage<sizeof(ValueType), std::alignment_of<ValueType>::value>::type deletedValueBuffer;
        ValueType* deletedValuePtr = reinterpret_cast_ptr<ValueType*>(&deletedValueBuffer);
        ValueType& deletedValue = *deletedValuePtr;
        Traits::constructDeletedValue(deletedValue);
        ASSERT(!HashTranslator::equal(Extractor::extract(deletedValue), key));
    }

#endif

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T>
    inline auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::lookup(const T& key) -> ValueType*
    {
        return inlineLookup<HashTranslator>(key);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T>
    ALWAYS_INLINE auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::inlineLookup(const T& key) -> ValueType*
    {
        checkKey<HashTranslator>(key);

        unsigned k = 0;
        unsigned sizeMask = m_tableSizeMask;
        ValueType* table = m_table;
        unsigned h = HashTranslator::hash(key);
        unsigned i = h & sizeMask;

        if (!table)
            return 0;

#if DUMP_HASHTABLE_STATS
        ++HashTableStats::numAccesses;
        unsigned probeCount = 0;
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
        ++m_stats->numAccesses;
#endif

        while (1) {
            ValueType* entry = table + i;
                
            // we count on the compiler to optimize out this branch
            if (HashFunctions::safeToCompareToEmptyOrDeleted) {
                if (HashTranslator::equal(Extractor::extract(*entry), key))
                    return entry;
                
                if (isEmptyBucket(*entry))
                    return 0;
            } else {
                if (isEmptyBucket(*entry))
                    return 0;
                
                if (!isDeletedBucket(*entry) && HashTranslator::equal(Extractor::extract(*entry), key))
                    return entry;
            }
#if DUMP_HASHTABLE_STATS
            ++probeCount;
            HashTableStats::recordCollisionAtCount(probeCount);
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
            m_stats->recordCollisionAtCount(probeCount);
#endif

            if (k == 0)
                k = 1 | doubleHash(h);
            i = (i + k) & sizeMask;
        }
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T>
    inline auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::lookupForWriting(const T& key) -> LookupType
    {
        ASSERT(m_table);
        checkKey<HashTranslator>(key);

        unsigned k = 0;
        ValueType* table = m_table;
        unsigned sizeMask = m_tableSizeMask;
        unsigned h = HashTranslator::hash(key);
        unsigned i = h & sizeMask;

#if DUMP_HASHTABLE_STATS
        ++HashTableStats::numAccesses;
        unsigned probeCount = 0;
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
        ++m_stats->numAccesses;
#endif

        ValueType* deletedEntry = 0;

        while (1) {
            ValueType* entry = table + i;
            
            // we count on the compiler to optimize out this branch
            if (HashFunctions::safeToCompareToEmptyOrDeleted) {
                if (isEmptyBucket(*entry))
                    return LookupType(deletedEntry ? deletedEntry : entry, false);
                
                if (HashTranslator::equal(Extractor::extract(*entry), key))
                    return LookupType(entry, true);
                
                if (isDeletedBucket(*entry))
                    deletedEntry = entry;
            } else {
                if (isEmptyBucket(*entry))
                    return LookupType(deletedEntry ? deletedEntry : entry, false);
            
                if (isDeletedBucket(*entry))
                    deletedEntry = entry;
                else if (HashTranslator::equal(Extractor::extract(*entry), key))
                    return LookupType(entry, true);
            }
#if DUMP_HASHTABLE_STATS
            ++probeCount;
            HashTableStats::recordCollisionAtCount(probeCount);
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
            m_stats->recordCollisionAtCount(probeCount);
#endif

            if (k == 0)
                k = 1 | doubleHash(h);
            i = (i + k) & sizeMask;
        }
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T>
    inline auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::fullLookupForWriting(const T& key) -> FullLookupType
    {
        ASSERT(m_table);
        checkKey<HashTranslator>(key);

        unsigned k = 0;
        ValueType* table = m_table;
        unsigned sizeMask = m_tableSizeMask;
        unsigned h = HashTranslator::hash(key);
        unsigned i = h & sizeMask;

#if DUMP_HASHTABLE_STATS
        ++HashTableStats::numAccesses;
        unsigned probeCount = 0;
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
        ++m_stats->numAccesses;
#endif

        ValueType* deletedEntry = 0;

        while (1) {
            ValueType* entry = table + i;
            
            // we count on the compiler to optimize out this branch
            if (HashFunctions::safeToCompareToEmptyOrDeleted) {
                if (isEmptyBucket(*entry))
                    return makeLookupResult(deletedEntry ? deletedEntry : entry, false, h);
                
                if (HashTranslator::equal(Extractor::extract(*entry), key))
                    return makeLookupResult(entry, true, h);
                
                if (isDeletedBucket(*entry))
                    deletedEntry = entry;
            } else {
                if (isEmptyBucket(*entry))
                    return makeLookupResult(deletedEntry ? deletedEntry : entry, false, h);
            
                if (isDeletedBucket(*entry))
                    deletedEntry = entry;
                else if (HashTranslator::equal(Extractor::extract(*entry), key))
                    return makeLookupResult(entry, true, h);
            }
#if DUMP_HASHTABLE_STATS
            ++probeCount;
            HashTableStats::recordCollisionAtCount(probeCount);
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
            m_stats->recordCollisionAtCount(probeCount);
#endif

            if (k == 0)
                k = 1 | doubleHash(h);
            i = (i + k) & sizeMask;
        }
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T, typename Extra>
    ALWAYS_INLINE void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::addUniqueForInitialization(T&& key, Extra&& extra)
    {
        ASSERT(m_table);

        checkKey<HashTranslator>(key);

        invalidateIterators();

        internalCheckTableConsistency();

        unsigned k = 0;
        ValueType* table = m_table;
        unsigned sizeMask = m_tableSizeMask;
        unsigned h = HashTranslator::hash(key);
        unsigned i = h & sizeMask;

#if DUMP_HASHTABLE_STATS
        ++HashTableStats::numAccesses;
        unsigned probeCount = 0;
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
        ++m_stats->numAccesses;
#endif

        ValueType* entry;
        while (1) {
            entry = table + i;

            if (isEmptyBucket(*entry))
                break;

#if DUMP_HASHTABLE_STATS
            ++probeCount;
            HashTableStats::recordCollisionAtCount(probeCount);
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
            m_stats->recordCollisionAtCount(probeCount);
#endif

            if (k == 0)
                k = 1 | doubleHash(h);
            i = (i + k) & sizeMask;
        }

        HashTranslator::translate(*entry, std::forward<T>(key), std::forward<Extra>(extra));

        internalCheckTableConsistency();
    }

    template<bool emptyValueIsZero> struct HashTableBucketInitializer;

    template<> struct HashTableBucketInitializer<false> {
        template<typename Traits, typename Value> static void initialize(Value& bucket)
        {
            new (NotNull, std::addressof(bucket)) Value(Traits::emptyValue());
        }
    };

    template<> struct HashTableBucketInitializer<true> {
        template<typename Traits, typename Value> static void initialize(Value& bucket)
        {
            // This initializes the bucket without copying the empty value.
            // That makes it possible to use this with types that don't support copying.
            // The memset to 0 looks like a slow operation but is optimized by the compilers.
            memset(std::addressof(bucket), 0, sizeof(bucket));
        }
    };
    
    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::initializeBucket(ValueType& bucket)
    {
        HashTableBucketInitializer<Traits::emptyValueIsZero>::template initialize<Traits>(bucket);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T, typename Extra>
    ALWAYS_INLINE auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::add(T&& key, Extra&& extra) -> AddResult
    {
        checkKey<HashTranslator>(key);

        invalidateIterators();

        if (!m_table)
            expand(nullptr);

        internalCheckTableConsistency();

        ASSERT(m_table);

        unsigned k = 0;
        ValueType* table = m_table;
        unsigned sizeMask = m_tableSizeMask;
        unsigned h = HashTranslator::hash(key);
        unsigned i = h & sizeMask;

#if DUMP_HASHTABLE_STATS
        ++HashTableStats::numAccesses;
        unsigned probeCount = 0;
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
        ++m_stats->numAccesses;
#endif

        ValueType* deletedEntry = 0;
        ValueType* entry;
        while (1) {
            entry = table + i;
            
            // we count on the compiler to optimize out this branch
            if (HashFunctions::safeToCompareToEmptyOrDeleted) {
                if (isEmptyBucket(*entry))
                    break;
                
                if (HashTranslator::equal(Extractor::extract(*entry), key))
                    return AddResult(makeKnownGoodIterator(entry), false);
                
                if (isDeletedBucket(*entry))
                    deletedEntry = entry;
            } else {
                if (isEmptyBucket(*entry))
                    break;
            
                if (isDeletedBucket(*entry))
                    deletedEntry = entry;
                else if (HashTranslator::equal(Extractor::extract(*entry), key))
                    return AddResult(makeKnownGoodIterator(entry), false);
            }
#if DUMP_HASHTABLE_STATS
            ++probeCount;
            HashTableStats::recordCollisionAtCount(probeCount);
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
            m_stats->recordCollisionAtCount(probeCount);
#endif

            if (k == 0)
                k = 1 | doubleHash(h);
            i = (i + k) & sizeMask;
        }

        if (deletedEntry) {
            initializeBucket(*deletedEntry);
            entry = deletedEntry;
            --m_deletedCount; 
        }

        HashTranslator::translate(*entry, std::forward<T>(key), std::forward<Extra>(extra));
        ++m_keyCount;
        
        if (shouldExpand())
            entry = expand(entry);

        internalCheckTableConsistency();
        
        return AddResult(makeKnownGoodIterator(entry), true);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename HashTranslator, typename T, typename Extra>
    inline auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::addPassingHashCode(T&& key, Extra&& extra) -> AddResult
    {
        checkKey<HashTranslator>(key);

        invalidateIterators();

        if (!m_table)
            expand();

        internalCheckTableConsistency();

        FullLookupType lookupResult = fullLookupForWriting<HashTranslator>(key);

        ValueType* entry = lookupResult.first.first;
        bool found = lookupResult.first.second;
        unsigned h = lookupResult.second;
        
        if (found)
            return AddResult(makeKnownGoodIterator(entry), false);
        
        if (isDeletedBucket(*entry)) {
            initializeBucket(*entry);
            --m_deletedCount;
        }

        HashTranslator::translate(*entry, std::forward<T>(key), std::forward<Extra>(extra), h);
        ++m_keyCount;

        if (shouldExpand())
            entry = expand(entry);

        internalCheckTableConsistency();

        return AddResult(makeKnownGoodIterator(entry), true);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::reinsert(ValueType&& entry) -> ValueType*
    {
        ASSERT(m_table);
        ASSERT(!lookupForWriting(Extractor::extract(entry)).second);
        ASSERT(!isDeletedBucket(*(lookupForWriting(Extractor::extract(entry)).first)));
#if DUMP_HASHTABLE_STATS
        ++HashTableStats::numReinserts;
#endif
#if DUMP_HASHTABLE_STATS_PER_TABLE
        ++m_stats->numReinserts;
#endif

        Value* newEntry = lookupForWriting(Extractor::extract(entry)).first;
        newEntry->~Value();
        new (NotNull, newEntry) ValueType(WTFMove(entry));

        return newEntry;
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template <typename HashTranslator, typename T> 
    auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::find(const T& key) -> iterator
    {
        if (!m_table)
            return end();

        ValueType* entry = lookup<HashTranslator>(key);
        if (!entry)
            return end();

        return makeKnownGoodIterator(entry);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template <typename HashTranslator, typename T> 
    auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::find(const T& key) const -> const_iterator
    {
        if (!m_table)
            return end();

        ValueType* entry = const_cast<HashTable*>(this)->lookup<HashTranslator>(key);
        if (!entry)
            return end();

        return makeKnownGoodConstIterator(entry);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template <typename HashTranslator, typename T> 
    bool HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::contains(const T& key) const
    {
        if (!m_table)
            return false;

        return const_cast<HashTable*>(this)->lookup<HashTranslator>(key);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::removeAndInvalidateWithoutEntryConsistencyCheck(ValueType* pos)
    {
        invalidateIterators();
        remove(pos);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::removeAndInvalidate(ValueType* pos)
    {
        invalidateIterators();
        internalCheckTableConsistency();
        remove(pos);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::remove(ValueType* pos)
    {
#if DUMP_HASHTABLE_STATS
        ++HashTableStats::numRemoves;
#endif
#if DUMP_HASHTABLE_STATS_PER_TABLE
        ++m_stats->numRemoves;
#endif

        deleteBucket(*pos);
        ++m_deletedCount;
        --m_keyCount;

        if (shouldShrink())
            shrink();

        internalCheckTableConsistency();
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::remove(iterator it)
    {
        if (it == end())
            return;

        removeAndInvalidate(const_cast<ValueType*>(it.m_iterator.m_position));
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::removeWithoutEntryConsistencyCheck(iterator it)
    {
        if (it == end())
            return;

        removeAndInvalidateWithoutEntryConsistencyCheck(const_cast<ValueType*>(it.m_iterator.m_position));
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::removeWithoutEntryConsistencyCheck(const_iterator it)
    {
        if (it == end())
            return;

        removeAndInvalidateWithoutEntryConsistencyCheck(const_cast<ValueType*>(it.m_position));
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::remove(const KeyType& key)
    {
        remove(find(key));
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    template<typename Functor>
    inline void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::removeIf(const Functor& functor)
    {
        // We must use local copies in case "functor" or "deleteBucket"
        // make a function call, which prevents the compiler from keeping
        // the values in register.
        unsigned removedBucketCount = 0;
        ValueType* table = m_table;

        for (unsigned i = m_tableSize; i--;) {
            ValueType& bucket = table[i];
            if (isEmptyOrDeletedBucket(bucket))
                continue;
            
            if (!functor(bucket))
                continue;
            
            deleteBucket(bucket);
            ++removedBucketCount;
        }
        m_deletedCount += removedBucketCount;
        m_keyCount -= removedBucketCount;

        if (shouldShrink())
            shrink();
        
        internalCheckTableConsistency();
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::allocateTable(unsigned size) -> ValueType*
    {
        // would use a template member function with explicit specializations here, but
        // gcc doesn't appear to support that
        if (Traits::emptyValueIsZero)
            return static_cast<ValueType*>(fastZeroedMalloc(size * sizeof(ValueType)));
        ValueType* result = static_cast<ValueType*>(fastMalloc(size * sizeof(ValueType)));
        for (unsigned i = 0; i < size; i++)
            initializeBucket(result[i]);
        return result;
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::deallocateTable(ValueType* table, unsigned size)
    {
        for (unsigned i = 0; i < size; ++i) {
            if (!isDeletedBucket(table[i]))
                table[i].~ValueType();
        }
        fastFree(table);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::expand(ValueType* entry) -> ValueType*
    {
        unsigned newSize;
        if (m_tableSize == 0)
            newSize = KeyTraits::minimumTableSize;
        else if (mustRehashInPlace())
            newSize = m_tableSize;
        else
            newSize = m_tableSize * 2;

        return rehash(newSize, entry);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::rehash(unsigned newTableSize, ValueType* entry) -> ValueType*
    {
        internalCheckTableConsistencyExceptSize();

        unsigned oldTableSize = m_tableSize;
        ValueType* oldTable = m_table;

#if DUMP_HASHTABLE_STATS
        if (oldTableSize != 0)
            ++HashTableStats::numRehashes;
#endif

#if DUMP_HASHTABLE_STATS_PER_TABLE
        if (oldTableSize != 0)
            ++m_stats->numRehashes;
#endif

        m_tableSize = newTableSize;
        m_tableSizeMask = newTableSize - 1;
        m_table = allocateTable(newTableSize);

        Value* newEntry = nullptr;
        for (unsigned i = 0; i != oldTableSize; ++i) {
            if (isDeletedBucket(oldTable[i])) {
                ASSERT(std::addressof(oldTable[i]) != entry);
                continue;
            }

            if (isEmptyBucket(oldTable[i])) {
                ASSERT(std::addressof(oldTable[i]) != entry);
                oldTable[i].~ValueType();
                continue;
            }

            Value* reinsertedEntry = reinsert(WTFMove(oldTable[i]));
            oldTable[i].~ValueType();
            if (std::addressof(oldTable[i]) == entry) {
                ASSERT(!newEntry);
                newEntry = reinsertedEntry;
            }
        }

        m_deletedCount = 0;

        fastFree(oldTable);

        internalCheckTableConsistency();
        return newEntry;
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::clear()
    {
        invalidateIterators();
        if (!m_table)
            return;

        deallocateTable(m_table, m_tableSize);
        m_table = 0;
        m_tableSize = 0;
        m_tableSizeMask = 0;
        m_keyCount = 0;
        m_deletedCount = 0;
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::HashTable(const HashTable& other)
        : m_table(nullptr)
        , m_tableSize(0)
        , m_tableSizeMask(0)
        , m_keyCount(0)
        , m_deletedCount(0)
#if CHECK_HASHTABLE_ITERATORS
        , m_iterators(nullptr)
        , m_mutex(std::make_unique<Lock>())
#endif
#if DUMP_HASHTABLE_STATS_PER_TABLE
        , m_stats(std::make_unique<Stats>(*other.m_stats))
#endif
    {
        unsigned otherKeyCount = other.size();
        if (!otherKeyCount)
            return;

        unsigned bestTableSize = WTF::roundUpToPowerOfTwo(otherKeyCount) * 2;

        // With maxLoad at 1/2 and minLoad at 1/6, our average load is 2/6.
        // If we are getting halfway between 2/6 and 1/2 (past 5/12), we double the size to avoid being too close to
        // loadMax and bring the ratio close to 2/6. This give us a load in the bounds [3/12, 5/12).
        bool aboveThreeQuarterLoad = otherKeyCount * 12 >= bestTableSize * 5;
        if (aboveThreeQuarterLoad)
            bestTableSize *= 2;

        unsigned minimumTableSize = KeyTraits::minimumTableSize;
        m_tableSize = std::max<unsigned>(bestTableSize, minimumTableSize);
        m_tableSizeMask = m_tableSize - 1;
        m_keyCount = otherKeyCount;
        m_table = allocateTable(m_tableSize);

        for (const auto& otherValue : other)
            addUniqueForInitialization<IdentityTranslatorType>(Extractor::extract(otherValue), otherValue);
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::swap(HashTable& other)
    {
        invalidateIterators();
        other.invalidateIterators();

        std::swap(m_table, other.m_table);
        std::swap(m_tableSize, other.m_tableSize);
        std::swap(m_tableSizeMask, other.m_tableSizeMask);
        std::swap(m_keyCount, other.m_keyCount);
        std::swap(m_deletedCount, other.m_deletedCount);

#if DUMP_HASHTABLE_STATS_PER_TABLE
        m_stats.swap(other.m_stats);
#endif
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::operator=(const HashTable& other) -> HashTable&
    {
        HashTable tmp(other);
        swap(tmp);
        return *this;
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::HashTable(HashTable&& other)
#if CHECK_HASHTABLE_ITERATORS
        : m_iterators(nullptr)
        , m_mutex(std::make_unique<Lock>())
#endif
    {
        other.invalidateIterators();

        m_table = other.m_table;
        m_tableSize = other.m_tableSize;
        m_tableSizeMask = other.m_tableSizeMask;
        m_keyCount = other.m_keyCount;
        m_deletedCount = other.m_deletedCount;

        other.m_table = nullptr;
        other.m_tableSize = 0;
        other.m_tableSizeMask = 0;
        other.m_keyCount = 0;
        other.m_deletedCount = 0;

#if DUMP_HASHTABLE_STATS_PER_TABLE
        m_stats = WTFMove(other.m_stats);
        other.m_stats = nullptr;
#endif
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    inline auto HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::operator=(HashTable&& other) -> HashTable&
    {
        HashTable temp = WTFMove(other);
        swap(temp);
        return *this;
    }

#if !ASSERT_DISABLED

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::checkTableConsistency() const
    {
        checkTableConsistencyExceptSize();
        ASSERT(!m_table || !shouldExpand());
        ASSERT(!shouldShrink());
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::checkTableConsistencyExceptSize() const
    {
        if (!m_table)
            return;

        unsigned count = 0;
        unsigned deletedCount = 0;
        for (unsigned j = 0; j < m_tableSize; ++j) {
            ValueType* entry = m_table + j;
            if (isEmptyBucket(*entry))
                continue;

            if (isDeletedBucket(*entry)) {
                ++deletedCount;
                continue;
            }

            const_iterator it = find(Extractor::extract(*entry));
            ASSERT(entry == it.m_position);
            ++count;

            ValueCheck<Key>::checkConsistency(it->key);
        }

        ASSERT(count == m_keyCount);
        ASSERT(deletedCount == m_deletedCount);
        ASSERT(m_tableSize >= KeyTraits::minimumTableSize);
        ASSERT(m_tableSizeMask);
        ASSERT(m_tableSize == m_tableSizeMask + 1);
    }

#endif // ASSERT_DISABLED

#if CHECK_HASHTABLE_ITERATORS

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>::invalidateIterators()
    {
        std::lock_guard<Lock> lock(*m_mutex);
        const_iterator* next;
        for (const_iterator* p = m_iterators; p; p = next) {
            next = p->m_next;
            p->m_table = 0;
            p->m_next = 0;
            p->m_previous = 0;
        }
        m_iterators = 0;
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void addIterator(const HashTable<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>* table,
        HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>* it)
    {
        it->m_table = table;
        it->m_previous = 0;

        // Insert iterator at head of doubly-linked list of iterators.
        if (!table) {
            it->m_next = 0;
        } else {
            std::lock_guard<Lock> lock(*table->m_mutex);
            ASSERT(table->m_iterators != it);
            it->m_next = table->m_iterators;
            table->m_iterators = it;
            if (it->m_next) {
                ASSERT(!it->m_next->m_previous);
                it->m_next->m_previous = it;
            }
        }
    }

    template<typename Key, typename Value, typename Extractor, typename HashFunctions, typename Traits, typename KeyTraits>
    void removeIterator(HashTableConstIterator<Key, Value, Extractor, HashFunctions, Traits, KeyTraits>* it)
    {
        // Delete iterator from doubly-linked list of iterators.
        if (!it->m_table) {
            ASSERT(!it->m_next);
            ASSERT(!it->m_previous);
        } else {
            std::lock_guard<Lock> lock(*it->m_table->m_mutex);
            if (it->m_next) {
                ASSERT(it->m_next->m_previous == it);
                it->m_next->m_previous = it->m_previous;
            }
            if (it->m_previous) {
                ASSERT(it->m_table->m_iterators != it);
                ASSERT(it->m_previous->m_next == it);
                it->m_previous->m_next = it->m_next;
            } else {
                ASSERT(it->m_table->m_iterators == it);
                it->m_table->m_iterators = it->m_next;
            }
        }

        it->m_table = 0;
        it->m_next = 0;
        it->m_previous = 0;
    }

#endif // CHECK_HASHTABLE_ITERATORS

    // iterator adapters

    template<typename HashTableType, typename ValueType> struct HashTableConstIteratorAdapter : public std::iterator<std::forward_iterator_tag, ValueType, std::ptrdiff_t, const ValueType*, const ValueType&> {
        HashTableConstIteratorAdapter() {}
        HashTableConstIteratorAdapter(const typename HashTableType::const_iterator& impl) : m_impl(impl) {}

        const ValueType* get() const { return (const ValueType*)m_impl.get(); }
        const ValueType& operator*() const { return *get(); }
        const ValueType* operator->() const { return get(); }

        HashTableConstIteratorAdapter& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        typename HashTableType::const_iterator m_impl;
    };

    template<typename HashTableType, typename ValueType> struct HashTableIteratorAdapter : public std::iterator<std::forward_iterator_tag, ValueType, std::ptrdiff_t, ValueType*, ValueType&> {
        HashTableIteratorAdapter() {}
        HashTableIteratorAdapter(const typename HashTableType::iterator& impl) : m_impl(impl) {}

        ValueType* get() const { return (ValueType*)m_impl.get(); }
        ValueType& operator*() const { return *get(); }
        ValueType* operator->() const { return get(); }

        HashTableIteratorAdapter& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        operator HashTableConstIteratorAdapter<HashTableType, ValueType>() {
            typename HashTableType::const_iterator i = m_impl;
            return i;
        }

        typename HashTableType::iterator m_impl;
    };

    template<typename T, typename U>
    inline bool operator==(const HashTableConstIteratorAdapter<T, U>& a, const HashTableConstIteratorAdapter<T, U>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U>
    inline bool operator!=(const HashTableConstIteratorAdapter<T, U>& a, const HashTableConstIteratorAdapter<T, U>& b)
    {
        return a.m_impl != b.m_impl;
    }

    template<typename T, typename U>
    inline bool operator==(const HashTableIteratorAdapter<T, U>& a, const HashTableIteratorAdapter<T, U>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U>
    inline bool operator!=(const HashTableIteratorAdapter<T, U>& a, const HashTableIteratorAdapter<T, U>& b)
    {
        return a.m_impl != b.m_impl;
    }

    // All 4 combinations of ==, != and Const,non const.
    template<typename T, typename U>
    inline bool operator==(const HashTableConstIteratorAdapter<T, U>& a, const HashTableIteratorAdapter<T, U>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U>
    inline bool operator!=(const HashTableConstIteratorAdapter<T, U>& a, const HashTableIteratorAdapter<T, U>& b)
    {
        return a.m_impl != b.m_impl;
    }

    template<typename T, typename U>
    inline bool operator==(const HashTableIteratorAdapter<T, U>& a, const HashTableConstIteratorAdapter<T, U>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U>
    inline bool operator!=(const HashTableIteratorAdapter<T, U>& a, const HashTableConstIteratorAdapter<T, U>& b)
    {
        return a.m_impl != b.m_impl;
    }

} // namespace WTF

#include <wtf/HashIterators.h>
