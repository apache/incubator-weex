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

#include "ExceptionHelpers.h"
#include "JSObject.h"

namespace JSC {

JS_EXPORT_PRIVATE const ClassInfo* getHashMapBucketKeyClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getHashMapBucketKeyValueClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getHashMapImplKeyClassInfo();
JS_EXPORT_PRIVATE const ClassInfo* getHashMapImplKeyValueClassInfo();

enum class HashTableType {
    Key,
    KeyValue
};

struct HashMapBucketDataKey {
    static const HashTableType Type = HashTableType::Key;
    WriteBarrier<Unknown> key;
};

struct HashMapBucketDataKeyValue {
    static const HashTableType Type = HashTableType::KeyValue;
    WriteBarrier<Unknown> key;
    WriteBarrier<Unknown> value;
};

template <typename Data>
class HashMapBucket : public JSCell {
    typedef JSCell Base;

    template <typename T = Data>
    static typename std::enable_if<std::is_same<T, HashMapBucketDataKey>::value, Structure*>::type selectStructure(VM& vm)
    {
        return vm.hashMapBucketSetStructure.get();
    }

    template <typename T = Data>
    static typename std::enable_if<std::is_same<T, HashMapBucketDataKeyValue>::value, Structure*>::type selectStructure(VM& vm)
    {
        return vm.hashMapBucketMapStructure.get();
    }

public:
    static const HashTableType Type = Data::Type;
    static const ClassInfo s_info; // This is never accessed directly, since that would break linkage on some compilers.

    static const ClassInfo* info()
    {
        switch (Type) {
        case HashTableType::Key:
            return getHashMapBucketKeyClassInfo();
        case HashTableType::KeyValue:
            return getHashMapBucketKeyValueClassInfo();
        }
        RELEASE_ASSERT_NOT_REACHED();
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    static HashMapBucket* create(VM& vm)
    {
        HashMapBucket* bucket = new (NotNull, allocateCell<HashMapBucket<Data>>(vm.heap)) HashMapBucket(vm, selectStructure(vm));
        bucket->finishCreation(vm);
        ASSERT(!bucket->next());
        ASSERT(!bucket->prev());
        return bucket;
    }

    HashMapBucket(VM& vm, Structure* structure)
        : Base(vm, structure)
    { }

    ALWAYS_INLINE void setNext(VM& vm, HashMapBucket* bucket)
    {
        m_next.set(vm, this, bucket);
    }
    ALWAYS_INLINE void setPrev(VM& vm, HashMapBucket* bucket)
    {
        m_prev.set(vm, this, bucket);
    }

    ALWAYS_INLINE void setKey(VM& vm, JSValue key)
    {
        m_data.key.set(vm, this, key);
    }

    template <typename T = Data>
    ALWAYS_INLINE typename std::enable_if<std::is_same<T, HashMapBucketDataKeyValue>::value>::type setValue(VM& vm, JSValue value)
    {
        m_data.value.set(vm, this, value);
    }
    template <typename T = Data>
    ALWAYS_INLINE typename std::enable_if<std::is_same<T, HashMapBucketDataKey>::value>::type setValue(VM&, JSValue) { }

    ALWAYS_INLINE JSValue key() const { return m_data.key.get(); }

    template <typename T = Data>
    ALWAYS_INLINE typename std::enable_if<std::is_same<T, HashMapBucketDataKeyValue>::value, JSValue>::type value() const
    {
        return m_data.value.get();
    }

    static void visitChildren(JSCell*, SlotVisitor&);

    ALWAYS_INLINE HashMapBucket* next() const { return m_next.get(); }
    ALWAYS_INLINE HashMapBucket* prev() const { return m_prev.get(); }

    ALWAYS_INLINE bool deleted() const { return m_deleted; }
    ALWAYS_INLINE void setDeleted(bool deleted) { m_deleted = deleted; }

    static ptrdiff_t offsetOfKey()
    {
        return OBJECT_OFFSETOF(HashMapBucket, m_data) + OBJECT_OFFSETOF(Data, key);
    }

    template <typename T = Data>
    static typename std::enable_if<std::is_same<T, HashMapBucketDataKeyValue>::value, ptrdiff_t>::type offsetOfValue()
    {
        return OBJECT_OFFSETOF(HashMapBucket, m_data) + OBJECT_OFFSETOF(Data, value);
    }

private:
    Data m_data;
    WriteBarrier<HashMapBucket> m_next;
    WriteBarrier<HashMapBucket> m_prev;
    bool m_deleted { false };
};

template <typename BucketType>
class HashMapBuffer {
public:
    HashMapBuffer() = delete;

