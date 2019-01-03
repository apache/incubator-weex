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

#include "IndexingHeader.h"
#include "PropertyOffset.h"
#include "PropertyStorage.h"
#include <wtf/Noncopyable.h>

namespace JSC {

class VM;
class CopyVisitor;
struct ArrayStorage;

template <typename T> struct ContiguousData {
    ContiguousData()
        : m_data(0)
#if !ASSERT_DISABLED
        , m_length(0)
#endif
    {
    }
    ContiguousData(T* data, size_t length)
        : m_data(data)
#if !ASSERT_DISABLED
        , m_length(length)
#endif
    {
        UNUSED_PARAM(length);
    }

    const T& operator[](size_t index) const { ASSERT(index < m_length); return m_data[index]; }
    T& operator[](size_t index) { ASSERT(index < m_length); return m_data[index]; }

    T* data() const { return m_data; }
#if !ASSERT_DISABLED
    size_t length() const { return m_length; }
#endif

private:
    T* m_data;
#if !ASSERT_DISABLED
    size_t m_length;
#endif
};

typedef ContiguousData<double> ContiguousDoubles;
typedef ContiguousData<WriteBarrier<Unknown>> ContiguousJSValues;

class Butterfly {
    WTF_MAKE_NONCOPYABLE(Butterfly);
private:
    Butterfly() { } // Not instantiable.
public:
    
    static size_t totalSize(size_t preCapacity, size_t propertyCapacity, bool hasIndexingHeader, size_t indexingPayloadSizeInBytes)
    {
        ASSERT(indexingPayloadSizeInBytes ? hasIndexingHeader : true);
        ASSERT(sizeof(EncodedJSValue) == sizeof(IndexingHeader));
        return (preCapacity + propertyCapacity) * sizeof(EncodedJSValue) + (hasIndexingHeader ? sizeof(IndexingHeader) : 0) + indexingPayloadSizeInBytes;
    }

    static Butterfly* fromBase(void* base, size_t preCapacity, size_t propertyCapacity)
    {
        return reinterpret_cast<Butterfly*>(static_cast<EncodedJSValue*>(base) + preCapacity + propertyCapacity + 1);
    }
    
    ALWAYS_INLINE static unsigned availableContiguousVectorLength(size_t propertyCapacity, unsigned vectorLength);
    static unsigned availableContiguousVectorLength(Structure*, unsigned vectorLength);
    
    ALWAYS_INLINE static unsigned optimalContiguousVectorLength(size_t propertyCapacity, unsigned vectorLength);
    static unsigned optimalContiguousVectorLength(Structure*, unsigned vectorLength);
    
    // This method is here not just because it's handy, but to remind you that
    // the whole point of butterflies is to do evil pointer arithmetic.
    static Butterfly* fromPointer(char* ptr)
    {
        return reinterpret_cast<Butterfly*>(ptr);
    }
    
    char* pointer() { return reinterpret_cast<char*>(this); }
    
    static ptrdiff_t offsetOfIndexingHeader() { return IndexingHeader::offsetOfIndexingHeader(); }
    static ptrdiff_t offsetOfArrayBuffer() { return offsetOfIndexingHeader() + IndexingHeader::offsetOfArrayBuffer(); }
    static ptrdiff_t offsetOfPublicLength() { return offsetOfIndexingHeader() + IndexingHeader::offsetOfPublicLength(); }
    static ptrdiff_t offsetOfVectorLength() { return offsetOfIndexingHeader() + IndexingHeader::offsetOfVectorLength(); }
    
    static Butterfly* createUninitialized(VM&, JSCell* intendedOwner, size_t preCapacity, size_t propertyCapacity, bool hasIndexingHeader, size_t indexingPayloadSizeInBytes);

    static Butterfly* tryCreate(VM& vm, JSCell*, size_t preCapacity, size_t propertyCapacity, bool hasIndexingHeader, const IndexingHeader& indexingHeader, size_t indexingPayloadSizeInBytes);
    static Butterfly* create(VM&, JSCell* intendedOwner, size_t preCapacity, size_t propertyCapacity, bool hasIndexingHeader, const IndexingHeader&, size_t indexingPayloadSizeInBytes);
    static Butterfly* create(VM&, JSCell* intendedOwner, Structure*);
    