    static size_t allocationSize(uint32_t capacity)
    {
        return capacity * sizeof(BucketType*);
    }

    ALWAYS_INLINE BucketType** buffer() const
    {
        return bitwise_cast<BucketType**>(this);
    }

    static HashMapBuffer* create(ExecState* exec, VM& vm, JSCell*, uint32_t capacity)
    {
        auto scope = DECLARE_THROW_SCOPE(vm);
        size_t allocationSize = HashMapBuffer::allocationSize(capacity);
        void* data = vm.auxiliarySpace.tryAllocate(allocationSize);
        if (!data) {
            throwOutOfMemoryError(exec, scope);
            return nullptr;
        }

        HashMapBuffer* buffer = static_cast<HashMapBuffer*>(data);
        buffer->reset(capacity);
        return buffer;
    }

    ALWAYS_INLINE void reset(uint32_t capacity)
    {
        memset(this, -1, allocationSize(capacity));
    }
};

ALWAYS_INLINE static bool areKeysEqual(ExecState* exec, JSValue a, JSValue b)
{
    // We want +0 and -0 to be compared to true here. sameValue() itself doesn't
    // guarantee that, however, we normalize all keys before comparing and storing
    // them in the map. The normalization will convert -0.0 and 0.0 to the integer
    // representation for 0.
    return sameValue(exec, a, b);
}

ALWAYS_INLINE JSValue normalizeMapKey(JSValue key)
{
    if (!key.isNumber())
        return key;

    if (key.isInt32())
        return key;

    double d = key.asDouble();
    if (std::isnan(d))
        return key;

    int i = static_cast<int>(d);
    if (i == d) {
        // When a key is -0, we convert it to positive zero.
        // When a key is the double representation for an integer, we convert it to an integer.
        return jsNumber(i);
    }
    // This means key is definitely not negative zero, and it's definitely not a double representation of an integer. 
    return key;
}

static ALWAYS_INLINE uint32_t wangsInt64Hash(uint64_t key)
{
    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
    return static_cast<unsigned>(key);
}

ALWAYS_INLINE uint32_t jsMapHash(ExecState* exec, VM& vm, JSValue value)
{
    ASSERT_WITH_MESSAGE(normalizeMapKey(value) == value, "We expect normalized values flowing into this function.");

    if (value.isString()) {
        auto scope = DECLARE_THROW_SCOPE(vm);
        const String& wtfString = asString(value)->value(exec);
        RETURN_IF_EXCEPTION(scope, UINT_MAX);
        return wtfString.impl()->hash();
    }

    return wangsInt64Hash(JSValue::encode(value));
}

ALWAYS_INLINE std::optional<uint32_t> concurrentJSMapHash(JSValue key)
{
    key = normalizeMapKey(key);
    if (key.isString()) {
        JSString* string = asString(key);
        if (string->length() > 10 * 1024)
            return std::nullopt;
        const StringImpl* impl = string->tryGetValueImpl();
        if (!impl)
            return std::nullopt;
        return impl->concurrentHash();
    }

    uint64_t rawValue = JSValue::encode(key);
    return wangsInt64Hash(rawValue);
}

template <typename HashMapBucketType>
class HashMapImpl : public JSCell {
    typedef JSCell Base;
    typedef HashMapBuffer<HashMapBucketType> HashMapBufferType;

    template <typename T = HashMapBucketType>
    static typename std::enable_if<std::is_same<T, HashMapBucket<HashMapBucketDataKey>>::value, Structure*>::type selectStructure(VM& vm)
    {
        return vm.hashMapImplSetStructure.get();
    }

    template <typename T = HashMapBucketType>
    static typename std::enable_if<std::is_same<T, HashMapBucket<HashMapBucketDataKeyValue>>::value, Structure*>::type selectStructure(VM& vm)
    {
        return vm.hashMapImplMapStructure.get();
    }

public:
    static const ClassInfo s_info; // This is never accessed directly, since that would break linkage on some compilers.

    static const ClassInfo* info()
    {
        switch (HashMapBucketType::Type) {
        case HashTableType::Key:
            return getHashMapImplKeyClassInfo();
        case HashTableType::KeyValue:
            return getHashMapImplKeyValueClassInfo();
        }
        RELEASE_ASSERT_NOT_REACHED();
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    static HashMapImpl* create(ExecState* exec, VM& vm)
    {
        ASSERT_WITH_MESSAGE(HashMapBucket<HashMapBucketDataKey>::offsetOfKey() == HashMapBucket<HashMapBucketDataKeyValue>::offsetOfKey(), "We assume this to be true in the DFG and FTL JIT.");

        HashMapImpl* impl = new (NotNull, allocateCell<HashMapImpl>(vm.heap)) HashMapImpl(vm, selectStructure(vm));
        impl->finishCreation(exec, vm);
        return impl;
    }

    static void visitChildren(JSCell*, SlotVisitor&);

    HashMapImpl(VM& vm, Structure* structure)
        : Base(vm, structure)
        , m_keyCount(0)
        , m_deleteCount(0)
        , m_capacity(4)
    {
    }

    ALWAYS_INLINE HashMapBucketType** buffer() const
    {
        return m_buffer.get()->buffer();
    }

    void finishCreation(ExecState* exec, VM& vm)
    {
        auto scope = DECLARE_THROW_SCOPE(vm);
        Base::finishCreation(vm);

        makeAndSetNewBuffer(exec, vm);
        RETURN_IF_EXCEPTION(scope, void());

        m_head.set(vm, this, HashMapBucketType::create(vm));
        m_tail.set(vm, this, HashMapBucketType::create(vm));

        m_head->setNext(vm, m_tail.get());
        m_tail->setPrev(vm, m_head.get());
        m_head->setDeleted(true);
        m_tail->setDeleted(true);

    }

    static HashMapBucketType* emptyValue()
    {
        return bitwise_cast<HashMapBucketType*>(static_cast<uintptr_t>(-1));
    }

    static ALWAYS_INLINE bool isEmpty(HashMapBucketType* bucket)
    {
        return bucket == emptyValue();
    }

    static HashMapBucketType* deletedValue()
    {
        return bitwise_cast<HashMapBucketType*>(static_cast<uintptr_t>(-3));
    }

    static ALWAYS_INLINE bool isDeleted(HashMapBucketType* bucket)
    {
        return bucket == deletedValue();
    }

    ALWAYS_INLINE HashMapBucketType** findBucket(ExecState* exec, JSValue key)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);
        key = normalizeMapKey(key);
        uint32_t hash = jsMapHash(exec, vm, key);
        RETURN_IF_EXCEPTION(scope, nullptr);
        return findBucket(exec, key, hash);
    }

    ALWAYS_INLINE HashMapBucketType** findBucket(ExecState* exec, JSValue key, uint32_t hash)
    {
        ASSERT_WITH_MESSAGE(normalizeMapKey(key) == key, "We expect normalized values flowing into this function.");
        return findBucketAlreadyHashedAndNormalized(exec, key, hash);
    }

    ALWAYS_INLINE JSValue get(ExecState* exec, JSValue key)
    {
        if (HashMapBucketType** bucket = findBucket(exec, key))
            return (*bucket)->value();
        return jsUndefined();
    }

    ALWAYS_INLINE bool has(ExecState* exec, JSValue key)
    {
        return !!findBucket(exec, key);
    }