    IndexingHeader* indexingHeader() { return IndexingHeader::from(this); }
    const IndexingHeader* indexingHeader() const { return IndexingHeader::from(this); }
    PropertyStorage propertyStorage() { return indexingHeader()->propertyStorage(); }
    ConstPropertyStorage propertyStorage() const { return indexingHeader()->propertyStorage(); }
    
    uint32_t publicLength() { return indexingHeader()->publicLength(); }
    uint32_t vectorLength() { return indexingHeader()->vectorLength(); }
    void setPublicLength(uint32_t value) { indexingHeader()->setPublicLength(value); }
    void setVectorLength(uint32_t value) { indexingHeader()->setVectorLength(value); }
    
    template<typename T>
    T* indexingPayload() { return reinterpret_cast_ptr<T*>(this); }
    ArrayStorage* arrayStorage() { return indexingPayload<ArrayStorage>(); }
    ContiguousJSValues contiguousInt32() { return ContiguousJSValues(indexingPayload<WriteBarrier<Unknown>>(), vectorLength()); }

    ContiguousDoubles contiguousDouble() { return ContiguousDoubles(indexingPayload<double>(), vectorLength()); }
    ContiguousJSValues contiguous() { return ContiguousJSValues(indexingPayload<WriteBarrier<Unknown>>(), vectorLength()); }
    
    static Butterfly* fromContiguous(WriteBarrier<Unknown>* contiguous)
    {
        return reinterpret_cast<Butterfly*>(contiguous);
    }
    static Butterfly* fromContiguous(double* contiguous)
    {
        return reinterpret_cast<Butterfly*>(contiguous);
    }
    
    static ptrdiff_t offsetOfPropertyStorage() { return -static_cast<ptrdiff_t>(sizeof(IndexingHeader)); }
    static int indexOfPropertyStorage()
    {
        ASSERT(sizeof(IndexingHeader) == sizeof(EncodedJSValue));
        return -1;
    }

    void* base(size_t preCapacity, size_t propertyCapacity) { return propertyStorage() - propertyCapacity - preCapacity; }
    void* base(Structure*);

    static Butterfly* createOrGrowArrayRight(
        Butterfly*, VM&, JSCell* intendedOwner, Structure* oldStructure,
        size_t propertyCapacity, bool hadIndexingHeader,
        size_t oldIndexingPayloadSizeInBytes, size_t newIndexingPayloadSizeInBytes); 

    // The butterfly reallocation methods perform the reallocation itself but do not change any
    // of the meta-data to reflect that the reallocation occurred. Note that this set of
    // methods is not exhaustive and is not intended to encapsulate all possible allocation
    // modes of butterflies - there are code paths that allocate butterflies by calling
    // directly into Heap::tryAllocateStorage.
    static Butterfly* createOrGrowPropertyStorage(Butterfly*, VM&, JSCell* intendedOwner, Structure*, size_t oldPropertyCapacity, size_t newPropertyCapacity);
    Butterfly* growArrayRight(VM&, JSCell* intendedOwner, Structure* oldStructure, size_t propertyCapacity, bool hadIndexingHeader, size_t oldIndexingPayloadSizeInBytes, size_t newIndexingPayloadSizeInBytes); // Assumes that preCapacity is zero, and asserts as much.
    Butterfly* growArrayRight(VM&, JSCell* intendedOwner, Structure*, size_t newIndexingPayloadSizeInBytes);
    Butterfly* resizeArray(VM&, JSCell* intendedOwner, size_t propertyCapacity, bool oldHasIndexingHeader, size_t oldIndexingPayloadSizeInBytes, size_t newPreCapacity, bool newHasIndexingHeader, size_t newIndexingPayloadSizeInBytes);
    Butterfly* resizeArray(VM&, JSCell* intendedOwner, Structure*, size_t newPreCapacity, size_t newIndexingPayloadSizeInBytes); // Assumes that you're not changing whether or not the object has an indexing header.
    Butterfly* unshift(Structure*, size_t numberOfSlots);
    Butterfly* shift(Structure*, size_t numberOfSlots);
};

} // namespace JSC