    ALWAYS_INLINE void add(ExecState* exec, JSValue key, JSValue value = JSValue())
    {
        key = normalizeMapKey(key);

        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        const uint32_t mask = m_capacity - 1;
        uint32_t index = jsMapHash(exec, vm, key) & mask;
        RETURN_IF_EXCEPTION(scope, void());
        HashMapBucketType** buffer = this->buffer();
        HashMapBucketType* bucket = buffer[index];
        while (!isEmpty(bucket)) {
            if (!isDeleted(bucket) && areKeysEqual(exec, key, bucket->key())) {
                bucket->setValue(vm, value);
                return;
            }
            index = (index + 1) & mask;
            bucket = buffer[index];
        }

        HashMapBucketType* newEntry = m_tail.get();
        buffer[index] = newEntry;
        newEntry->setKey(vm, key);
        newEntry->setValue(vm, value);
        newEntry->setDeleted(false);
        HashMapBucketType* newTail = HashMapBucketType::create(vm);
        m_tail.set(vm, this, newTail);
        newTail->setPrev(vm, newEntry);
        newTail->setDeleted(true);
        newEntry->setNext(vm, newTail);

        ++m_keyCount;

        if (shouldRehashAfterAdd())
            rehash(exec);
    }

    ALWAYS_INLINE bool remove(ExecState* exec, JSValue key)
    {
        HashMapBucketType** bucket = findBucket(exec, key);
        if (!bucket)
            return false;

        VM& vm = exec->vm();
        HashMapBucketType* impl = *bucket;
        impl->next()->setPrev(vm, impl->prev());
        impl->prev()->setNext(vm, impl->next());
        impl->setDeleted(true);

        *bucket = deletedValue();

        ++m_deleteCount;
        ASSERT(m_keyCount > 0);
        --m_keyCount;

        if (shouldShrink())
            rehash(exec);

        return true;
    }

    ALWAYS_INLINE uint32_t size() const
    {
        return m_keyCount;
    }

    ALWAYS_INLINE void clear(ExecState* exec)
    {
        VM& vm = exec->vm();
        m_keyCount = 0;
        m_deleteCount = 0;
        HashMapBucketType* head = m_head.get();
        HashMapBucketType* bucket = m_head->next();
        HashMapBucketType* tail = m_tail.get();
        while (bucket != tail) {
            HashMapBucketType* next = bucket->next();
            // We restart each iterator by pointing it to the head of the list.
            bucket->setNext(vm, head);
            bucket->setDeleted(true);
            bucket = next;
        }
        m_head->setNext(vm, m_tail.get());
        m_tail->setPrev(vm, m_head.get());
        m_capacity = 4;
        makeAndSetNewBuffer(exec, vm);
        checkConsistency();
    }

    ALWAYS_INLINE size_t bufferSizeInBytes() const
    {
        return m_capacity * sizeof(HashMapBucketType*);
    }

    static ptrdiff_t offsetOfBuffer()
    {
        return OBJECT_OFFSETOF(HashMapImpl<HashMapBucketType>, m_buffer);
    }

    static ptrdiff_t offsetOfCapacity()
    {
        return OBJECT_OFFSETOF(HashMapImpl<HashMapBucketType>, m_capacity);
    }

    HashMapBucketType* head() { return m_head.get(); }
    HashMapBucketType* tail() { return m_tail.get(); }

    size_t approximateSize() const
    {
        size_t size = sizeof(HashMapImpl);
        size += bufferSizeInBytes();
        size += 2 * sizeof(HashMapBucketType); // Head and tail members.
        size += m_keyCount * sizeof(HashMapBucketType); // Number of members that are on the list.
        return size;
    }

private:
    ALWAYS_INLINE uint32_t shouldRehashAfterAdd() const
    {
        return 2 * (m_keyCount + m_deleteCount) >= m_capacity;
    }

    ALWAYS_INLINE uint32_t shouldShrink() const
    {
        return 8 * m_keyCount <= m_capacity && m_capacity > 4;
    }

    ALWAYS_INLINE HashMapBucketType** findBucketAlreadyHashedAndNormalized(ExecState* exec, JSValue key, uint32_t hash)
    {
        const uint32_t mask = m_capacity - 1;
        uint32_t index = hash & mask;
        HashMapBucketType** buffer = this->buffer();
        HashMapBucketType* bucket = buffer[index];

        while (!isEmpty(bucket)) {
            if (!isDeleted(bucket) && areKeysEqual(exec, key, bucket->key()))
                return buffer + index;
            index = (index + 1) & mask;
            bucket = buffer[index];
        }
        return nullptr;
    }

    void rehash(ExecState* exec)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);

        uint32_t oldCapacity = m_capacity;
        if (shouldShrink()) {
            m_capacity = m_capacity / 2;
            ASSERT(m_capacity >= 4);
        } else if (3 * m_keyCount <= m_capacity && m_capacity > 64) {
            // We stay at the same size if rehashing would cause us to be no more than
            // 1/3rd full. This comes up for programs like this:
            // Say the hash table grew to a key count of 64, causing it to grow to a capacity of 256.
            // Then, the table added 63 items. The load is now 127. Then, 63 items are deleted.
            // The load is still 127. Then, another item is added. The load is now 128, and we
            // decide that we need to rehash. The key count is 65, almost exactly what it was
            // when we grew to a capacity of 256. We don't really need to grow to a capacity
            // of 512 in this situation. Instead, we choose to rehash at the same size. This
            // will bring the load down to 65. We rehash into the same size when we determine
            // that the new load ratio will be under 1/3rd. (We also pick a minumum capacity
            // at which this rule kicks in because otherwise we will be too sensitive to rehashing
            // at the same capacity).
        } else
            m_capacity = (Checked<uint32_t>(m_capacity) * 2).unsafeGet();

        if (m_capacity != oldCapacity) {
            makeAndSetNewBuffer(exec, vm);
            RETURN_IF_EXCEPTION(scope, void());
        } else {
            m_buffer.get()->reset(m_capacity);
            assertBufferIsEmpty();
        }

        HashMapBucketType* iter = m_head->next();
        HashMapBucketType* end = m_tail.get();
        const uint32_t mask = m_capacity - 1;
        RELEASE_ASSERT(!(m_capacity & (m_capacity - 1)));
        HashMapBucketType** buffer = this->buffer();
        while (iter != end) {
            uint32_t index = jsMapHash(exec, vm, iter->key()) & mask;
            ASSERT_WITH_MESSAGE(!scope.exception(), "All keys should already be hashed before, so this should not throw because it won't resolve ropes.");
            {
                HashMapBucketType* bucket = buffer[index];
                while (!isEmpty(bucket)) {
                    index = (index + 1) & mask;
                    bucket = buffer[index];
                }
            }
            buffer[index] = iter;
            iter = iter->next();
        }

        m_deleteCount = 0;

        checkConsistency();
    }

    ALWAYS_INLINE void checkConsistency() const
    {
        if (!ASSERT_DISABLED) {
            HashMapBucketType* iter = m_head->next();
            HashMapBucketType* end = m_tail.get();
            uint32_t size = 0;
            while (iter != end) {
                ++size;
                iter = iter->next();
            }
            ASSERT(size == m_keyCount);
        }
    }

    void makeAndSetNewBuffer(ExecState* exec, VM& vm)
    {
        ASSERT(!(m_capacity & (m_capacity - 1)));

        HashMapBufferType* buffer = HashMapBufferType::create(exec, vm, this, m_capacity);
        if (UNLIKELY(!buffer))
            return;

        m_buffer.set(vm, this, buffer);
        assertBufferIsEmpty();
    }

    ALWAYS_INLINE void assertBufferIsEmpty() const
    {
        if (!ASSERT_DISABLED) {
            for (unsigned i = 0; i < m_capacity; i++)
                ASSERT(isEmpty(buffer()[i]));
        }
    }

    WriteBarrier<HashMapBucketType> m_head;
    WriteBarrier<HashMapBucketType> m_tail;
    AuxiliaryBarrier<HashMapBufferType*> m_buffer;
    uint32_t m_keyCount;
    uint32_t m_deleteCount;
    uint32_t m_capacity;
};

} // namespace JSC
